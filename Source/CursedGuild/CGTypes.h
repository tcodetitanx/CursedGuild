#pragma once

#include "CoreMinimal.h"
#include "CGTypes.generated.h"

USTRUCT()
struct FCGResourceBundle
{
	GENERATED_BODY()

	UPROPERTY() int32 gold = 0;
	UPROPERTY() int32 reputation = 0;
	UPROPERTY() int32 supplies = 0;
	UPROPERTY() int32 arcaneDust = 0;

	bool IsZero() const { return gold == 0 && reputation == 0 && supplies == 0 && arcaneDust == 0; }
};

USTRUCT()
struct FCGStatBlock
{
	GENERATED_BODY()

	UPROPERTY() int32 might = 0;
	UPROPERTY() int32 wits = 0;
	UPROPERTY() int32 magic = 0;
	UPROPERTY() int32 guile = 0;
	UPROPERTY() int32 spirit = 0;

	int32 Total() const { return might + wits + magic + guile + spirit; }

	FCGStatBlock& operator+=(const FCGStatBlock& O)
	{
		might += O.might; wits += O.wits; magic += O.magic; guile += O.guile; spirit += O.spirit;
		return *this;
	}
};

USTRUCT()
struct FCGRelicEffect
{
	GENERATED_BODY()

	UPROPERTY() FString type;
	UPROPERTY() float value = 0.f;
};

USTRUCT()
struct FCGMember
{
	GENERATED_BODY()

	UPROPERTY() FString id;
	UPROPERTY() FString name;
	UPROPERTY() FString className;
	UPROPERTY() int32 level = 1;
	UPROPERTY() int32 xp = 0;
	UPROPERTY() int32 hp = 10;
	UPROPERTY() int32 maxHp = 10;
	UPROPERTY() FString mood = TEXT("Content");
	UPROPERTY() FString portraitSpriteId;
	UPROPERTY() FString bodySpriteId;
	UPROPERTY() TArray<FString> traits;
	UPROPERTY() TArray<FString> skills;
	UPROPERTY() FCGStatBlock stats;
	UPROPERTY() FString status = TEXT("available"); // available|injured|wounded|unavailable
	UPROPERTY() int32 daysUnavailable = 0;
	UPROPERTY() bool starting = false;
	UPROPERTY() int32 hireCostGold = 0;
	UPROPERTY() int32 hireCostRep = 0;

	bool CanEmbark() const { return status == TEXT("available") || status == TEXT("injured"); }
};

USTRUCT()
struct FCGContract
{
	GENERATED_BODY()

	UPROPERTY() FString id;
	UPROPERTY() FString title;
	UPROPERTY() FString description;
	UPROPERTY() FString biome;
	UPROPERTY() FString difficulty; // Easy|Medium|Hard
	UPROPERTY() int32 lengthDays = 1;
	UPROPERTY() int32 requiredSupplies = 0;
	UPROPERTY() TArray<FString> riskTags;
	UPROPERTY() TArray<FString> enemyTags;
	UPROPERTY() FCGStatBlock statChallenges;
	UPROPERTY() FCGResourceBundle rewards;
	UPROPERTY() float relicChance = 0.f;
	UPROPERTY() FString specialModifierId;
	UPROPERTY() FString specialModifierName;
	UPROPERTY() FString specialModifierDesc;
};

USTRUCT()
struct FCGRelic
{
	GENERATED_BODY()

	UPROPERTY() FString id;
	UPROPERTY() FString name;
	UPROPERTY() FString rarity; // Common|Uncommon|Rare|Cursed|Legendary
	UPROPERTY() FString iconSpriteId;
	UPROPERTY() FString description;
	UPROPERTY() TArray<FString> tags;
	UPROPERTY() FCGStatBlock statModifiers;
	UPROPERTY() TArray<FCGRelicEffect> effects;

	float EffectValue(const FString& Type) const
	{
		for (const FCGRelicEffect& E : effects)
		{
			if (E.type == Type) { return E.value; }
		}
		return 0.f;
	}
};

USTRUCT()
struct FCGRoom
{
	GENERATED_BODY()

	UPROPERTY() FString id;
	UPROPERTY() FString name;
	UPROPERTY() FString description;
	UPROPERTY() int32 level = 1;
	UPROPERTY() int32 maxLevel = 5;
	UPROPERTY() FString iconSpriteId;
	UPROPERTY() bool isUnlocked = true;
	UPROPERTY() FString effectType;
	UPROPERTY() float effectPerLevel = 0.f;
	UPROPERTY() FString effectDesc;
	UPROPERTY() TArray<FCGResourceBundle> upgradeCosts; // index = current level

	float Bonus() const { return isUnlocked ? effectPerLevel * level : 0.f; }
};

USTRUCT()
struct FCGEnemy
{
	GENERATED_BODY()

	UPROPERTY() FString id;
	UPROPERTY() FString tag;
	UPROPERTY() FString name;
	UPROPERTY() FString spriteId;
};

USTRUCT()
struct FCGSpriteDef
{
	GENERATED_BODY()

	UPROPERTY() FString id;
	UPROPERTY() FString path;
};

// ---- JSON file wrappers ----

USTRUCT()
struct FCGMemberList { GENERATED_BODY() UPROPERTY() TArray<FCGMember> members; };

USTRUCT()
struct FCGContractList { GENERATED_BODY() UPROPERTY() TArray<FCGContract> contracts; };

USTRUCT()
struct FCGRelicList { GENERATED_BODY() UPROPERTY() TArray<FCGRelic> relics; };

USTRUCT()
struct FCGRoomList { GENERATED_BODY() UPROPERTY() TArray<FCGRoom> rooms; };

USTRUCT()
struct FCGEnemyList { GENERATED_BODY() UPROPERTY() TArray<FCGEnemy> enemies; };

USTRUCT()
struct FCGSpriteList { GENERATED_BODY() UPROPERTY() TArray<FCGSpriteDef> sprites; };

// ---- Runtime helper structs ----

USTRUCT()
struct FCGRoomSave
{
	GENERATED_BODY()

	UPROPERTY() FString id;
	UPROPERTY() int32 level = 0;
	UPROPERTY() bool isUnlocked = true;
};

USTRUCT()
struct FCGSaveData
{
	GENERATED_BODY()

	UPROPERTY() FCGResourceBundle resources;
	UPROPERTY() int32 week = 1;
	UPROPERTY() int32 day = 1;
	UPROPERTY() TArray<FCGMember> roster;
	UPROPERTY() TArray<FString> ownedRelicIds;
	UPROPERTY() TArray<FString> equippedRelicIds;
	UPROPERTY() TArray<FCGRoomSave> rooms;
	UPROPERTY() TArray<FString> boardContractIds;
	UPROPERTY() TArray<FString> usedContractIds;
	UPROPERTY() TArray<FString> ledger;
	UPROPERTY() int32 tutorialStep = 0;
	UPROPERTY() int32 expeditionsRun = 0;
};

// Expedition preview computed for the current contract/party selection.
struct FCGPreview
{
	FCGStatBlock PartyStats;
	int32 PartyPower = 0;
	int32 ContractPower = 0;
	int32 FinalChance = 0;
	FString Band;         // Safe | Risky | Dangerous | Suicidal
	FString DisplayChance; // possibly fuzzed by Library level
	FString Warning;
	TArray<FString> ModifierLines;
};

struct FCGExpeditionResult
{
	FString OutcomeTitle;   // Great Success! etc
	FString Flavor;
	FCGResourceBundle Rewards;
	int32 XPGained = 0;
	TArray<FString> InjuryLines;
	FString RelicFoundId;
	int32 ReputationLost = 0;
	int32 Roll = 0;
	int32 Chance = 0;
};
