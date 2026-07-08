#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Styling/SlateBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "CGTypes.h"
#include "CGGameInstance.generated.h"

class UTexture2D;

/**
 * Owns all game state and systems for Cursed Guild:
 * data loading, resources, roster, contracts, relics, rooms,
 * expedition resolution, calendar, save/load, and the sprite library.
 */
UCLASS()
class CURSEDGUILD_API UCGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	// ------------------------------------------------------------------
	// Static data (loaded from Content/Data/*.json)
	// ------------------------------------------------------------------
	TArray<FCGMember>   AllMembers;
	TArray<FCGContract> AllContracts;
	TArray<FCGRelic>    AllRelics;
	TArray<FCGRoom>     DefaultRooms;
	TArray<FCGEnemy>    AllEnemies;

	const FCGContract* FindContract(const FString& Id) const;
	const FCGRelic*    FindRelic(const FString& Id) const;
	const FCGEnemy*    FindEnemyByTag(const FString& Tag) const;
	FCGMember*         FindRosterMember(const FString& Id);
	const FCGMember*   FindRosterMember(const FString& Id) const;
	FCGRoom*           FindRoom(const FString& Id);
	const FCGRoom*     FindRoom(const FString& Id) const;

	// ------------------------------------------------------------------
	// Run state
	// ------------------------------------------------------------------
	bool bInGame = false;
	FCGResourceBundle Resources;
	int32 Week = 1;
	int32 Day = 1;
	TArray<FCGMember> Roster;
	TArray<FString>   OwnedRelicIds;
	TArray<FString>   EquippedRelicIds;
	TArray<FCGRoom>   Rooms;
	TArray<FString>   BoardContractIds;
	TArray<FString>   UsedContractIds;
	TArray<FString>   Ledger;
	int32 TutorialStep = 0;
	int32 ExpeditionsRun = 0;

	// UI session selection
	FString SelectedContractId;
	TArray<FString> SelectedPartyIds;

	// ------------------------------------------------------------------
	// Game flow
	// ------------------------------------------------------------------
	void NewGame();
	bool SaveGame();
	bool LoadGame();
	bool HasSaveFile() const;

	// Calendar
	FString MoonPhase() const;            // Waxing Moon / Full Moon / Waning Moon / New Moon
	FString MoonPhaseHint() const;
	void AdvanceDays(int32 NumDays);

	// Roster
	int32 RosterCapacity() const;
	TArray<const FCGMember*> RecruitCandidates() const;
	bool CanHire(const FCGMember& Candidate, FString& OutReason) const;
	bool Hire(const FString& MemberId);
	bool RestGuild(FString& OutSummary);
	int32 RestSupplyCost() const { return 5; }

	// Contracts / party
	void RefreshBoard(bool bFullRefresh);
	void SelectContract(const FString& ContractId);
	bool TogglePartyMember(const FString& MemberId);
	bool IsInParty(const FString& MemberId) const { return SelectedPartyIds.Contains(MemberId); }

	// Relics
	int32 RelicSlots() const;
	int32 RelicStorage() const;
	TArray<FString> StoredRelicIds() const;
	bool EquipRelic(const FString& RelicId);
	bool UnequipRelic(const FString& RelicId);

	// Rooms
	bool CanUpgradeRoom(const FCGRoom& Room, FString& OutReason) const;
	bool UpgradeRoom(const FString& RoomId);

	// Economy actions
	FCGResourceBundle CraftCost() const;
	bool CanCraft(FString& OutReason) const;
	bool CraftRelic(FString& OutRelicId);
	FCGResourceBundle BrewCost() const;
	bool BrewPotions(FString& OutSummary);

	bool CanAfford(const FCGResourceBundle& Cost) const;
	void Spend(const FCGResourceBundle& Cost);

	// Expedition
	FCGPreview ComputePreview() const;
	bool CanEmbark(FString& OutReason) const;
	FCGExpeditionResult Embark();

	// Tutorial
	static const int32 NumTutorialSteps = 7;
	FString TutorialText() const;
	void AdvanceTutorial();
	bool TutorialActive() const { return bInGame && TutorialStep < NumTutorialSteps; }

	void AddLedger(const FString& Line);

	// Audio hook points (placeholder for MVP — no audio assets yet)
	void PlayUISound(const FString& Cue) {}

	// ------------------------------------------------------------------
	// Sprites & fonts
	// ------------------------------------------------------------------
	const FSlateBrush* GetBrush(const FString& SpriteId);
	const FSlateBrush* GetBoxBrush(const FString& SpriteId, float MarginFraction);
	FSlateFontInfo GetFont(int32 Size, bool bBold = false) const;

private:
	void LoadStaticData();
	void LoadSpriteManifest();
	UTexture2D* GetOrLoadTexture(const FString& SpriteId);

	int32 ComputeChanceInternal(const FCGContract& Contract, TArray<FString>* OutLines) const;
	float InjuryChance(const FCGContract& Contract, float Base) const;
	void ApplyXP(FCGMember& M, int32 XP, TArray<FString>& OutLines);
	static FString MoodShift(const FString& Mood, int32 Steps);
	static int32 MoodModifier(const FString& Mood);
	FString SavePath() const;

	// sprite id -> content-relative png path
	TMap<FString, FString> SpritePaths;

	UPROPERTY()
	TMap<FString, TObjectPtr<UTexture2D>> LoadedTextures;

	TMap<FString, TSharedPtr<FSlateBrush>> BrushCache;

	TSharedPtr<struct FStandaloneCompositeFont> RegularFont;
	TSharedPtr<struct FStandaloneCompositeFont> BoldFont;
};
