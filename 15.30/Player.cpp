#include "pch.h"
#include "Player.h"
#include "Abilities.h"
#include "Inventory.h"
#include "Options.h"
#include "Lategame.h"
#include "Quests.h"
#include "backend.h"

void Player::ServerReadyToStartMatch(UObject* Context, FFrame& Stack)
{
	Stack.IncrementCode();
	auto PlayerController = (AFortPlayerController*)Context;

	callOG(PlayerController, Stack.CurrentNativeFunction, ServerReadyToStartMatch);
}

void Player::ServerAcknowledgePossession(UObject* Context, FFrame& Stack)
{
	APawn* Pawn;
	Stack.StepCompiledIn(&Pawn);
	Stack.IncrementCode();
	auto PlayerController = (AFortPlayerController*)Context;
	PlayerController->AcknowledgedPawn = Pawn;
}

void Player::GetPlayerViewPoint(APlayerController* PlayerController, FVector& Loc, FRotator& Rot)
{
	static auto SFName = FName(L"Spectating");
	if (PlayerController->StateName == SFName)
	{
		Loc = PlayerController->LastSpectatorSyncLocation;
		Rot = PlayerController->LastSpectatorSyncRotation;
	}
	else if (PlayerController->GetViewTarget())
	{
		Loc = PlayerController->GetViewTarget()->K2_GetActorLocation();
		Rot = PlayerController->GetControlRotation();
	}
	else return GetPlayerViewPointOG(PlayerController, Loc, Rot);
}

void Player::ServerExecuteInventoryItem(UObject* Context, FFrame& Stack)
{
	FGuid ItemGuid;
	Stack.StepCompiledIn(&ItemGuid);
	Stack.IncrementCode();
	auto PlayerController = (AFortPlayerController*)Context;
	if (!PlayerController) return;
	auto entry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry) {
		return entry.ItemGuid == ItemGuid;
		});

	if (!entry || !PlayerController->MyFortPawn) return;
	UFortWeaponItemDefinition* ItemDefinition = entry->ItemDefinition->IsA<UFortGadgetItemDefinition>() ? ((UFortGadgetItemDefinition*)entry->ItemDefinition)->GetWeaponItemDefinition() : (UFortWeaponItemDefinition*)entry->ItemDefinition;
	
	if (auto Deco = (UFortContextTrapItemDefinition*)ItemDefinition->Cast<UFortDecoItemDefinition>()) {
		PlayerController->MyFortPawn->PickUpActor(nullptr, Deco);
		PlayerController->MyFortPawn->CurrentWeapon->ItemEntryGuid = ItemGuid;

		if (auto ContextTrap = PlayerController->MyFortPawn->CurrentWeapon->Cast<AFortDecoTool_ContextTrap>()) ContextTrap->ContextTrapItemDefinition = Deco;
		return;
	}
	PlayerController->MyFortPawn->EquipWeaponDefinition(ItemDefinition, ItemGuid, entry->TrackerGuid);
}

void Player::ServerReturnToMainMenu(UObject* Context, FFrame& Stack)
{
	Stack.IncrementCode();
	return ((AFortPlayerController*)Context)->ClientReturnToMainMenu(L"");
}

void Player::ServerAttemptAircraftJump(UObject* Context, FFrame& Stack)
{
    FRotator Rotation;
    Stack.StepCompiledIn(&Rotation);
    Stack.IncrementCode();

    auto Component = (UFortControllerComponent_Aircraft*)Context;
    auto PlayerController = (AFortPlayerController*)Component->GetOwner();
    auto PlayerState = (AFortPlayerState*)PlayerController->PlayerState;
    auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
    auto GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

		GameMode->RestartPlayer(PlayerController);
		if (bLateGame && PlayerController->MyFortPawn)
		{
			FVector AircraftLocation = GameState->Aircrafts[0]->K2_GetActorLocation();

			float Angle = UKismetMathLibrary::RandomFloatInRange(0.0f, 6.2831853f);
			float Radius = (float)(rand() % 1000);

			float OffsetX = UKismetMathLibrary::Cos(Angle) * Radius;
			float OffsetY = UKismetMathLibrary::Sin(Angle) * Radius;

			FVector Offset;
			Offset.X = OffsetX;
			Offset.Y = OffsetY;
			Offset.Z = 0.0f;

			FVector NewLoc = AircraftLocation + Offset;

			PlayerController->MyFortPawn->K2_SetActorLocation(NewLoc, false, nullptr, false);
		}

		PlayerController->ClientSetRotation(Rotation, true);
    if (PlayerController->MyFortPawn)
    {
        PlayerController->MyFortPawn->BeginSkydiving(true);
        PlayerController->MyFortPawn->SetHealth(100);

        if (bLateGame)
        {
            PlayerController->MyFortPawn->SetShield(100);

            auto Shotgun = Lategame::GetShotguns();
            auto AssaultRifle = Lategame::GetAssaultRifles();
            auto Sniper = Lategame::GetSnipers();
            auto Heal = Lategame::GetExtra();
            auto HealSlot2 = Lategame::GetHeals();

            int ShotgunClipSize = Inventory::GetStats((UFortWeaponItemDefinition*)Shotgun.Item)->ClipSize;
            int AssaultRifleClipSize = Inventory::GetStats((UFortWeaponItemDefinition*)AssaultRifle.Item)->ClipSize;
            int SniperClipSize = Inventory::GetStats((UFortWeaponItemDefinition*)Sniper.Item)->ClipSize;
            // for grappler
            int HealClipSize = Heal.Item->IsA<UFortWeaponItemDefinition>() ? Inventory::GetStats((UFortWeaponItemDefinition*)Heal.Item)->ClipSize : 0;
            int HealSlot2ClipSize = HealSlot2.Item->IsA<UFortWeaponItemDefinition>() ? Inventory::GetStats((UFortWeaponItemDefinition*)HealSlot2.Item)->ClipSize : 0;

            Inventory::GiveItem(PlayerController, Lategame::GetResource(EFortResourceType::Wood), 500);
            Inventory::GiveItem(PlayerController, Lategame::GetResource(EFortResourceType::Stone), 500);
            Inventory::GiveItem(PlayerController, Lategame::GetResource(EFortResourceType::Metal), 500);

            Inventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Assault), 250);
            Inventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Shotgun), 50);
            Inventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Submachine), 400);
            Inventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Rocket), 6);
            Inventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Sniper), 20);

            Inventory::GiveItem(PlayerController, AssaultRifle.Item, AssaultRifle.Count, AssaultRifleClipSize, true);
            Inventory::GiveItem(PlayerController, Shotgun.Item, Shotgun.Count, ShotgunClipSize, true);
            Inventory::GiveItem(PlayerController, Sniper.Item, Sniper.Count, SniperClipSize, true);
            Inventory::GiveItem(PlayerController, Heal.Item, Heal.Count, HealClipSize, true);
            Inventory::GiveItem(PlayerController, HealSlot2.Item, HealSlot2.Count, HealSlot2ClipSize, true);
        }
    }

    //Inventory::GiveItem(PlayerController, Utils::FindObject<UFortItemDefinition>(L"/CampsiteGameplay/Items/Campsite/WID_Athena_DeployableCampsite_Thrown.WID_Athena_DeployableCampsite_Thrown"), 5);
}

void Player::ServerPlayEmoteItem(UObject* Context, FFrame& Stack)
{
	UFortMontageItemDefinitionBase* Asset;
	float RandomNumber;
	Stack.StepCompiledIn(&Asset);
	Stack.StepCompiledIn(&RandomNumber);
	Stack.IncrementCode();
	auto PlayerController = (AFortPlayerController*)Context;

	if (!PlayerController || !PlayerController->MyFortPawn || !Asset) return;

	auto* AbilitySystemComponent = ((AFortPlayerStateAthena*)PlayerController->PlayerState)->AbilitySystemComponent;
	FGameplayAbilitySpec NewSpec = {};
	UObject* AbilityToUse = nullptr;

	if (Asset->IsA<UAthenaSprayItemDefinition>()) {
		static auto SprayAbilityClass = Utils::FindObject<UBlueprintGeneratedClass>(L"/Game/Abilities/Sprays/GAB_Spray_Generic.GAB_Spray_Generic_C");
		AbilityToUse = SprayAbilityClass->DefaultObject;
	}
	else if (auto ToyAsset = Asset->Cast<UAthenaToyItemDefinition>()) {
		AbilityToUse = ToyAsset->ToySpawnAbility->DefaultObject;
	}
	else if (auto DanceAsset = Asset->Cast<UAthenaDanceItemDefinition>())
	{
		PlayerController->MyFortPawn->bMovingEmote = DanceAsset->bMovingEmote;
		PlayerController->MyFortPawn->EmoteWalkSpeed = DanceAsset->WalkForwardSpeed;
		static auto EmoteAbilityClass = Utils::FindObject<UBlueprintGeneratedClass>(L"/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C");
		AbilityToUse = DanceAsset->CustomDanceAbility ? DanceAsset->CustomDanceAbility->DefaultObject : EmoteAbilityClass->DefaultObject;
	}

	if (AbilityToUse) {
		((void (*)(FGameplayAbilitySpec*, UObject*, int, int, UObject*))(ImageBase + 0xa4c210))(&NewSpec, AbilityToUse, 1, -1, Asset);
		FGameplayAbilitySpecHandle handle;
		((void (*)(UFortAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec*, void*))(ImageBase + 0xa6fc20))(AbilitySystemComponent, &handle, &NewSpec, nullptr);
	}
}

void Player::ServerSendZiplineState(UObject* Context, FFrame& Stack)
{
	FZiplinePawnState State;

	Stack.StepCompiledIn(&State);
	Stack.IncrementCode();

	auto Pawn = (AFortPlayerPawn*)Context;

	if (!Pawn)
		return;

	Pawn->ZiplineState = State;

	((void (*)(AFortPlayerPawn*))(ImageBase + 0x672c300))(Pawn);

	if (State.bJumped)
	{
		auto Velocity = Pawn->CharacterMovement->Velocity;
		auto VelocityX = Velocity.X * -0.5f;
		auto VelocityY = Velocity.Y * -0.5f;
		Pawn->LaunchCharacterJump({ VelocityX >= -750 ? min(VelocityX, 750) : -750, VelocityY >= -750 ? min(VelocityY, 750) : -750, 1200 }, false, false, true, true);
	}
}

void Player::ServerHandlePickupInfo(UObject* Context, FFrame& Stack)
{
	AFortPickup* Pickup;
	FFortPickupRequestInfo Params;
	Stack.StepCompiledIn(&Pickup);
	Stack.StepCompiledIn(&Params);
	Stack.IncrementCode();
	auto Pawn = (AFortPlayerPawn*)Context;

	if (!Pawn || !Pickup || Pickup->bPickedUp)
		return;

	if ((Params.bTrySwapWithWeapon || Params.bUseRequestedSwap) && Pawn->CurrentWeapon && Inventory::GetQuickbar(Pawn->CurrentWeapon->WeaponData) == EFortQuickBars::Primary && Inventory::GetQuickbar(Pickup->PrimaryPickupItemEntry.ItemDefinition) == EFortQuickBars::Primary)
	{
		auto PC = (AFortPlayerControllerAthena*)Pawn->Controller;
		auto SwapEntry = PC->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry)
			{ return entry.ItemGuid == Params.SwapWithItem; });
		PC->SwappingItemDefinition = (UFortWorldItemDefinition*)SwapEntry; // proper af
	}
	Pawn->IncomingPickups.Add(Pickup);

	Pickup->PickupLocationData.bPlayPickupSound = Params.bPlayPickupSound;
	Pickup->PickupLocationData.FlyTime = 0.4f;
	Pickup->PickupLocationData.ItemOwner = Pawn;
	Pickup->PickupLocationData.PickupGuid = Pickup->PrimaryPickupItemEntry.ItemGuid;
	Pickup->PickupLocationData.PickupTarget = Pawn;
	//Pickup->PickupLocationData.StartDirection = Params.Direction.QuantizeNormal();
	Pickup->OnRep_PickupLocationData();

	Pickup->bPickedUp = true;
	Pickup->OnRep_bPickedUp();
}

void Player::MovingEmoteStopped(UObject* Context, FFrame& Stack)
{
	Stack.IncrementCode();

	AFortPawn* Pawn = (AFortPawn*)Context;
	Pawn->bMovingEmote = false;
	Pawn->bMovingEmoteFollowingOnly = false;
}

void Player::InternalPickup(AFortPlayerControllerAthena* PlayerController, FFortItemEntry PickupEntry)
{
	auto MaxStack = (int32)Utils::EvaluateScalableFloat(PickupEntry.ItemDefinition->MaxStackSize);
	int ItemCount = 0;
	for (auto& Item : PlayerController->WorldInventory->Inventory.ReplicatedEntries)
	{
		if (Inventory::GetQuickbar(Item.ItemDefinition) == EFortQuickBars::Primary)
			ItemCount += ((UFortWorldItemDefinition*)Item.ItemDefinition)->NumberOfSlotsToTake;
	}
	auto GiveOrSwap = [&]() {
		if (ItemCount == 5 && Inventory::GetQuickbar(PickupEntry.ItemDefinition) == EFortQuickBars::Primary) {
			if (Inventory::GetQuickbar(PlayerController->MyFortPawn->CurrentWeapon->WeaponData) == EFortQuickBars::Primary) {
				auto itemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([PlayerController](FFortItemEntry& entry)
					{ return entry.ItemGuid == PlayerController->MyFortPawn->CurrentWeapon->ItemEntryGuid; });
				Inventory::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), *itemEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PlayerController->MyFortPawn);
				Inventory::Remove(PlayerController, PlayerController->MyFortPawn->CurrentWeapon->ItemEntryGuid);
				Inventory::GiveItem(PlayerController, PickupEntry, PickupEntry.Count, true);
			}
			else {
				Inventory::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), (FFortItemEntry&)PickupEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PlayerController->MyFortPawn);
			}
		}
		else
			Inventory::GiveItem(PlayerController, PickupEntry, PickupEntry.Count, true);
		};
	auto GiveOrSwapStack = [&](int32 OriginalCount) {
		if (PickupEntry.ItemDefinition->bAllowMultipleStacks && ItemCount < 5)
			Inventory::GiveItem(PlayerController, PickupEntry, OriginalCount - MaxStack, true);
		else
			Inventory::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), (FFortItemEntry&)PickupEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PlayerController->MyFortPawn, OriginalCount - MaxStack);
		};
	if (PickupEntry.ItemDefinition->IsStackable()) {
		auto itemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([PickupEntry, MaxStack](FFortItemEntry& entry)
			{ return entry.ItemDefinition == PickupEntry.ItemDefinition && entry.Count < MaxStack; });
		if (itemEntry) {
			auto State = itemEntry->StateValues.Search([](FFortItemEntryStateValue& Value)
				{ return Value.StateType == EFortItemEntryState::ShouldShowItemToast; });
			if (!State) {
				FFortItemEntryStateValue Value{};
				Value.StateType = EFortItemEntryState::ShouldShowItemToast;
				Value.IntValue = true;
				itemEntry->StateValues.Add(Value);
			}
			else State->IntValue = true;

			if ((itemEntry->Count += PickupEntry.Count) > MaxStack) {
				auto OriginalCount = itemEntry->Count;
				itemEntry->Count = MaxStack;

				GiveOrSwapStack(OriginalCount);
			}
			Inventory::ReplaceEntry(PlayerController, *itemEntry);
		}
		else {
			if (PickupEntry.Count > MaxStack) {
				auto OriginalCount = PickupEntry.Count;
				PickupEntry.Count = MaxStack;

				GiveOrSwapStack(OriginalCount);
			}
			GiveOrSwap();
		}
	}
	else {
		GiveOrSwap();
	}
}

bool Player::CompletePickupAnimation(AFortPickup* Pickup) {
	auto Pawn = (AFortPlayerPawnAthena*)Pickup->PickupLocationData.PickupTarget;
	if (!Pawn)
		return CompletePickupAnimationOG(Pickup);
	auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;
	if (!PlayerController)
		return CompletePickupAnimationOG(Pickup);
	if (auto entry = (FFortItemEntry*)PlayerController->SwappingItemDefinition)
	{
		Inventory::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), *entry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PlayerController->MyFortPawn);
		// SwapEntry(PC, *entry, Pickup->PrimaryPickupItemEntry);
		Inventory::Remove(PlayerController, entry->ItemGuid);
		Inventory::GiveItem(PlayerController, Pickup->PrimaryPickupItemEntry);
		PlayerController->SwappingItemDefinition = nullptr;
	}
	else
	{
		InternalPickup(PlayerController, Pickup->PrimaryPickupItemEntry);
	}
	return CompletePickupAnimationOG(Pickup);
}

void Player::NetMulticast_Athena_BatchedDamageCues(AFortPlayerPawnAthena* Pawn, FAthenaBatchedDamageGameplayCues_Shared SharedData, FAthenaBatchedDamageGameplayCues_NonShared NonSharedData)
{
	if (!Pawn || !Pawn->Controller || !Pawn->CurrentWeapon) return;

	if (Pawn->CurrentWeapon && !Pawn->CurrentWeapon->WeaponData->bUsesPhantomReserveAmmo && Inventory::GetStats(Pawn->CurrentWeapon->WeaponData) && Inventory::GetStats(Pawn->CurrentWeapon->WeaponData)->ClipSize > 0)
	{
		auto ent = ((AFortPlayerControllerAthena*)Pawn->Controller)->WorldInventory->Inventory.ReplicatedEntries.Search([Pawn](FFortItemEntry& entry)
			{ return entry.ItemGuid == Pawn->CurrentWeapon->ItemEntryGuid; });
		if (ent)
		{
			ent->LoadedAmmo = Pawn->CurrentWeapon->AmmoCount;
			Inventory::ReplaceEntry((AFortPlayerControllerAthena*)Pawn->Controller, *ent);
		}
	}
	else if (Pawn->CurrentWeapon && Pawn->CurrentWeapon->WeaponData->bUsesPhantomReserveAmmo)
	{
		auto ent = ((AFortPlayerControllerAthena*)Pawn->Controller)->WorldInventory->Inventory.ReplicatedEntries.Search([Pawn](FFortItemEntry& entry)
			{ return entry.ItemGuid == Pawn->CurrentWeapon->ItemEntryGuid; });
		if (ent)
		{
			ent->LoadedAmmo = Pawn->CurrentWeapon->AmmoCount;
			Inventory::ReplaceEntry((AFortPlayerControllerAthena*)Pawn->Controller, *ent);
		}
	}

	return NetMulticast_Athena_BatchedDamageCuesOG(Pawn, SharedData, NonSharedData);
}

void Player::ReloadWeapon(AFortWeapon* Weapon, int AmmoToRemove)
{
	AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)((AFortPlayerPawnAthena*)Weapon->Owner)->Controller;
	AFortInventory* Inventory;
	if (auto Bot = PC->Cast<AFortAthenaAIBotController>())
	{
		Inventory = Bot->Inventory;
	}
	else
	{
		Inventory = PC->WorldInventory;
	}
	if (!PC || !Inventory || !Weapon)
		return;
	if (Weapon->WeaponData->bUsesPhantomReserveAmmo)
	{
		Weapon->PhantomReserveAmmo -= AmmoToRemove;
		Weapon->OnRep_PhantomReserveAmmo();
		return;
	}
	auto Ammo = Weapon->WeaponData->GetAmmoWorldItemDefinition_BP();
	auto ent = Inventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry)
		{ return Weapon->WeaponData == Ammo ? entry.ItemGuid == Weapon->ItemEntryGuid : entry.ItemDefinition == Ammo; });
	auto WeaponEnt = Inventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry)
		{ return entry.ItemGuid == Weapon->ItemEntryGuid; });
	if (!WeaponEnt)
		return;

	if (ent)
	{
		ent->Count -= AmmoToRemove;
		if (ent->Count <= 0)
			Inventory::Remove(PC, ent->ItemGuid);
		else
			Inventory::ReplaceEntry(PC, *ent);
	}
	WeaponEnt->LoadedAmmo += AmmoToRemove;
	Inventory::ReplaceEntry(PC, *WeaponEnt);
}


void Player::ClientOnPawnDied(AFortPlayerControllerAthena* PlayerController, FFortPlayerDeathReport& DeathReport)
{
	if (!PlayerController)
		return ClientOnPawnDiedOG(PlayerController, DeathReport);
	auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
	auto GameState = (AFortGameStateAthena*)GameMode->GameState;
	auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;


	if (!GameState->IsRespawningAllowed(PlayerState) && PlayerController->WorldInventory && PlayerController->MyFortPawn)
	{
		bool bHasMats = false;
		for (auto& entry : PlayerController->WorldInventory->Inventory.ReplicatedEntries)
		{
			if (!entry.ItemDefinition->IsA<UFortWeaponMeleeItemDefinition>() && (entry.ItemDefinition->IsA<UFortResourceItemDefinition>() || entry.ItemDefinition->IsA<UFortWeaponRangedItemDefinition>() || entry.ItemDefinition->IsA<UFortConsumableItemDefinition>() || entry.ItemDefinition->IsA<UFortAmmoItemDefinition>()))
			{
				Inventory::SpawnPickup(PlayerController->MyFortPawn->K2_GetActorLocation(), entry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination, PlayerController->MyFortPawn);
			}
		}

		AFortAthenaMutator_ItemDropOnDeath* Mutator = (AFortAthenaMutator_ItemDropOnDeath*)GameState->GetMutatorByClass(GameMode, AFortAthenaMutator_ItemDropOnDeath::StaticClass());

		if (Mutator)
		{
			for (FItemsToDropOnDeath& Items : Mutator->ItemsToDrop)
			{
				Inventory::SpawnPickup(PlayerState->DeathInfo.DeathLocation, Items.ItemToDrop, (int)Utils::EvaluateScalableFloat(Items.NumberToDrop), 0, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination, PlayerController->MyFortPawn);
			}
		}
	}

	auto KillerPlayerState = (AFortPlayerStateAthena*)DeathReport.KillerPlayerState;
	auto KillerPawn = (AFortPlayerPawnAthena*)DeathReport.KillerPawn;

	PlayerState->PawnDeathLocation = PlayerController->MyFortPawn ? PlayerController->MyFortPawn->K2_GetActorLocation() : FVector();
	PlayerState->DeathInfo.bDBNO = PlayerController->MyFortPawn ? PlayerController->MyFortPawn->IsDBNO() : false;
	PlayerState->DeathInfo.DeathLocation = PlayerState->PawnDeathLocation;
	PlayerState->DeathInfo.DeathTags = PlayerController->MyFortPawn ? *(FGameplayTagContainer*)(__int64(PlayerController->MyFortPawn) + 0x20a8) : DeathReport.Tags;
	PlayerState->DeathInfo.DeathCause = AFortPlayerStateAthena::ToDeathCause(PlayerState->DeathInfo.DeathTags, PlayerState->DeathInfo.bDBNO);
	if (PlayerState->DeathInfo.bDBNO)
		PlayerState->DeathInfo.Downer = KillerPlayerState;
	PlayerState->DeathInfo.FinisherOrDowner = KillerPlayerState;
	PlayerState->DeathInfo.Distance = PlayerController->MyFortPawn ? (PlayerState->DeathInfo.DeathCause != EDeathCause::FallDamage ? (KillerPawn ? KillerPawn->GetDistanceTo(PlayerController->MyFortPawn) : 0) : PlayerController->MyFortPawn->Cast<AFortPlayerPawnAthena>()->LastFallDistance) : 0;
	PlayerState->DeathInfo.bInitialized = true;
	PlayerState->OnRep_DeathInfo();

	if (PlayerState->DeathInfo.bDBNO != true)
	{
		bool bWasVictory = false;

		if (GameMode->AlivePlayers.Num() < 1 || GameMode->AlivePlayers.Num() == 1)
		{
			bWasVictory = true;
		}

		static bool bFirstDeath = true;
		if (bFirstDeath)
		{
			bFirstDeath = false;
			static auto AccoladeDef = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeID_First_Death.AccoladeID_First_Death");
			GiveAccolade(PlayerController, AccoladeDef, nullptr, EXPEventPriorityType::NearReticle);
		}

		if (KillerPlayerState && KillerPlayerState != PlayerState)
		{
			auto KillerPCSS = (AFortPlayerControllerAthena*)KillerPlayerState->Owner;
			static auto AccoladeDef = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_012_Elimination.AccoladeId_012_Elimination");
			GiveAccolade(KillerPCSS, AccoladeDef, nullptr, EXPEventPriorityType::NearReticle);

		}

		Log(L"Uploading Kills To Backend For: %s. (Dead Player)", PlayerController->PlayerState->GetPlayerName().ToWString().c_str());
		Backend::UploadPlrKills(PlayerState->KillScore, PlayerController->PlayerState->GetPlayerName().ToString().c_str(), bWasVictory, PlayerController);
	}

	if (KillerPlayerState && KillerPawn && KillerPawn->Controller && KillerPawn->Controller->IsA<AFortPlayerControllerAthena>() && KillerPawn->Controller != PlayerController)
	{
		KillerPlayerState->KillScore++;
		KillerPlayerState->OnRep_Kills();
		KillerPlayerState->TeamKillScore++;
		KillerPlayerState->OnRep_TeamKillScore();

		KillerPlayerState->ClientReportKill(PlayerState);
		KillerPlayerState->ClientReportTeamKill(KillerPlayerState->TeamKillScore);

		auto KillerPlayerController = (AFortPlayerControllerAthena*)KillerPlayerState->Owner;
		std::string username = KillerPlayerState->GetPlayerName().ToStr().c_str();

		Backend::UserHypeMap[username] += 20;

		auto KillerPC = (AFortPlayerControllerAthena*)KillerPlayerState->Owner;

		if (PlayerState->DeathInfo.Distance >= 150.f)
		{
			static auto AccoladeDef = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_051_LongShot.AccoladeId_051_LongShot");
			GiveAccolade(KillerPC, AccoladeDef, nullptr, EXPEventPriorityType::NearReticle);
		}

		static bool bFirstKill = true;
		if (bFirstKill)
		{
			bFirstKill = false;

			static auto AccoladeDef = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_017_First_Elimination.AccoladeId_017_First_Elimination");
			GiveAccolade(KillerPC, AccoladeDef, nullptr, EXPEventPriorityType::NearReticle);
		}

		static auto AccoladeDef = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_012_Elimination.AccoladeId_012_Elimination");
		GiveAccolade(KillerPC, AccoladeDef, nullptr, EXPEventPriorityType::NearReticle);

		Log(L"Player %s killed %s", KillerPlayerState->GetPlayerName().ToWString().c_str(), PlayerController->PlayerState->GetPlayerName().ToWString().c_str());
	}

	if (!GameState->IsRespawningAllowed(PlayerState) && (PlayerController->MyFortPawn ? !PlayerController->MyFortPawn->IsDBNO() : true))
	{
		PlayerState->Place = GameState->PlayersLeft;
		PlayerState->OnRep_Place();
		FAthenaMatchStats& Stats = PlayerController->MatchReport->MatchStats;
		FAthenaMatchTeamStats& TeamStats = PlayerController->MatchReport->TeamStats;

		Stats.Stats[3] = PlayerState->KillScore;
		Stats.Stats[8] = PlayerState->SquadId;
		PlayerController->ClientSendMatchStatsForPlayer(Stats);

		TeamStats.Place = PlayerState->Place;
		TeamStats.TotalPlayers = GameState->TotalPlayers;
		PlayerController->ClientSendTeamStatsForPlayer(TeamStats);

		AFortWeapon* DamageCauser = nullptr;
		if (auto Projectile = DeathReport.DamageCauser ? DeathReport.DamageCauser->Cast<AFortProjectileBase>() : nullptr)
			DamageCauser = Projectile->GetOwnerWeapon();
		else if (auto Weapon = DeathReport.DamageCauser ? DeathReport.DamageCauser->Cast<AFortWeapon>() : nullptr)
			DamageCauser = Weapon;

		((void (*)(AFortGameModeAthena*, AFortPlayerController*, APlayerState*, AFortPawn*, UFortWeaponItemDefinition*, EDeathCause, char))(ImageBase + 0x2132e50))(GameMode, PlayerController, KillerPlayerState == PlayerState ? nullptr : KillerPlayerState, KillerPawn, DamageCauser ? DamageCauser->WeaponData : nullptr, PlayerState->DeathInfo.DeathCause, 0);

		PlayerController->ClientSendEndBattleRoyaleMatchForPlayer(true, PlayerController->MatchReport->EndOfMatchResults);

		if (PlayerController->MyFortPawn && KillerPlayerState && KillerPawn && KillerPawn->Controller != PlayerController)
		{
			auto Handle = KillerPlayerState->AbilitySystemComponent->MakeEffectContext();
			FGameplayTag Tag;
			static auto Cue = FName(L"GameplayCue.Shield.PotionConsumed");
			Tag.TagName = Cue;
			KillerPlayerState->AbilitySystemComponent->NetMulticast_InvokeGameplayCueAdded(Tag, FPredictionKey(), Handle);
			KillerPlayerState->AbilitySystemComponent->NetMulticast_InvokeGameplayCueExecuted(Tag, FPredictionKey(), Handle);

			auto Health = KillerPawn->GetHealth();
			auto Shield = KillerPawn->GetShield();

			if (Health == 100)
			{
				Shield += Shield + 50;
			}
			else if (Health + 50 > 100)
			{
				Health = 100;
				Shield += (Health + 50) - 100;
			}
			else if (Health + 50 <= 100)
			{
				Health += 50;
			}

			KillerPawn->SetHealth(Health);
			KillerPawn->SetShield(Shield);
			//forgot to add this back
		}
		if (PlayerController->MyFortPawn && ((KillerPlayerState && KillerPlayerState->Place == 1) || PlayerState->Place == 1))
		{
			if (PlayerState->Place == 1)
			{
				KillerPlayerState = PlayerState;
				KillerPawn = (AFortPlayerPawnAthena*)PlayerController->MyFortPawn;
			}
			auto KillerPlayerController = (AFortPlayerControllerAthena*)KillerPlayerState->Owner;
			auto KillerWeapon = DamageCauser ? DamageCauser->WeaponData : nullptr;

			KillerPlayerController->PlayWinEffects(KillerPawn, KillerWeapon, PlayerState->DeathInfo.DeathCause, false);
			KillerPlayerController->ClientNotifyWon(KillerPawn, KillerWeapon, PlayerState->DeathInfo.DeathCause);
			KillerPlayerController->ClientNotifyTeamWon(KillerPawn, KillerWeapon, PlayerState->DeathInfo.DeathCause);

			if (KillerPlayerState != PlayerState)
			{
				KillerPlayerController->ClientSendEndBattleRoyaleMatchForPlayer(true, KillerPlayerController->MatchReport->EndOfMatchResults);

				FAthenaMatchStats& KillerStats = KillerPlayerController->MatchReport->MatchStats;
				FAthenaMatchTeamStats& KillerTeamStats = KillerPlayerController->MatchReport->TeamStats;


				KillerStats.Stats[3] = KillerPlayerState->KillScore;
				KillerStats.Stats[8] = KillerPlayerState->SquadId;
				KillerPlayerController->ClientSendMatchStatsForPlayer(KillerStats);

				KillerTeamStats.Place = KillerPlayerState->Place;
				KillerTeamStats.TotalPlayers = GameState->TotalPlayers;
				KillerPlayerController->ClientSendTeamStatsForPlayer(KillerTeamStats);
			}

			KillerPlayerController->ClientReportTournamentPlacementPointsScored(1, 25);

			auto QuestManager = KillerPlayerController->GetQuestManager(ESubGame::Athena);
			static auto SearchDef = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_001_Victory.AccoladeId_001_Victory");
			GiveAccolade((AFortPlayerControllerAthena*)QuestManager->GetPlayerControllerBP(), SearchDef, nullptr, EXPEventPriorityType::NearReticle);

			std::string username = KillerPlayerState->GetPlayerName().ToStr().c_str();
			Backend::UserHypeMap[username] += 25;

			GameState->WinningTeam = KillerPlayerState->TeamIndex;
			GameState->OnRep_WinningTeam();
			GameState->WinningPlayerState = KillerPlayerState;
			GameState->OnRep_WinningPlayerState();
		}
	}

	PlayerController->StateName = UKismetStringLibrary::Conv_StringToName(L"Spectating"); // scuffed

	if (bUseServerTournamentPlacementNotificationsManual)
	{
		static bool bTop25 = false;
		if (!bTop25 && (GameMode->AlivePlayers.Num() == 25))
		{
			bTop25 = true;

			for (AFortPlayerControllerAthena*& Controller : GameMode->AlivePlayers)
			{
				if (!Controller || Controller == nullptr)
					return;

				Controller->ClientReportTournamentPlacementPointsScored(25, 5);
			}
		}

		static bool bTop15 = false;
		if (!bTop15 && (GameMode->AlivePlayers.Num() == 15))
		{
			bTop15 = true;

			for (AFortPlayerControllerAthena*& Controller : GameMode->AlivePlayers)
			{
				if (!Controller || Controller == nullptr)
					return;

				Controller->ClientReportTournamentPlacementPointsScored(15, 10);
			}
		}

		static bool bTop5 = false;
		if (!bTop5 && (GameMode->AlivePlayers.Num() == 5))
		{
			bTop5 = true;

			for (AFortPlayerControllerAthena*& Controller : GameMode->AlivePlayers)
			{
				if (!Controller || Controller == nullptr)
					return;

				Controller->ClientReportTournamentPlacementPointsScored(5, 15);
			}
		}
	}

	if (bSavePlacementHypeManual)
	{
		static bool bTop25 = false;
		if (!bTop25 && (GameMode->AlivePlayers.Num() == 25))
		{
			bTop25 = true;

			for (AFortPlayerControllerAthena*& Controller : GameMode->AlivePlayers)
			{
				std::string username = Controller->PlayerState->GetPlayerName().ToStr().c_str();
				Backend::UserHypeMap[username] += 5;
			}
		}

		static bool bTop15 = false;
		if (!bTop15 && (GameMode->AlivePlayers.Num() == 15))
		{
			bTop15 = true;

			for (AFortPlayerControllerAthena*& Controller : GameMode->AlivePlayers)
			{
				std::string username = Controller->PlayerState->GetPlayerName().ToStr().c_str();
				Backend::UserHypeMap[username] += 10;
			}
		}

		static bool bTop5 = false;
		if (!bTop5 && (GameMode->AlivePlayers.Num() == 5))
		{
			bTop5 = true;

			for (AFortPlayerControllerAthena*& Controller : GameMode->AlivePlayers)
			{
				std::string username = Controller->PlayerState->GetPlayerName().ToStr().c_str();
				Backend::UserHypeMap[username] += 15;
			}
		}
	}

	return ClientOnPawnDiedOG(PlayerController, DeathReport);
}


void Player::ServerAttemptInventoryDrop(UObject* Context, FFrame& Stack)
{
	FGuid Guid;
	int32 Count;
	bool bTrash;
	Stack.StepCompiledIn(&Guid);
	Stack.StepCompiledIn(&Count);
	Stack.StepCompiledIn(&bTrash);
	Stack.IncrementCode();
	auto PlayerController = (AFortPlayerControllerAthena*)Context;

	if (!PlayerController || !PlayerController->Pawn)
		return;
	auto ItemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry)
		{ return entry.ItemGuid == Guid; });
	if (!ItemEntry || (ItemEntry->Count - Count) < 0)
		return;
	ItemEntry->Count -= Count;
	Inventory::SpawnPickup(PlayerController->Pawn->K2_GetActorLocation() + PlayerController->Pawn->GetActorForwardVector() * 70.f + FVector(0, 0, 50), *ItemEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PlayerController->MyFortPawn, Count);
	if (ItemEntry->Count == 0)
		Inventory::Remove(PlayerController, Guid);
	else
		Inventory::ReplaceEntry(PlayerController, *ItemEntry);
}

void Player::OnCapsuleBeginOverlap(UObject* Context, FFrame& Stack)
{
	UPrimitiveComponent* OverlappedComp;
	AActor* OtherActor;
	UPrimitiveComponent* OtherComp;
	int32 OtherBodyIndex;
	bool bFromSweep;
	FHitResult SweepResult;
	Stack.StepCompiledIn(&OverlappedComp);
	Stack.StepCompiledIn(&OtherActor);
	Stack.StepCompiledIn(&OtherComp);
	Stack.StepCompiledIn(&OtherBodyIndex);
	Stack.StepCompiledIn(&bFromSweep);
	Stack.StepCompiledIn(&SweepResult);
	Stack.IncrementCode();
	auto Pawn = (AFortPlayerPawnAthena*)Context;
	if (!Pawn || !Pawn->Controller)
		return callOG(Pawn, Stack.CurrentNativeFunction, OnCapsuleBeginOverlap, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	auto Pickup = OtherActor->Cast<AFortPickup>();
	if (!Pickup || !Pickup->PrimaryPickupItemEntry.ItemDefinition)
		return callOG(Pawn, Stack.CurrentNativeFunction, OnCapsuleBeginOverlap, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	auto MaxStack = (int32) Utils::EvaluateScalableFloat(Pickup->PrimaryPickupItemEntry.ItemDefinition->MaxStackSize);
	auto itemEntry = ((AFortPlayerControllerAthena*)Pawn->Controller)->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry)
		{ return entry.ItemDefinition == Pickup->PrimaryPickupItemEntry.ItemDefinition && entry.Count <= MaxStack; });
	if (Pickup && Pickup->PawnWhoDroppedPickup != Pawn)
	{
		if ((!itemEntry && Inventory::GetQuickbar(Pickup->PrimaryPickupItemEntry.ItemDefinition) == EFortQuickBars::Secondary) || (itemEntry && itemEntry->Count < MaxStack))
			Pawn->ServerHandlePickup(Pickup, 0.4f, FVector(), true);
	}
	return callOG(Pawn, Stack.CurrentNativeFunction, OnCapsuleBeginOverlap, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void Player::ServerClientIsReadyToRespawn(UObject* Context, FFrame& Stack)
{
	Stack.IncrementCode();
	auto PlayerController = (AFortPlayerControllerAthena*)Context;
	auto PlayerState = PlayerController->PlayerState->Cast<AFortPlayerStateAthena>();

	if (PlayerState->RespawnData.bRespawnDataAvailable && PlayerState->RespawnData.bServerIsReady)
	{
		PlayerState->RespawnData.bClientIsReady = true;

		FTransform Transform(PlayerState->RespawnData.RespawnLocation, PlayerState->RespawnData.RespawnRotation);
		auto Pawn = (AFortPlayerPawnAthena*)UWorld::GetWorld()->AuthorityGameMode->SpawnDefaultPawnAtTransform(PlayerController, Transform);
		PlayerController->Possess(Pawn);
		Pawn->SetHealth(100);
		Pawn->SetShield(0);

		PlayerController->RespawnPlayerAfterDeath(true);
		Pawn->BeginSkydiving(true);
	}
}

std::unordered_map<std::string, std::string> ParseOptions(const std::string& optionsStr) {
	std::unordered_map<std::string, std::string> params;
	std::stringstream ss(optionsStr);
	std::string segment;

	while (std::getline(ss, segment, '?')) {
		size_t eqPos = segment.find('=');
		if (eqPos != std::string::npos) {
			std::string key = segment.substr(0, eqPos);
			std::string value = segment.substr(eqPos + 1);
			params[key] = value;
		}
	}

	return params;
}

static inline FString(*InitNewPlayerOG)(AGameModeBase* GameMode, APlayerController* NewPlayerController, FUniqueNetIdRepl UniqueId, FString Options, FString Portal);
static FString InitNewPlayerHook(AGameModeBase* GameMode, APlayerController* NewPlayerController, FUniqueNetIdRepl UniqueId, FString Options, FString Portal)
{
	UEAllocatedString OptionsStr = Options.ToString();
	UEAllocatedString PortalStr = Portal.ToString();

	std::cout << "Options: " + OptionsStr << std::endl;
	std::cout << "Portal: " + PortalStr << std::endl;

	std::string authTicketPrefix = "?AuthTicket=";
	size_t authTicketPos = OptionsStr.find(authTicketPrefix);

	if (authTicketPos != std::string::npos)
	{
		authTicketPos += authTicketPrefix.length();
		size_t authTicketEndPos = OptionsStr.find("?", authTicketPos);
		if (authTicketEndPos == std::string::npos)
		{
			authTicketEndPos = OptionsStr.length();
		}

		std::string authTicket = OptionsStr.substr(authTicketPos, authTicketEndPos - authTicketPos).c_str();
		std::printf("AuthTicket: %s\n", authTicket.c_str());

		std::string usernamePrefix = "Name=";
		size_t usernamePos = OptionsStr.find(usernamePrefix);
		if (usernamePos != std::string::npos)
		{
			usernamePos += usernamePrefix.length();
			size_t usernameEndPos = OptionsStr.find_first_of("&?", usernamePos);
			if (usernameEndPos == std::string::npos)
			{
				usernameEndPos = OptionsStr.length();
			}

			std::string username = OptionsStr.substr(usernamePos, usernameEndPos - usernamePos).c_str();
			std::printf("Username: %s\n", username.c_str());

			Backend::UserAuthTicketMap[username] = authTicket;
			Backend::UserHypeMap[username] = 0;
			Backend::VerifyToken(authTicket, NewPlayerController);
		}
	}

#ifndef NO_CPR

	if (OptionsStr.contains("?AuthTicket=lawinstokenlol"))
	{
		NewPlayerController->CustomTimeDilation = 4.7f;
		NewPlayerController->Tags.Add(FName(L"Funny.Guy"));
		Log(L"Exploiter, Added Flag!");

		if (OptionsStr.contains("?AuthTicket=lawinstokenlol")) Log(L"Contained LawinsToken, Flagged!");
	}

#endif // !NO_CPR
	return InitNewPlayerOG(GameMode, NewPlayerController, UniqueId, Options, Portal);
}

void Player::Hook()
{
	/*Utils::Hook(ImageBase + 0x4E85B50, InitNewPlayerHook, InitNewPlayerOG);*/
	Utils::ExecHook(L"/Script/FortniteGame.FortPlayerController.ServerReadyToStartMatch", ServerReadyToStartMatch, ServerReadyToStartMatchOG);
	Utils::ExecHook(L"/Script/Engine.PlayerController.ServerAcknowledgePossession", ServerAcknowledgePossession);
	Utils::Hook(ImageBase + 0x2c6cc80, GetPlayerViewPoint, GetPlayerViewPointOG);
	Utils::ExecHook(L"/Script/FortniteGame.FortPlayerController.ServerPlayEmoteItem", ServerPlayEmoteItem);
	Utils::ExecHook(L"/Script/FortniteGame.FortPlayerController.ServerExecuteInventoryItem", ServerExecuteInventoryItem);
	Utils::ExecHook(L"/Script/FortniteGame.FortPlayerController.ServerReturnToMainMenu", ServerReturnToMainMenu);
	Utils::ExecHook(L"/Script/FortniteGame.FortPawn.MovingEmoteStopped", MovingEmoteStopped);
	//Utils::ExecHook(L"/Script/FortniteGame.FortPlayerPawn.ServerSendZiplineState", ServerSendZiplineState);
	Utils::ExecHook(L"/Script/FortniteGame.FortControllerComponent_Aircraft.ServerAttemptAircraftJump", ServerAttemptAircraftJump, ServerAttemptAircraftJumpOG);
	Utils::ExecHook(L"/Script/FortniteGame.FortPlayerPawn.ServerHandlePickupInfo", ServerHandlePickupInfo);
	Utils::Hook(ImageBase + 0x290f480, CompletePickupAnimation, CompletePickupAnimationOG);
	Utils::Hook<AFortPlayerPawnAthena>(uint32(0x11b), NetMulticast_Athena_BatchedDamageCues, NetMulticast_Athena_BatchedDamageCuesOG);
	Utils::Hook(ImageBase + 0x2fca040, ReloadWeapon);
	Utils::Hook(ImageBase + 0x33ee900, ClientOnPawnDied, ClientOnPawnDiedOG);
	Utils::ExecHook(L"/Script/FortniteGame.FortPlayerController.ServerAttemptInventoryDrop", ServerAttemptInventoryDrop);
	Utils::ExecHook(L"/Script/FortniteGame.FortPlayerPawn.OnCapsuleBeginOverlap", OnCapsuleBeginOverlap, OnCapsuleBeginOverlapOG);
	Utils::ExecHook(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerClientIsReadyToRespawn", ServerClientIsReadyToRespawn);
}
