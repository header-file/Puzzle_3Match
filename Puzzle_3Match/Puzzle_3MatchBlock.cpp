#include "Puzzle_3MatchBlock.h"
#include "Global.h"
#include "Puzzle_3MatchBlockGrid.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/TextRenderComponent.h"

APuzzle_3MatchBlock::APuzzle_3MatchBlock()
{
	Type = (EBlockType)(rand() % 4);

	ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh(TEXT("StaticMesh'/Game/MobileStarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));

	ConstructorHelpers::FObjectFinder<UMaterialInstance> BaseRed(TEXT("MaterialInstanceConstant'/Game/Materials/Red.Red'"));
	ConstructorHelpers::FObjectFinder<UMaterialInstance> BaseBlue(TEXT("MaterialInstanceConstant'/Game/Materials/Blue.Blue'"));
	ConstructorHelpers::FObjectFinder<UMaterialInstance> BaseGreen(TEXT("MaterialInstanceConstant'/Game/Materials/Green.Green'"));
	ConstructorHelpers::FObjectFinder<UMaterialInstance> BaseYellow(TEXT("MaterialInstanceConstant'/Game/Materials/Yellow.Yellow'"));

	ConstructorHelpers::FObjectFinder<UMaterialInstance> OverRed(TEXT("MaterialInstanceConstant'/Game/Materials/ShakeRed.ShakeRed'"));
	ConstructorHelpers::FObjectFinder<UMaterialInstance> OverBlue(TEXT("MaterialInstanceConstant'/Game/Materials/ShakeBlue.ShakeBlue'"));
	ConstructorHelpers::FObjectFinder<UMaterialInstance> OverGreen(TEXT("MaterialInstanceConstant'/Game/Materials/ShakeGreen.ShakeGreen'"));
	ConstructorHelpers::FObjectFinder<UMaterialInstance> OverYellow(TEXT("MaterialInstanceConstant'/Game/Materials/ShakeYellow.ShakeYellow'"));

	switch (Type)
	{
		case EBlockType::RED:
			if (BaseRed.Succeeded()) BaseMaterial = BaseRed.Object;
			if (OverRed.Succeeded()) OverMaterial = OverRed.Object;
			break;
		case EBlockType::BLUE:
			if (BaseBlue.Succeeded()) BaseMaterial = BaseBlue.Object;
			if (OverBlue.Succeeded()) OverMaterial = OverBlue.Object;
			break;
		case EBlockType::GREEN:
			if (BaseGreen.Succeeded()) BaseMaterial = BaseGreen.Object;
			if (OverGreen.Succeeded()) OverMaterial = OverGreen.Object;
			break;
		case EBlockType::YELLOW:
			if (BaseYellow.Succeeded()) BaseMaterial = BaseYellow.Object;
			if (OverYellow.Succeeded()) OverMaterial = OverYellow.Object;
			break;
	}

	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	// Create static mesh component
	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh0"));
	BlockMesh->SetStaticMesh(PlaneMesh.Get());
	BlockMesh->SetRelativeScale3D(FVector(1.2f,1.2f,1.2f));
	BlockMesh->SetRelativeLocation(FVector(0.f,0.f,25.f));
	BlockMesh->SetMaterial(0, BaseMaterial);
	BlockMesh->SetupAttachment(DummyRoot);
	BlockMesh->OnClicked.AddDynamic(this, &APuzzle_3MatchBlock::BlockClicked);
	BlockMesh->OnInputTouchBegin.AddDynamic(this, &APuzzle_3MatchBlock::OnFingerPressedBlock);

	Text = CreateDefaultSubobject<UTextRenderComponent>("Text");
	Text->SetupAttachment(RootComponent);
	Text->SetWorldRotation(FRotator(90.0f, -90.0f, 90.0f).Quaternion());
	FString str = "";
	Text->Text = FText::FromString(*str);

	IsFall = false;
	IsDestroyed = false;
}

void APuzzle_3MatchBlock::BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked)
{
	//HandleClicked();
}


void APuzzle_3MatchBlock::OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent)
{
	//HandleClicked();
}

void APuzzle_3MatchBlock::Highlight(bool bOn)
{
	if (!bOn)
	{
		BlockMesh->SetMaterial(0, BaseMaterial);
	}
	else
	{
		BlockMesh->SetMaterial(0, OverMaterial);
	}
}

void APuzzle_3MatchBlock::SetIndex(int a, int b)
{
	Index.Key = (unsigned int)a; 
	Index.Value = (unsigned int)b;
}

void APuzzle_3MatchBlock::BeginPlay()
{
	Super::BeginPlay();

	BlockMesh->SetMaterial(0, BaseMaterial);
}
