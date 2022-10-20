// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Puzzle_3MatchBlock.h"
#include "SpecialBlock.generated.h"


UENUM()
enum class ESpecialType : uint8
{
	HORIZONTAL = 0,
	VERTICAL,
	CROSS,
};

UCLASS()
class PUZZLE_3MATCH_API ASpecialBlock : public APuzzle_3MatchBlock
{
	GENERATED_BODY()

public:
	ASpecialBlock();

	void SetSpecialType(int SType) { SpecialType = (ESpecialType)SType; }

	UFUNCTION(BlueprintPure)
		int GetSpecialType() { return (int)SpecialType; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
		ESpecialType SpecialType;
};
