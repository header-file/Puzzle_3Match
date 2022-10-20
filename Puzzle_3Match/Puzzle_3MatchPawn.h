// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Puzzle_3MatchPawn.generated.h"

UCLASS(config=Game)
class APuzzle_3MatchPawn : public APawn
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

	UFUNCTION(BlueprintCallable)
		void SetGrid(class APuzzle_3MatchBlockGrid* BlockGrid) { Grid = BlockGrid; }

	UFUNCTION(BlueprintCallable)
		void SetLeftCount(int i) { LeftCount = i; }

	UFUNCTION(BlueprintPure)
		int GetLeftCount() { return LeftCount; }

	UFUNCTION(BlueprintPure)
		int GetScore() { return (int)Score; }

	UFUNCTION(BlueprintPure)
		int GetObjectiveCount() { return ObjectiveCount; }

	UFUNCTION(BlueprintPure)
		bool GetFinish() { return bFinish; }

	UFUNCTION(BlueprintPure)
		APuzzle_3MatchBlockGrid* GetGrid() { return Grid; }

	void AddScore(float f) { Score += f; }
	void SetObjectiveCount(int i) { ObjectiveCount = i; }
	void SetFinish(bool b) { bFinish = b; }

protected:
	void OnResetVR();
	void TriggerClick();
	void TriggerClickRelease();
	void TraceForBlock(const FVector& Start, const FVector& End, bool bDrawDebugHelpers);

	void SwitchBlock();

	class APuzzle_3MatchBlockGrid* Grid;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	class APuzzle_3MatchBlock* CurrentBlockFocus;

	TPair<unsigned int, unsigned int> StartIndex;
	TPair<unsigned int, unsigned int> EndIndex;

private:
	int LeftCount = 0;
	float Score = 0.0f;
	int ObjectiveCount = 0;
	bool bFinish = false;
};
