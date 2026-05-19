#include "WeaponTraceAnimNotifyState.h"

#include "Components/SkeletalMeshComponent.h"
#include "WeaponTraceComponent.h"

void UWeaponTraceAnimNotifyState::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UWeaponTraceComponent* WeaponTraceComponent = FindWeaponTraceComponent(MeshComp))
	{
		if (bResetHitActorsOnBegin)
		{
			WeaponTraceComponent->ResetHitActors();
		}

		WeaponTraceComponent->BeginWeaponTraceWindow();
	}
}

void UWeaponTraceAnimNotifyState::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UWeaponTraceComponent* WeaponTraceComponent = FindWeaponTraceComponent(MeshComp))
	{
		WeaponTraceComponent->EndWeaponTraceWindow();
	}
}

UWeaponTraceComponent* UWeaponTraceAnimNotifyState::FindWeaponTraceComponent(const USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp)
	{
		return nullptr;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	TArray<UWeaponTraceComponent*> WeaponTraceComponents;
	Owner->GetComponents<UWeaponTraceComponent>(WeaponTraceComponents);

	if (WeaponTraceComponents.IsEmpty())
	{
		return nullptr;
	}

	if (TraceComponentName.IsNone())
	{
		return WeaponTraceComponents[0];
	}

	for (UWeaponTraceComponent* WeaponTraceComponent : WeaponTraceComponents)
	{
		if (WeaponTraceComponent && WeaponTraceComponent->GetFName() == TraceComponentName)
		{
			return WeaponTraceComponent;
		}
	}

	return WeaponTraceComponents[0];
}
