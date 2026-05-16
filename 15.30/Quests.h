#pragma once

static bool bHasPlayerReceivedElim = false;
static bool bHasPlayerSearchedChest = false;
static bool bHasPlayerSearchedSupplyDrop = false;

static inline void GiveAccolade(AFortPlayerControllerAthena* PC, UFortAccoladeItemDefinition* AccoladeDef, UFortQuestItemDefinition* QuestDef, EXPEventPriorityType Priority)
{
	if (!PC || !PC->XPComponent || !AccoladeDef)
		return;

	FXPEventInfo Info{};
	Info.Accolade = UKismetSystemLibrary::GetPrimaryAssetIdFromObject(AccoladeDef);
	Info.EventXpValue = AccoladeDef->GetAccoladeXpValue() * 3;//idk if proepr functiosn
	Info.Priority = Priority;
	Info.QuestDef = QuestDef;
	Info.RestedXPRemaining = PC->XPComponent->RestXP - Info.EventXpValue;
	Info.RestedValuePortion = 100;
	Info.SeasonBoostValuePortion = PC->XPComponent->CachedSeasonMatchXpBoost;
	Info.TotalXpEarnedInMatch = PC->XPComponent->TotalXpEarned += Info.EventXpValue;
	Info.SimulatedText = AccoladeDef->ShortDescription;
	PC->XPComponent->OnXPEvent(Info);
	//FXPEventEntry Entry{};
	//Entry.Accolade = UKismetSystemLibrary::GetPrimaryAssetIdFromObject(AccoladeDef);
	//Entry.EventXpValue = AccoladeDef->GetAccoladeXpValue();
	//Entry.QuestDef = QuestDef;
	//Entry.Time = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
	//Entry.TotalXpEarnedInMatch = Entry.EventXpValue;
	//Entry.SimulatedXpEvent = AccoladeDef->ShortDescription;
	//PC->XPComponent->EventArray.Entries.Add(Entry);
	//PC->XPComponent->EventArray.MarkItemDirty(Entry);
	//PC->XPComponent->HighPrioXPEvent(Entry);
}

static inline bool ContainsTag(FGameplayTagContainer Container, FName Tag)
{
	for (auto tag : Container.GameplayTags)
	{
		if (tag.TagName.ComparisonIndex == Tag.ComparisonIndex)
			return true;
	}
	return false;
}


static inline void SendStatEvent(UFortQuestManager* ManagerComp, UObject* TargetObject, FGameplayTagContainer& AdditionalSourceTags, FGameplayTagContainer& TargetTags, bool* QuestActive, bool* QuestCompleted, int32 Count, EFortQuestObjectiveStatEvent StatEvent)
{
	Log(L"SendStatEvent!");
	if (!ManagerComp)
		return;

	auto QuestManager = ManagerComp;
	
	FGameplayTagContainer Source;
	FGameplayTagContainer Context;
	ManagerComp->GetSourceAndContextTags(&Source, &Context);
	ManagerComp->AppendTemporaryRelevancyTags(Source, Context, TargetTags);

	if (TargetObject)
	{
		Log(TargetObject->GetWName().c_str());
	}

	for (auto tag : Source.GameplayTags)
	{
		bool contains = false;
		for (auto aTag : AdditionalSourceTags.GameplayTags)
		{
			if (aTag.TagName.ComparisonIndex == tag.TagName.ComparisonIndex)
			{
				contains = true;
				break;
			}
		}
		if (!contains)
			AdditionalSourceTags.GameplayTags.Add(tag);
	}

	for (auto tag : Source.ParentTags)
	{
		bool contains = false;
		for (auto aTag : AdditionalSourceTags.ParentTags)
		{
			if (aTag.TagName.ComparisonIndex == tag.TagName.ComparisonIndex)
			{
				contains = true;
				break;
			}
		}
		if (!contains)
			AdditionalSourceTags.ParentTags.Add(tag);
	}

	Source.GameplayTags.Free();
	Source.ParentTags.Free();
	Context.GameplayTags.Free();
	Context.ParentTags.Free();

	if (StatEvent == EFortQuestObjectiveStatEvent::Kill)
	{
		int Kills = ((AFortPlayerStateAthena*)ManagerComp->GetPlayerControllerBP()->PlayerState)->KillScore;

		static UFortAccoladeItemDefinition* ElimDef = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_012_Elimination.AccoladeId_012_Elimination");
		static UFortAccoladeItemDefinition* ElimDef1 = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_014_Elimination_Bronze.AccoladeId_014_Elimination_Bronze");
		static UFortAccoladeItemDefinition* ElimDef4 = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_014_Elimination_Silver.AccoladeId_014_Elimination_Silver");
		static UFortAccoladeItemDefinition* ElimDef8 = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_014_Elimination_Gold.AccoladeId_014_Elimination_Gold");

		GiveAccolade((AFortPlayerControllerAthena*)ManagerComp->GetPlayerControllerBP(), ElimDef, nullptr, EXPEventPriorityType::NearReticle);

		if (Kills == 1)
		{
			GiveAccolade((AFortPlayerControllerAthena*)ManagerComp->GetPlayerControllerBP(), ElimDef1, nullptr, EXPEventPriorityType::NearReticle);
		}
		if (Kills == 4)
		{
			GiveAccolade((AFortPlayerControllerAthena*)ManagerComp->GetPlayerControllerBP(), ElimDef4, nullptr, EXPEventPriorityType::NearReticle);
		}
		if (Kills == 8)
		{
			GiveAccolade((AFortPlayerControllerAthena*)ManagerComp->GetPlayerControllerBP(), ElimDef8, nullptr, EXPEventPriorityType::NearReticle);
		}
	}

	if (StatEvent == EFortQuestObjectiveStatEvent::Interact)
	{
		Log(L"InteractStatEvent!");

		if (TargetObject)
		{
			if (TargetObject->GetName().contains("Chest"))
			{
				static bool bFirstToSearchChest = true;
				if (bFirstToSearchChest)
				{
					bFirstToSearchChest = false;
					static auto SearchDef = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_069_FirstSearchedChest.AccoladeId_069_FirstSearchedChest");
					GiveAccolade((AFortPlayerControllerAthena*)QuestManager->GetPlayerControllerBP(), SearchDef, nullptr, EXPEventPriorityType::NearReticle);
				}

				Log(L"IsChest!!");
				static auto SearchDef = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_007_SearchChests.AccoladeId_007_SearchChests");
				GiveAccolade((AFortPlayerControllerAthena*)QuestManager->GetPlayerControllerBP(), SearchDef, nullptr, EXPEventPriorityType::NearReticle);
			}

			if (TargetObject->GetName().contains("Ammo"))
			{
				Log(L"AmmoBoxBro!!!!");
				static auto SearchDef = Utils::FindObject<UFortAccoladeItemDefinition>(L"/Game/Athena/Items/Accolades/AccoladeId_011_SearchAmmoBox.AccoladeId_011_SearchAmmoBox");
				GiveAccolade((AFortPlayerControllerAthena*)QuestManager->GetPlayerControllerBP(), SearchDef, nullptr, EXPEventPriorityType::NearReticle);
			}
		}
	}
}

static inline void (*SendComplexCustomStatEventOG)(UFortQuestManager* QuestManager, UObject* TargetObject, FGameplayTagContainer& AdditionalSourceTags, FGameplayTagContainer& TargetTags, bool* QuestActive, bool* QuestCompleted, int32 Count);
static void SendComplexCustomStatEvent(UFortQuestManager* QuestManager, UObject* TargetObject, FGameplayTagContainer& AdditionalSourceTags, FGameplayTagContainer& TargetTags, bool* QuestActive, bool* QuestCompleted, int32 Count)
{
	if (__int64(_ReturnAddress()) == ImageBase + 0x19e2e3e)
	{
		SendStatEvent(QuestManager, TargetObject, AdditionalSourceTags, TargetTags, QuestActive, QuestCompleted, Count, EFortQuestObjectiveStatEvent::ComplexCustom);
	}
	else
	{
		SendStatEvent(QuestManager, TargetObject, AdditionalSourceTags, TargetTags, nullptr, nullptr, 1, EFortQuestObjectiveStatEvent::ComplexCustom);
	}

	return SendComplexCustomStatEventOG(QuestManager, TargetObject, AdditionalSourceTags, TargetTags, QuestActive, QuestCompleted, Count);
}

static void HookQuests()
{
	Utils::Hook(ImageBase + 0x2D3D010, SendComplexCustomStatEvent, SendComplexCustomStatEventOG);
}