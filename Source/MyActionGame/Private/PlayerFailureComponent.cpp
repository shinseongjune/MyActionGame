#include "PlayerFailureComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerController.h"

UPlayerFailureComponent::UPlayerFailureComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	DefeatedActorTags = {
		TEXT("Dead"),
		TEXT("Defeated"),
		TEXT("PlayerDefeated")
	};
}

void UPlayerFailureComponent::BeginPlay()
{
	Super::BeginPlay();

	bIsDefeated = false;
	RemoveDefeatedTags();
	LogFailureMessage(TEXT("Player failure handler ready."));
}

void UPlayerFailureComponent::ResetPlayerFailure()
{
	const bool bWasDefeated = bIsDefeated;
	bIsDefeated = false;
	RemoveDefeatedTags();
	if (bWasDefeated)
	{
		RestoreOwnerControl();
	}
	OnPlayerFailureReset.Broadcast(GetOwner());
	LogFailureMessage(TEXT("Player failure reset."));
}

void UPlayerFailureComponent::TriggerPlayerDefeated(AActor* DamageCauser)
{
	if (bIsDefeated)
	{
		return;
	}

	bIsDefeated = true;
	ApplyDefeatedTags();
	DisableOwnerControl();
	OnPlayerDefeated.Broadcast(GetOwner(), DamageCauser);
	LogFailureMessage(TEXT("Player defeated."));
}

void UPlayerFailureComponent::ApplyDefeatedTags()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	for (const FName Tag : DefeatedActorTags)
	{
		if (!Tag.IsNone())
		{
			Owner->Tags.AddUnique(Tag);
		}
	}
}

void UPlayerFailureComponent::RemoveDefeatedTags()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	for (const FName Tag : DefeatedActorTags)
	{
		if (!Tag.IsNone())
		{
			Owner->Tags.Remove(Tag);
		}
	}
}

void UPlayerFailureComponent::DisableOwnerControl()
{
	AActor* Owner = GetOwner();
	APawn* Pawn = Cast<APawn>(Owner);
	if (!Owner || !Pawn)
	{
		return;
	}

	if (bStopMovementOnDefeat)
	{
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
			{
				MovementComponent->StopMovementImmediately();
				MovementComponent->DisableMovement();
			}
		}
		else if (UPawnMovementComponent* MovementComponent = Pawn->GetMovementComponent())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->Deactivate();
		}
	}

	if (bDisableInputOnDefeat)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
		{
			Pawn->DisableInput(PlayerController);
		}
	}
}

void UPlayerFailureComponent::RestoreOwnerControl()
{
	AActor* Owner = GetOwner();
	APawn* Pawn = Cast<APawn>(Owner);
	if (!Owner || !Pawn)
	{
		return;
	}

	if (bStopMovementOnDefeat)
	{
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
			{
				MovementComponent->SetMovementMode(MOVE_Walking);
			}
		}
		else if (UPawnMovementComponent* MovementComponent = Pawn->GetMovementComponent())
		{
			MovementComponent->Activate(true);
		}
	}

	if (bDisableInputOnDefeat)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
		{
			Pawn->EnableInput(PlayerController);
		}
	}
}

void UPlayerFailureComponent::LogFailureMessage(const FString& Message) const
{
	if (!bLogFailureState)
	{
		return;
	}

	const FString OwnerName = GetOwner() ? GetOwner()->GetName() : FString(TEXT("NoOwner"));
	UE_LOG(LogTemp, Log, TEXT("[PlayerFailure:%s] %s"), *OwnerName, *Message);
}
