// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Puzzle_3MatchBlock.generated.h"

UENUM()
enum class EBlockType : uint8
{
	RED = 0,
	BLUE,
	GREEN,
	YELLOW,
	SPECIAL,
	OBJECTIVE
};

/** A block that can be clicked */
UCLASS(Blueprintable)
class APuzzle_3MatchBlock : public AActor
{
	GENERATED_BODY()

	/** Dummy root component */
	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

	/** StaticMesh component for the clickable block */
	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BlockMesh;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UTextRenderComponent* Text;


public:
	APuzzle_3MatchBlock();

	/** Are we currently active? */
	bool bIsActive;

	/** Pointer to white material used on the focused block */
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* BaseMaterial;

	UPROPERTY(EditAnywhere)
	class UMaterialInstance* OverMaterial;

	/** Grid that owns us */
	UPROPERTY()
	class APuzzle_3MatchBlockGrid* OwningGrid;

	/** Handle the block being clicked */
	UFUNCTION()
	void BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked);

	/** Handle the block being touched  */
	UFUNCTION()
	void OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent);

	void Highlight(bool bOn);

	void SetIndex(int a, int b); //{ Index.Key = (unsigned int)a; Index.Value = (unsigned int)b; }
	void SetType(int i) { Type = (EBlockType)i; }

	TPair<unsigned int, unsigned int> GetIndex() { return Index; }
	
	int GetColor() { return (int)Type; }

	bool GetFall() { return IsFall; }
	void SetFall(bool b) { IsFall = b; }

public:
	UFUNCTION(BlueprintPure)
		int GetHorIndex() { return Index.Key; };

	UFUNCTION(BlueprintPure)
		int GetVerIndex() { return Index.Value; }

	UFUNCTION(BlueprintPure)
		bool GetDestroyed() { return IsDestroyed; }

protected:
	virtual void BeginPlay() override;

public:
	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }
	/** Returns BlockMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetBlockMesh() const { return BlockMesh; }

	void SetDestroyed(bool b) { IsDestroyed = b; }

protected:
	EBlockType Type;
	TPair<unsigned int, unsigned int> Index;

private:
	class UMaterialInstanceDynamic* BaseDynamic;
	class UMaterialInstanceDynamic* OverDynamic;

	bool IsFall;
	bool IsDestroyed;
};



