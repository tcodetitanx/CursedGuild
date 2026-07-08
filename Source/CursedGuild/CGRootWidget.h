#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Styling/SlateTypes.h"
#include "CGTypes.h"

class UCGGameInstance;
class SBox;

/**
 * The entire Cursed Guild UI as one code-built Slate widget.
 * The tree is a pure function of game state: every interaction mutates the
 * game instance and calls Rebuild().
 */
class SCGRoot : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCGRoot) : _GameInstance(nullptr) {}
		SLATE_ARGUMENT(UCGGameInstance*, GameInstance)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void Rebuild();

private:
	enum class EScreen { MainMenu, Hub };
	enum class EModal { None, Result, Recruit, Ledger, Settings, MemberDetail, Message };

	TWeakObjectPtr<UCGGameInstance> GI;
	EScreen Screen = EScreen::MainMenu;
	EModal Modal = EModal::None;
	int32 RightTab = 0; // 0 Guild, 1 Rooms, 2 Relics
	FCGExpeditionResult LastResult;
	FString DetailMemberId;
	FString MessageTitle;
	FString MessageBody;

	TSharedPtr<SBox> Container;

	// persistent button styles (Slate keeps references to these)
	FButtonStyle DarkButton;
	FButtonStyle GoldButton;
	FButtonStyle TabButton;
	FButtonStyle TabButtonActive;
	FButtonStyle CardButton;
	FButtonStyle CardButtonSelected;

	// ---- screens ----
	TSharedRef<SWidget> BuildMainMenu();
	TSharedRef<SWidget> BuildHub();

	// ---- hub sections ----
	TSharedRef<SWidget> BuildTopBar();
	TSharedRef<SWidget> BuildRosterPanel();
	TSharedRef<SWidget> BuildMemberCard(const FCGMember& M);
	TSharedRef<SWidget> BuildCenterPanel();
	TSharedRef<SWidget> BuildContractSheet(const FCGContract& C);
	TSharedRef<SWidget> BuildPreviewStrip(const FCGContract& C);
	TSharedRef<SWidget> BuildRightPanel();
	TSharedRef<SWidget> BuildGuildTab();
	TSharedRef<SWidget> BuildRoomsTab();
	TSharedRef<SWidget> BuildRelicsTab();
	TSharedRef<SWidget> BuildBottomBar();

	// ---- overlays ----
	TSharedRef<SWidget> BuildModalOverlay();
	TSharedRef<SWidget> BuildResultModal();
	TSharedRef<SWidget> BuildRecruitModal();
	TSharedRef<SWidget> BuildLedgerModal();
	TSharedRef<SWidget> BuildSettingsModal();
	TSharedRef<SWidget> BuildMemberDetailModal();
	TSharedRef<SWidget> BuildMessageModal();
	TSharedRef<SWidget> BuildTutorialPopup();

	// ---- helpers ----
	const FSlateBrush* Brush(const FString& Id) const;
	const FSlateBrush* BoxBrush(const FString& Id, float Margin) const;
	FSlateFontInfo Font(int32 Size, bool bBold = false) const;
	static const FSlateBrush* White();

	TSharedRef<SWidget> Framed(TSharedRef<SWidget> Content, const FLinearColor& Bg, const FLinearColor& Border, const FMargin& Pad) const;
	TSharedRef<SWidget> Text(const FString& S, int32 Size, const FLinearColor& Color, bool bBold = false, bool bWrap = false) const;
	TSharedRef<SWidget> Btn(const FString& Label, TFunction<void()> OnClick, bool bEnabled = true,
		const FButtonStyle* Style = nullptr, int32 FontSize = 14, const FLinearColor* TextColor = nullptr,
		const FString& Tooltip = FString());
	TSharedRef<SWidget> IconValue(const FString& IconId, const FString& Value, const FString& Tooltip, int32 IconSize = 26, int32 FontSize = 16);
	TSharedRef<SWidget> HpBar(const FCGMember& M, float Width) const;
	TSharedRef<SWidget> ModalShell(TSharedRef<SWidget> Content, float Width);

	static FLinearColor RarityColor(const FString& Rarity);
	static FLinearColor DifficultyColor(const FString& Difficulty);
	static FLinearColor MoodColor(const FString& Mood);
	static FLinearColor BandColor(const FString& Band);

	void ShowMessage(const FString& Title, const FString& Body);
	void DoQuit();
};
