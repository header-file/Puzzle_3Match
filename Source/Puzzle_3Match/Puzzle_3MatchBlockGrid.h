// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Puzzle_3MatchBlockGrid.generated.h"

#define SIZE 9
#define DESTROY_MAX 45
#define TEMPSIZE 45
#define OBJECTIVES 8

enum class EGameMode : uint8
{
	SCORING = 0,
	OBJECT_FALL,
	TIME_ATTACK
};

/** Class used to spawn blocks and manage score */
UCLASS(minimalapi)
class APuzzle_3MatchBlockGrid : public AActor
{
	GENERATED_BODY()

	/** Dummy root component */
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

public:
	APuzzle_3MatchBlockGrid();

	/** Number of blocks along each side of grid */
	UPROPERTY(Category=Grid, EditAnywhere, BlueprintReadOnly)
	int32 Size;

	/** Spacing of blocks */
	UPROPERTY(Category=Grid, EditAnywhere, BlueprintReadOnly)
	float BlockSpacing;

	class APuzzle_3MatchBlock* GetBlock(int hor, int ver) { return Blocks[hor][ver]; }
	void SwitchBlock(int a, int b, int c, int d);

	UFUNCTION(BlueprintCallable)
		void SetGameMode(int i);

	UFUNCTION(BlueprintPure)
		int GetGameMode() { return (int)GameMode; }

	void Switching(float Delta);
	void Falling(float Delta);
	bool GetMoving() { return bMoving; }
	bool GetFalling() { return bFalling; }

protected:
	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

public:
	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }

	void SpawnBlock(FVector Location, int i, int j);
	void SpawnSpecialBlock(int Type, int h, int v);
	void SpawnObjective(int h, int v);

	void LineCheckHorizon(int h, int v);
	void LineCheckVertical(int h, int v);
	void CheckAll();
	void CheckMatch();
	void AfterCheck();
	void CheckHorizontal(int h, int v);
	void CheckVertical(int h, int v);
	void MatchSpecialBlock(int Type, int h, int v);
	void AfterDestroy();

	void BeamTick();

	class APuzzle_3MatchBlock* Blocks[SIZE][SIZE];
	class APuzzle_3MatchBlock* Destroys[DESTROY_MAX];
	int DestroyCount = 0;

	class APuzzle_3MatchBlock* TempBlocks[TEMPSIZE];

	FVector TargetLocations[SIZE][SIZE];

	float MoveSpeed;

	bool bMoving;
	bool bFalling;
	bool bReturn;

	int aX, aY;
	int bX, bY;
	FVector aLocEnd;
	FVector bLocEnd;


private:
	TSubclassOf<class APuzzle_3MatchBlock> BlockClass;
	TSubclassOf<class ASpecialBlock> SBlockClass;
	TSubclassOf<class AObjective> ObjectiveClass;

	class APuzzle_3MatchPawn* Player;

	FTimerHandle DestroyHandle;

	float ComboRate = 1.0f;
	float ScorePoint = 10.0f;

	EGameMode GameMode;

	class UParticleSystem* BeamSystem[2];
	class UParticleSystemComponent* PSCBeam[2];
	FVector StartLoc[2];
	FVector EndLoc[2];
	int SpecialBlockType;
};



