#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "WeaponTraceComponent.generated.h"

class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWeaponTraceWindowSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWeaponTraceHitSignature, AActor*, HitActor, const FHitResult&, Hit, int32, HitCount);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class MYACTIONGAME_API UWeaponTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponTraceComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace|Source")
	TObjectPtr<USceneComponent> TraceSourceComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace|Source")
	FName TraceStartSocketName = TEXT("WeaponTrace_Start");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace|Source")
	FName TraceEndSocketName = TEXT("WeaponTrace_End");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace|Trace", meta=(ClampMin="1.0"))
	float TraceRadius = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace|Trace")
	bool bTraceCurrentBladeSegment = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace|Filter")
	TSubclassOf<AActor> HitActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace|Filter")
	TArray<FName> HitActorTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace|Debug")
	bool bDrawDebugTrace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trace|Debug", meta=(ClampMin="0.0"))
	float DebugDrawTime = 0.05f;

	UPROPERTY(BlueprintAssignable, Category="Weapon Trace|Events")
	FWeaponTraceWindowSignature OnTraceWindowStarted;

	UPROPERTY(BlueprintAssignable, Category="Weapon Trace|Events")
	FWeaponTraceWindowSignature OnTraceWindowEnded;

	UPROPERTY(BlueprintAssignable, Category="Weapon Trace|Events")
	FWeaponTraceHitSignature OnWeaponTraceHit;

	UFUNCTION(BlueprintCallable, Category="Weapon Trace")
	void BeginWeaponTraceWindow();

	UFUNCTION(BlueprintCallable, Category="Weapon Trace")
	void EndWeaponTraceWindow();

	UFUNCTION(BlueprintCallable, Category="Weapon Trace")
	void ResetHitActors();

	UFUNCTION(BlueprintCallable, Category="Weapon Trace")
	void SetTraceSourceComponent(USceneComponent* NewTraceSourceComponent);

	UFUNCTION(BlueprintPure, Category="Weapon Trace")
	bool IsTraceWindowActive() const { return bTraceWindowActive; }

	UFUNCTION(BlueprintPure, Category="Weapon Trace")
	TArray<AActor*> GetHitActorsThisWindow() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> HitActorsThisWindow;

	bool bTraceWindowActive = false;
	bool bHasPreviousTraceLocations = false;
	FVector PreviousTraceStart = FVector::ZeroVector;
	FVector PreviousTraceEnd = FVector::ZeroVector;

	USceneComponent* ResolveTraceSourceComponent() const;
	bool ReadTraceSocketLocations(FVector& OutTraceStart, FVector& OutTraceEnd) const;
	void TraceWeaponMovement();
	void TraceSegment(const FVector& Start, const FVector& End);
	bool ShouldAcceptHitActor(const AActor* HitActor) const;
	bool HasAnyConfiguredTag(const AActor* Actor) const;
	void RegisterHit(const FHitResult& Hit);
	void DrawTraceDebug(const FVector& Start, const FVector& End, bool bHit) const;
	void LogWeaponTraceMessage(const FString& Message) const;
};
