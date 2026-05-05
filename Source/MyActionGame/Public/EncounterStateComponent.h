#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EncounterStateComponent.generated.h"

UENUM(BlueprintType)
enum class EEncounterState : uint8
{
	Dormant,
	Active,
	Cleared,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEncounterStateSimpleSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEncounterRemainingEnemyCountSignature, int32, RemainingEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEncounterEnemyStateSignature, AActor*, Enemy, int32, RemainingEnemies);

UCLASS(ClassGroup=(Stage), meta=(BlueprintSpawnableComponent))
class MYACTIONGAME_API UEncounterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEncounterStateComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Activation")
	bool bAutoStartOnPlayerOverlap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Activation")
	bool bStartEncounterOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Activation")
	bool bAllowRestartAfterClear = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Participants")
	bool bCollectOverlappingEnemiesOnStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Participants")
	bool bRegisterEnemiesThatEnterWhileActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Participants")
	bool bUseEnemyNameFallback = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Participants")
	bool bCanClearWithoutRegisteredEnemies = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Participants")
	TSubclassOf<AActor> EnemyActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Participants")
	TArray<FName> EnemyActorTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Participants")
	TArray<TObjectPtr<AActor>> EncounterEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Player")
	TSubclassOf<AActor> PlayerActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Defeat Detection")
	float DefeatedPollInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Defeat Detection")
	bool bTreatHiddenActorsAsDefeated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Defeat Detection")
	TArray<FName> DefeatedBoolNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Defeat Detection")
	TArray<FName> DefeatedActorTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Completion")
	bool bDisableOwnerCollisionWhenCleared = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Debug")
	bool bLogEncounterState = true;

	UPROPERTY(BlueprintAssignable, Category="Encounter|Events")
	FEncounterStateSimpleSignature OnEncounterStarted;

	UPROPERTY(BlueprintAssignable, Category="Encounter|Events")
	FEncounterStateSimpleSignature OnEncounterCleared;

	UPROPERTY(BlueprintAssignable, Category="Encounter|Events")
	FEncounterRemainingEnemyCountSignature OnRemainingEnemyCountChanged;

	UPROPERTY(BlueprintAssignable, Category="Encounter|Events")
	FEncounterEnemyStateSignature OnEnemyDefeated;

	UFUNCTION(BlueprintCallable, Category="Encounter")
	void StartEncounter();

	UFUNCTION(BlueprintCallable, Category="Encounter")
	void ResetEncounter();

	UFUNCTION(BlueprintCallable, Category="Encounter")
	void RegisterEnemy(AActor* Enemy);

	UFUNCTION(BlueprintCallable, Category="Encounter")
	void NotifyEnemyDefeated(AActor* Enemy);

	UFUNCTION(BlueprintPure, Category="Encounter")
	EEncounterState GetEncounterState() const { return EncounterState; }

	UFUNCTION(BlueprintPure, Category="Encounter")
	bool IsEncounterActive() const { return EncounterState == EEncounterState::Active; }

	UFUNCTION(BlueprintPure, Category="Encounter")
	bool IsEncounterCleared() const { return EncounterState == EEncounterState::Cleared; }

	UFUNCTION(BlueprintPure, Category="Encounter")
	int32 GetRemainingEnemyCount() const { return RemainingEnemyCount; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Encounter", meta=(AllowPrivateAccess="true"))
	EEncounterState EncounterState = EEncounterState::Dormant;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Encounter", meta=(AllowPrivateAccess="true"))
	int32 RemainingEnemyCount = 0;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> DefeatedEnemies;

	FTimerHandle DefeatedPollTimerHandle;
	bool bBoundOwnerOverlapEvents = false;

	UFUNCTION()
	void HandleOwnerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void BindOwnerOverlapEvents();
	void CollectOverlappingEnemies();
	void EvaluateEncounter();
	void CompleteEncounter();
	void SetRemainingEnemyCount(int32 NewCount);
	bool MatchesEnemyFilter(const AActor* Actor) const;
	bool IsPlayerActor(const AActor* Actor) const;
	bool IsEnemyDefeated(const AActor* Enemy) const;
	bool HasDefeatedBoolFlag(const AActor* Enemy) const;
	bool HasAnyConfiguredTag(const AActor* Actor, const TArray<FName>& Tags) const;
	void LogEncounterMessage(const FString& Message) const;
};
