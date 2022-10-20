#include "Puzzle_3MatchPawn.h"
#include "Global.h"
#include "Puzzle_3MatchBlock.h"
#include "Puzzle_3MatchBlockGrid.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Objective.h"

APuzzle_3MatchPawn::APuzzle_3MatchPawn(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	StartIndex.Key = 0;
	StartIndex.Value = 0;
	EndIndex.Key = 0;
	EndIndex.Value = 0;

	ObjectiveCount = 0;
}

void APuzzle_3MatchPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bFinish) return;

	if (LeftCount <= 0)
		bFinish = true;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			if (UCameraComponent* OurCamera = PC->GetViewTarget()->FindComponentByClass<UCameraComponent>())
			{
				FVector Start = OurCamera->GetComponentLocation();
				FVector End = Start + (OurCamera->GetComponentRotation().Vector() * 8000.0f);
				TraceForBlock(Start, End, true);
			}
		}
		else
		{
			FVector Start, Dir, End;
			PC->DeprojectMousePositionToWorld(Start, Dir);
			End = Start + (Dir * 8000.0f);
			TraceForBlock(Start, End, false);
		}
	}

	if (Grid->GetMoving())
		Grid->Switching(DeltaSeconds);

	if (Grid->GetFalling())
		Grid->Falling(DeltaSeconds);

	Grid->BeamTick();
}

void APuzzle_3MatchPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", EInputEvent::IE_Pressed, this, &APuzzle_3MatchPawn::OnResetVR);
	PlayerInputComponent->BindAction("TriggerClick", EInputEvent::IE_Pressed, this, &APuzzle_3MatchPawn::TriggerClick);
	PlayerInputComponent->BindAction("TriggerClick", EInputEvent::IE_Released, this, &APuzzle_3MatchPawn::TriggerClickRelease);
}

void APuzzle_3MatchPawn::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{
	Super::CalcCamera(DeltaTime, OutResult);

	OutResult.Rotation = FRotator(-90.0f, -90.0f, 0.0f);
}

void APuzzle_3MatchPawn::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void APuzzle_3MatchPawn::TriggerClick()
{
	if (Grid->GetFalling() || Grid->GetMoving()) return;

	if (CurrentBlockFocus)
	{
		if (CurrentBlockFocus->GetColor() == 5) return;
		
		StartIndex = CurrentBlockFocus->GetIndex();
	}
}

void APuzzle_3MatchPawn::TriggerClickRelease()
{
	if (CurrentBlockFocus)
	{
		if (CurrentBlockFocus->GetColor() == 5) return;

		EndIndex = CurrentBlockFocus->GetIndex();

		SwitchBlock();	
	}
}

void APuzzle_3MatchPawn::TraceForBlock(const FVector& Start, const FVector& End, bool bDrawDebugHelpers)
{
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
	if (bDrawDebugHelpers)
	{
		DrawDebugLine(GetWorld(), Start, HitResult.Location, FColor::Red);
		DrawDebugSolidBox(GetWorld(), HitResult.Location, FVector(20.0f), FColor::Red);
	}
	if (HitResult.Actor.IsValid())
	{
		APuzzle_3MatchBlock* HitBlock = Cast<APuzzle_3MatchBlock>(HitResult.Actor.Get());
		if (CurrentBlockFocus != HitBlock)
		{
			if (CurrentBlockFocus)
			{
				CurrentBlockFocus->Highlight(false);
			}
			if (HitBlock)
			{
				HitBlock->Highlight(true);
			}
			CurrentBlockFocus = HitBlock;
		}
	}
	else if (CurrentBlockFocus)
	{
		CurrentBlockFocus->Highlight(false);
		CurrentBlockFocus = nullptr;
	}
}

void APuzzle_3MatchPawn::SwitchBlock()
{
	if (EndIndex.Key > StartIndex.Key)
	{
		EndIndex.Key = StartIndex.Key + 1;
		EndIndex.Value = StartIndex.Value;
	}
	else if (EndIndex.Key < StartIndex.Key)
	{
		EndIndex.Key = StartIndex.Key - 1;
		EndIndex.Value = StartIndex.Value;
	}
	else if (EndIndex.Value > StartIndex.Value)
	{
		EndIndex.Key = StartIndex.Key;
		EndIndex.Value = StartIndex.Value + 1;
	}
	else if (EndIndex.Value < StartIndex.Value)
	{
		EndIndex.Key = StartIndex.Key;
		EndIndex.Value = StartIndex.Value - 1;
	}

	if (Grid->GetBlock(EndIndex.Key, EndIndex.Value)->GetColor() == 5 ||
		Grid->GetBlock(StartIndex.Key, StartIndex.Value)->GetColor() == 5) return;

	Grid->SwitchBlock(StartIndex.Key, StartIndex.Value, EndIndex.Key, EndIndex.Value);
}