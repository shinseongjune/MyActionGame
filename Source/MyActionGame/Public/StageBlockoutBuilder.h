#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StageBlockoutBuilder.generated.h"

class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;

USTRUCT(BlueprintType)
struct FStageBlockoutTileDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	FString Symbol = TEXT("W");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	FName ComponentName = TEXT("Wall");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	TSoftObjectPtr<UMaterialInterface> MaterialOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	bool bApplyDebugColor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	FLinearColor DebugColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	FVector LocalScale = FVector(4.0, 4.0, 3.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	FVector LocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	FRotator LocalRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	bool bSnapBottomToCellFloor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	bool bGenerate = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	bool bCollisionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout")
	bool bPlaceFloorUnderTile = false;
};

UCLASS()
class MYACTIONGAME_API AStageBlockoutBuilder : public AActor
{
	GENERATED_BODY()

public:
	AStageBlockoutBuilder();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout|Layout")
	TArray<FString> InlineLayoutRows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout|Layout", meta=(FilePathFilter="csv"))
	FFilePath CsvLayoutFile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout|Layout")
	float CellSize = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout|Layout")
	float LayerHeight = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout|Layout")
	bool bCenterGeneratedLayout = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout|Layout")
	bool bClearBeforeBuild = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout|Floor")
	FString EmptySymbol = TEXT(".");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout|Floor")
	FString FloorOnlySymbol = TEXT("_");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout|Floor")
	bool bGenerateFloorUnderMarkedTiles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout|Floor")
	FStageBlockoutTileDefinition FloorTile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage Blockout|Tiles")
	TArray<FStageBlockoutTileDefinition> TileDefinitions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage Blockout|Generated")
	int32 LastGeneratedInstanceCount = 0;

	UFUNCTION(CallInEditor, BlueprintCallable, Category="Stage Blockout")
	void BuildFromInlineLayout();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="Stage Blockout")
	void BuildFromCsvFile();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="Stage Blockout")
	void ClearGeneratedBlockout();

private:
	static const FName GeneratedComponentTag;

	using FLayoutRow = TArray<FString>;
	using FLayoutLayer = TArray<FLayoutRow>;

	void BuildFromLines(const TArray<FString>& Lines, const FString& SourceName);
	bool ParseLayoutLines(const TArray<FString>& Lines, TArray<FLayoutLayer>& OutLayers) const;
	TArray<FString> ParseLayoutRow(const FString& Line) const;
	const FStageBlockoutTileDefinition* FindDefinitionForSymbol(const FString& Symbol) const;
	bool IsEmptySymbol(const FString& Symbol) const;
	bool IsFloorOnlySymbol(const FString& Symbol) const;
	bool ShouldPlaceFloorForSymbol(const FString& Symbol, const FStageBlockoutTileDefinition* TileDefinition) const;
	void AddTileInstance(
		const FStageBlockoutTileDefinition& Definition,
		const FVector& CellLocation,
		TMap<FName, TObjectPtr<UInstancedStaticMeshComponent>>& GeneratedComponents);
	UInstancedStaticMeshComponent* FindOrCreateGeneratedComponent(
		const FStageBlockoutTileDefinition& Definition,
		TMap<FName, TObjectPtr<UInstancedStaticMeshComponent>>& GeneratedComponents);
	void ApplyTileMaterial(
		const FStageBlockoutTileDefinition& Definition,
		UInstancedStaticMeshComponent* Component,
		const UStaticMesh* Mesh) const;
	void LogBuilderMessage(const FString& Message) const;
};
