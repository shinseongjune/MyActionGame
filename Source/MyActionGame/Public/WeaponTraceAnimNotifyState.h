#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "WeaponTraceAnimNotifyState.generated.h"

class UWeaponTraceComponent;

UCLASS(meta=(DisplayName="Weapon Trace Window"))
class MYACTIONGAME_API UWeaponTraceAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace")
	FName TraceComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace")
	bool bResetHitActorsOnBegin = true;

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:
	UWeaponTraceComponent* FindWeaponTraceComponent(const USkeletalMeshComponent* MeshComp) const;
};
