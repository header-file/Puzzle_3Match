#include "Puzzle_3MatchBlockGrid.h"
#include "Global.h"
#include "Puzzle_3MatchBlock.h"
#include "SpecialBlock.h"
#include "Components/TextRenderComponent.h"
#include "Puzzle_3MatchPawn.h"
#include "TimerManager.h"
#include "Objective.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"


#define LOCTEXT_NAMESPACE "PuzzleBlockGrid"

APuzzle_3MatchBlockGrid::APuzzle_3MatchBlockGrid()
{
	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	ConstructorHelpers::FClassFinder<APuzzle_3MatchBlock> block(L"Blueprint'/Game/BP/BpCPuzzle_3MatchBlock.BpCPuzzle_3MatchBlock_C'");
	if (block.Succeeded()) BlockClass = block.Class;

	ConstructorHelpers::FClassFinder<APuzzle_3MatchBlock> sblock(L"Blueprint'/Game/BP/BpCSpecialBlock.BpCSpecialBlock_C'");
	if (sblock.Succeeded()) SBlockClass = sblock.Class;

	ConstructorHelpers::FClassFinder<AObjective> objective(L"Blueprint'/Game/BP/BpCObjective.BpCObjective_C'");
	if (objective.Succeeded()) ObjectiveClass = objective.Class;

	ConstructorHelpers::FObjectFinder<UParticleSystem> beam(L"ParticleSystem'/Game/Assets/PBeam.PBeam'");
	if (beam.Succeeded()) BeamSystem[0] = beam.Object;

	ConstructorHelpers::FObjectFinder<UParticleSystem> beam2(L"ParticleSystem'/Game/Assets/PBeam2.PBeam2'");
	if (beam2.Succeeded()) BeamSystem[1] = beam2.Object;

	// Set defaults
	Size = SIZE;
	BlockSpacing = 125.f;
	MoveSpeed = 10.0f;
	GameMode = EGameMode::SCORING;
	SpecialBlockType = -1;
}

void APuzzle_3MatchBlockGrid::SwitchBlock(int a, int b, int c, int d)
{
	if (!bMoving)
	{
		aX = a;
		aY = b;
		bX = c;
		bY = d;

		aLocEnd = Blocks[bX][bY]->GetActorLocation();
		bLocEnd = Blocks[aX][aY]->GetActorLocation();

		bMoving = true;
	}
}

void APuzzle_3MatchBlockGrid::SetGameMode(int i)
{
	GameMode = (EGameMode)i;

	if (i == 1)
	{
		for (int n = 0; n < OBJECTIVES; n++)
		{
			int h = rand() % SIZE;
			int v = rand() % SIZE;

			SpawnObjective(h, v);
		}

		Player->SetObjectiveCount(OBJECTIVES);
	}
		
}

void APuzzle_3MatchBlockGrid::Switching(float Delta)
{
	if (bMoving)
	{
		FVector aLoc = Blocks[aX][aY]->GetActorLocation();
		FVector bLoc = Blocks[bX][bY]->GetActorLocation();

		aLoc = UKismetMathLibrary::VLerp(aLoc, aLocEnd, Delta * MoveSpeed);
		bLoc = UKismetMathLibrary::VLerp(bLoc, bLocEnd, Delta * MoveSpeed);
		Blocks[aX][aY]->SetActorLocation(aLoc);
		Blocks[bX][bY]->SetActorLocation(bLoc);

		if (aLoc.Equals(aLocEnd, 0.01f))
		{
			Blocks[aX][aY]->SetIndex(bX, bY);
			Blocks[bX][bY]->SetIndex(aX, aY);

			Blocks[aX][aY]->SetActorLocation(aLocEnd);
			Blocks[bX][bY]->SetActorLocation(bLocEnd);

			APuzzle_3MatchBlock* tmp = Blocks[aX][aY];
			Blocks[aX][aY] = Blocks[bX][bY];
			Blocks[bX][bY] = tmp;

			bMoving = false;

			CheckMatch();
		}
	}
}

void APuzzle_3MatchBlockGrid::Falling(float Delta)
{
	int FallCount = 0;

	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
			if (Blocks[i][j]->GetFall() == false) continue;

			FVector curLocation = Blocks[i][j]->GetActorLocation();

			curLocation = UKismetMathLibrary::VLerp(curLocation, TargetLocations[i][j], Delta * MoveSpeed * 3.0f);

			Blocks[i][j]->SetActorLocation(curLocation);

			if(FallCount == 0)
				FallCount++;

			if (curLocation.Equals(TargetLocations[i][j], 0.01f))
			{
				Blocks[i][j]->SetActorLocation(TargetLocations[i][j]);
				Blocks[i][j]->SetFall(false);
			}
		}
	}

	if (FallCount == 0)
	{
		bFalling = false;

		CheckAll();
	}
}

void APuzzle_3MatchBlockGrid::BeginPlay()
{
	Super::BeginPlay();

	// Number of blocks
	const int32 NumBlocks = SIZE * SIZE;

	if (BlockClass == NULL) return;
	if (SBlockClass == NULL) return;

	// Loop to spawn each block
	for (int32 BlockIndex = 0; BlockIndex < NumBlocks; BlockIndex++)
	{
		int i = BlockIndex / SIZE;
		int j = BlockIndex % SIZE;

		const float XOffset = i * BlockSpacing; // Divide by dimension
		const float YOffset = j * BlockSpacing; // Modulo gives remainder

		// Make position vector, offset from Grid location
		const FVector BlockLocation = FVector(XOffset, YOffset, 0.f) + GetActorLocation();

		TargetLocations[i][j] = FVector::ZeroVector;

		SpawnBlock(BlockLocation, i, j);
	}

	for (int i = 0; i < DESTROY_MAX; i++)
		Destroys[i] = nullptr;

	for (int i = 0; i < TEMPSIZE; i++)
		TempBlocks[i] = nullptr;

	Player = Cast<APuzzle_3MatchPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	Player->SetLeftCount(20);
	ComboRate = 1.0f;
}

void APuzzle_3MatchBlockGrid::SpawnBlock(FVector Location, int i, int j)
{
	APuzzle_3MatchBlock* NewBlock = GetWorld()->SpawnActor<APuzzle_3MatchBlock>(BlockClass, Location, FRotator(0, 0, 0));
	Blocks[i][j] = NewBlock;
	Blocks[i][j]->SetIndex(i, j);

	int targetColor = Blocks[i][j]->GetColor();

	if (i > 1)
	{
		if (Blocks[i - 1][j]->GetColor() == targetColor &&
			Blocks[i - 2][j]->GetColor() == targetColor)
		{
			Blocks[i][j]->Destroy();
			Blocks[i][j] = nullptr;
			SpawnBlock(Location, i, j);
		}
			
	}
	if (j > 1)
	{
		if (Blocks[i][j - 1]->GetColor() == targetColor &&
			Blocks[i][j - 2]->GetColor() == targetColor)
		{
			Blocks[i][j]->Destroy();
			Blocks[i][j] = nullptr;
			SpawnBlock(Location, i, j);
		}
	}

	// Tell the block about its owner
	if (NewBlock != nullptr)
	{
		NewBlock->OwningGrid = this;
	}
}

void APuzzle_3MatchBlockGrid::SpawnSpecialBlock(int Type, int h, int v)
{
	float xLoc = h * BlockSpacing;
	float yLoc = v * BlockSpacing;
	const FVector Location = FVector(xLoc, yLoc, 0.0f) + GetActorLocation();

	ASpecialBlock* SpecialBlock = GetWorld()->SpawnActor<ASpecialBlock>(SBlockClass, Location, FRotator(0, 0, 0));
	SpecialBlock->SetSpecialType(Type);

	Blocks[h][v]->SetDestroyed(true);
	Blocks[h][v] = SpecialBlock;
	Blocks[h][v]->SetIndex(h, v);
}

void APuzzle_3MatchBlockGrid::SpawnObjective(int h, int v)
{
	if (Blocks[h][v]->GetColor() == 5 || h == 0)
	{
		int i = rand() % SIZE;
		int j = rand() % SIZE;

		SpawnObjective(i, j);
	}
	else
	{
		FVector Location = Blocks[h][v]->GetActorLocation();
		AObjective* NewObjective = GetWorld()->SpawnActor<AObjective>(ObjectiveClass, Location, FRotator(0, 0, 0));

		Blocks[h][v]->Destroy();
		Blocks[h][v] = NewObjective;
		Blocks[h][v]->SetIndex(h, v);
		Blocks[h][v]->SetType(5);
	}
}

void APuzzle_3MatchBlockGrid::LineCheckHorizon(int h, int v)
{
	int color = Blocks[h][v]->GetColor();

	if (color > 3) return;

	{
		if (Blocks[h][v + 1]->GetColor() == color &&
			Blocks[h][v + 2]->GetColor() == color)
		{
			Destroys[DestroyCount] = Blocks[h][v + 1];
			DestroyCount++;
			Destroys[DestroyCount] = Blocks[h][v + 2];
			DestroyCount++;

			if (v + 3 < SIZE &&
				Blocks[h][v + 3]->GetColor() == color)
			{
				Destroys[DestroyCount] = Blocks[h][v + 3];
				DestroyCount++;

				if (v + 4 < SIZE &&
					Blocks[h][v + 4]->GetColor() == color)
				{
					Destroys[DestroyCount] = Blocks[h][v + 4];
					DestroyCount++;
				}
			}
		}
	}
}

void APuzzle_3MatchBlockGrid::LineCheckVertical(int h, int v)
{
	int color = Blocks[h][v]->GetColor();

	if (color > 3) return;

	//Vertical
	{
		if (Blocks[h + 1][v]->GetColor() == color &&
			Blocks[h + 2][v]->GetColor() == color)
		{
			Destroys[DestroyCount] = Blocks[h + 1][v];
			DestroyCount++;
			Destroys[DestroyCount] = Blocks[h + 2][v];
			DestroyCount++;

			if (h + 3 < SIZE &&
				Blocks[h + 3][v]->GetColor() == color)
			{
				Destroys[DestroyCount] = Blocks[h + 3][v];
				DestroyCount++;

				if (h + 4 < SIZE &&
					Blocks[h + 4][v]->GetColor() == color)
				{
					Destroys[DestroyCount] = Blocks[h + 4][v];
					DestroyCount++;
				}
			}
		}
	}
}

void APuzzle_3MatchBlockGrid::CheckAll()
{
	int Count = 0;

	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE - 2; j++)
		{
			LineCheckHorizon(i, j);

			if (DestroyCount - Count > 1)
			{
				Destroys[DestroyCount] = Blocks[i][j];
				DestroyCount++;
			}
			Count = DestroyCount;

			LineCheckVertical(j, i);

			if (DestroyCount - Count > 1)
			{
				Destroys[DestroyCount] = Blocks[j][i];
				DestroyCount++;
			}
			Count = DestroyCount;
		}
		
		//Barrel Check
		if (Blocks[0][i]->GetColor() == 5)
		{
			Destroys[DestroyCount] = Blocks[0][i];
			DestroyCount++;

			Player->SetObjectiveCount(Player->GetObjectiveCount() - 1);

			if (Player->GetObjectiveCount() <= 0)
			{
				Player->AddScore(Player->GetLeftCount() * 1000.0f);

				Player->SetFinish(true);
			}
		}
	}

	for (int i = 0; i < DESTROY_MAX; i++)
	{
		if (Destroys[i] == nullptr) continue;

		int h = Destroys[i]->GetHorIndex();
		int v = Destroys[i]->GetVerIndex();

		Blocks[h][v] = nullptr;
		Destroys[i]->SetDestroyed(true);;
		Destroys[i] = nullptr;

		Player->AddScore(ComboRate * ScorePoint);
	}
	DestroyCount = 0;
	ComboRate += 0.1f;

	//AfterDestroy();
	//0.5초 후 AfterDestroy() 실행
	GetWorld()->GetTimerManager().SetTimer(DestroyHandle, this, &APuzzle_3MatchBlockGrid::AfterDestroy, 0.25f, false, 0.25f);
}

void APuzzle_3MatchBlockGrid::CheckMatch()
{
	int Count = 0;
	int bCount = 0;
	bool bSpecialMove = false;
	ComboRate = 1.0f;

	//특수 블록 처리
	if (Blocks[aX][aY]->GetColor() == 4)
	{
		int sType = Cast<ASpecialBlock>(Blocks[aX][aY])->GetSpecialType();
		MatchSpecialBlock(sType, aX, aY);
		bSpecialMove = true;

		Count = DestroyCount;
		bCount = DestroyCount;
	}
	if (Blocks[bX][bY]->GetColor() == 4)
	{
		int sType = Cast<ASpecialBlock>(Blocks[bX][bY])->GetSpecialType();
		MatchSpecialBlock(sType, bX, bY);
		bSpecialMove = true;

		Count = DestroyCount;
	}

	//일반 블록 처리
	if (Blocks[aX][aY]->GetColor() < 4)
	{
		//Switching한 대상
		//가로
		CheckHorizontal(aX, aY);

		if (DestroyCount > 2 && !bSpecialMove)
		{
			if(DestroyCount > 3)
				SpawnSpecialBlock(2, aX, aY);
			else
				SpawnSpecialBlock(0, aX, aY);
			Count = DestroyCount;
		}

		//세로
		CheckVertical(aX, aY);

		if (DestroyCount - Count > 2 && !bSpecialMove)
		{
			if (DestroyCount - Count > 3)
				SpawnSpecialBlock(2, aX, aY);
			else
				SpawnSpecialBlock(1, aX, aY);
		}

		if (DestroyCount - Count > 1)
		{
			if (Blocks[aX][aY]->GetColor() < 4)
			{
				Destroys[DestroyCount] = Blocks[aX][aY];
				DestroyCount++;
			}
			Count = DestroyCount;
		}

		bCount = DestroyCount;
	}

	if (Blocks[bX][bY]->GetColor() < 4)
	{
		//Switching할 대상
		//가로
		CheckHorizontal(bX, bY);

		if (DestroyCount - Count > 2 && !bSpecialMove)
		{
			if (DestroyCount - Count > 3)
				SpawnSpecialBlock(2, bX, bY);
			else
				SpawnSpecialBlock(0, bX, bY);
			Count = DestroyCount;
		}

		//세로
		CheckVertical(bX, bY);

		if (DestroyCount - Count > 2 && !bSpecialMove)
		{
			if (DestroyCount - Count > 3)
				SpawnSpecialBlock(2, bX, bY);
			else
				SpawnSpecialBlock(1, bX, bY);
		}

		if (DestroyCount - bCount > 1)
		{
			if (Blocks[bX][bY]->GetColor() < 4)
			{
				Destroys[DestroyCount] = Blocks[bX][bY];
				DestroyCount++;
			}

			Count = DestroyCount;
		}
	}

	GetWorld()->GetTimerManager().SetTimer(DestroyHandle, this, &APuzzle_3MatchBlockGrid::AfterCheck, 0.75f, false, 0.75f);
}

void APuzzle_3MatchBlockGrid::AfterCheck()
{
	for (int i = 0; i < DestroyCount; i++)
	{
		if (Destroys[i] == nullptr) continue;

		int h = Destroys[i]->GetHorIndex();
		int v = Destroys[i]->GetVerIndex();

		if (Blocks[h][v] == Destroys[i])
			Blocks[h][v] = nullptr;
		Destroys[i]->SetDestroyed(true);
		Destroys[i] = nullptr;

		Player->AddScore(ComboRate * ScorePoint);
	}

	//다시 스위치
	if (DestroyCount <= 0)
	{
		if (!bReturn)
		{
			bReturn = true;

			SwitchBlock(aX, aY, bX, bY);
		}
		else
			bReturn = false;
	}
	else
		Player->SetLeftCount(Player->GetLeftCount() - 1);

	DestroyCount = 0;

	//빈 공간 처리
	//AfterDestroy();
	GetWorld()->GetTimerManager().SetTimer(DestroyHandle, this, &APuzzle_3MatchBlockGrid::AfterDestroy, 0.25f, false, 0.25f);
}

void APuzzle_3MatchBlockGrid::CheckHorizontal(int h, int v)
{
	int TargetColor = Blocks[h][v]->GetColor();

	if (v == 0)
	{
		if (Blocks[h][v + 1]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h][v + 1];
			DestroyCount++;

			if (Blocks[h][v + 2]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h][v + 2];
				DestroyCount++;
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
	}
	else if (v < 2)
	{
		if (Blocks[h][v - 1]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h][v - 1];
			DestroyCount++;

			if (Blocks[h][v + 1]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h][v + 1];
				DestroyCount++;

				if (Blocks[h][v + 2]->GetColor() == TargetColor)
				{
					Destroys[DestroyCount] = Blocks[h][v + 2];
					DestroyCount++;
				}
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
		else if (Blocks[h][v + 1]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h][v + 1];
			DestroyCount++;

			if (Blocks[h][v + 2]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h][v + 2];
				DestroyCount++;
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
	}
	else if (v < SIZE - 2)
	{
		if (Blocks[h][v - 2]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h][v - 2];
			DestroyCount++;

			if (Blocks[h][v - 1]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h][v - 1];
				DestroyCount++;

				if (Blocks[h][v + 1]->GetColor() == TargetColor)
				{
					Destroys[DestroyCount] = Blocks[h][v + 1];
					DestroyCount++;

					if (Blocks[h][v + 2]->GetColor() == TargetColor)
					{
						Destroys[DestroyCount] = Blocks[h][v + 2];
						DestroyCount++;
					}
				}
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
		else if (Blocks[h][v - 1]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h][v - 1];
			DestroyCount++;

			if (Blocks[h][v + 1]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h][v + 1];
				DestroyCount++;

				if (Blocks[h][v + 2]->GetColor() == TargetColor)
				{
					Destroys[DestroyCount] = Blocks[h][v + 2];
					DestroyCount++;
				}
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
		else if (Blocks[h][v + 1]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h][v + 1];
			DestroyCount++;

			if (Blocks[h][v + 2]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h][v + 2];
				DestroyCount++;
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
	}
	else if (v < SIZE - 1)
	{
		if (Blocks[h][v + 1]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h][v + 1];
			DestroyCount++;

			if (Blocks[h][v - 1]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h][v - 1];
				DestroyCount++;

				if (Blocks[h][v - 2]->GetColor() == TargetColor)
				{
					Destroys[DestroyCount] = Blocks[h][v - 2];
					DestroyCount++;
				}
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
		else
		{
			if (Blocks[h][v - 1]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h][v - 1];
				DestroyCount++;

				if (Blocks[h][v - 2]->GetColor() == TargetColor)
				{
					Destroys[DestroyCount] = Blocks[h][v - 2];
					DestroyCount++;
				}
				else
				{
					DestroyCount--;
					Destroys[DestroyCount] = nullptr;
				}
			}
		}
	}
	else if (v == SIZE - 1)
	{
		if (Blocks[h][v - 1]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h][v - 1];
			DestroyCount++;

			if (Blocks[h][v - 2]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h][v - 2];
				DestroyCount++;
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
	}
}

void APuzzle_3MatchBlockGrid::CheckVertical(int h, int v)
{
	int TargetColor = Blocks[h][v]->GetColor();

	if (h == 0)
	{
		if (Blocks[h + 1][v]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h + 1][v];
			DestroyCount++;

			if (Blocks[h + 2][v]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h + 2][v];
				DestroyCount++;
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
	}
	else if (h < 2)
	{
		if (Blocks[h - 1][v]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h - 1][v];
			DestroyCount++;

			if (Blocks[h + 1][v]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h + 1][v];
				DestroyCount++;

				if (Blocks[h + 2][v]->GetColor() == TargetColor)
				{
					Destroys[DestroyCount] = Blocks[h + 2][v];
					DestroyCount++;
				}
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
		else if (Blocks[h + 1][v]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h + 1][v];
			DestroyCount++;

			if (Blocks[h + 2][v]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h + 2][v];
				DestroyCount++;
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
	}
	else if (h < SIZE - 2)
	{
		if (Blocks[h - 2][v]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h - 2][v];
			DestroyCount++;

			if (Blocks[h - 1][v]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h - 1][v];
				DestroyCount++;

				if (Blocks[h + 1][v]->GetColor() == TargetColor)
				{
					Destroys[DestroyCount] = Blocks[h + 1][v];
					DestroyCount++;

					if (Blocks[h + 2][v]->GetColor() == TargetColor)
					{
						Destroys[DestroyCount] = Blocks[h + 2][v];
						DestroyCount++;
					}
				}
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
		else if (Blocks[h - 1][v]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h - 1][v];
			DestroyCount++;

			if (Blocks[h + 1][v]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h + 1][v];
				DestroyCount++;

				if (Blocks[h + 2][v]->GetColor() == TargetColor)
				{
					Destroys[DestroyCount] = Blocks[h + 2][v];
					DestroyCount++;
				}
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
		else if (Blocks[h + 1][v]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h + 1][v];
			DestroyCount++;

			if (Blocks[h + 2][v]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h + 2][v];
				DestroyCount++;
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
	}
	else if (h < SIZE - 1)
	{
		if (Blocks[h + 1][v]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h + 1][v];
			DestroyCount++;

			if (Blocks[h - 1][v]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h - 1][v];
				DestroyCount++;

				if (Blocks[h - 2][v]->GetColor() == TargetColor)
				{
					Destroys[DestroyCount] = Blocks[h - 2][v];
					DestroyCount++;
				}
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
		else
		{
			if (Blocks[h - 1][v]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h - 1][v];
				DestroyCount++;

				if (Blocks[h - 2][v]->GetColor() == TargetColor)
				{
					Destroys[DestroyCount] = Blocks[h - 2][v];
					DestroyCount++;
				}
				else
				{
					DestroyCount--;
					Destroys[DestroyCount] = nullptr;
				}
			}
		}
	}
	else if (h == SIZE - 1)
	{
		if (Blocks[h - 1][v]->GetColor() == TargetColor)
		{
			Destroys[DestroyCount] = Blocks[h - 1][v];
			DestroyCount++;

			if (Blocks[h - 2][v]->GetColor() == TargetColor)
			{
				Destroys[DestroyCount] = Blocks[h - 2][v];
				DestroyCount++;
			}
			else
			{
				DestroyCount--;
				Destroys[DestroyCount] = nullptr;
			}
		}
	}
}

void APuzzle_3MatchBlockGrid::MatchSpecialBlock(int Type, int h, int v)
{
	SpecialBlockType = Type;

	switch (Type)
	{
		case 0:
		{
			for (int i = 0; i < SIZE; i++)
			{
				if (Blocks[h][i]->GetColor() < 4)
				{
					Destroys[DestroyCount] = Blocks[h][i];
					DestroyCount++;
				}
				else if (Blocks[h][i]->GetColor() == 4)
				{
					if (i == v)
					{
						Destroys[DestroyCount] = Blocks[h][v];
						DestroyCount++;
					}
					else
					{
						int sType = Cast<ASpecialBlock>(Blocks[h][i])->GetSpecialType();

						if(Type != sType)
							MatchSpecialBlock(sType, h, i);
					}
				}
			}

			PSCBeam[0] = UGameplayStatics::SpawnEmitterAttached(BeamSystem[0], Blocks[h][0]->GetRootComponent());
			StartLoc[0] = Blocks[h][0]->GetActorLocation();
			StartLoc[0] = FVector(StartLoc[0].X, StartLoc[0].Y - 50.0f, StartLoc[0].Z + 200.0f);
			EndLoc[0] = Blocks[h][SIZE - 1]->GetActorLocation();
			EndLoc[0] = FVector(EndLoc[0].X, EndLoc[0].Y + 50.0f, EndLoc[0].Z + 200.0f);

			PSCBeam[0]->SetBeamSourcePoint(0, StartLoc[0], 0);
			PSCBeam[0]->SetBeamTargetPoint(0, EndLoc[0], 0);
		}
		break;

		case 1:
		{
			for (int i = 0; i < SIZE; i++)
			{
				if (Blocks[i][v]->GetColor() < 4)
				{
					Destroys[DestroyCount] = Blocks[i][v];
					DestroyCount++;
				}
				else if (Blocks[i][v]->GetColor() == 4)
				{
					if (i == h)
					{
						Destroys[DestroyCount] = Blocks[h][v];
						DestroyCount++;
					}
					else
					{
						int sType = Cast<ASpecialBlock>(Blocks[i][v])->GetSpecialType();

						if (Type != sType)
							MatchSpecialBlock(sType, i, v);
					}
				}
			}

			PSCBeam[0] = UGameplayStatics::SpawnEmitterAttached(BeamSystem[1], Blocks[0][v]->GetRootComponent());
			StartLoc[0] = Blocks[0][v]->GetActorLocation();
			StartLoc[0] = FVector(StartLoc[0].X - 50.0f, StartLoc[0].Y, StartLoc[0].Z + 200.0f);
			EndLoc[0] = Blocks[SIZE - 1][v]->GetActorLocation();
			EndLoc[0] = FVector(EndLoc[0].X + 50.0f, EndLoc[0].Y, EndLoc[0].Z + 200.0f);

			PSCBeam[0]->SetBeamSourcePoint(0, StartLoc[0], 0);
			PSCBeam[0]->SetBeamTargetPoint(0, EndLoc[0], 0);
		}
		break;

		case 2:
		{
			for (int i = 0; i < SIZE; i++)
			{
				if (Blocks[h][i] == Blocks[h][v] || Blocks[i][v] == Blocks[h][v]) continue;

				if (Blocks[h][i]->GetColor() < 4)
				{
					Destroys[DestroyCount] = Blocks[h][i];
					DestroyCount++;
				}
				else if (Blocks[h][i]->GetColor() == 4)
				{
					int sType = Cast<ASpecialBlock>(Blocks[h][i])->GetSpecialType();
					MatchSpecialBlock(sType, h, i);
				}

				if (Blocks[i][v]->GetColor() < 4)
				{
					Destroys[DestroyCount] = Blocks[i][v];
					DestroyCount++;
				}
				else if (Blocks[i][v]->GetColor() == 4)
				{
					int sType = Cast<ASpecialBlock>(Blocks[i][v])->GetSpecialType();
					MatchSpecialBlock(sType, i, v);
				}
			}

			Destroys[DestroyCount] = Blocks[h][v];
			DestroyCount++;

			PSCBeam[0] = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamSystem[0], Blocks[h][v]->GetActorTransform());
			StartLoc[0] = Blocks[h][0]->GetActorLocation();
			StartLoc[0] = FVector(StartLoc[0].X, StartLoc[0].Y - 50.0f, StartLoc[0].Z + 200.0f);
			EndLoc[0] = Blocks[h][SIZE - 1]->GetActorLocation();
			EndLoc[0] = FVector(EndLoc[0].X, EndLoc[0].Y + 50.0f, EndLoc[0].Z + 200.0f);
			
			PSCBeam[0]->SetBeamSourcePoint(0, StartLoc[0], 0);
			PSCBeam[0]->SetBeamTargetPoint(0, EndLoc[0], 0);


			PSCBeam[1] = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamSystem[1], Blocks[h][v]->GetActorTransform());
			StartLoc[1] = Blocks[0][v]->GetActorLocation();
			StartLoc[1] = FVector(StartLoc[1].X - 50.0f, StartLoc[1].Y, StartLoc[1].Z + 200.0f);
			EndLoc[1] = Blocks[SIZE - 1][v]->GetActorLocation();
			EndLoc[1] = FVector(EndLoc[1].X + 50.0f, EndLoc[1].Y, EndLoc[1].Z + 200.0f);
			
			PSCBeam[1]->SetBeamSourcePoint(0, StartLoc[1], 0);
			PSCBeam[1]->SetBeamTargetPoint(0, EndLoc[1], 0);
		}
		break;
	}
}

void APuzzle_3MatchBlockGrid::AfterDestroy()
{
	int x = 0;
	int y = 0;

	SpecialBlockType = -1;

	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
			if (Blocks[i][j] == nullptr)
			{
				x = i;
				y = j;

				int UpCount = 0;
				while (Blocks[x][y] == nullptr)
				{
					x++;
					UpCount++;

					if (x == SIZE)
						break;
				}

				//밑으로 하강
				if (x != SIZE)
				{
					bFalling = true;

					for (x; x < SIZE; x++)
					{
						if (Blocks[x][y] == nullptr)
						{
							UpCount++;
							continue;
						}

						Blocks[x][y]->SetIndex(x - UpCount, y);

						float xLoc = (x - UpCount) * BlockSpacing;
						float yLoc = y * BlockSpacing;
						FVector targetLoc = FVector(xLoc, yLoc, 0.0f) + GetActorLocation();
						TargetLocations[x - UpCount][y] = targetLoc;

						Blocks[x - UpCount][y] = Blocks[x][y];
						Blocks[x - UpCount][y]->SetFall(true);
						Blocks[x][y] = nullptr;
					}
				}

				//빈만큼 추가 생성
				for (int k = 0; k < UpCount; k++)
				{
					float xLoc = (SIZE + k) * BlockSpacing;
					float yLoc = y * BlockSpacing;
					FVector Location = FVector(xLoc, yLoc, 0.0f) + GetActorLocation();
					APuzzle_3MatchBlock* NewBlock = GetWorld()->SpawnActor<APuzzle_3MatchBlock>(BlockClass, Location, FRotator(0, 0, 0));
					TempBlocks[k] = NewBlock;
				}

				//빈 부분 채워넣기
				for (int l = 0; l < TEMPSIZE; l++)
				{
					if (TempBlocks[l] == nullptr) break;

					bFalling = true;

					for (int m = 0; m < SIZE; m++)
					{
						if (Blocks[m][y] != nullptr) continue;

						float xLoc = m * BlockSpacing;
						float yLoc = y * BlockSpacing;
						FVector targetLoc = FVector(xLoc, yLoc, 0.0f) + GetActorLocation();
						TargetLocations[m][y] = targetLoc;
						TempBlocks[l]->SetIndex(m, y);

						Blocks[m][y] = TempBlocks[l];
						Blocks[m][y]->SetFall(true);

						break;
					}
				}
			}
		}
	}
}

void APuzzle_3MatchBlockGrid::BeamTick()
{
	if (SpecialBlockType < 0) return;
	if (PSCBeam[0] == nullptr && PSCBeam[1] == nullptr) return;

	if (SpecialBlockType < 2)
	{
		PSCBeam[0]->SetBeamSourcePoint(0, StartLoc[0], 0);
		PSCBeam[0]->SetBeamTargetPoint(0, EndLoc[0], 1);
		//PSCBeam[0]->SetBeamEndPoint(0, EndLoc[0]);
	}
	else
	{
		PSCBeam[0]->SetBeamSourcePoint(0, StartLoc[0], 0);
		PSCBeam[0]->SetBeamTargetPoint(0, EndLoc[0], 1);

		PSCBeam[1]->SetBeamSourcePoint(1, StartLoc[1], 2);
		PSCBeam[1]->SetBeamTargetPoint(1, EndLoc[1], 3);
	}
}

#undef LOCTEXT_NAMESPACE


/*
for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
			if(Blocks[i][j]->GetColor() < 4)
				CheckHorizontal(i, j);


			if (Blocks[i][j]->GetColor() < 4)
				CheckVertical(i, j);


			if (Blocks[i][j]->GetColor() == 5)
			{
				if (i == 0)
				{
					Destroys[DestroyCount] = Blocks[i][j];
					DestroyCount++;

					Player->SetObjectiveCount(Player->GetObjectiveCount() - 1);

					if (Player->GetObjectiveCount() <= 0)
					{
						Player->AddScore(Player->GetLeftCount() * 1000.0f);

						Player->SetFinish(true);
					}
				}
			}


			for (int k = 0; k < DESTROY_MAX - 1; k++)
			{
				if (Destroys[k] == nullptr) continue;

				for (int l = k + 1; l < DESTROY_MAX; l++)
				{
					if (Destroys[l] == nullptr) break;

					if (Destroys[k] == Destroys[l])
					{
						DestroyCount--;
						Destroys[l] = nullptr;
					}
				}
			}
		}
	}
*/