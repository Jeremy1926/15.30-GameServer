#include "pch.h"
#include "Misc.h"
#include "Quests.h"
#include "Options.h"

int Misc::GetNetMode() {
	return 1;
}

float Misc::GetMaxTickRate(UEngine* Engine, float DeltaTime, bool bAllowFrameRateSmoothing) {
	// improper, DS is supposed to do hitching differently
	return std::clamp(1.f / DeltaTime, 1.f, 30.f);
}

bool Misc::RetTrue() { return true; }

void Misc::TickFlush(UNetDriver* Driver, float DeltaTime)
{
	if (Driver->ReplicationDriver)
		Funcs::ServerReplicateActors(Driver->ReplicationDriver, DeltaTime);

	static bool startedBus = false;
	if (!startedBus)
	{
		auto Time = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
		if (((AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode)->bWorldIsReady && ((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->WarmupCountdownEndTime <= Time)
		{
			startedBus = true;

			((void (*)(AGameModeBase*, int))(ImageBase + 0x2141480))(UWorld::GetWorld()->AuthorityGameMode, 0);
		}
	}

	return TickFlushOG(Driver, DeltaTime);
}

void* Misc::DispatchRequest(void* Arg1, void* MCPData, int)
{
	return DispatchRequestOG(Arg1, MCPData, 3);
}

bool Misc::Listen(UWorld *World) {
	auto Engine = UEngine::GetEngine();
	auto NetDriverName = FName(L"GameNetDriver");
	auto NetDriver = World->NetDriver = ((UNetDriver * (*)(UEngine*, UWorld*, FName))(ImageBase + Sarah::Offsets::CreateNetDriver))(Engine, World, NetDriverName);

	NetDriver->NetDriverName = NetDriverName;
	NetDriver->World = World;

	for (auto& Collection : World->LevelCollections) Collection.NetDriver = NetDriver;

	FString Err;
	if (Funcs::InitListen(NetDriver, World, World->PersistentLevel->URL, false, Err)) {
		Funcs::SetWorld(NetDriver, World);
	}
	else {
		Log(L"Failed to listen");
	}
	return true;
}

bool Misc::RetFalse()
{
	return false;
}

void Misc::SetDynamicFoundationEnabled(UObject* Context, FFrame& Stack)
{
	auto Foundation = (ABuildingFoundation*)Context;
	bool bEnabled;
	Stack.StepCompiledIn(&bEnabled);
	Stack.IncrementCode();
	Foundation->DynamicFoundationRepData.EnabledState = bEnabled ? EDynamicFoundationEnabledState::Enabled : EDynamicFoundationEnabledState::Disabled;
	Foundation->OnRep_DynamicFoundationRepData();
	Foundation->FoundationEnabledState = bEnabled ? EDynamicFoundationEnabledState::Enabled : EDynamicFoundationEnabledState::Disabled;

	if (bEnabled)
	{
		for (const auto& AdditionalWorld : Foundation->AdditionalWorlds)
		{
			ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), AdditionalWorld, Foundation->K2_GetActorLocation(), Foundation->K2_GetActorRotation(), nullptr, FString());
		}
	}
}

void Misc::SetDynamicFoundationTransform(UObject* Context, FFrame& Stack)
{
	auto Foundation = (ABuildingFoundation*)Context;
	auto& Transform = Stack.StepCompiledInRef<FTransform>();
	Stack.IncrementCode();
	Foundation->DynamicFoundationTransform = Transform;
	Foundation->DynamicFoundationRepData.Rotation = Transform.Rotation.Rotator();
	Foundation->DynamicFoundationRepData.Translation = Transform.Translation;
	Foundation->StreamingData.FoundationLocation = Transform.Translation;
	Foundation->StreamingData.BoundingBox = Foundation->StreamingBoundingBox;
	Foundation->OnRep_DynamicFoundationRepData();
}



void Misc::StartNewSafeZonePhase(AFortGameModeAthena* GameMode, int a2)
{
	auto GameState = (AFortGameStateAthena*)GameMode->GameState;

	FFortSafeZoneDefinition* SafeZoneDefinition = &GameState->MapInfo->SafeZoneDefinition;
	TArray<float>& Durations = *(TArray<float>*)(__int64(SafeZoneDefinition) + 0x258);
	TArray<float>& HoldDurations = *(TArray<float>*)(__int64(SafeZoneDefinition) + 0x248);

	constexpr static std::array<float, 8> LateGameDurations{
		0.f,
		65.f,
		60.f,
		50.f,
		45.f,
		35.f,
		30.f,
		40.f,
	};

	constexpr static std::array<float, 8> LateGameHoldDurations{
		0.f,
		60.f,
		55.f,
		50.f,
		45.f,
		30.f,
		0.f,
		0.f,
	};

	auto DurationSum = 0.f;
	for (auto& _v : Durations) DurationSum += _v;
	if (DurationSum == 0)
	{
		auto GameData = GameState->CurrentPlaylistInfo.BasePlaylist->GameData.Get();

		if (!GameData)
			GameData = Utils::FindObject<UCurveTable>(L"/Game/Balance/AthenaGameData.AthenaGameData");

		auto ShrinkTime = FName(L"Default.SafeZone.ShrinkTime");
		auto HoldTime = FName(L"Default.SafeZone.WaitTime");

		for (int i = 0; i < Durations.Num(); i++)
		{
			UDataTableFunctionLibrary::EvaluateCurveTableRow(GameData, ShrinkTime, (float) i, nullptr, &Durations[i], FString());
		}
		for (int i = 0; i < HoldDurations.Num(); i++)
		{
			UDataTableFunctionLibrary::EvaluateCurveTableRow(GameData, HoldTime, (float) i, nullptr, &HoldDurations[i], FString());
		}
	}


	if (bLateGame && GameMode->SafeZonePhase < 3)
	{
		GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
		GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime + 0.05f;
	}
	else if (bLateGame && GameMode->SafeZonePhase == 3)
	{
		auto Duration = bLateGame ? LateGameDurations[GameMode->SafeZonePhase - 1] : Durations[GameMode->SafeZonePhase + 1];

		GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + 30.f;
		GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime + Duration;
	}
	else
	{
		auto Duration = bLateGame ? LateGameDurations[GameMode->SafeZonePhase - 1] : Durations[GameMode->SafeZonePhase + 1];
		auto HoldDuration = bLateGame ? LateGameHoldDurations[GameMode->SafeZonePhase - 1] : HoldDurations[GameMode->SafeZonePhase + 1];

		GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + HoldDuration;
		GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime + Duration;
	}

	for (auto& Player : GameMode->AlivePlayers)
	{
		auto QuestManager = Player->GetQuestManager(ESubGame::Athena);
		static auto SearchDef = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeID_SurviveStormCircle.AccoladeID_SurviveStormCircle");
		GiveAccolade((AFortPlayerControllerAthena*)QuestManager->GetPlayerControllerBP(), SearchDef, nullptr, EXPEventPriorityType::NearReticle);
	}

	StartNewSafeZonePhaseOG(GameMode, a2);
}

bool Misc::StartAircraftPhase(AFortGameModeAthena* GameMode, char a2)
{
	auto Ret = StartAircraftPhaseOG(GameMode, a2);

	if (bLateGame)
	{
		auto GameState = (AFortGameStateAthena*)GameMode->GameState;

		auto Aircraft = GameState->Aircrafts[0];
		Aircraft->FlightInfo.FlightSpeed = 0.f;
		FVector Loc{ 0, 0, 0 };
		while (Loc.X == 0 && Loc.Y == 0 && Loc.Z == 0)
			Loc = GameMode->SafeZoneLocations[rand() % (GameMode->SafeZoneLocations.Num() - 1)];
		Loc.Z = 17500.f;

		Aircraft->FlightInfo.FlightStartLocation = (FVector_NetQuantize100) Loc;
		
		Aircraft->FlightInfo.TimeTillFlightEnd = 7.f;
		Aircraft->FlightInfo.TimeTillDropEnd = 0.f;
		Aircraft->FlightInfo.TimeTillDropStart = 0.f;
		Aircraft->FlightStartTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
		Aircraft->FlightEndTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + 7.f;
		//GameState->bAircraftIsLocked = false;
		GameState->SafeZonesStartTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + 7.f;
	}

	return Ret;
}

const wchar_t* Misc::GetCommandLet()
{
	static auto cmdLine = UEAllocatedWString(GetCommandLetOG()) + L" -AllowAllPlaylistsInShipping";
	return cmdLine.c_str();
}

FServicePermissionsMcp* Misc::MatchMakingServicePerms(int64, int64)
{
	FServicePermissionsMcp* Perms = new FServicePermissionsMcp();
	Perms->Id = L"ec684b8c687f479fadea3cb2ad83f5c6";
	return Perms;
}

void Misc::ServerMove_PerformMovement(UCharacterMovementComponent* Comp, FCharacterNetworkMoveData& MoveData)
{
	if (MoveData.TimeStamp >= 0.f && MoveData.TimeStamp < 1.f)
	{
		// running w/o anticheat
		auto PlayerController = Comp->CharacterOwner->Controller->Cast<APlayerController>();
		PlayerController->ClientReturnToMainMenu(L"Update required. Please restart your game.");
		return;
	}
	else if (MoveData.TimeStamp < 0)
		MoveData.TimeStamp += 2.f;

	ServerMove_PerformMovementOG(Comp, MoveData);
}

void Misc::Hook() {
	Utils::Patch<uint8>(ImageBase + 0x2a1c2a5, 0x85);
	Utils::Patch<uint32>(ImageBase + 0x2a320d8, 0x90909090);
	Utils::Patch<uint32>(ImageBase + 0x2a32c3f, 0x90909090);
	Utils::Patch<uint16>(ImageBase + 0x2a32c43, 0x9090);
	Utils::Patch<uint16>(ImageBase + 0x2a320dc, 0x9090);
	Utils::Hook(ImageBase + 0x532d3c0, Listen);
	Utils::Hook(ImageBase + Sarah::Offsets::GetNetMode, GetNetMode);
	Utils::Hook(ImageBase + Sarah::Offsets::GetMaxTickRate, GetMaxTickRate, GetMaxTickRateOG);
	Utils::Hook(ImageBase + Sarah::Offsets::TickFlush, TickFlush, TickFlushOG);
	Utils::Hook(ImageBase + Sarah::Offsets::DispatchRequest, DispatchRequest, DispatchRequestOG);
	Utils::ExecHook(L"/Script/FortniteGame.BuildingFoundation.SetDynamicFoundationTransform", SetDynamicFoundationTransform);
	Utils::ExecHook(L"/Script/FortniteGame.BuildingFoundation.SetDynamicFoundationEnabled", SetDynamicFoundationEnabled);
	Utils::Hook(ImageBase + 0x2144dd0, StartNewSafeZonePhase, StartNewSafeZonePhaseOG);
	Utils::Patch<uint8_t>(ImageBase + EncryptionPatch, 0x74);
	Utils::Patch<uint8_t>(ImageBase + GameSessionPatch, 0x85);
	for (auto& NullFunc : Sarah::Offsets::NullFuncs)
		Utils::Patch<uint8_t>(ImageBase + NullFunc, 0xc3);
	for (auto& RetTrueFunc : Sarah::Offsets::RetTrueFuncs) {
		Utils::Patch<uint32_t>(ImageBase + RetTrueFunc, 0xc0ffc031);
		Utils::Patch<uint8_t>(ImageBase + RetTrueFunc + 4, 0xc3);
	}
	Utils::Patch<uint32_t>(ImageBase + 0x52978e8, 0x95ad4);
	Utils::Hook(ImageBase + 0x2141480, StartAircraftPhase, StartAircraftPhaseOG);
	Utils::Hook(ImageBase + 0x361f690, GetCommandLet, GetCommandLetOG);
	Utils::Hook(ImageBase + 0x15003e0, MatchMakingServicePerms);

	if (bAntiCheat)
	{
		Utils::Hook(ImageBase + 0x4d35970, ServerMove_PerformMovement, ServerMove_PerformMovementOG);
	}
}
