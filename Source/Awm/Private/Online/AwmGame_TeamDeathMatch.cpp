// Copyright 2015 Mail.Ru Group. All Rights Reserved.

#include "Awm.h"

AAwmGame_TeamDeathMatch::AAwmGame_TeamDeathMatch(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NumTeams = 2;
	bDelayedStart = true;
    bRespawn = false;
    bOneRound = true;
}

void AAwmGame_TeamDeathMatch::PostLogin(APlayerController* NewPlayer)
{
	// Place player on a team before Super (VoIP team based init, findplayerstart, etc)
	AAwmPlayerState* NewPlayerState = CastChecked<AAwmPlayerState>(NewPlayer->PlayerState);
	const int32 TeamNum = ChooseTeam(NewPlayerState);
	NewPlayerState->SetTeamNum(TeamNum);

	Super::PostLogin(NewPlayer);
}

void AAwmGame_TeamDeathMatch::InitGameState()
{
	Super::InitGameState();

	AAwmGameState* const MyGameState = Cast<AAwmGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->NumTeams = NumTeams;
	}
}

bool AAwmGame_TeamDeathMatch::CanDealDamage(AAwmPlayerState* DamageInstigator, class AAwmPlayerState* DamagedPlayer) const
{
	return DamageInstigator && DamagedPlayer && (DamagedPlayer == DamageInstigator || DamagedPlayer->GetTeamNum() != DamageInstigator->GetTeamNum());
}

int32 AAwmGame_TeamDeathMatch::ChooseTeam(AAwmPlayerState* ForPlayerState) const
{
	TArray<int32> TeamBalance;
	TeamBalance.AddZeroed(NumTeams);

	// get current team balance
	for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
	{
		AAwmPlayerState const* const TestPlayerState = Cast<AAwmPlayerState>(GameState->PlayerArray[i]);
		if (TestPlayerState && TestPlayerState != ForPlayerState && TeamBalance.IsValidIndex(TestPlayerState->GetTeamNum()))
		{
			TeamBalance[TestPlayerState->GetTeamNum()]++;
		}
	}

	// find least populated one
	int32 BestTeamScore = TeamBalance[0];
	for (int32 i = 1; i < TeamBalance.Num(); i++)
	{
		if (BestTeamScore > TeamBalance[i])
		{
			BestTeamScore = TeamBalance[i];
		}
	}

	// there could be more than one...
	TArray<int32> BestTeams;
	for (int32 i = 0; i < TeamBalance.Num(); i++)
	{
		if (TeamBalance[i] == BestTeamScore)
		{
			BestTeams.Add(i);
		}
	}

	// get random from best list
	const int32 RandomBestTeam = BestTeams[FMath::RandHelper(BestTeams.Num())];
	return RandomBestTeam;
}

void AAwmGame_TeamDeathMatch::DetermineMatchWinner()
{
    WinnerTeam = CheckWinnerTeam();
}

int32 AAwmGame_TeamDeathMatch::CheckWinnerTeam() {
    
    if ( !bRespawn && OnlyOneTeamIsAlive() )
        return GetMoreLiveTeam();
    
    return -1;
}

bool AAwmGame_TeamDeathMatch::IsWinner(AAwmPlayerState* PlayerState) const
{
	return PlayerState && !PlayerState->IsQuitter() && PlayerState->GetTeamNum() == WinnerTeam;
}

bool AAwmGame_TeamDeathMatch::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	if (Player)
	{
		AAwmTeamStart* TeamStart = Cast<AAwmTeamStart>(SpawnPoint);
		AAwmPlayerState* PlayerState = Cast<AAwmPlayerState>(Player->PlayerState);

		if (PlayerState && TeamStart && TeamStart->SpawnTeam != PlayerState->GetTeamNum())
		{
			return false;
		}
	}

	return Super::IsSpawnpointAllowed(SpawnPoint, Player);
}

void AAwmGame_TeamDeathMatch::InitBot(AAwmAIController* AIC, int32 BotNum)
{	
	AAwmPlayerState* BotPlayerState = CastChecked<AAwmPlayerState>(AIC->PlayerState);
	const int32 TeamNum = ChooseTeam(BotPlayerState);
	BotPlayerState->SetTeamNum(TeamNum);		

	Super::InitBot(AIC, BotNum);
}