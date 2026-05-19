#include "WeaponTraceComponent.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

UWeaponTraceComponent::UWeaponTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	HitActorTags = {
		TEXT("Enemy")
	};
}

void UWeaponTraceComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(false);
}

void UWeaponTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bTraceWindowActive)
	{
		TraceWeaponMovement();
	}
}

void UWeaponTraceComponent::BeginWeaponTraceWindow()
{
	if (bTraceWindowActive)
	{
		ResetHitActors();
	}

	FVector CurrentTraceStart;
	FVector CurrentTraceEnd;
	if (!ReadTraceSocketLocations(CurrentTraceStart, CurrentTraceEnd))
	{
		LogWeaponTraceMessage(TEXT("Trace window did not start because trace sockets were not found."));
		return;
	}

	ResetHitActors();
	PreviousTraceStart = CurrentTraceStart;
	PreviousTraceEnd = CurrentTraceEnd;
	bHasPreviousTraceLocations = true;
	bTraceWindowActive = true;
	SetComponentTickEnabled(true);

	if (bTraceCurrentBladeSegment)
	{
		TraceSegment(CurrentTraceStart, CurrentTraceEnd);
	}

	OnTraceWindowStarted.Broadcast();
	LogWeaponTraceMessage(TEXT("Trace window started."));
}

void UWeaponTraceComponent::EndWeaponTraceWindow()
{
	if (!bTraceWindowActive)
	{
		return;
	}

	bTraceWindowActive = false;
	bHasPreviousTraceLocations = false;
	SetComponentTickEnabled(false);
	OnTraceWindowEnded.Broadcast();
	LogWeaponTraceMessage(TEXT("Trace window ended."));
}

void UWeaponTraceComponent::ResetHitActors()
{
	HitActorsThisWindow.Reset();
}

void UWeaponTraceComponent::SetTraceSourceComponent(USceneComponent* NewTraceSourceComponent)
{
	TraceSourceComponent = NewTraceSourceComponent;
}

TArray<AActor*> UWeaponTraceComponent::GetHitActorsThisWindow() const
{
	TArray<AActor*> HitActors;
	HitActors.Reserve(HitActorsThisWindow.Num());

	for (const TObjectPtr<AActor>& HitActorPtr : HitActorsThisWindow)
	{
		AActor* HitActor = HitActorPtr.Get();
		if (IsValid(HitActor))
		{
			HitActors.Add(HitActor);
		}
	}

	return HitActors;
}

USceneComponent* UWeaponTraceComponent::ResolveTraceSourceComponent() const
{
	if (TraceSourceComponent)
	{
		return TraceSourceComponent;
	}

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	if (const ACharacter* Character = Cast<ACharacter>(Owner))
	{
		if (USkeletalMeshComponent* MeshComponent = Character->GetMesh())
		{
			return MeshComponent;
		}
	}

	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	Owner->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
	if (!SkeletalMeshComponents.IsEmpty())
	{
		return SkeletalMeshComponents[0];
	}

	return Owner->GetRootComponent();
}

bool UWeaponTraceComponent::ReadTraceSocketLocations(FVector& OutTraceStart, FVector& OutTraceEnd) const
{
	const USceneComponent* SourceComponent = ResolveTraceSourceComponent();
	if (!SourceComponent)
	{
		LogWeaponTraceMessage(TEXT("No trace source component found."));
		return false;
	}

	if (!SourceComponent->DoesSocketExist(TraceStartSocketName))
	{
		LogWeaponTraceMessage(FString::Printf(TEXT("Missing trace start socket: %s"), *TraceStartSocketName.ToString()));
		return false;
	}

	if (!SourceComponent->DoesSocketExist(TraceEndSocketName))
	{
		LogWeaponTraceMessage(FString::Printf(TEXT("Missing trace end socket: %s"), *TraceEndSocketName.ToString()));
		return false;
	}

	OutTraceStart = SourceComponent->GetSocketLocation(TraceStartSocketName);
	OutTraceEnd = SourceComponent->GetSocketLocation(TraceEndSocketName);
	return true;
}

void UWeaponTraceComponent::TraceWeaponMovement()
{
	FVector CurrentTraceStart;
	FVector CurrentTraceEnd;
	if (!ReadTraceSocketLocations(CurrentTraceStart, CurrentTraceEnd))
	{
		EndWeaponTraceWindow();
		return;
	}

	if (!bHasPreviousTraceLocations)
	{
		PreviousTraceStart = CurrentTraceStart;
		PreviousTraceEnd = CurrentTraceEnd;
		bHasPreviousTraceLocations = true;
		return;
	}

	const FVector PreviousTraceMidpoint = (PreviousTraceStart + PreviousTraceEnd) * 0.5f;
	const FVector CurrentTraceMidpoint = (CurrentTraceStart + CurrentTraceEnd) * 0.5f;

	TraceSegment(PreviousTraceStart, CurrentTraceStart);
	TraceSegment(PreviousTraceEnd, CurrentTraceEnd);
	TraceSegment(PreviousTraceMidpoint, CurrentTraceMidpoint);

	if (bTraceCurrentBladeSegment)
	{
		TraceSegment(CurrentTraceStart, CurrentTraceEnd);
	}

	PreviousTraceStart = CurrentTraceStart;
	PreviousTraceEnd = CurrentTraceEnd;
}

void UWeaponTraceComponent::TraceSegment(const FVector& Start, const FVector& End)
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!World || !Owner)
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WeaponTrace), false);
	QueryParams.AddIgnoredActor(Owner);

	for (const TObjectPtr<AActor>& HitActorPtr : HitActorsThisWindow)
	{
		if (AActor* HitActor = HitActorPtr.Get())
		{
			QueryParams.AddIgnoredActor(HitActor);
		}
	}

	TArray<FHitResult> Hits;
	const bool bHit = World->SweepMultiByChannel(
		Hits,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(FMath::Max(TraceRadius, 1.0f)),
		QueryParams);

	DrawTraceDebug(Start, End, bHit);

	if (!bHit)
	{
		return;
	}

	for (const FHitResult& Hit : Hits)
	{
		RegisterHit(Hit);
	}
}

bool UWeaponTraceComponent::ShouldAcceptHitActor(const AActor* HitActor) const
{
	if (!IsValid(HitActor) || HitActor == GetOwner())
	{
		return false;
	}

	if (HitActorsThisWindow.Contains(HitActor))
	{
		return false;
	}

	if (HitActorClass && HitActor->IsA(HitActorClass))
	{
		return true;
	}

	if (!HitActorTags.IsEmpty())
	{
		return HasAnyConfiguredTag(HitActor);
	}

	return HitActorClass == nullptr;
}

bool UWeaponTraceComponent::HasAnyConfiguredTag(const AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	for (const FName Tag : HitActorTags)
	{
		if (!Tag.IsNone() && Actor->ActorHasTag(Tag))
		{
			return true;
		}
	}

	return false;
}

void UWeaponTraceComponent::RegisterHit(const FHitResult& Hit)
{
	AActor* HitActor = Hit.GetActor();
	if (!ShouldAcceptHitActor(HitActor))
	{
		return;
	}

	HitActorsThisWindow.Add(HitActor);
	OnWeaponTraceHit.Broadcast(HitActor, Hit, HitActorsThisWindow.Num());
	LogWeaponTraceMessage(FString::Printf(TEXT("Weapon trace hit: %s"), *HitActor->GetName()));
}

void UWeaponTraceComponent::DrawTraceDebug(const FVector& Start, const FVector& End, bool bHit) const
{
	if (!bDrawDebugTrace)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FColor TraceColor = bHit ? FColor::Red : FColor::Green;
	DrawDebugLine(World, Start, End, TraceColor, false, DebugDrawTime, 0, 1.5f);
	DrawDebugSphere(World, Start, TraceRadius, 8, TraceColor, false, DebugDrawTime);
	DrawDebugSphere(World, End, TraceRadius, 8, TraceColor, false, DebugDrawTime);
}

void UWeaponTraceComponent::LogWeaponTraceMessage(const FString& Message) const
{
	const FString OwnerName = GetOwner() ? GetOwner()->GetName() : FString(TEXT("NoOwner"));
	UE_LOG(LogTemp, Log, TEXT("[WeaponTrace:%s] %s"), *OwnerName, *Message);
}
