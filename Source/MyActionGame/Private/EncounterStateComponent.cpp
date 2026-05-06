#include "EncounterStateComponent.h"

#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

UEncounterStateComponent::UEncounterStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	DefeatedBoolNames = {
		TEXT("bIsDead"),
		TEXT("IsDead"),
		TEXT("bDead"),
		TEXT("Dead"),
		TEXT("bIsDefeated"),
		TEXT("IsDefeated"),
		TEXT("bExecuted"),
		TEXT("IsExecuted")
	};

	DefeatedActorTags = {
		TEXT("Dead"),
		TEXT("Defeated"),
		TEXT("Executed")
	};

	EnemyActorTags = {
		TEXT("Enemy")
	};
}

void UEncounterStateComponent::BeginPlay()
{
	Super::BeginPlay();

	LogEncounterMessage(FString::Printf(
		TEXT("BeginPlay. EnemyClass=%s."),
		EnemyActorClass ? *EnemyActorClass->GetName() : TEXT("None")));

	BindOwnerOverlapEvents();

	if (bCollectEnemiesFromZoneOnStart)
	{
		CollectEnemiesNow();
	}

	if (bStartEncounterOnBeginPlay)
	{
		StartEncounter();
	}
}

void UEncounterStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DefeatedPollTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UEncounterStateComponent::StartEncounter()
{
	if (EncounterState == EEncounterState::Active)
	{
		return;
	}

	if (EncounterState == EEncounterState::Cleared && !bAllowRestartAfterClear)
	{
		LogEncounterMessage(TEXT("Start ignored because this encounter is already cleared."));
		return;
	}

	DefeatedEnemies.Reset();

	if (bCollectEnemiesFromZoneOnStart)
	{
		CollectEnemiesNow();
	}
	else
	{
		EncounterEnemies.RemoveAllSwap([](const TObjectPtr<AActor>& Enemy)
		{
			return !IsValid(Enemy);
		});
	}

	EncounterState = EEncounterState::Active;
	RemainingEnemyCount = -1;
	OnEncounterStarted.Broadcast();
	LogEncounterMessage(FString::Printf(TEXT("Encounter started with %d tracked enemies."), EncounterEnemies.Num()));

	EvaluateEncounter();

	if (UWorld* World = GetWorld())
	{
		const float PollRate = FMath::Max(DefeatedPollInterval, 0.05f);
		World->GetTimerManager().SetTimer(DefeatedPollTimerHandle, this, &UEncounterStateComponent::EvaluateEncounter, PollRate, true);
	}
}

void UEncounterStateComponent::ResetEncounter()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DefeatedPollTimerHandle);
	}

	EncounterState = EEncounterState::Dormant;
	DefeatedEnemies.Reset();
	SetRemainingEnemyCount(0);
	LogEncounterMessage(TEXT("Encounter reset."));

	if (bCollectEnemiesFromZoneOnStart)
	{
		CollectEnemiesNow();
	}
}

void UEncounterStateComponent::RegisterEnemy(AActor* Enemy)
{
	if (!IsValid(Enemy) || Enemy == GetOwner() || IsPlayerActor(Enemy))
	{
		return;
	}

	const int32 PreviousCount = EncounterEnemies.Num();
	EncounterEnemies.AddUnique(Enemy);

	if (EncounterEnemies.Num() != PreviousCount)
	{
		LogEncounterMessage(FString::Printf(TEXT("Registered enemy: %s"), *Enemy->GetName()));
	}

	if (EncounterState == EEncounterState::Active)
	{
		EvaluateEncounter();
	}
}

void UEncounterStateComponent::NotifyEnemyDefeated(AActor* Enemy)
{
	if (!IsValid(Enemy))
	{
		return;
	}

	RegisterEnemy(Enemy);
	DefeatedEnemies.AddUnique(Enemy);

	if (EncounterState == EEncounterState::Active)
	{
		EvaluateEncounter();
	}
}

void UEncounterStateComponent::CollectEnemiesNow()
{
	EncounterEnemies.Reset();
	CollectEnemiesFromZoneCollider();
}

TArray<AActor*> UEncounterStateComponent::GetEncounterEnemies() const
{
	TArray<AActor*> ValidEnemies;
	ValidEnemies.Reserve(EncounterEnemies.Num());

	for (const TObjectPtr<AActor>& EnemyPtr : EncounterEnemies)
	{
		AActor* Enemy = EnemyPtr.Get();
		if (IsValid(Enemy))
		{
			ValidEnemies.Add(Enemy);
		}
	}

	return ValidEnemies;
}

UBoxComponent* UEncounterStateComponent::GetZoneBoxComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	TArray<UBoxComponent*> BoxComponents;
	Owner->GetComponents<UBoxComponent>(BoxComponents);
	if (BoxComponents.IsEmpty())
	{
		return nullptr;
	}

	for (UBoxComponent* BoxComponent : BoxComponents)
	{
		if (BoxComponent && BoxComponent->GetGenerateOverlapEvents())
		{
			return BoxComponent;
		}
	}

	return BoxComponents[0];
}

void UEncounterStateComponent::BindOwnerOverlapEvents()
{
	if (bBoundOwnerOverlapEvents)
	{
		return;
	}

	UBoxComponent* ZoneBox = GetZoneBoxComponent();
	if (!ZoneBox)
	{
		LogEncounterMessage(TEXT("No Box Collision component found on owner. Encounter cannot auto-start or auto-collect enemies."));
		return;
	}

	if (!ZoneBox->GetGenerateOverlapEvents())
	{
		LogEncounterMessage(FString::Printf(TEXT("Zone box '%s' has Generate Overlap Events disabled."), *ZoneBox->GetName()));
	}

	ZoneBox->OnComponentBeginOverlap.AddDynamic(this, &UEncounterStateComponent::HandleOwnerBeginOverlap);
	bBoundOwnerOverlapEvents = true;
	LogEncounterMessage(FString::Printf(TEXT("Bound zone box overlap: %s"), *ZoneBox->GetName()));
}

void UEncounterStateComponent::HandleOwnerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner())
	{
		return;
	}

	if (bAutoStartOnPlayerOverlap && IsPlayerActor(OtherActor))
	{
		StartEncounter();
	}
}

void UEncounterStateComponent::CollectEnemiesFromZoneCollider()
{
	UWorld* World = GetWorld();
	const UBoxComponent* ZoneBox = GetZoneBoxComponent();
	if (!World || !ZoneBox)
	{
		return;
	}

	int32 CandidateCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Candidate = *It;
		if (!MatchesEnemyFilter(Candidate))
		{
			continue;
		}

		++CandidateCount;

		if (IsActorInsideZoneBox(Candidate, ZoneBox))
		{
			RegisterEnemy(Candidate);
		}
	}

	LogEncounterMessage(FString::Printf(
		TEXT("Zone box collection checked %d enemy candidates and registered %d enemies."),
		CandidateCount,
		EncounterEnemies.Num()));
}

void UEncounterStateComponent::EvaluateEncounter()
{
	if (EncounterState != EEncounterState::Active)
	{
		return;
	}

	TArray<AActor*> NewlyDefeatedEnemies;
	int32 TrackedEnemyCount = 0;
	int32 ActiveEnemyCount = 0;

	for (TObjectPtr<AActor>& EnemyPtr : EncounterEnemies)
	{
		AActor* Enemy = EnemyPtr.Get();
		if (!IsValid(Enemy))
		{
			continue;
		}

		++TrackedEnemyCount;

		if (IsEnemyDefeated(Enemy))
		{
			if (!DefeatedEnemies.Contains(Enemy))
			{
				DefeatedEnemies.Add(Enemy);
				NewlyDefeatedEnemies.Add(Enemy);
			}
		}
		else
		{
			++ActiveEnemyCount;
		}
	}

	SetRemainingEnemyCount(ActiveEnemyCount);

	for (AActor* DefeatedEnemy : NewlyDefeatedEnemies)
	{
		OnEnemyDefeated.Broadcast(DefeatedEnemy, RemainingEnemyCount);
		LogEncounterMessage(FString::Printf(TEXT("Enemy defeated: %s (%d remaining)."), *DefeatedEnemy->GetName(), RemainingEnemyCount));
	}

	if (ActiveEnemyCount == 0 && (TrackedEnemyCount > 0 || bCanClearWithoutRegisteredEnemies))
	{
		CompleteEncounter();
	}
}

void UEncounterStateComponent::CompleteEncounter()
{
	if (EncounterState == EEncounterState::Cleared)
	{
		return;
	}

	EncounterState = EEncounterState::Cleared;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DefeatedPollTimerHandle);
	}

	if (bDisableOwnerCollisionWhenCleared)
	{
		if (AActor* Owner = GetOwner())
		{
			TArray<UPrimitiveComponent*> PrimitiveComponents;
			Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

			for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
			{
				if (PrimitiveComponent)
				{
					PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
			}
		}
	}

	SetRemainingEnemyCount(0);
	OnEncounterCleared.Broadcast();
	LogEncounterMessage(TEXT("Encounter cleared."));
}

void UEncounterStateComponent::SetRemainingEnemyCount(int32 NewCount)
{
	if (RemainingEnemyCount == NewCount)
	{
		return;
	}

	RemainingEnemyCount = NewCount;
	OnRemainingEnemyCountChanged.Broadcast(RemainingEnemyCount);
}

bool UEncounterStateComponent::MatchesEnemyFilter(const AActor* Actor) const
{
	if (!IsValid(Actor) || Actor == GetOwner() || IsPlayerActor(Actor))
	{
		return false;
	}

	if (EnemyActorClass && Actor->IsA(EnemyActorClass))
	{
		return true;
	}

	return HasAnyConfiguredTag(Actor, EnemyActorTags);
}

bool UEncounterStateComponent::IsActorInsideZoneBox(const AActor* Actor, const UBoxComponent* ZoneBox) const
{
	if (!IsValid(Actor) || !ZoneBox)
	{
		return false;
	}

	const FVector LocalActorLocation = ZoneBox->GetComponentTransform().InverseTransformPosition(Actor->GetActorLocation());
	const FVector BoxExtent = ZoneBox->GetUnscaledBoxExtent();

	return FMath::Abs(LocalActorLocation.X) <= BoxExtent.X
		&& FMath::Abs(LocalActorLocation.Y) <= BoxExtent.Y
		&& FMath::Abs(LocalActorLocation.Z) <= BoxExtent.Z;
}

bool UEncounterStateComponent::IsPlayerActor(const AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	if (PlayerActorClass && Actor->IsA(PlayerActorClass))
	{
		return true;
	}

	const APawn* Pawn = Cast<APawn>(Actor);
	if (!Pawn)
	{
		return false;
	}

	const AController* Controller = Pawn->GetController();
	if (Controller && Controller->IsPlayerController())
	{
		return true;
	}

	const UWorld* World = GetWorld();
	const APlayerController* FirstPlayerController = World ? World->GetFirstPlayerController() : nullptr;
	return FirstPlayerController && FirstPlayerController->GetPawn() == Pawn;
}

bool UEncounterStateComponent::IsEnemyDefeated(const AActor* Enemy) const
{
	if (!IsValid(Enemy) || Enemy->IsActorBeingDestroyed())
	{
		return true;
	}

	if (DefeatedEnemies.Contains(Enemy))
	{
		return true;
	}

	if (bTreatHiddenActorsAsDefeated && Enemy->IsHidden())
	{
		return true;
	}

	return HasAnyConfiguredTag(Enemy, DefeatedActorTags) || HasDefeatedBoolFlag(Enemy);
}

bool UEncounterStateComponent::HasDefeatedBoolFlag(const AActor* Enemy) const
{
	if (!IsValid(Enemy))
	{
		return true;
	}

	const UClass* EnemyClass = Enemy->GetClass();
	if (!EnemyClass)
	{
		return false;
	}

	for (const FName FlagName : DefeatedBoolNames)
	{
		if (FlagName.IsNone())
		{
			continue;
		}

		const FBoolProperty* BoolProperty = FindFProperty<FBoolProperty>(EnemyClass, FlagName);
		if (BoolProperty && BoolProperty->GetPropertyValue_InContainer(Enemy))
		{
			return true;
		}
	}

	return false;
}

bool UEncounterStateComponent::HasAnyConfiguredTag(const AActor* Actor, const TArray<FName>& Tags) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	for (const FName Tag : Tags)
	{
		if (!Tag.IsNone() && Actor->ActorHasTag(Tag))
		{
			return true;
		}
	}

	return false;
}

void UEncounterStateComponent::LogEncounterMessage(const FString& Message) const
{
	if (!bLogEncounterState)
	{
		return;
	}

	const FString OwnerName = GetOwner() ? GetOwner()->GetName() : FString(TEXT("NoOwner"));
	UE_LOG(LogTemp, Log, TEXT("[EncounterState:%s] %s"), *OwnerName, *Message);
}
