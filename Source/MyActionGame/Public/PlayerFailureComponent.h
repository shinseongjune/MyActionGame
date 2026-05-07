#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerFailureComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerDefeatedSignature, AActor*, Player, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerFailureResetSignature, AActor*, Player);

UCLASS(ClassGroup=(Stage), meta=(BlueprintSpawnableComponent))
class MYACTIONGAME_API UPlayerFailureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerFailureComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Failure|Defeat")
	bool bDisableInputOnDefeat = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Failure|Defeat")
	bool bStopMovementOnDefeat = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Failure|Defeat")
	TArray<FName> DefeatedActorTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Failure|Debug")
	bool bLogFailureState = true;

	UPROPERTY(BlueprintAssignable, Category="Failure|Events")
	FPlayerDefeatedSignature OnPlayerDefeated;

	UPROPERTY(BlueprintAssignable, Category="Failure|Events")
	FPlayerFailureResetSignature OnPlayerFailureReset;

	UFUNCTION(BlueprintCallable, Category="Failure")
	void ResetPlayerFailure();

	UFUNCTION(BlueprintCallable, Category="Failure")
	void TriggerPlayerDefeated(AActor* DamageCauser);

	UFUNCTION(BlueprintPure, Category="Failure")
	bool IsPlayerDefeated() const { return bIsDefeated; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Failure", meta=(AllowPrivateAccess="true"))
	bool bIsDefeated = false;

	void ApplyDefeatedTags();
	void RemoveDefeatedTags();
	void DisableOwnerControl();
	void RestoreOwnerControl();
	void LogFailureMessage(const FString& Message) const;
};
