#include "CGGameInstance.h"

#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Fonts/CompositeFont.h"

namespace
{
	const TCHAR* MoodLadder[] = { TEXT("Miserable"), TEXT("Tired"), TEXT("Content"), TEXT("High"), TEXT("Ecstatic") };

	FString PrimaryStatForClass(const FString& ClassName)
	{
		if (ClassName == TEXT("Paladin") || ClassName == TEXT("Demon Blacksmith") || ClassName == TEXT("Shield Bearer")) return TEXT("might");
		if (ClassName == TEXT("Witch")) return TEXT("magic");
		if (ClassName == TEXT("Rogue")) return TEXT("guile");
		if (ClassName == TEXT("Grave Monk") || ClassName == TEXT("Herbalist") || ClassName == TEXT("Ooze Familiar")) return TEXT("spirit");
		return TEXT("wits");
	}

	// class -> risk tag that class counters
	const TMap<FString, FString>& ClassRiskCounters()
	{
		static TMap<FString, FString> M = {
			{ TEXT("Paladin"),   TEXT("undead") },
			{ TEXT("Herbalist"), TEXT("poison") },
			{ TEXT("Rogue"),     TEXT("trap") },
			{ TEXT("Witch"),     TEXT("curse") },
			{ TEXT("Ranger"),    TEXT("beast") },
		};
		return M;
	}
}

void UCGGameInstance::Init()
{
	Super::Init();
	LoadStaticData();
	LoadSpriteManifest();

	// pick a serif font for the gothic look, if the system has one
	const TCHAR* Candidates[][2] = {
		{ TEXT("C:/Windows/Fonts/constan.ttf"), TEXT("C:/Windows/Fonts/constanb.ttf") },
		{ TEXT("C:/Windows/Fonts/georgia.ttf"), TEXT("C:/Windows/Fonts/georgiab.ttf") },
	};
	for (const auto& Pair : Candidates)
	{
		if (FPaths::FileExists(Pair[0]) && FPaths::FileExists(Pair[1]))
		{
			RegularFont = MakeShared<FStandaloneCompositeFont>();
			RegularFont->DefaultTypeface.AppendFont(TEXT("Regular"), Pair[0], EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
			BoldFont = MakeShared<FStandaloneCompositeFont>();
			BoldFont->DefaultTypeface.AppendFont(TEXT("Regular"), Pair[1], EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
			break;
		}
	}
}

// ----------------------------------------------------------------------
// Data
// ----------------------------------------------------------------------

template <typename TWrapper>
static bool LoadJsonFile(const FString& FileName, TWrapper& Out)
{
	const FString Path = FPaths::ProjectContentDir() / TEXT("Data") / FileName;
	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *Path))
	{
		UE_LOG(LogTemp, Error, TEXT("CursedGuild: failed to read %s"), *Path);
		return false;
	}
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(Json, &Out))
	{
		UE_LOG(LogTemp, Error, TEXT("CursedGuild: failed to parse %s"), *Path);
		return false;
	}
	return true;
}

void UCGGameInstance::LoadStaticData()
{
	FCGMemberList MemberList;
	if (LoadJsonFile(TEXT("members.json"), MemberList)) { AllMembers = MoveTemp(MemberList.members); }

	FCGContractList ContractList;
	if (LoadJsonFile(TEXT("contracts.json"), ContractList)) { AllContracts = MoveTemp(ContractList.contracts); }

	FCGRelicList RelicList;
	if (LoadJsonFile(TEXT("relics.json"), RelicList)) { AllRelics = MoveTemp(RelicList.relics); }

	FCGRoomList RoomList;
	if (LoadJsonFile(TEXT("rooms.json"), RoomList)) { DefaultRooms = MoveTemp(RoomList.rooms); }

	FCGEnemyList EnemyList;
	if (LoadJsonFile(TEXT("enemies.json"), EnemyList)) { AllEnemies = MoveTemp(EnemyList.enemies); }

	UE_LOG(LogTemp, Log, TEXT("CursedGuild: loaded %d members, %d contracts, %d relics, %d rooms, %d enemies"),
		AllMembers.Num(), AllContracts.Num(), AllRelics.Num(), DefaultRooms.Num(), AllEnemies.Num());
}

void UCGGameInstance::LoadSpriteManifest()
{
	FCGSpriteList SpriteList;
	if (LoadJsonFile(TEXT("sprites.json"), SpriteList))
	{
		for (const FCGSpriteDef& Def : SpriteList.sprites)
		{
			SpritePaths.Add(Def.id, FPaths::ProjectContentDir() / TEXT("Art/Sprites") / Def.path);
		}
	}
	SpritePaths.Add(TEXT("bg_guild_hall"), FPaths::ProjectContentDir() / TEXT("Art/Backgrounds/guild_hall.png"));
}

const FCGContract* UCGGameInstance::FindContract(const FString& Id) const
{
	return AllContracts.FindByPredicate([&](const FCGContract& C) { return C.id == Id; });
}

const FCGRelic* UCGGameInstance::FindRelic(const FString& Id) const
{
	return AllRelics.FindByPredicate([&](const FCGRelic& R) { return R.id == Id; });
}

const FCGEnemy* UCGGameInstance::FindEnemyByTag(const FString& Tag) const
{
	return AllEnemies.FindByPredicate([&](const FCGEnemy& E) { return E.tag == Tag; });
}

FCGMember* UCGGameInstance::FindRosterMember(const FString& Id)
{
	return Roster.FindByPredicate([&](const FCGMember& M) { return M.id == Id; });
}

const FCGMember* UCGGameInstance::FindRosterMember(const FString& Id) const
{
	return Roster.FindByPredicate([&](const FCGMember& M) { return M.id == Id; });
}

FCGRoom* UCGGameInstance::FindRoom(const FString& Id)
{
	return Rooms.FindByPredicate([&](const FCGRoom& R) { return R.id == Id; });
}

const FCGRoom* UCGGameInstance::FindRoom(const FString& Id) const
{
	return Rooms.FindByPredicate([&](const FCGRoom& R) { return R.id == Id; });
}

// ----------------------------------------------------------------------
// Game flow
// ----------------------------------------------------------------------

void UCGGameInstance::NewGame()
{
	bInGame = true;
	Resources = FCGResourceBundle{};
	Resources.gold = 1000;
	Resources.reputation = 0;
	Resources.supplies = 30;
	Resources.arcaneDust = 50;
	Week = 1;
	Day = 1;

	Roster.Reset();
	for (const FCGMember& M : AllMembers)
	{
		if (M.starting) { Roster.Add(M); }
	}

	OwnedRelicIds = { TEXT("relic_haunted_lantern"), TEXT("relic_thorn_ring") };
	EquippedRelicIds.Reset();
	Rooms = DefaultRooms;
	BoardContractIds.Reset();
	UsedContractIds.Reset();
	Ledger.Reset();
	TutorialStep = 0;
	ExpeditionsRun = 0;
	SelectedContractId.Reset();
	SelectedPartyIds.Reset();

	RefreshBoard(true);
	if (BoardContractIds.Num() > 0) { SelectedContractId = BoardContractIds[0]; }
	AddLedger(TEXT("The Cursed Guild opens its doors. Starting funds: 1,000 gold."));
	SaveGame();
}

FString UCGGameInstance::SavePath() const
{
	return FPaths::ProjectSavedDir() / TEXT("SaveGames/cursedguild.json");
}

bool UCGGameInstance::HasSaveFile() const
{
	return FPaths::FileExists(SavePath());
}

bool UCGGameInstance::SaveGame()
{
	if (!bInGame) { return false; }

	FCGSaveData Save;
	Save.resources = Resources;
	Save.week = Week;
	Save.day = Day;
	Save.roster = Roster;
	Save.ownedRelicIds = OwnedRelicIds;
	Save.equippedRelicIds = EquippedRelicIds;
	for (const FCGRoom& R : Rooms)
	{
		FCGRoomSave RS;
		RS.id = R.id;
		RS.level = R.level;
		RS.isUnlocked = R.isUnlocked;
		Save.rooms.Add(RS);
	}
	Save.boardContractIds = BoardContractIds;
	Save.usedContractIds = UsedContractIds;
	Save.ledger = Ledger;
	Save.tutorialStep = TutorialStep;
	Save.expeditionsRun = ExpeditionsRun;

	FString Json;
	if (!FJsonObjectConverter::UStructToJsonObjectString(Save, Json))
	{
		return false;
	}
	return FFileHelper::SaveStringToFile(Json, *SavePath());
}

bool UCGGameInstance::LoadGame()
{
	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *SavePath()))
	{
		return false;
	}
	FCGSaveData Save;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(Json, &Save))
	{
		return false;
	}

	bInGame = true;
	Resources = Save.resources;
	Week = Save.week;
	Day = Save.day;
	Roster = Save.roster;
	OwnedRelicIds = Save.ownedRelicIds;
	EquippedRelicIds = Save.equippedRelicIds;
	Rooms = DefaultRooms;
	for (const FCGRoomSave& RS : Save.rooms)
	{
		if (FCGRoom* R = FindRoom(RS.id))
		{
			R->level = RS.level;
			R->isUnlocked = RS.isUnlocked;
		}
	}
	BoardContractIds = Save.boardContractIds;
	UsedContractIds = Save.usedContractIds;
	Ledger = Save.ledger;
	TutorialStep = Save.tutorialStep;
	ExpeditionsRun = Save.expeditionsRun;
	SelectedPartyIds.Reset();
	SelectedContractId = BoardContractIds.Num() > 0 ? BoardContractIds[0] : FString();
	return true;
}

// ----------------------------------------------------------------------
// Calendar
// ----------------------------------------------------------------------

FString UCGGameInstance::MoonPhase() const
{
	static const TCHAR* Phases[] = { TEXT("Waxing Moon"), TEXT("Full Moon"), TEXT("Waning Moon"), TEXT("New Moon") };
	return Phases[(Week - 1) % 4];
}

FString UCGGameInstance::MoonPhaseHint() const
{
	const FString Phase = MoonPhase();
	if (Phase == TEXT("Waxing Moon")) return TEXT("Reputation gains +5%");
	if (Phase == TEXT("Full Moon"))   return TEXT("Magic surges; curse risk +10%");
	if (Phase == TEXT("Waning Moon")) return TEXT("Resting is more effective");
	return TEXT("Stealthy contracts are easier");
}

void UCGGameInstance::AdvanceDays(int32 NumDays)
{
	for (int32 i = 0; i < NumDays; ++i)
	{
		Day++;
		for (FCGMember& M : Roster)
		{
			if (M.daysUnavailable > 0)
			{
				M.daysUnavailable--;
				if (M.daysUnavailable == 0 && (M.status == TEXT("unavailable") || M.status == TEXT("wounded")))
				{
					M.status = TEXT("injured");
				}
			}
		}
		if (Day > 7)
		{
			Day = 1;
			Week++;
			RefreshBoard(true);
			const int32 Upkeep = Roster.Num() * 15;
			if (Resources.gold >= Upkeep)
			{
				Resources.gold -= Upkeep;
				AddLedger(FString::Printf(TEXT("Week %d: paid %d gold in wages and upkeep."), Week, Upkeep));
			}
			else
			{
				Resources.gold = 0;
				for (FCGMember& M : Roster) { M.mood = MoodShift(M.mood, -1); }
				AddLedger(FString::Printf(TEXT("Week %d: the coffers ran dry. Wages went unpaid and spirits sank."), Week));
			}
		}
	}
}

// ----------------------------------------------------------------------
// Roster
// ----------------------------------------------------------------------

int32 UCGGameInstance::RosterCapacity() const
{
	const FCGRoom* Hall = FindRoom(TEXT("room_guild_hall"));
	return 4 + (Hall ? static_cast<int32>(Hall->Bonus()) : 0);
}

TArray<const FCGMember*> UCGGameInstance::RecruitCandidates() const
{
	TArray<const FCGMember*> Out;
	for (const FCGMember& M : AllMembers)
	{
		if (!FindRosterMember(M.id)) { Out.Add(&M); }
	}
	return Out;
}

bool UCGGameInstance::CanHire(const FCGMember& Candidate, FString& OutReason) const
{
	if (Roster.Num() >= RosterCapacity())
	{
		OutReason = TEXT("The guild hall is full. Upgrade it to house more members.");
		return false;
	}
	if (Resources.reputation < Candidate.hireCostRep)
	{
		OutReason = FString::Printf(TEXT("Requires %d reputation."), Candidate.hireCostRep);
		return false;
	}
	if (Resources.gold < Candidate.hireCostGold)
	{
		OutReason = FString::Printf(TEXT("Requires %d gold."), Candidate.hireCostGold);
		return false;
	}
	return true;
}

bool UCGGameInstance::Hire(const FString& MemberId)
{
	const FCGMember* Candidate = AllMembers.FindByPredicate([&](const FCGMember& M) { return M.id == MemberId; });
	FString Reason;
	if (!Candidate || FindRosterMember(MemberId) || !CanHire(*Candidate, Reason))
	{
		return false;
	}
	Resources.gold -= Candidate->hireCostGold;
	Roster.Add(*Candidate);
	AddLedger(FString::Printf(TEXT("Recruited %s the %s for %d gold."), *Candidate->name, *Candidate->className, Candidate->hireCostGold));
	SaveGame();
	return true;
}

bool UCGGameInstance::RestGuild(FString& OutSummary)
{
	if (Resources.supplies < RestSupplyCost())
	{
		OutSummary = TEXT("Not enough supplies to rest.");
		return false;
	}
	Resources.supplies -= RestSupplyCost();

	float HealFrac = 0.25f;
	if (const FCGRoom* Barracks = FindRoom(TEXT("room_barracks"))) { HealFrac += Barracks->Bonus(); }
	if (MoonPhase() == TEXT("Waning Moon")) { HealFrac *= 1.5f; }

	int32 MoodSteps = 1;
	if (const FCGRoom* Kitchen = FindRoom(TEXT("room_kitchen"))) { MoodSteps += Kitchen->level >= 3 ? 1 : 0; }

	for (FCGMember& M : Roster)
	{
		M.hp = FMath::Min(M.maxHp, M.hp + FMath::CeilToInt(M.maxHp * HealFrac));
		M.mood = MoodShift(M.mood, MoodSteps);
		if (M.status == TEXT("injured") && M.hp >= M.maxHp * 0.6f)
		{
			M.status = TEXT("available");
		}
	}
	AdvanceDays(1);
	AddLedger(FString::Printf(TEXT("The guild rested for a day (%d supplies). Wounds mended and moods lifted."), RestSupplyCost()));
	OutSummary = FString::Printf(TEXT("The guild rests. Everyone recovers %d%% HP and feels better."), FMath::RoundToInt(HealFrac * 100));
	SaveGame();
	return true;
}

// ----------------------------------------------------------------------
// Contracts / party
// ----------------------------------------------------------------------

void UCGGameInstance::RefreshBoard(bool bFullRefresh)
{
	if (bFullRefresh) { BoardContractIds.Reset(); }

	auto PoolFor = [this](const FString& Difficulty) -> TArray<FString>
	{
		TArray<FString> Pool;
		for (const FCGContract& C : AllContracts)
		{
			if ((Difficulty.IsEmpty() || C.difficulty == Difficulty)
				&& !UsedContractIds.Contains(C.id)
				&& !BoardContractIds.Contains(C.id))
			{
				Pool.Add(C.id);
			}
		}
		return Pool;
	};

	// When the pool runs dry, forget the oldest completions so the game can run indefinitely.
	if (PoolFor(FString()).Num() < 3 && UsedContractIds.Num() > 0)
	{
		UsedContractIds.Reset();
	}

	const TCHAR* Difficulties[] = { TEXT("Easy"), TEXT("Medium"), TEXT("Hard") };
	int32 DiffIndex = 0;
	while (BoardContractIds.Num() < 3)
	{
		TArray<FString> Pool = PoolFor(Difficulties[DiffIndex % 3]);
		if (Pool.Num() == 0) { Pool = PoolFor(FString()); }
		if (Pool.Num() == 0) { break; }
		BoardContractIds.Add(Pool[FMath::RandRange(0, Pool.Num() - 1)]);
		DiffIndex++;
	}

	if (!SelectedContractId.IsEmpty() && !BoardContractIds.Contains(SelectedContractId))
	{
		SelectedContractId = BoardContractIds.Num() > 0 ? BoardContractIds[0] : FString();
	}
}

void UCGGameInstance::SelectContract(const FString& ContractId)
{
	if (BoardContractIds.Contains(ContractId))
	{
		SelectedContractId = ContractId;
	}
}

bool UCGGameInstance::TogglePartyMember(const FString& MemberId)
{
	if (SelectedPartyIds.Contains(MemberId))
	{
		SelectedPartyIds.Remove(MemberId);
		return true;
	}
	const FCGMember* M = FindRosterMember(MemberId);
	if (!M || !M->CanEmbark() || SelectedPartyIds.Num() >= 3)
	{
		return false;
	}
	SelectedPartyIds.Add(MemberId);
	return true;
}

// ----------------------------------------------------------------------
// Relics
// ----------------------------------------------------------------------

int32 UCGGameInstance::RelicSlots() const
{
	const FCGRoom* Vault = FindRoom(TEXT("room_relic_vault"));
	const int32 Level = Vault && Vault->isUnlocked ? Vault->level : 0;
	return FMath::Clamp(1 + Level, 0, 3);
}

int32 UCGGameInstance::RelicStorage() const
{
	const FCGRoom* Vault = FindRoom(TEXT("room_relic_vault"));
	const int32 Level = Vault && Vault->isUnlocked ? Vault->level : 0;
	return 6 + 2 * Level;
}

TArray<FString> UCGGameInstance::StoredRelicIds() const
{
	TArray<FString> Out;
	for (const FString& Id : OwnedRelicIds)
	{
		if (!EquippedRelicIds.Contains(Id)) { Out.Add(Id); }
	}
	return Out;
}

bool UCGGameInstance::EquipRelic(const FString& RelicId)
{
	if (!OwnedRelicIds.Contains(RelicId) || EquippedRelicIds.Contains(RelicId) || EquippedRelicIds.Num() >= RelicSlots())
	{
		return false;
	}
	EquippedRelicIds.Add(RelicId);
	return true;
}

bool UCGGameInstance::UnequipRelic(const FString& RelicId)
{
	return EquippedRelicIds.Remove(RelicId) > 0;
}

// ----------------------------------------------------------------------
// Rooms
// ----------------------------------------------------------------------

bool UCGGameInstance::CanUpgradeRoom(const FCGRoom& Room, FString& OutReason) const
{
	if (Room.level >= Room.maxLevel)
	{
		OutReason = TEXT("Already at maximum level.");
		return false;
	}
	if (!Room.upgradeCosts.IsValidIndex(Room.level))
	{
		OutReason = TEXT("No further upgrades.");
		return false;
	}
	const FCGResourceBundle& Cost = Room.upgradeCosts[Room.level];
	if (!CanAfford(Cost))
	{
		OutReason = TEXT("Cannot afford this upgrade.");
		return false;
	}
	return true;
}

bool UCGGameInstance::UpgradeRoom(const FString& RoomId)
{
	FCGRoom* Room = FindRoom(RoomId);
	FString Reason;
	if (!Room || !CanUpgradeRoom(*Room, Reason))
	{
		return false;
	}
	Spend(Room->upgradeCosts[Room->level]);
	Room->level++;
	Room->isUnlocked = true;
	AddLedger(FString::Printf(TEXT("%s %s to level %d."),
		Room->level == 1 ? TEXT("Unlocked") : TEXT("Upgraded"), *Room->name, Room->level));
	SaveGame();
	return true;
}

// ----------------------------------------------------------------------
// Economy
// ----------------------------------------------------------------------

bool UCGGameInstance::CanAfford(const FCGResourceBundle& Cost) const
{
	return Resources.gold >= Cost.gold
		&& Resources.reputation >= Cost.reputation
		&& Resources.supplies >= Cost.supplies
		&& Resources.arcaneDust >= Cost.arcaneDust;
}

void UCGGameInstance::Spend(const FCGResourceBundle& Cost)
{
	Resources.gold -= Cost.gold;
	Resources.supplies -= Cost.supplies;
	Resources.arcaneDust -= Cost.arcaneDust;
	// reputation costs are thresholds, not spent
}

FCGResourceBundle UCGGameInstance::CraftCost() const
{
	float Discount = 0.f;
	if (const FCGRoom* Workshop = FindRoom(TEXT("room_workshop"))) { Discount = Workshop->Bonus(); }
	Discount = FMath::Clamp(Discount, 0.f, 0.6f);
	FCGResourceBundle Cost;
	Cost.gold = FMath::RoundToInt(400 * (1.f - Discount));
	Cost.arcaneDust = FMath::RoundToInt(60 * (1.f - Discount));
	return Cost;
}

bool UCGGameInstance::CanCraft(FString& OutReason) const
{
	TArray<FString> Unowned;
	for (const FCGRelic& R : AllRelics)
	{
		if (!OwnedRelicIds.Contains(R.id)) { Unowned.Add(R.id); }
	}
	if (Unowned.Num() == 0)
	{
		OutReason = TEXT("Every relic known to the guild is already in its vault.");
		return false;
	}
	if (StoredRelicIds().Num() >= RelicStorage())
	{
		OutReason = TEXT("The relic vault is full. Upgrade it for more storage.");
		return false;
	}
	if (!CanAfford(CraftCost()))
	{
		OutReason = TEXT("Cannot afford the materials.");
		return false;
	}
	return true;
}

bool UCGGameInstance::CraftRelic(FString& OutRelicId)
{
	FString Reason;
	if (!CanCraft(Reason)) { return false; }

	TArray<FString> Unowned;
	for (const FCGRelic& R : AllRelics)
	{
		if (!OwnedRelicIds.Contains(R.id)) { Unowned.Add(R.id); }
	}
	Spend(CraftCost());
	OutRelicId = Unowned[FMath::RandRange(0, Unowned.Num() - 1)];
	OwnedRelicIds.Add(OutRelicId);
	const FCGRelic* Relic = FindRelic(OutRelicId);
	AddLedger(FString::Printf(TEXT("The workshop produced: %s."), Relic ? *Relic->name : *OutRelicId));
	SaveGame();
	return true;
}

FCGResourceBundle UCGGameInstance::BrewCost() const
{
	FCGResourceBundle Cost;
	Cost.arcaneDust = 30;
	return Cost;
}

bool UCGGameInstance::BrewPotions(FString& OutSummary)
{
	if (!CanAfford(BrewCost()))
	{
		OutSummary = TEXT("Not enough arcane dust to brew.");
		return false;
	}
	Spend(BrewCost());
	float HealFrac = 0.30f;
	if (const FCGRoom* Lab = FindRoom(TEXT("room_alchemy_lab"))) { HealFrac += Lab->Bonus(); }
	for (FCGMember& M : Roster)
	{
		M.hp = FMath::Min(M.maxHp, M.hp + FMath::CeilToInt(M.maxHp * HealFrac));
		if (M.status == TEXT("injured") && M.hp >= M.maxHp * 0.6f)
		{
			M.status = TEXT("available");
		}
	}
	AddLedger(TEXT("Potions were brewed and bravely swallowed."));
	OutSummary = FString::Printf(TEXT("Potions restore %d%% HP to everyone."), FMath::RoundToInt(HealFrac * 100));
	SaveGame();
	return true;
}

// ----------------------------------------------------------------------
// Expedition math
// ----------------------------------------------------------------------

int32 UCGGameInstance::MoodModifier(const FString& Mood)
{
	if (Mood == TEXT("Ecstatic"))  return 10;
	if (Mood == TEXT("High"))      return 5;
	if (Mood == TEXT("Tired"))     return -10;
	if (Mood == TEXT("Miserable")) return -20;
	return 0;
}

FString UCGGameInstance::MoodShift(const FString& Mood, int32 Steps)
{
	int32 Index = 2;
	for (int32 i = 0; i < UE_ARRAY_COUNT(MoodLadder); ++i)
	{
		if (Mood == MoodLadder[i]) { Index = i; break; }
	}
	Index = FMath::Clamp(Index + Steps, 0, static_cast<int32>(UE_ARRAY_COUNT(MoodLadder)) - 1);
	return MoodLadder[Index];
}

int32 UCGGameInstance::ComputeChanceInternal(const FCGContract& Contract, TArray<FString>* OutLines) const
{
	auto Line = [OutLines](const FString& S) { if (OutLines) { OutLines->Add(S); } };

	// party stats + relic modifiers
	FCGStatBlock Party;
	TSet<FString> PartyTraits;
	TSet<FString> PartyClasses;
	int32 MoodTotal = 0;
	int32 PartyCount = 0;
	for (const FString& Id : SelectedPartyIds)
	{
		if (const FCGMember* M = FindRosterMember(Id))
		{
			Party += M->stats;
			for (const FString& T : M->traits) { PartyTraits.Add(T); }
			PartyClasses.Add(M->className);
			MoodTotal += MoodModifier(M->mood);
			PartyCount++;
		}
	}
	if (PartyCount == 0) { return 5; }

	for (const FString& RelicId : EquippedRelicIds)
	{
		if (const FCGRelic* R = FindRelic(RelicId)) { Party += R->statModifiers; }
	}
	if (const FCGRoom* Ritual = FindRoom(TEXT("room_ritual_chamber")))
	{
		Party.magic += static_cast<int32>(Ritual->Bonus());
	}

	// relevant party power vs contract challenge
	const FCGStatBlock& Ch = Contract.statChallenges;
	int32 PartyPower = 0;
	if (Ch.might > 0)  PartyPower += Party.might;
	if (Ch.wits > 0)   PartyPower += Party.wits;
	if (Ch.magic > 0)  PartyPower += Party.magic;
	if (Ch.guile > 0)  PartyPower += Party.guile;
	if (Ch.spirit > 0) PartyPower += Party.spirit;

	int32 ContractPower = Ch.Total();
	if (MoonPhase() == TEXT("Full Moon") && Ch.magic > 0)
	{
		ContractPower += 2;
		Line(TEXT("Full Moon: hostile magic surges (+2 challenge)"));
	}

	int32 Chance = 50 + (PartyPower - ContractPower) * 5;

	// difficulty
	if (Contract.difficulty == TEXT("Easy"))   { Chance += 10; }
	if (Contract.difficulty == TEXT("Hard"))   { Chance -= 10; }

	// mood (party average)
	const int32 MoodMod = MoodTotal / PartyCount;
	if (MoodMod != 0)
	{
		Chance += MoodMod;
		Line(FString::Printf(TEXT("Party mood %+d%%"), MoodMod));
	}

	// trait interactions
	if (Contract.riskTags.Contains(TEXT("undead")) && PartyTraits.Contains(TEXT("holy")))
	{
		Chance += 10;
		Line(TEXT("Holy vs undead +10%"));
	}
	if (Contract.riskTags.Contains(TEXT("trap")) && PartyTraits.Contains(TEXT("stealthy")))
	{
		Chance += 10;
		Line(TEXT("Stealthy vs traps +10%"));
	}
	if (Contract.riskTags.Contains(TEXT("curse")) && PartyTraits.Contains(TEXT("cursed")))
	{
		Line(TEXT("Cursed kinship: injuries less likely"));
	}

	// class counters
	for (const auto& Pair : ClassRiskCounters())
	{
		if (PartyClasses.Contains(Pair.Key) && Contract.riskTags.Contains(Pair.Value))
		{
			Chance += 8;
			Line(FString::Printf(TEXT("%s counters %s +8%%"), *Pair.Key, *Pair.Value));
		}
	}

	// relic effects
	for (const FString& RelicId : EquippedRelicIds)
	{
		const FCGRelic* R = FindRelic(RelicId);
		if (!R) { continue; }
		int32 RelicMod = FMath::RoundToInt(R->EffectValue(TEXT("success_modifier")));
		static const TCHAR* TagMods[] = { TEXT("undead"), TEXT("poison"), TEXT("fire"), TEXT("shadow"), TEXT("ambush"), TEXT("trap"), TEXT("curse") };
		for (const TCHAR* Tag : TagMods)
		{
			if (Contract.riskTags.Contains(Tag))
			{
				RelicMod += FMath::RoundToInt(R->EffectValue(FString::Printf(TEXT("%s_success_modifier"), Tag)));
			}
		}
		if (MoonPhase() == TEXT("Full Moon"))
		{
			RelicMod += FMath::RoundToInt(R->EffectValue(TEXT("full_moon_success_modifier")));
		}
		if (RelicMod != 0)
		{
			Chance += RelicMod;
			Line(FString::Printf(TEXT("%s %+d%%"), *R->name, RelicMod));
		}
	}

	// moon: stealth under new moon
	if (MoonPhase() == TEXT("New Moon") && Ch.guile > 0)
	{
		Chance += 8;
		Line(TEXT("New Moon favors stealth +8%"));
	}

	// special contract modifiers
	if (Contract.specialModifierId == TEXT("consecrated_ground") && PartyTraits.Contains(TEXT("holy")))
	{
		Chance += 8;
		Line(TEXT("Consecrated Ground: holy +8%"));
	}
	if (Contract.specialModifierId == TEXT("living_metal") && PartyTraits.Contains(TEXT("crafter")))
	{
		Chance += 8;
		Line(TEXT("Living Metal: crafter +8%"));
	}
	if (Contract.specialModifierId == TEXT("eye_of_ruin") && Party.spirit < 8)
	{
		Chance -= 8;
		Line(TEXT("Eye of Ruin: weak spirits falter -8%"));
	}

	return FMath::Clamp(Chance, 5, 95);
}

FCGPreview UCGGameInstance::ComputePreview() const
{
	FCGPreview Preview;
	const FCGContract* Contract = FindContract(SelectedContractId);
	if (!Contract) { return Preview; }

	for (const FString& Id : SelectedPartyIds)
	{
		if (const FCGMember* M = FindRosterMember(Id)) { Preview.PartyStats += M->stats; }
	}
	for (const FString& RelicId : EquippedRelicIds)
	{
		if (const FCGRelic* R = FindRelic(RelicId)) { Preview.PartyStats += R->statModifiers; }
	}

	Preview.ContractPower = Contract->statChallenges.Total();
	Preview.FinalChance = ComputeChanceInternal(*Contract, &Preview.ModifierLines);

	const FCGStatBlock& Ch = Contract->statChallenges;
	const FCGStatBlock& P = Preview.PartyStats;
	Preview.PartyPower =
		(Ch.might > 0 ? P.might : 0) + (Ch.wits > 0 ? P.wits : 0) + (Ch.magic > 0 ? P.magic : 0) +
		(Ch.guile > 0 ? P.guile : 0) + (Ch.spirit > 0 ? P.spirit : 0);

	const int32 C = Preview.FinalChance;
	Preview.Band = C >= 75 ? TEXT("Safe") : C >= 50 ? TEXT("Risky") : C >= 25 ? TEXT("Dangerous") : TEXT("Suicidal");

	// Library fuzzes or sharpens the number
	const FCGRoom* Library = FindRoom(TEXT("room_library"));
	const int32 LibLevel = Library ? Library->level : 1;
	if (SelectedPartyIds.Num() == 0)
	{
		Preview.DisplayChance = TEXT("--");
	}
	else if (LibLevel >= 3)
	{
		Preview.DisplayChance = FString::Printf(TEXT("%d%%"), C);
	}
	else
	{
		Preview.DisplayChance = FString::Printf(TEXT("~%d%%"), FMath::RoundToInt(C / 5.f) * 5);
	}

	// warning text
	TArray<FString> Risks = Contract->riskTags;
	if (Risks.Num() > 0)
	{
		FString RiskList;
		for (int32 i = 0; i < Risks.Num(); ++i)
		{
			FString Pretty = Risks[i];
			if (Pretty.Len() > 0) { Pretty[0] = FChar::ToUpper(Pretty[0]); }
			RiskList += Pretty + (i + 1 < Risks.Num() ? TEXT(", ") : TEXT(""));
		}
		FString Advice;
		if (Risks.Contains(TEXT("undead")) || Risks.Contains(TEXT("curse")))
		{
			Advice = TEXT(" Holy or magical guild members are recommended.");
		}
		else if (Risks.Contains(TEXT("poison")))
		{
			Advice = TEXT(" Bring a healer or herbalist.");
		}
		else if (Risks.Contains(TEXT("trap")) || Risks.Contains(TEXT("ambush")))
		{
			Advice = TEXT(" Stealthy and clever members fare better.");
		}
		else if (Risks.Contains(TEXT("elite")))
		{
			Advice = TEXT(" An elite foe awaits. Bring your strongest.");
		}
		Preview.Warning = FString::Printf(TEXT("Warning: this contract carries %s risks.%s"), *RiskList, *Advice);
	}
	return Preview;
}

float UCGGameInstance::InjuryChance(const FCGContract& Contract, float Base) const
{
	float Chance = Base;
	for (const FString& RelicId : EquippedRelicIds)
	{
		if (const FCGRelic* R = FindRelic(RelicId))
		{
			Chance += R->EffectValue(TEXT("injury_chance_modifier"));
			if (Contract.riskTags.Contains(TEXT("shadow")))
			{
				Chance += R->EffectValue(TEXT("shadow_injury_modifier"));
			}
		}
	}
	if (Contract.riskTags.Contains(TEXT("curse")))
	{
		bool bCursedParty = false;
		for (const FString& Id : SelectedPartyIds)
		{
			const FCGMember* M = FindRosterMember(Id);
			if (M && M->traits.Contains(TEXT("cursed"))) { bCursedParty = true; }
		}
		if (bCursedParty) { Chance -= 0.05f; }
		if (MoonPhase() == TEXT("Full Moon")) { Chance += 0.10f; }
		if (const FCGRoom* Ritual = FindRoom(TEXT("room_ritual_chamber")))
		{
			Chance += 0.05f * Ritual->level;
		}
		if (Contract.specialModifierId == TEXT("cursed_waters")) { Chance += 0.10f; }
	}
	return FMath::Clamp(Chance, 0.f, 1.f);
}

void UCGGameInstance::ApplyXP(FCGMember& M, int32 XP, TArray<FString>& OutLines)
{
	M.xp += XP;
	while (M.xp >= M.level * 100)
	{
		M.xp -= M.level * 100;
		M.level++;
		M.maxHp += 3;
		M.hp = FMath::Min(M.maxHp, M.hp + 3);
		const FString Primary = PrimaryStatForClass(M.className);
		if (Primary == TEXT("might"))  M.stats.might++;
		else if (Primary == TEXT("magic"))  M.stats.magic++;
		else if (Primary == TEXT("guile"))  M.stats.guile++;
		else if (Primary == TEXT("spirit")) M.stats.spirit++;
		else M.stats.wits++;
		OutLines.Add(FString::Printf(TEXT("%s reached level %d!"), *M.name, M.level));
	}
}

bool UCGGameInstance::CanEmbark(FString& OutReason) const
{
	const FCGContract* Contract = FindContract(SelectedContractId);
	if (!Contract)
	{
		OutReason = TEXT("Choose a contract from the board.");
		return false;
	}
	if (SelectedPartyIds.Num() < 1 || SelectedPartyIds.Num() > 3)
	{
		OutReason = TEXT("Assign 1-3 guild members.");
		return false;
	}
	for (const FString& Id : SelectedPartyIds)
	{
		const FCGMember* M = FindRosterMember(Id);
		if (!M || !M->CanEmbark())
		{
			OutReason = TEXT("A selected member cannot travel.");
			return false;
		}
	}
	if (Resources.supplies < Contract->requiredSupplies)
	{
		OutReason = FString::Printf(TEXT("Requires %d supplies."), Contract->requiredSupplies);
		return false;
	}
	return true;
}

FCGExpeditionResult UCGGameInstance::Embark()
{
	FCGExpeditionResult Result;
	FString Reason;
	if (!CanEmbark(Reason))
	{
		Result.OutcomeTitle = TEXT("Cannot Embark");
		Result.Flavor = Reason;
		return Result;
	}
	const FCGContract Contract = *FindContract(SelectedContractId); // copy: board mutates below

	Resources.supplies -= Contract.requiredSupplies;

	const int32 Chance = ComputeChanceInternal(Contract, nullptr);
	const int32 Roll = FMath::RandRange(1, 100);
	Result.Roll = Roll;
	Result.Chance = Chance;

	enum class EOutcome { Great, Success, Partial, Failure, Disaster };
	EOutcome Outcome;
	if (Roll <= Chance - 25)      { Outcome = EOutcome::Great; }
	else if (Roll <= Chance)      { Outcome = EOutcome::Success; }
	else if (Roll <= Chance + 15) { Outcome = EOutcome::Partial; }
	else if (Roll <= Chance + 35) { Outcome = EOutcome::Failure; }
	else                          { Outcome = EOutcome::Disaster; }

	float RewardMult = 0.f, XPMult = 0.f, InjuryBase = 0.f, RelicMult = 0.f;
	int32 MoodDelta = 0;
	switch (Outcome)
	{
	case EOutcome::Great:    Result.OutcomeTitle = TEXT("Great Success!"); RewardMult = 1.f;  XPMult = 1.5f;  InjuryBase = 0.f;   RelicMult = 1.5f; MoodDelta = 1;  break;
	case EOutcome::Success:  Result.OutcomeTitle = TEXT("Success");        RewardMult = 1.f;  XPMult = 1.f;   InjuryBase = 0.15f; RelicMult = 1.f;  MoodDelta = 0;  break;
	case EOutcome::Partial:  Result.OutcomeTitle = TEXT("Partial Success");RewardMult = 0.6f; XPMult = 0.6f;  InjuryBase = 0.35f; RelicMult = 0.5f; MoodDelta = 0;  break;
	case EOutcome::Failure:  Result.OutcomeTitle = TEXT("Failure");        RewardMult = 0.2f; XPMult = 0.3f;  InjuryBase = 0.7f;  RelicMult = 0.f;  MoodDelta = -1; break;
	case EOutcome::Disaster: Result.OutcomeTitle = TEXT("Disaster!");      RewardMult = 0.f;  XPMult = 0.15f; InjuryBase = 1.f;   RelicMult = 0.f;  MoodDelta = -2; break;
	}

	// ---- rewards ----
	float GoldMult = RewardMult, RepMult = RewardMult, DustMult = RewardMult;
	if (Outcome == EOutcome::Failure) { RepMult = 0.f; }
	for (const FString& RelicId : EquippedRelicIds)
	{
		if (const FCGRelic* R = FindRelic(RelicId))
		{
			GoldMult *= 1.f + R->EffectValue(TEXT("gold_bonus"));
			RepMult *= 1.f - R->EffectValue(TEXT("rep_penalty"));
		}
	}
	if (MoonPhase() == TEXT("Waxing Moon")) { RepMult *= 1.05f; }
	if (Contract.specialModifierId == TEXT("kingly_hoard")) { GoldMult *= 1.25f; }
	if (Contract.specialModifierId == TEXT("the_hollow_bell")) { RepMult *= 1.2f; }

	Result.Rewards.gold = FMath::RoundToInt(Contract.rewards.gold * GoldMult);
	Result.Rewards.reputation = FMath::RoundToInt(Contract.rewards.reputation * RepMult);
	Result.Rewards.arcaneDust = FMath::RoundToInt(Contract.rewards.arcaneDust * DustMult);

	const bool bWon = Outcome == EOutcome::Great || Outcome == EOutcome::Success || Outcome == EOutcome::Partial;
	if (bWon)
	{
		bool bScholar = false;
		for (const FString& Id : SelectedPartyIds)
		{
			const FCGMember* M = FindRosterMember(Id);
			if (M && M->traits.Contains(TEXT("scholar"))) { bScholar = true; }
		}
		if (Contract.specialModifierId == TEXT("venom_harvest")) { Result.Rewards.arcaneDust += 50; }
		if (Contract.specialModifierId == TEXT("forbidden_pages") && bScholar) { Result.Rewards.arcaneDust += 40; }
	}

	if (Outcome == EOutcome::Disaster)
	{
		Result.ReputationLost = FMath::Max(25, Contract.rewards.reputation / 10);
		Resources.reputation = FMath::Max(0, Resources.reputation - Result.ReputationLost);
	}

	Resources.gold += Result.Rewards.gold;
	Resources.reputation += Result.Rewards.reputation;
	Resources.arcaneDust += Result.Rewards.arcaneDust;

	// ---- XP ----
	TArray<FString> LevelLines;
	Result.XPGained = FMath::RoundToInt(Contract.statChallenges.Total() * 8 * XPMult);
	for (const FString& Id : SelectedPartyIds)
	{
		if (FCGMember* M = FindRosterMember(Id)) { ApplyXP(*M, Result.XPGained, LevelLines); }
	}

	// ---- injuries & mood ----
	float HealBonus = 0.f;
	for (const FString& RelicId : EquippedRelicIds)
	{
		if (const FCGRelic* R = FindRelic(RelicId)) { HealBonus += R->EffectValue(TEXT("heal_bonus")); }
	}

	const float InjChance = InjuryChance(Contract, InjuryBase);
	bool bAnyUnavailable = false;
	for (const FString& Id : SelectedPartyIds)
	{
		FCGMember* M = FindRosterMember(Id);
		if (!M) { continue; }
		M->mood = MoodShift(M->mood, MoodDelta);

		if (Outcome == EOutcome::Disaster)
		{
			const bool bWorst = !bAnyUnavailable && (FMath::FRand() < 0.6f || Id == SelectedPartyIds.Last());
			M->hp = FMath::Max(1, FMath::RoundToInt(M->maxHp * FMath::FRandRange(0.05f, 0.25f)));
			if (bWorst)
			{
				bAnyUnavailable = true;
				M->status = TEXT("unavailable");
				M->daysUnavailable = 3;
				Result.InjuryLines.Add(FString::Printf(TEXT("%s was carried home and needs 3 days to recover."), *M->name));
			}
			else
			{
				M->status = TEXT("wounded");
				M->daysUnavailable = 2;
				Result.InjuryLines.Add(FString::Printf(TEXT("%s was gravely wounded."), *M->name));
			}
		}
		else if (FMath::FRand() < InjChance)
		{
			const bool bSevere = Outcome == EOutcome::Failure && FMath::FRand() < 0.5f;
			const float Loss = bSevere ? 0.5f : FMath::FRandRange(0.2f, 0.35f);
			M->hp = FMath::Max(1, M->hp - FMath::RoundToInt(M->maxHp * Loss));
			if (bSevere)
			{
				M->status = TEXT("wounded");
				M->daysUnavailable = 2;
				Result.InjuryLines.Add(FString::Printf(TEXT("%s was badly wounded."), *M->name));
			}
			else
			{
				M->status = TEXT("injured");
				Result.InjuryLines.Add(FString::Printf(TEXT("%s took an injury."), *M->name));
			}
		}
		else if (bWon && HealBonus > 0.f)
		{
			M->hp = FMath::Min(M->maxHp, M->hp + FMath::CeilToInt(M->maxHp * HealBonus));
		}
	}
	Result.InjuryLines.Append(LevelLines);

	// ---- relic find ----
	if (RelicMult > 0.f && FMath::FRand() < Contract.relicChance * RelicMult)
	{
		TArray<FString> Unowned, UnownedCursed;
		for (const FCGRelic& R : AllRelics)
		{
			if (OwnedRelicIds.Contains(R.id)) { continue; }
			Unowned.Add(R.id);
			if (R.rarity == TEXT("Cursed") || R.rarity == TEXT("Legendary")) { UnownedCursed.Add(R.id); }
		}
		const bool bPreferCursed = Contract.specialModifierId == TEXT("hidden_cursed_relic") && UnownedCursed.Num() > 0;
		TArray<FString>& Pool = bPreferCursed ? UnownedCursed : Unowned;
		if (Pool.Num() > 0 && StoredRelicIds().Num() < RelicStorage())
		{
			Result.RelicFoundId = Pool[FMath::RandRange(0, Pool.Num() - 1)];
			OwnedRelicIds.Add(Result.RelicFoundId);
		}
	}

	// ---- flavor text ----
	const FString Lead = SelectedPartyIds.Num() > 0 && FindRosterMember(SelectedPartyIds[0])
		? FindRosterMember(SelectedPartyIds[0])->name : TEXT("The party");
	const FString Second = SelectedPartyIds.Num() > 1 && FindRosterMember(SelectedPartyIds[1])
		? FindRosterMember(SelectedPartyIds[1])->name : FString();
	switch (Outcome)
	{
	case EOutcome::Great:
		Result.Flavor = Second.IsEmpty()
			? FString::Printf(TEXT("%s swept through %s without a scratch. The guild's name is spoken with new respect in %s."), *Lead, *Contract.title, *Contract.biome)
			: FString::Printf(TEXT("%s held the line while %s finished the job. The guild recovered everything of value and gained favor in %s."), *Lead, *Second, *Contract.biome);
		break;
	case EOutcome::Success:
		Result.Flavor = FString::Printf(TEXT("The contract \"%s\" was fulfilled. %s led the party home, tired but paid."), *Contract.title, *Lead);
		break;
	case EOutcome::Partial:
		Result.Flavor = FString::Printf(TEXT("The job got done — mostly. Half the promised prize was lost somewhere in %s, along with some of the party's confidence."), *Contract.biome);
		break;
	case EOutcome::Failure:
		Result.Flavor = FString::Printf(TEXT("%s called the retreat. The client kept most of the fee, and %s keeps its dead."), *Lead, *Contract.biome);
		break;
	case EOutcome::Disaster:
		Result.Flavor = FString::Printf(TEXT("It went wrong fast. What crawled out of %s was not on the contract, and the guild's name is now a cautionary tale."), *Contract.biome);
		break;
	}

	// ---- bookkeeping ----
	ExpeditionsRun++;
	UsedContractIds.Add(Contract.id);
	BoardContractIds.Remove(Contract.id);
	RefreshBoard(false);
	SelectedContractId = BoardContractIds.Num() > 0 ? BoardContractIds[0] : FString();
	SelectedPartyIds.Reset();

	AddLedger(FString::Printf(TEXT("%s — %s. Gained %d gold, %d reputation, %d dust."),
		*Contract.title, *Result.OutcomeTitle, Result.Rewards.gold, Result.Rewards.reputation, Result.Rewards.arcaneDust));

	AdvanceDays(Contract.lengthDays);
	SaveGame();
	return Result;
}

// ----------------------------------------------------------------------
// Tutorial
// ----------------------------------------------------------------------

FString UCGGameInstance::TutorialText() const
{
	static const TCHAR* Steps[NumTutorialSteps] = {
		TEXT("This is your guild roster. Click a member's portrait to add them to the expedition party."),
		TEXT("Choose a contract from the board — the tabs above the parchment switch between offers."),
		TEXT("Assign up to 3 guild members. Their combined stats decide the outcome."),
		TEXT("Check the estimated success chance in the Expedition Preview below the contract."),
		TEXT("Equip relics in the Relics tab on the right, if any are available."),
		TEXT("When you are ready, click EMBARK to send the party out."),
		TEXT("Use your rewards to upgrade rooms, craft relics, and recruit new members. Good luck."),
	};
	return TutorialStep >= 0 && TutorialStep < NumTutorialSteps ? Steps[TutorialStep] : FString();
}

void UCGGameInstance::AdvanceTutorial()
{
	if (TutorialStep < NumTutorialSteps)
	{
		TutorialStep++;
		SaveGame();
	}
}

void UCGGameInstance::AddLedger(const FString& Line)
{
	Ledger.Insert(FString::Printf(TEXT("W%d D%d — %s"), Week, Day, *Line), 0);
	if (Ledger.Num() > 200) { Ledger.SetNum(200); }
}

// ----------------------------------------------------------------------
// Sprites & fonts
// ----------------------------------------------------------------------

UTexture2D* UCGGameInstance::GetOrLoadTexture(const FString& SpriteId)
{
	if (TObjectPtr<UTexture2D>* Found = LoadedTextures.Find(SpriteId))
	{
		return *Found;
	}
	const FString* Path = SpritePaths.Find(SpriteId);
	if (!Path)
	{
		UE_LOG(LogTemp, Warning, TEXT("CursedGuild: unknown sprite id '%s'"), *SpriteId);
		return nullptr;
	}
	UTexture2D* Tex = FImageUtils::ImportFileAsTexture2D(*Path);
	if (Tex)
	{
		LoadedTextures.Add(SpriteId, Tex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CursedGuild: failed to load sprite '%s' from %s"), *SpriteId, **Path);
	}
	return Tex;
}

const FSlateBrush* UCGGameInstance::GetBrush(const FString& SpriteId)
{
	if (TSharedPtr<FSlateBrush>* Found = BrushCache.Find(SpriteId))
	{
		return Found->Get();
	}
	UTexture2D* Tex = GetOrLoadTexture(SpriteId);
	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	if (Tex)
	{
		Brush->SetResourceObject(Tex);
		Brush->ImageSize = FVector2D(Tex->GetSizeX(), Tex->GetSizeY());
		Brush->DrawAs = ESlateBrushDrawType::Image;
	}
	else
	{
		Brush->DrawAs = ESlateBrushDrawType::NoDrawType;
	}
	BrushCache.Add(SpriteId, Brush);
	return Brush.Get();
}

const FSlateBrush* UCGGameInstance::GetBoxBrush(const FString& SpriteId, float MarginFraction)
{
	const FString Key = FString::Printf(TEXT("%s__box%.2f"), *SpriteId, MarginFraction);
	if (TSharedPtr<FSlateBrush>* Found = BrushCache.Find(Key))
	{
		return Found->Get();
	}
	UTexture2D* Tex = GetOrLoadTexture(SpriteId);
	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	if (Tex)
	{
		Brush->SetResourceObject(Tex);
		Brush->ImageSize = FVector2D(Tex->GetSizeX(), Tex->GetSizeY());
		Brush->DrawAs = ESlateBrushDrawType::Box;
		Brush->Margin = FMargin(MarginFraction);
	}
	else
	{
		Brush->DrawAs = ESlateBrushDrawType::NoDrawType;
	}
	BrushCache.Add(Key, Brush);
	return Brush.Get();
}

FSlateFontInfo UCGGameInstance::GetFont(int32 Size, bool bBold) const
{
	const TSharedPtr<FStandaloneCompositeFont>& Composite = bBold ? BoldFont : RegularFont;
	if (Composite.IsValid())
	{
		return FSlateFontInfo(Composite, static_cast<float>(Size), TEXT("Regular"));
	}
	return FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size);
}
