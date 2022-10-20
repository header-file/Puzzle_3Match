#include "SpecialBlock.h"
#include "Global.h"
#include "Puzzle_3MatchBlock.h"

ASpecialBlock::ASpecialBlock()
{
	Type = (EBlockType)4;
	//SpecialType = (ESpecialType)SType;

	//Ÿ�Կ� ���� ������
	ConstructorHelpers::FObjectFinder<UMaterialInstance> Base(TEXT("MaterialInstanceConstant'/Game/ForceShields/Materials/Basic/Basic.Basic'"));
	ConstructorHelpers::FObjectFinder<UMaterialInstance> Shake(TEXT("MaterialInstanceConstant'/Game/ForceShields/Materials/Basic/Basic.Basic'"));

	BaseMaterial = Base.Object;
	OverMaterial = Base.Object;
}

void ASpecialBlock::BeginPlay()
{
	Super::BeginPlay();
}
