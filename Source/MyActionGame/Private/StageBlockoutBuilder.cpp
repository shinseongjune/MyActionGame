#include "StageBlockoutBuilder.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

const FName AStageBlockoutBuilder::GeneratedComponentTag = TEXT("GeneratedStageBlockout");

namespace
{
	TSoftObjectPtr<UStaticMesh> MeshRef(const TCHAR* Path)
	{
		return TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(Path));
	}

	TSoftObjectPtr<UMaterialInterface> MaterialRef(const TCHAR* Path)
	{
		return TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Path));
	}

	FString NormalizeToken(const FString& Token)
	{
		FString Result = Token;
		Result.TrimStartAndEndInline();
		return Result;
	}

	FLinearColor DefaultColorForSymbol(const FString& Symbol)
	{
		if (Symbol.Equals(TEXT("_"), ESearchCase::IgnoreCase))
		{
			return FLinearColor(0.34f, 0.48f, 0.28f, 1.0f);
		}
		if (Symbol.Equals(TEXT("W"), ESearchCase::IgnoreCase))
		{
			return FLinearColor(0.78f, 0.78f, 0.72f, 1.0f);
		}
		if (Symbol.Equals(TEXT("C"), ESearchCase::IgnoreCase))
		{
			return FLinearColor(0.18f, 0.62f, 0.78f, 1.0f);
		}
		if (Symbol.Equals(TEXT("G"), ESearchCase::IgnoreCase))
		{
			return FLinearColor(1.0f, 0.32f, 0.02f, 1.0f);
		}
		if (Symbol.Equals(TEXT("X"), ESearchCase::IgnoreCase))
		{
			return FLinearColor(0.95f, 0.18f, 0.12f, 1.0f);
		}
		if (Symbol.Equals(TEXT("E"), ESearchCase::IgnoreCase))
		{
			return FLinearColor(0.95f, 0.1f, 0.48f, 1.0f);
		}
		if (Symbol.Equals(TEXT("Z"), ESearchCase::IgnoreCase))
		{
			return FLinearColor(0.12f, 0.28f, 1.0f, 1.0f);
		}
		if (Symbol.Equals(TEXT("R"), ESearchCase::IgnoreCase))
		{
			return FLinearColor(1.0f, 0.88f, 0.08f, 1.0f);
		}
		if (Symbol.Equals(TEXT("S"), ESearchCase::IgnoreCase))
		{
			return FLinearColor(0.0f, 0.95f, 0.25f, 1.0f);
		}
		if (Symbol.Equals(TEXT("F"), ESearchCase::IgnoreCase))
		{
			return FLinearColor(0.55f, 0.2f, 1.0f, 1.0f);
		}
		return FLinearColor::White;
	}
}

AStageBlockoutBuilder::AStageBlockoutBuilder()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	const TSoftObjectPtr<UMaterialInterface> DebugMaterial = MaterialRef(TEXT("/Game/LevelPrototyping/Materials/MI_DefaultColorway.MI_DefaultColorway"));

	FloorTile.Symbol = TEXT("_");
	FloorTile.ComponentName = TEXT("Floor");
	FloorTile.Mesh = MeshRef(TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
	FloorTile.MaterialOverride = DebugMaterial;
	FloorTile.DebugColor = FLinearColor(0.34f, 0.48f, 0.28f, 1.0f);
	FloorTile.LocalScale = FVector(4.0, 4.0, 0.12);
	FloorTile.LocalOffset = FVector(0.0, 0.0, -6.0);
	FloorTile.bSnapBottomToCellFloor = false;
	FloorTile.bCollisionEnabled = true;
	FloorTile.bPlaceFloorUnderTile = false;

	FStageBlockoutTileDefinition Wall;
	Wall.Symbol = TEXT("W");
	Wall.ComponentName = TEXT("Wall");
	Wall.Mesh = MeshRef(TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
	Wall.MaterialOverride = DebugMaterial;
	Wall.DebugColor = FLinearColor(0.78f, 0.78f, 0.72f, 1.0f);
	Wall.LocalScale = FVector(4.0, 4.0, 3.0);
	Wall.LocalOffset = FVector::ZeroVector;

	FStageBlockoutTileDefinition Pillar;
	Pillar.Symbol = TEXT("C");
	Pillar.ComponentName = TEXT("CoverPillar");
	Pillar.Mesh = MeshRef(TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube.SM_ChamferCube"));
	Pillar.MaterialOverride = DebugMaterial;
	Pillar.DebugColor = FLinearColor(0.18f, 0.62f, 0.78f, 1.0f);
	Pillar.LocalScale = FVector(1.2, 1.2, 2.2);
	Pillar.LocalOffset = FVector::ZeroVector;
	Pillar.bPlaceFloorUnderTile = true;

	FStageBlockoutTileDefinition Gate;
	Gate.Symbol = TEXT("G");
	Gate.ComponentName = TEXT("GateMarker");
	Gate.Mesh = MeshRef(TEXT("/Game/LevelPrototyping/Meshes/SM_Cylinder.SM_Cylinder"));
	Gate.MaterialOverride = DebugMaterial;
	Gate.DebugColor = FLinearColor(1.0f, 0.32f, 0.02f, 1.0f);
	Gate.LocalScale = FVector(1.2, 1.2, 0.3);
	Gate.LocalOffset = FVector::ZeroVector;
	Gate.bCollisionEnabled = false;
	Gate.bPlaceFloorUnderTile = true;

	FStageBlockoutTileDefinition Obstruction;
	Obstruction.Symbol = TEXT("X");
	Obstruction.ComponentName = TEXT("BreakableObstruction");
	Obstruction.Mesh = MeshRef(TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube.SM_ChamferCube"));
	Obstruction.MaterialOverride = DebugMaterial;
	Obstruction.DebugColor = FLinearColor(0.95f, 0.18f, 0.12f, 1.0f);
	Obstruction.LocalScale = FVector(0.9, 3.35, 0.75);
	Obstruction.LocalOffset = FVector::ZeroVector;
	Obstruction.LocalRotation = FRotator::ZeroRotator;
	Obstruction.bPlaceFloorUnderTile = true;

	FStageBlockoutTileDefinition Reward;
	Reward.Symbol = TEXT("R");
	Reward.ComponentName = TEXT("RewardMarker");
	Reward.Mesh = MeshRef(TEXT("/Game/LevelPrototyping/Meshes/SM_Cylinder.SM_Cylinder"));
	Reward.MaterialOverride = DebugMaterial;
	Reward.DebugColor = FLinearColor(1.0f, 0.88f, 0.08f, 1.0f);
	Reward.LocalScale = FVector(1.0, 1.0, 0.65);
	Reward.LocalOffset = FVector::ZeroVector;
	Reward.bCollisionEnabled = false;
	Reward.bPlaceFloorUnderTile = true;

	FStageBlockoutTileDefinition Enemy;
	Enemy.Symbol = TEXT("E");
	Enemy.ComponentName = TEXT("EnemyMarker");
	Enemy.Mesh = MeshRef(TEXT("/Game/LevelPrototyping/Meshes/SM_Cylinder.SM_Cylinder"));
	Enemy.MaterialOverride = DebugMaterial;
	Enemy.DebugColor = FLinearColor(0.95f, 0.1f, 0.48f, 1.0f);
	Enemy.LocalScale = FVector(0.65, 0.65, 1.8);
	Enemy.LocalOffset = FVector::ZeroVector;
	Enemy.bCollisionEnabled = false;
	Enemy.bPlaceFloorUnderTile = true;

	FStageBlockoutTileDefinition Encounter;
	Encounter.Symbol = TEXT("Z");
	Encounter.ComponentName = TEXT("EncounterMarker");
	Encounter.Mesh = MeshRef(TEXT("/Game/LevelPrototyping/Meshes/SM_Cylinder.SM_Cylinder"));
	Encounter.MaterialOverride = DebugMaterial;
	Encounter.DebugColor = FLinearColor(0.12f, 0.28f, 1.0f, 1.0f);
	Encounter.LocalScale = FVector(1.35, 1.35, 0.35);
	Encounter.LocalOffset = FVector::ZeroVector;
	Encounter.bCollisionEnabled = false;
	Encounter.bPlaceFloorUnderTile = true;

	FStageBlockoutTileDefinition Start;
	Start.Symbol = TEXT("S");
	Start.ComponentName = TEXT("StartMarker");
	Start.Mesh = MeshRef(TEXT("/Game/LevelPrototyping/Meshes/SM_Cylinder.SM_Cylinder"));
	Start.MaterialOverride = DebugMaterial;
	Start.DebugColor = FLinearColor(0.0f, 0.95f, 0.25f, 1.0f);
	Start.LocalScale = FVector(0.9, 0.9, 0.35);
	Start.LocalOffset = FVector::ZeroVector;
	Start.bCollisionEnabled = false;
	Start.bPlaceFloorUnderTile = true;

	FStageBlockoutTileDefinition Finish;
	Finish.Symbol = TEXT("F");
	Finish.ComponentName = TEXT("FinishMarker");
	Finish.Mesh = MeshRef(TEXT("/Game/LevelPrototyping/Meshes/SM_Cylinder.SM_Cylinder"));
	Finish.MaterialOverride = DebugMaterial;
	Finish.DebugColor = FLinearColor(0.55f, 0.2f, 1.0f, 1.0f);
	Finish.LocalScale = FVector(1.15, 1.15, 0.4);
	Finish.LocalOffset = FVector::ZeroVector;
	Finish.bCollisionEnabled = false;
	Finish.bPlaceFloorUnderTile = true;

	TileDefinitions = {
		Wall,
		Pillar,
		Gate,
		Obstruction,
		Reward,
		Enemy,
		Encounter,
		Start,
		Finish
	};

	InlineLayoutRows = {
		TEXT(".,W,W,W,W,W,W,.,.,.,.,.,W,W,W,W,W,W,W"),
		TEXT(".,W,S,_,_,C,W,W,W,W,W,W,W,_,_,C,_,F,W"),
		TEXT(".,W,_,E,Z,E,G,_,_,X,_,_,G,E,Z,E,_,R,W"),
		TEXT(".,W,_,_,_,C,W,W,W,W,W,W,W,_,_,E,_,_,W"),
		TEXT(".,W,W,W,W,W,W,.,.,.,.,.,W,W,W,W,W,W,W")
	};
}

void AStageBlockoutBuilder::BuildFromInlineLayout()
{
	BuildFromLines(InlineLayoutRows, TEXT("Inline Layout"));
}

void AStageBlockoutBuilder::BuildFromCsvFile()
{
	FString FilePath = CsvLayoutFile.FilePath;
	FilePath.TrimStartAndEndInline();

	if (FilePath.IsEmpty())
	{
		LogBuilderMessage(TEXT("CSV path is empty."));
		return;
	}

	if (FPaths::IsRelative(FilePath))
	{
		FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), FilePath);
	}

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath))
	{
		LogBuilderMessage(FString::Printf(TEXT("Failed to read CSV layout: %s"), *FilePath));
		return;
	}

	BuildFromLines(Lines, FilePath);
}

void AStageBlockoutBuilder::ClearGeneratedBlockout()
{
	Modify();

	TArray<UActorComponent*> ComponentsToRemove;
	GetComponents(ComponentsToRemove);

	int32 RemovedCount = 0;
	for (UActorComponent* Component : ComponentsToRemove)
	{
		if (!Component || !Component->ComponentHasTag(GeneratedComponentTag))
		{
			continue;
		}

		Component->Modify();
		RemoveInstanceComponent(Component);
		Component->DestroyComponent();
		++RemovedCount;
	}

	LastGeneratedInstanceCount = 0;
	LogBuilderMessage(FString::Printf(TEXT("Cleared %d generated components."), RemovedCount));
}

void AStageBlockoutBuilder::BuildFromLines(const TArray<FString>& Lines, const FString& SourceName)
{
	if (bClearBeforeBuild)
	{
		ClearGeneratedBlockout();
	}

	TArray<FLayoutLayer> ParsedLayers;
	if (!ParseLayoutLines(Lines, ParsedLayers) || ParsedLayers.Num() == 0)
	{
		LogBuilderMessage(FString::Printf(TEXT("No layout rows found in %s."), *SourceName));
		return;
	}

	Modify();

	int32 MaxRows = 0;
	int32 MaxColumns = 0;
	for (const FLayoutLayer& Layer : ParsedLayers)
	{
		MaxRows = FMath::Max(MaxRows, Layer.Num());
		for (const FLayoutRow& Row : Layer)
		{
			MaxColumns = FMath::Max(MaxColumns, Row.Num());
		}
	}

	const FVector CenterOffset = bCenterGeneratedLayout
		? FVector(-(MaxColumns - 1) * CellSize * 0.5f, (MaxRows - 1) * CellSize * 0.5f, 0.0f)
		: FVector::ZeroVector;

	TMap<FName, TObjectPtr<UInstancedStaticMeshComponent>> GeneratedComponents;
	TSet<FString> WarnedUnknownSymbols;
	LastGeneratedInstanceCount = 0;

	for (int32 LayerIndex = 0; LayerIndex < ParsedLayers.Num(); ++LayerIndex)
	{
		const FLayoutLayer& Layer = ParsedLayers[LayerIndex];

		for (int32 RowIndex = 0; RowIndex < Layer.Num(); ++RowIndex)
		{
			const FLayoutRow& Row = Layer[RowIndex];

			for (int32 ColumnIndex = 0; ColumnIndex < Row.Num(); ++ColumnIndex)
			{
				const FString Symbol = NormalizeToken(Row[ColumnIndex]);
				if (IsEmptySymbol(Symbol))
				{
					continue;
				}

				const FStageBlockoutTileDefinition* TileDefinition = FindDefinitionForSymbol(Symbol);
				if (!IsFloorOnlySymbol(Symbol) && !TileDefinition)
				{
					if (!WarnedUnknownSymbols.Contains(Symbol))
					{
						LogBuilderMessage(FString::Printf(TEXT("Unknown layout symbol skipped: %s"), *Symbol));
						WarnedUnknownSymbols.Add(Symbol);
					}
					continue;
				}

				const FVector CellLocation(
					ColumnIndex * CellSize,
					-RowIndex * CellSize,
					LayerIndex * LayerHeight);
				const FVector LocalCellLocation = CellLocation + CenterOffset;

				if (ShouldPlaceFloorForSymbol(Symbol, TileDefinition))
				{
					AddTileInstance(FloorTile, LocalCellLocation, GeneratedComponents);
				}

				if (TileDefinition && TileDefinition->bGenerate)
				{
					AddTileInstance(*TileDefinition, LocalCellLocation, GeneratedComponents);
				}
			}
		}
	}

	LogBuilderMessage(FString::Printf(
		TEXT("Built %d instances from %s across %d layer(s)."),
		LastGeneratedInstanceCount,
		*SourceName,
		ParsedLayers.Num()));
}

bool AStageBlockoutBuilder::ParseLayoutLines(const TArray<FString>& Lines, TArray<FLayoutLayer>& OutLayers) const
{
	FLayoutLayer CurrentLayer;

	auto CommitLayer = [&CurrentLayer, &OutLayers]()
	{
		if (CurrentLayer.Num() > 0)
		{
			OutLayers.Add(CurrentLayer);
			CurrentLayer.Reset();
		}
	};

	for (const FString& RawLine : Lines)
	{
		const FString Line = NormalizeToken(RawLine);

		if (Line.IsEmpty())
		{
			CommitLayer();
			continue;
		}

		if (Line.StartsWith(TEXT("#")) || Line.StartsWith(TEXT("//")))
		{
			continue;
		}

		if (Line.StartsWith(TEXT("Layer"), ESearchCase::IgnoreCase)
			|| (Line.StartsWith(TEXT("[")) && Line.Contains(TEXT("Layer"), ESearchCase::IgnoreCase)))
		{
			CommitLayer();
			continue;
		}

		const FLayoutRow Row = ParseLayoutRow(Line);
		if (Row.Num() > 0)
		{
			CurrentLayer.Add(Row);
		}
	}

	CommitLayer();
	return OutLayers.Num() > 0;
}

TArray<FString> AStageBlockoutBuilder::ParseLayoutRow(const FString& Line) const
{
	TArray<FString> Tokens;

	if (Line.Contains(TEXT(",")))
	{
		Line.ParseIntoArray(Tokens, TEXT(","), false);
		for (FString& Token : Tokens)
		{
			Token = NormalizeToken(Token);
		}
		return Tokens;
	}

	for (int32 Index = 0; Index < Line.Len(); ++Index)
	{
		const TCHAR Character = Line[Index];
		if (!FChar::IsWhitespace(Character))
		{
			Tokens.Add(FString(1, &Character));
		}
	}

	return Tokens;
}

const FStageBlockoutTileDefinition* AStageBlockoutBuilder::FindDefinitionForSymbol(const FString& Symbol) const
{
	for (const FStageBlockoutTileDefinition& Definition : TileDefinitions)
	{
		if (Definition.Symbol.Equals(Symbol, ESearchCase::IgnoreCase))
		{
			return &Definition;
		}
	}

	return nullptr;
}

bool AStageBlockoutBuilder::IsEmptySymbol(const FString& Symbol) const
{
	return Symbol.IsEmpty() || Symbol.Equals(EmptySymbol, ESearchCase::IgnoreCase);
}

bool AStageBlockoutBuilder::IsFloorOnlySymbol(const FString& Symbol) const
{
	return Symbol.Equals(FloorOnlySymbol, ESearchCase::IgnoreCase);
}

bool AStageBlockoutBuilder::ShouldPlaceFloorForSymbol(const FString& Symbol, const FStageBlockoutTileDefinition* TileDefinition) const
{
	if (IsFloorOnlySymbol(Symbol))
	{
		return true;
	}

	return bGenerateFloorUnderMarkedTiles && TileDefinition && TileDefinition->bPlaceFloorUnderTile;
}

void AStageBlockoutBuilder::AddTileInstance(
	const FStageBlockoutTileDefinition& Definition,
	const FVector& CellLocation,
	TMap<FName, TObjectPtr<UInstancedStaticMeshComponent>>& GeneratedComponents)
{
	UInstancedStaticMeshComponent* Component = FindOrCreateGeneratedComponent(Definition, GeneratedComponents);
	if (!Component)
	{
		return;
	}

	FVector InstanceLocation = CellLocation + FVector(Definition.LocalOffset.X, Definition.LocalOffset.Y, 0.0);
	const UStaticMesh* StaticMesh = Component->GetStaticMesh();
	if (StaticMesh)
	{
		const FBoxSphereBounds MeshBounds = StaticMesh->GetBounds();
		const FVector ScaledBoundsOrigin(
			MeshBounds.Origin.X * Definition.LocalScale.X,
			MeshBounds.Origin.Y * Definition.LocalScale.Y,
			MeshBounds.Origin.Z * Definition.LocalScale.Z);
		const FVector RotatedBoundsOrigin = Definition.LocalRotation.RotateVector(ScaledBoundsOrigin);
		InstanceLocation.X -= RotatedBoundsOrigin.X;
		InstanceLocation.Y -= RotatedBoundsOrigin.Y;
	}

	if (Definition.bSnapBottomToCellFloor)
	{
		if (StaticMesh)
		{
			const FBoxSphereBounds MeshBounds = StaticMesh->GetBounds();
			const float BottomToPivot = (MeshBounds.BoxExtent.Z - MeshBounds.Origin.Z) * Definition.LocalScale.Z;
			const float SmallGroundClearance = FMath::Abs(Definition.LocalOffset.Z) <= 20.0f ? Definition.LocalOffset.Z : 0.0f;
			InstanceLocation.Z += BottomToPivot + SmallGroundClearance;
		}
	}
	else
	{
		InstanceLocation.Z += Definition.LocalOffset.Z;
	}

	const FTransform InstanceTransform(
		Definition.LocalRotation,
		InstanceLocation,
		Definition.LocalScale);

	Component->AddInstance(InstanceTransform);
	++LastGeneratedInstanceCount;
}

UInstancedStaticMeshComponent* AStageBlockoutBuilder::FindOrCreateGeneratedComponent(
	const FStageBlockoutTileDefinition& Definition,
	TMap<FName, TObjectPtr<UInstancedStaticMeshComponent>>& GeneratedComponents)
{
	if (TObjectPtr<UInstancedStaticMeshComponent>* ExistingComponent = GeneratedComponents.Find(Definition.ComponentName))
	{
		return ExistingComponent->Get();
	}

	UStaticMesh* Mesh = Definition.Mesh.LoadSynchronous();
	if (!Mesh)
	{
		LogBuilderMessage(FString::Printf(
			TEXT("Missing mesh for symbol %s (%s)."),
			*Definition.Symbol,
			*Definition.ComponentName.ToString()));
		return nullptr;
	}

	const FName ComponentObjectName = MakeUniqueObjectName(
		this,
		UInstancedStaticMeshComponent::StaticClass(),
		*FString::Printf(TEXT("Generated_%s"), *Definition.ComponentName.ToString()));

	UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(this, ComponentObjectName);
	Component->SetFlags(RF_Transactional);
	Component->CreationMethod = EComponentCreationMethod::Instance;
	Component->ComponentTags.Add(GeneratedComponentTag);
	Component->SetStaticMesh(Mesh);
	Component->SetMobility(EComponentMobility::Static);
	Component->SetCollisionEnabled(Definition.bCollisionEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	ApplyTileMaterial(Definition, Component, Mesh);
	Component->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	AddInstanceComponent(Component);
	Component->RegisterComponent();

	GeneratedComponents.Add(Definition.ComponentName, Component);
	return Component;
}

void AStageBlockoutBuilder::ApplyTileMaterial(
	const FStageBlockoutTileDefinition& Definition,
	UInstancedStaticMeshComponent* Component,
	const UStaticMesh* Mesh) const
{
	if (!Component)
	{
		return;
	}

	UMaterialInterface* BaseMaterial = Definition.MaterialOverride.LoadSynchronous();
	if (!BaseMaterial)
	{
		BaseMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Game/LevelPrototyping/Materials/MI_DefaultColorway.MI_DefaultColorway"));
	}
	if (!BaseMaterial)
	{
		BaseMaterial = Component->GetMaterial(0);
	}

	if (!BaseMaterial)
	{
		return;
	}

	UMaterialInterface* MaterialToApply = BaseMaterial;
	if (Definition.bApplyDebugColor)
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Component);
		if (DynamicMaterial)
		{
			const FLinearColor AppliedColor = Definition.DebugColor.Equals(FLinearColor::White)
				? DefaultColorForSymbol(Definition.Symbol)
				: Definition.DebugColor;
			DynamicMaterial->SetVectorParameterValue(TEXT("Base Color"), AppliedColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), AppliedColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), AppliedColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("Tint"), AppliedColor);
			DynamicMaterial->SetScalarParameterValue(TEXT("Metallic"), 0.0f);
			DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.72f);
			MaterialToApply = DynamicMaterial;
		}
	}

	const int32 MaterialSlotCount = Mesh ? FMath::Max(1, Mesh->GetStaticMaterials().Num()) : FMath::Max(1, Component->GetNumMaterials());
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
	{
		Component->SetMaterial(MaterialIndex, MaterialToApply);
	}
}

void AStageBlockoutBuilder::LogBuilderMessage(const FString& Message) const
{
	UE_LOG(LogTemp, Log, TEXT("[StageBlockoutBuilder:%s] %s"), *GetName(), *Message);
}
