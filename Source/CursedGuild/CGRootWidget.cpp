#include "CGRootWidget.h"
#include "CGGameInstance.h"

#include "Brushes/SlateColorBrush.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace CGColors
{
	static const FLinearColor Night       = FLinearColor::FromSRGBColor(FColor(12, 8, 11));
	static const FLinearColor Panel       = FLinearColor::FromSRGBColor(FColor(21, 14, 19));
	static const FLinearColor PanelLight  = FLinearColor::FromSRGBColor(FColor(34, 23, 31));
	static const FLinearColor PanelPurple = FLinearColor::FromSRGBColor(FColor(38, 20, 48));
	static const FLinearColor Scrim       = FLinearColor(0.f, 0.f, 0.f, 0.78f);
	static const FLinearColor Gold        = FLinearColor::FromSRGBColor(FColor(201, 155, 63));
	static const FLinearColor GoldBright  = FLinearColor::FromSRGBColor(FColor(244, 208, 111));
	static const FLinearColor GoldDark    = FLinearColor::FromSRGBColor(FColor(110, 84, 34));
	static const FLinearColor Ink         = FLinearColor::FromSRGBColor(FColor(41, 28, 18));
	static const FLinearColor InkFaded    = FLinearColor::FromSRGBColor(FColor(84, 62, 42));
	static const FLinearColor Parchment   = FLinearColor::FromSRGBColor(FColor(214, 190, 148));
	static const FLinearColor Bone        = FLinearColor::FromSRGBColor(FColor(224, 205, 169));
	static const FLinearColor Muted       = FLinearColor::FromSRGBColor(FColor(146, 128, 104));
	static const FLinearColor Danger      = FLinearColor::FromSRGBColor(FColor(196, 55, 43));
	static const FLinearColor DangerDark  = FLinearColor::FromSRGBColor(FColor(96, 24, 20));
	static const FLinearColor Green       = FLinearColor::FromSRGBColor(FColor(112, 177, 70));
	static const FLinearColor Blue        = FLinearColor::FromSRGBColor(FColor(94, 133, 214));
	static const FLinearColor Purple      = FLinearColor::FromSRGBColor(FColor(158, 96, 214));
	static const FLinearColor Orange      = FLinearColor::FromSRGBColor(FColor(219, 143, 55));
}

// ----------------------------------------------------------------------

void SCGRoot::Construct(const FArguments& InArgs)
{
	GI = InArgs._GameInstance;

	auto MakeButton = [](const FLinearColor& N, const FLinearColor& H, const FLinearColor& P)
	{
		FButtonStyle S;
		S.SetNormal(FSlateColorBrush(N));
		S.SetHovered(FSlateColorBrush(H));
		S.SetPressed(FSlateColorBrush(P));
		S.SetDisabled(FSlateColorBrush(FLinearColor(N.R * 0.5f, N.G * 0.5f, N.B * 0.5f, N.A)));
		S.SetNormalPadding(FMargin(0));
		S.SetPressedPadding(FMargin(0, 1, 0, 0));
		return S;
	};

	DarkButton         = MakeButton(CGColors::PanelLight, FLinearColor::FromSRGBColor(FColor(56, 38, 50)), CGColors::Panel);
	GoldButton         = MakeButton(CGColors::Gold, CGColors::GoldBright, CGColors::GoldDark);
	TabButton          = MakeButton(CGColors::Panel, CGColors::PanelLight, CGColors::Night);
	TabButtonActive    = MakeButton(CGColors::PanelPurple, CGColors::PanelPurple, CGColors::PanelPurple);
	CardButton         = MakeButton(FLinearColor(0, 0, 0, 0.35f), FLinearColor(0.16f, 0.1f, 0.14f, 0.7f), FLinearColor(0, 0, 0, 0.5f));
	CardButtonSelected = MakeButton(FLinearColor::FromSRGBColor(FColor(64, 44, 20)), FLinearColor::FromSRGBColor(FColor(78, 55, 26)), FLinearColor::FromSRGBColor(FColor(52, 36, 16)));

	ChildSlot
	[
		SAssignNew(Container, SBox)
	];
	Rebuild();
}

void SCGRoot::Rebuild()
{
	if (!Container.IsValid()) { return; }
	Container->SetContent(Screen == EScreen::MainMenu ? BuildMainMenu() : BuildHub());
}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

const FSlateBrush* SCGRoot::White()
{
	return FCoreStyle::Get().GetBrush("WhiteBrush");
}

const FSlateBrush* SCGRoot::Brush(const FString& Id) const
{
	return GI.IsValid() ? GI->GetBrush(Id) : White();
}

const FSlateBrush* SCGRoot::BoxBrush(const FString& Id, float Margin) const
{
	return GI.IsValid() ? GI->GetBoxBrush(Id, Margin) : White();
}

FSlateFontInfo SCGRoot::Font(int32 Size, bool bBold) const
{
	return GI.IsValid() ? GI->GetFont(Size, bBold) : FCoreStyle::GetDefaultFontStyle("Regular", Size);
}

TSharedRef<SWidget> SCGRoot::Framed(TSharedRef<SWidget> Content, const FLinearColor& Bg, const FLinearColor& Border, const FMargin& Pad) const
{
	return SNew(SBorder)
		.BorderImage(White())
		.BorderBackgroundColor(Border)
		.Padding(FMargin(1.f))
		[
			SNew(SBorder)
			.BorderImage(White())
			.BorderBackgroundColor(Bg)
			.Padding(Pad)
			[
				Content
			]
		];
}

TSharedRef<SWidget> SCGRoot::Text(const FString& S, int32 Size, const FLinearColor& Color, bool bBold, bool bWrap) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(S))
		.Font(Font(Size, bBold))
		.ColorAndOpacity(FSlateColor(Color))
		.AutoWrapText(bWrap);
}

TSharedRef<SWidget> SCGRoot::Btn(const FString& Label, TFunction<void()> OnClick, bool bEnabled,
	const FButtonStyle* Style, int32 FontSize, const FLinearColor* TextColor, const FString& Tooltip)
{
	const FLinearColor Color = TextColor ? *TextColor : CGColors::Bone;
	TSharedRef<SButton> Button = SNew(SButton)
		.ButtonStyle(Style ? Style : &DarkButton)
		.IsEnabled(bEnabled)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.ContentPadding(FMargin(12, 6))
		.OnClicked_Lambda([OnClick]() { OnClick(); return FReply::Handled(); })
		[
			Text(Label, FontSize, Color, true)
		];
	if (!Tooltip.IsEmpty())
	{
		Button->SetToolTipText(FText::FromString(Tooltip));
	}
	return Button;
}

TSharedRef<SWidget> SCGRoot::IconValue(const FString& IconId, const FString& Value, const FString& Tooltip, int32 IconSize, int32 FontSize)
{
	TSharedRef<SWidget> W = Framed(
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SBox).WidthOverride(IconSize).HeightOverride(IconSize)
			[
				SNew(SImage).Image(Brush(IconId))
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0, 2, 0)
		[
			Text(Value, FontSize, CGColors::Bone, true)
		],
		FLinearColor(0, 0, 0, 0.55f), CGColors::GoldDark, FMargin(8, 4));
	W->SetToolTipText(FText::FromString(Tooltip));
	return W;
}

TSharedRef<SWidget> SCGRoot::HpBar(const FCGMember& M, float Width) const
{
	const float Frac = M.maxHp > 0 ? FMath::Clamp(static_cast<float>(M.hp) / M.maxHp, 0.f, 1.f) : 0.f;
	const FLinearColor Fill = Frac > 0.6f ? CGColors::Green : Frac > 0.3f ? CGColors::Orange : CGColors::Danger;
	return SNew(SBox).WidthOverride(Width).HeightOverride(7.f)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder).BorderImage(White()).BorderBackgroundColor(FLinearColor(0, 0, 0, 0.8f)).Padding(0)
			[ SNew(SSpacer) ]
		]
		+ SOverlay::Slot().HAlign(HAlign_Left)
		[
			SNew(SBox).WidthOverride(Width * Frac).HeightOverride(7.f)
			[
				SNew(SBorder).BorderImage(White()).BorderBackgroundColor(Fill).Padding(0)
				[ SNew(SSpacer) ]
			]
		]
	];
}

FLinearColor SCGRoot::RarityColor(const FString& Rarity)
{
	if (Rarity == TEXT("Common"))    return CGColors::Muted;
	if (Rarity == TEXT("Uncommon"))  return CGColors::Green;
	if (Rarity == TEXT("Rare"))      return CGColors::Blue;
	if (Rarity == TEXT("Cursed"))    return CGColors::Purple;
	if (Rarity == TEXT("Legendary")) return CGColors::GoldBright;
	return CGColors::Bone;
}

FLinearColor SCGRoot::DifficultyColor(const FString& Difficulty)
{
	if (Difficulty == TEXT("Easy"))   return CGColors::Green;
	if (Difficulty == TEXT("Medium")) return CGColors::Orange;
	return CGColors::Danger;
}

FLinearColor SCGRoot::MoodColor(const FString& Mood)
{
	if (Mood == TEXT("Ecstatic"))  return CGColors::GoldBright;
	if (Mood == TEXT("High"))      return CGColors::Green;
	if (Mood == TEXT("Tired"))     return CGColors::Orange;
	if (Mood == TEXT("Miserable")) return CGColors::Danger;
	return CGColors::Bone;
}

FLinearColor SCGRoot::BandColor(const FString& Band)
{
	if (Band == TEXT("Safe"))      return CGColors::Green;
	if (Band == TEXT("Risky"))     return CGColors::Orange;
	if (Band == TEXT("Dangerous")) return CGColors::Danger;
	return CGColors::Purple;
}

void SCGRoot::ShowMessage(const FString& Title, const FString& Body)
{
	MessageTitle = Title;
	MessageBody = Body;
	Modal = EModal::Message;
	Rebuild();
}

void SCGRoot::DoQuit()
{
	if (GI.IsValid())
	{
		if (GI->bInGame) { GI->SaveGame(); }
		if (APlayerController* PC = GI->GetFirstLocalPlayerController())
		{
			PC->ConsoleCommand(TEXT("quit"));
		}
	}
}

// ----------------------------------------------------------------------
// Main menu
// ----------------------------------------------------------------------

TSharedRef<SWidget> SCGRoot::BuildMainMenu()
{
	UCGGameInstance* G = GI.Get();
	const bool bHasSave = G && G->HasSaveFile();

	auto MenuButton = [this](const FString& Label, TFunction<void()> Fn, bool bEnabled = true)
	{
		return SNew(SBox).WidthOverride(300.f).HeightOverride(52.f).Padding(FMargin(0, 6))
			[
				Btn(Label, Fn, bEnabled, &DarkButton, 18, &CGColors::GoldBright)
			];
	};

	TSharedRef<SOverlay> Overlay = SNew(SOverlay);
	Overlay->AddSlot()
	[
		SNew(SScaleBox).Stretch(EStretch::ScaleToFill)
		[
			SNew(SImage).Image(Brush(TEXT("bg_guild_hall")))
		]
	];
	Overlay->AddSlot()
	[
		SNew(SBorder).BorderImage(White()).BorderBackgroundColor(FLinearColor(0, 0, 0, 0.45f)).Padding(0)
		[ SNew(SSpacer) ]
	];
	Overlay->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Center)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SBox).WidthOverride(110.f).HeightOverride(110.f)
			[ SNew(SImage).Image(Brush(TEXT("ui_gem_emblem"))) ]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 6)
		[ Text(TEXT("CURSED GUILD"), 56, CGColors::GoldBright, true) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 26)
		[ Text(TEXT("Contracts. Curses. Cash flow."), 16, CGColors::Muted) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[ MenuButton(TEXT("New Game"), [this]() { if (GI.IsValid()) { GI->NewGame(); Screen = EScreen::Hub; Rebuild(); } }) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[ MenuButton(TEXT("Continue"), [this]()
			{
				if (GI.IsValid() && GI->LoadGame()) { Screen = EScreen::Hub; Rebuild(); }
				else { ShowMessage(TEXT("No Save Found"), TEXT("The ledger of past deeds is blank.")); }
			}, bHasSave) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[ MenuButton(TEXT("Settings"), [this]() { Modal = EModal::Settings; Rebuild(); }) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[ MenuButton(TEXT("Quit"), [this]() { DoQuit(); }) ]
	];
	Overlay->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(0, 0, 14, 10)
	[
		Text(TEXT("Cursed Guild MVP — Unreal Engine 5.8"), 11, CGColors::Muted)
	];
	if (Modal != EModal::None)
	{
		Overlay->AddSlot()[ BuildModalOverlay() ];
	}
	return Overlay;
}

// ----------------------------------------------------------------------
// Hub frame
// ----------------------------------------------------------------------

TSharedRef<SWidget> SCGRoot::BuildHub()
{
	TSharedRef<SOverlay> Overlay = SNew(SOverlay);

	Overlay->AddSlot()
	[
		SNew(SScaleBox).Stretch(EStretch::ScaleToFill)
		[
			SNew(SImage).Image(Brush(TEXT("bg_guild_hall"))).ColorAndOpacity(FLinearColor(0.55f, 0.55f, 0.55f, 1.f))
		]
	];

	Overlay->AddSlot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 8, 8, 4)
		[ BuildTopBar() ]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(8, 4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
			[ SNew(SBox).WidthOverride(330.f)[ BuildRosterPanel() ] ]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(4, 0)
			[ BuildCenterPanel() ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
			[ SNew(SBox).WidthOverride(370.f)[ BuildRightPanel() ] ]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 4, 8, 8)
		[ BuildBottomBar() ]
	];

	if (GI.IsValid() && GI->TutorialActive() && Modal == EModal::None)
	{
		Overlay->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0, 0, 0, 132)
		[ BuildTutorialPopup() ];
	}

	if (Modal != EModal::None)
	{
		Overlay->AddSlot()[ BuildModalOverlay() ];
	}
	return Overlay;
}

// ----------------------------------------------------------------------
// Top bar
// ----------------------------------------------------------------------

TSharedRef<SWidget> SCGRoot::BuildTopBar()
{
	UCGGameInstance* G = GI.Get();
	if (!G) { return SNew(SSpacer); }

	const FString WeekDay = FString::Printf(TEXT("Week %d, Day %d"), G->Week, G->Day);

	return Framed(
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6, 0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(44.f).HeightOverride(44.f)
				[ SNew(SImage).Image(Brush(TEXT("ui_gem_emblem"))) ]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10, 0)
			[ Text(TEXT("CURSED GUILD"), 24, CGColors::GoldBright, true) ]
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18, 0, 4, 0)
		[ IconValue(TEXT("icon_gold"), FString::FormatAsNumber(G->Resources.gold), TEXT("Gold — hiring, upgrades, healing, crafting")) ]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
		[ IconValue(TEXT("icon_reputation"), FString::FormatAsNumber(G->Resources.reputation), TEXT("Reputation — unlocks harder contracts and better recruits")) ]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
		[ IconValue(TEXT("icon_supplies"), FString::FormatAsNumber(G->Resources.supplies), TEXT("Supplies — spent on expeditions and resting")) ]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
		[ IconValue(TEXT("icon_arcane_dust"), FString::FormatAsNumber(G->Resources.arcaneDust), TEXT("Arcane Dust — relics, magic rooms, potions")) ]
		+ SHorizontalBox::Slot().FillWidth(1.f)
		[ SNew(SSpacer) ]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
		[
			Framed(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[ Text(WeekDay, 16, CGColors::Bone, true) ]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[ Text(FString::Printf(TEXT("%s — %s"), *G->MoonPhase(), *G->MoonPhaseHint()), 11, CGColors::Purple) ],
				FLinearColor(0, 0, 0, 0.55f), CGColors::GoldDark, FMargin(12, 4))
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6, 0)
		[ Btn(TEXT("Settings"), [this]() { Modal = EModal::Settings; Rebuild(); }, true, &DarkButton, 13) ],
		CGColors::Panel, CGColors::GoldDark, FMargin(6, 6));
}

// ----------------------------------------------------------------------
// Roster (left)
// ----------------------------------------------------------------------

TSharedRef<SWidget> SCGRoot::BuildRosterPanel()
{
	UCGGameInstance* G = GI.Get();
	if (!G) { return SNew(SSpacer); }

	TSharedRef<SScrollBox> List = SNew(SScrollBox);
	for (const FCGMember& M : G->Roster)
	{
		List->AddSlot().Padding(0, 3)
		[ BuildMemberCard(M) ];
	}

	return Framed(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 2, 4, 8)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[ Text(TEXT("YOUR GUILD"), 17, CGColors::GoldBright, true) ]
			+ SHorizontalBox::Slot().FillWidth(1.f)[ SNew(SSpacer) ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[ Text(FString::Printf(TEXT("%d / %d"), G->Roster.Num(), G->RosterCapacity()), 14, CGColors::Muted, true) ]
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[ List ]
		+ SVerticalBox::Slot().AutoHeight().Padding(2, 8, 2, 2)
		[
			SNew(SBox).HeightOverride(40.f)
			[ Btn(TEXT("+  Recruit Hero"), [this]() { Modal = EModal::Recruit; Rebuild(); }, true, &DarkButton, 14, &CGColors::GoldBright) ]
		],
		CGColors::Panel, CGColors::GoldDark, FMargin(8));
}

TSharedRef<SWidget> SCGRoot::BuildMemberCard(const FCGMember& M)
{
	UCGGameInstance* G = GI.Get();
	const bool bSelected = G && G->IsInParty(M.id);
	const bool bCanTravel = M.CanEmbark();

	FString StatusLine;
	FLinearColor StatusColor = CGColors::Muted;
	if (M.status == TEXT("injured"))          { StatusLine = TEXT("Injured");  StatusColor = CGColors::Orange; }
	else if (M.status == TEXT("wounded"))     { StatusLine = FString::Printf(TEXT("Wounded — %dd"), M.daysUnavailable); StatusColor = CGColors::Danger; }
	else if (M.status == TEXT("unavailable")) { StatusLine = FString::Printf(TEXT("Away — %dd"), M.daysUnavailable);    StatusColor = CGColors::Danger; }

	FString Traits;
	for (int32 i = 0; i < M.traits.Num(); ++i)
	{
		Traits += M.traits[i] + (i + 1 < M.traits.Num() ? TEXT(" · ") : TEXT(""));
	}

	const FString MemberId = M.id;

	TSharedRef<SVerticalBox> Info = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[ Text(M.name, 15, bSelected ? CGColors::GoldBright : CGColors::Bone, true) ]
			+ SHorizontalBox::Slot().FillWidth(1.f)[ SNew(SSpacer) ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[ Text(FString::Printf(TEXT("Lv %d"), M.level), 12, CGColors::Gold, true) ]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[ Text(M.className, 12, CGColors::Muted) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 3, 0, 1)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[ HpBar(M, 110.f) ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6, 0)
			[ Text(FString::Printf(TEXT("%d/%d"), M.hp, M.maxHp), 11, CGColors::Bone) ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[ Text(M.mood, 11, MoodColor(M.mood), true) ]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[ Text(Traits, 10, CGColors::Muted, false) ];

	if (!StatusLine.IsEmpty())
	{
		Info->AddSlot().AutoHeight()
		[ Text(StatusLine, 11, StatusColor, true) ];
	}

	TSharedRef<SOverlay> PortraitBox = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBox).WidthOverride(58.f).HeightOverride(58.f)
			[ SNew(SImage).Image(Brush(M.portraitSpriteId)) ]
		];
	if (bSelected)
	{
		PortraitBox->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Top)
		[
			SNew(SBox).WidthOverride(18.f).HeightOverride(18.f)
			[ SNew(SImage).Image(Brush(TEXT("ui_check"))) ]
		];
	}

	TSharedRef<SButton> Card = SNew(SButton)
		.ButtonStyle(bSelected ? &CardButtonSelected : &CardButton)
		.ContentPadding(FMargin(6))
		.OnClicked_Lambda([this, MemberId]()
		{
			if (GI.IsValid())
			{
				if (!GI->TogglePartyMember(MemberId) && !GI->IsInParty(MemberId))
				{
					const FCGMember* M2 = GI->FindRosterMember(MemberId);
					if (M2 && !M2->CanEmbark())
					{
						ShowMessage(TEXT("Unavailable"), FString::Printf(TEXT("%s cannot travel right now."), *M2->name));
						return FReply::Handled();
					}
				}
				Rebuild();
			}
			return FReply::Handled();
		})
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				Framed(PortraitBox, FLinearColor::Black, bSelected ? CGColors::GoldBright : CGColors::GoldDark, FMargin(0))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(8, 0, 2, 0)
			[ Info ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
			[
				SNew(SBox).WidthOverride(22.f).HeightOverride(22.f)
				[
					Btn(TEXT("i"), [this, MemberId]() { DetailMemberId = MemberId; Modal = EModal::MemberDetail; Rebuild(); },
						true, &TabButton, 11, &CGColors::Muted, TEXT("Member details"))
				]
			]
		];

	Card->SetToolTipText(FText::FromString(bCanTravel
		? FString::Printf(TEXT("Click to %s the expedition party"), bSelected ? TEXT("remove from") : TEXT("add to"))
		: TEXT("This member cannot travel right now")));
	return Card;
}

// ----------------------------------------------------------------------
// Center — contract board
// ----------------------------------------------------------------------

TSharedRef<SWidget> SCGRoot::BuildCenterPanel()
{
	UCGGameInstance* G = GI.Get();
	if (!G) { return SNew(SSpacer); }

	// contract selector tabs
	TSharedRef<SHorizontalBox> Tabs = SNew(SHorizontalBox);
	for (const FString& ContractId : G->BoardContractIds)
	{
		const FCGContract* C = G->FindContract(ContractId);
		if (!C) { continue; }
		const bool bActive = ContractId == G->SelectedContractId;
		const FString Id = ContractId;
		Tabs->AddSlot().FillWidth(1.f).Padding(2, 0)
		[
			SNew(SButton)
			.ButtonStyle(bActive ? &TabButtonActive : &TabButton)
			.ContentPadding(FMargin(8, 6))
			.HAlign(HAlign_Center)
			.OnClicked_Lambda([this, Id]() { if (GI.IsValid()) { GI->SelectContract(Id); Rebuild(); } return FReply::Handled(); })
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[ Text(C->title, 12, bActive ? CGColors::GoldBright : CGColors::Bone, bActive) ]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[ Text(FString::Printf(TEXT("%s · %s"), *C->difficulty, *C->biome), 10, DifficultyColor(C->difficulty)) ]
			]
		];
	}

	const FCGContract* Selected = G->FindContract(G->SelectedContractId);

	TSharedRef<SVerticalBox> Center = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 4)
		[
			Framed(Text(TEXT("CONTRACT  BOARD"), 18, CGColors::GoldBright, true),
				CGColors::PanelPurple, CGColors::Gold, FMargin(38, 5))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ Tabs ];

	if (Selected)
	{
		Center->AddSlot().FillHeight(1.f)
		[ BuildContractSheet(*Selected) ];
		Center->AddSlot().AutoHeight().Padding(0, 6, 0, 0)
		[ BuildPreviewStrip(*Selected) ];
	}
	else
	{
		Center->AddSlot().FillHeight(1.f)
		[
			Framed(SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
				[ Text(TEXT("No contracts posted. Rest a day to refresh the board."), 14, CGColors::Muted) ],
				CGColors::Panel, CGColors::GoldDark, FMargin(10))
		];
	}
	return Center;
}

TSharedRef<SWidget> SCGRoot::BuildContractSheet(const FCGContract& C)
{
	UCGGameInstance* G = GI.Get();

	// difficulty skulls
	const int32 Skulls = C.difficulty == TEXT("Hard") ? 3 : C.difficulty == TEXT("Medium") ? 2 : 1;
	TSharedRef<SHorizontalBox> SkullRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < Skulls; ++i)
	{
		SkullRow->AddSlot().AutoWidth().Padding(1, 0)
		[
			SNew(SBox).WidthOverride(16.f).HeightOverride(16.f)
			[ SNew(SImage).Image(Brush(TEXT("icon_skull"))) ]
		];
	}
	SkullRow->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(6, 0)
	[ Text(C.difficulty, 13, DifficultyColor(C.difficulty), true) ];

	auto SheetRow = [this](const FString& Label, TSharedRef<SWidget> Value)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[ SNew(SBox).WidthOverride(120.f)[ Text(Label, 13, CGColors::InkFaded, true) ] ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[ Value ];
	};

	const FString LengthText = C.lengthDays >= 3
		? FString::Printf(TEXT("%d–%d Days"), C.lengthDays, C.lengthDays + 1)
		: FString::Printf(TEXT("%d Day%s"), C.lengthDays, C.lengthDays > 1 ? TEXT("s") : TEXT(""));

	// rewards box
	TSharedRef<SHorizontalBox> Rewards = SNew(SHorizontalBox);
	auto AddReward = [&](const FString& Icon, int32 Amount, const FString& Tip)
	{
		if (Amount <= 0) { return; }
		Rewards->AddSlot().AutoWidth().Padding(4, 0)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(SBox).WidthOverride(34.f).HeightOverride(34.f)
				[ SNew(SImage).Image(Brush(Icon)).ToolTipText(FText::FromString(Tip)) ]
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[ Text(FString::FormatAsNumber(Amount), 13, CGColors::Bone, true) ]
		];
	};
	AddReward(TEXT("icon_gold_pile"), C.rewards.gold, TEXT("Gold"));
	AddReward(TEXT("icon_reputation"), C.rewards.reputation, TEXT("Reputation"));
	AddReward(TEXT("icon_arcane_dust"), C.rewards.arcaneDust, TEXT("Arcane Dust"));
	if (C.relicChance > 0.f)
	{
		Rewards->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0)
		[ Text(FString::Printf(TEXT("Relic %d%%"), FMath::RoundToInt(C.relicChance * 100)), 12, CGColors::Purple, true) ];
	}

	// risks box
	TSharedRef<SHorizontalBox> Risks = SNew(SHorizontalBox);
	for (const FString& Risk : C.riskTags)
	{
		FString Pretty = Risk;
		if (Pretty.Len() > 0) { Pretty[0] = FChar::ToUpper(Pretty[0]); }
		Risks->AddSlot().AutoWidth().Padding(3, 0)
		[ Framed(Text(Pretty, 11, CGColors::Bone, true), CGColors::DangerDark, CGColors::Danger, FMargin(8, 3)) ];
	}

	TSharedRef<SVerticalBox> Sheet = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 14, 0)
			[
				Framed(
					SNew(SBox).WidthOverride(210.f).HeightOverride(150.f)
					[
						SNew(SScaleBox).Stretch(EStretch::ScaleToFill)
						[ SNew(SImage).Image(Brush(TEXT("ui_graveyard"))) ]
					],
					FLinearColor::Black, CGColors::GoldDark, FMargin(0))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[ Text(C.title.ToUpper(), 22, CGColors::Ink, true, true) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 8)
				[ Text(C.description, 13, CGColors::Ink, false, true) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
				[ SheetRow(TEXT("Difficulty"), SkullRow) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
				[ SheetRow(TEXT("Length"), Text(LengthText, 13, CGColors::Ink, true)) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
				[ SheetRow(TEXT("Biome"), Text(C.biome, 13, CGColors::Ink, true)) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
				[ SheetRow(TEXT("Supplies"), Text(FString::Printf(TEXT("%d required"), C.requiredSupplies), 13,
					G && G->Resources.supplies >= C.requiredSupplies ? CGColors::Ink : CGColors::Danger, true)) ]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0, 0, 6, 0)
			[
				Framed(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 4)
					[ Text(TEXT("REWARDS"), 13, CGColors::GoldBright, true) ]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[ Rewards ],
					FLinearColor(0, 0, 0, 0.72f), CGColors::GoldDark, FMargin(10, 6))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(6, 0, 0, 0)
			[
				Framed(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 6)
					[ Text(TEXT("RISKS"), 13, CGColors::Danger, true) ]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[ Risks ],
					FLinearColor(0, 0, 0, 0.72f), CGColors::DangerDark, FMargin(10, 6))
			]
		];

	if (!C.specialModifierName.IsEmpty())
	{
		Sheet->AddSlot().AutoHeight().Padding(0, 8, 0, 0)
		[
			Framed(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
				[
					SNew(SBox).WidthOverride(30.f).HeightOverride(30.f)
					[ SNew(SImage).Image(Brush(TEXT("icon_reputation"))) ]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[ Text(C.specialModifierName, 13, CGColors::Purple, true) ]
					+ SVerticalBox::Slot().AutoHeight()
					[ Text(C.specialModifierDesc, 11, CGColors::Bone, false, true) ]
				],
				FLinearColor(0.09f, 0.04f, 0.13f, 0.9f), CGColors::Purple, FMargin(10, 6))
		];
	}

	return SNew(SBorder)
		.BorderImage(BoxBrush(TEXT("ui_parchment"), 0.12f))
		.Padding(FMargin(26, 20))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[ Sheet ]
		];
}

TSharedRef<SWidget> SCGRoot::BuildPreviewStrip(const FCGContract& C)
{
	UCGGameInstance* G = GI.Get();
	if (!G) { return SNew(SSpacer); }

	const FCGPreview Preview = G->ComputePreview();

	// enemy portraits with deterministic counts
	TSharedRef<SHorizontalBox> Enemies = SNew(SHorizontalBox);
	for (int32 i = 0; i < C.enemyTags.Num() && i < 3; ++i)
	{
		const FCGEnemy* Enemy = G->FindEnemyByTag(C.enemyTags[i]);
		if (!Enemy) { continue; }
		const int32 Count = FMath::Max(1, (C.lengthDays + (2 - i)) / 2);
		Enemies->AddSlot().AutoWidth().Padding(4, 0)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				Framed(
					SNew(SBox).WidthOverride(64.f).HeightOverride(64.f)
					[
						SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
						[ SNew(SImage).Image(Brush(Enemy->spriteId)).ToolTipText(FText::FromString(Enemy->name)) ]
					],
					FLinearColor(0, 0, 0, 0.8f), CGColors::DangerDark, FMargin(2))
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[ Text(FString::Printf(TEXT("x%d"), Count), 11, CGColors::Bone, true) ]
		];
	}

	// modifier lines (first few)
	TSharedRef<SVerticalBox> Mods = SNew(SVerticalBox);
	for (int32 i = 0; i < Preview.ModifierLines.Num() && i < 4; ++i)
	{
		Mods->AddSlot().AutoHeight()
		[ Text(Preview.ModifierLines[i], 10, CGColors::Muted) ];
	}

	return Framed(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 4)
		[ Text(TEXT("EXPEDITION  PREVIEW"), 14, CGColors::GoldBright, true) ]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[ Enemies ]
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(14, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[ Text(Preview.DisplayChance, 30, BandColor(Preview.Band), true) ]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10, 0)
					[ Text(G->SelectedPartyIds.Num() > 0 ? Preview.Band : TEXT("Assign a party"), 15, BandColor(Preview.Band), true) ]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(14, 0)
					[ Text(FString::Printf(TEXT("Party %d  vs  Challenge %d"), Preview.PartyPower, Preview.ContractPower), 12, CGColors::Muted) ]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 3, 0, 0)
				[ Mods ]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(260.f)
				[ Text(Preview.Warning, 11, CGColors::Orange, false, true) ]
			]
		],
		CGColors::Panel, CGColors::GoldDark, FMargin(10, 6));
}

// ----------------------------------------------------------------------
// Right panel
// ----------------------------------------------------------------------

TSharedRef<SWidget> SCGRoot::BuildRightPanel()
{
	auto Tab = [this](const FString& Label, int32 Index)
	{
		const bool bActive = RightTab == Index;
		return SNew(SButton)
			.ButtonStyle(bActive ? &TabButtonActive : &TabButton)
			.HAlign(HAlign_Center)
			.ContentPadding(FMargin(0, 7))
			.OnClicked_Lambda([this, Index]() { RightTab = Index; Rebuild(); return FReply::Handled(); })
			[ Text(Label, 14, bActive ? CGColors::GoldBright : CGColors::Muted, true) ];
	};

	TSharedRef<SWidget> Content =
		RightTab == 0 ? BuildGuildTab() :
		RightTab == 1 ? BuildRoomsTab() : BuildRelicsTab();

	return Framed(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(1, 0)[ Tab(TEXT("GUILD"), 0) ]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(1, 0)[ Tab(TEXT("ROOMS"), 1) ]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(1, 0)[ Tab(TEXT("RELICS"), 2) ]
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0, 8, 0, 0)
		[ Content ],
		CGColors::Panel, CGColors::GoldDark, FMargin(8));
}

TSharedRef<SWidget> SCGRoot::BuildGuildTab()
{
	UCGGameInstance* G = GI.Get();
	if (!G) { return SNew(SSpacer); }

	const FCGPreview Preview = G->ComputePreview();

	TSharedRef<SVerticalBox> Party = SNew(SVerticalBox);
	if (G->SelectedPartyIds.Num() == 0)
	{
		Party->AddSlot().AutoHeight()
		[ Text(TEXT("No members assigned. Click roster portraits to build a party (max 3)."), 12, CGColors::Muted, false, true) ];
	}
	for (const FString& Id : G->SelectedPartyIds)
	{
		const FCGMember* M = G->FindRosterMember(Id);
		if (!M) { continue; }
		Party->AddSlot().AutoHeight().Padding(0, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(34.f).HeightOverride(34.f)
				[ SNew(SImage).Image(Brush(M->portraitSpriteId)) ]
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(8, 0)
			[ Text(FString::Printf(TEXT("%s — %s"), *M->name, *M->className), 12, CGColors::Bone) ]
		];
	}

	auto StatRow = [this](const FString& Name, int32 Value, const FString& Tip)
	{
		TSharedRef<SWidget> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)[ Text(Name, 12, CGColors::Muted) ]
			+ SHorizontalBox::Slot().AutoWidth()[ Text(FString::FromInt(Value), 13, CGColors::Bone, true) ];
		Row->SetToolTipText(FText::FromString(Tip));
		return Row;
	};

	return SNew(SScrollBox)
	+ SScrollBox::Slot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ Text(TEXT("EXPEDITION PARTY"), 13, CGColors::GoldBright, true) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 10)
		[ Party ]
		+ SVerticalBox::Slot().AutoHeight()
		[ Text(TEXT("PARTY STATS (with relics)"), 13, CGColors::GoldBright, true) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 4, 4, 10)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[ StatRow(TEXT("Might"), Preview.PartyStats.might, TEXT("Physical combat, guarding, brute force")) ]
			+ SVerticalBox::Slot().AutoHeight()[ StatRow(TEXT("Wits"), Preview.PartyStats.wits, TEXT("Investigation, traps, planning, puzzles")) ]
			+ SVerticalBox::Slot().AutoHeight()[ StatRow(TEXT("Magic"), Preview.PartyStats.magic, TEXT("Curses, arcane hazards, rituals")) ]
			+ SVerticalBox::Slot().AutoHeight()[ StatRow(TEXT("Guile"), Preview.PartyStats.guile, TEXT("Stealth, deception, negotiation")) ]
			+ SVerticalBox::Slot().AutoHeight()[ StatRow(TEXT("Spirit"), Preview.PartyStats.spirit, TEXT("Resistance, morale, healing, holy defense")) ]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[ Text(TEXT("GUILD"), 13, CGColors::GoldBright, true) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 4, 4, 0)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[ Text(FString::Printf(TEXT("Roster capacity: %d"), G->RosterCapacity()), 12, CGColors::Muted) ]
			+ SVerticalBox::Slot().AutoHeight()[ Text(FString::Printf(TEXT("Relic slots: %d   Storage: %d"), G->RelicSlots(), G->RelicStorage()), 12, CGColors::Muted) ]
			+ SVerticalBox::Slot().AutoHeight()[ Text(FString::Printf(TEXT("Expeditions completed: %d"), G->ExpeditionsRun), 12, CGColors::Muted) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
			[ Text(FString::Printf(TEXT("%s: %s"), *G->MoonPhase(), *G->MoonPhaseHint()), 12, CGColors::Purple, false, true) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
			[ Text(TEXT("The game autosaves after contracts, upgrades, recruiting, and resting."), 10, CGColors::Muted, false, true) ]
		]
	];
}

TSharedRef<SWidget> SCGRoot::BuildRoomsTab()
{
	UCGGameInstance* G = GI.Get();
	if (!G) { return SNew(SSpacer); }

	TSharedRef<SScrollBox> List = SNew(SScrollBox);
	for (const FCGRoom& Room : G->Rooms)
	{
		// level pips
		TSharedRef<SHorizontalBox> Pips = SNew(SHorizontalBox);
		for (int32 i = 0; i < Room.maxLevel; ++i)
		{
			Pips->AddSlot().AutoWidth().Padding(1, 0)
			[
				SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
				[
					SNew(SBorder).BorderImage(White())
					.BorderBackgroundColor(i < Room.level ? CGColors::Purple : FLinearColor(0, 0, 0, 0.6f))
					.Padding(0)[ SNew(SSpacer) ]
				]
			];
		}

		FString CostText;
		bool bCanUpgrade = false;
		FString Reason;
		if (Room.level < Room.maxLevel && Room.upgradeCosts.IsValidIndex(Room.level))
		{
			const FCGResourceBundle& Cost = Room.upgradeCosts[Room.level];
			TArray<FString> Parts;
			if (Cost.gold > 0)       { Parts.Add(FString::Printf(TEXT("%dg"), Cost.gold)); }
			if (Cost.reputation > 0) { Parts.Add(FString::Printf(TEXT("%d rep"), Cost.reputation)); }
			if (Cost.supplies > 0)   { Parts.Add(FString::Printf(TEXT("%d sup"), Cost.supplies)); }
			if (Cost.arcaneDust > 0) { Parts.Add(FString::Printf(TEXT("%d dust"), Cost.arcaneDust)); }
			CostText = FString::Join(Parts, TEXT("  "));
			bCanUpgrade = G->CanUpgradeRoom(Room, Reason);
		}
		else
		{
			CostText = TEXT("MAX");
		}

		const FString RoomId = Room.id;
		const bool bLocked = !Room.isUnlocked;

		List->AddSlot().Padding(0, 3)
		[
			Framed(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(46.f).HeightOverride(46.f)
					[
						SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
						[ SNew(SImage).Image(Brush(Room.iconSpriteId)).ColorAndOpacity(bLocked ? FLinearColor(0.35f, 0.35f, 0.35f) : FLinearColor::White) ]
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(8, 0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[ Text(Room.name, 13, bLocked ? CGColors::Muted : CGColors::Bone, true) ]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0)
						[ bLocked ? Text(TEXT("LOCKED"), 10, CGColors::Danger, true) : Text(FString::Printf(TEXT("Level %d"), Room.level), 10, CGColors::Muted) ]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
					[ Pips ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 0)
					[ Text(Room.effectDesc, 10, CGColors::Muted, false, true) ]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[
						SNew(SBox).WidthOverride(78.f).HeightOverride(30.f)
						[
							Btn(bLocked ? TEXT("Unlock") : TEXT("Upgrade"),
								[this, RoomId]()
								{
									if (GI.IsValid() && GI->UpgradeRoom(RoomId)) { Rebuild(); }
								},
								bCanUpgrade, &DarkButton, 12, &CGColors::GoldBright,
								bCanUpgrade ? FString() : Reason)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 3, 0, 0)
					[ Text(CostText, 10, bCanUpgrade ? CGColors::Gold : CGColors::Muted) ]
				],
				FLinearColor(0, 0, 0, 0.45f), bLocked ? FLinearColor(0.25f, 0.2f, 0.18f) : CGColors::GoldDark, FMargin(7))
		];

		// room description tooltip on the whole row is set inside Framed's children implicitly; skip
	}
	return List;
}

TSharedRef<SWidget> SCGRoot::BuildRelicsTab()
{
	UCGGameInstance* G = GI.Get();
	if (!G) { return SNew(SSpacer); }

	const int32 Slots = G->RelicSlots();

	// equipped row
	TSharedRef<SHorizontalBox> Equipped = SNew(SHorizontalBox);
	for (int32 i = 0; i < Slots; ++i)
	{
		if (G->EquippedRelicIds.IsValidIndex(i))
		{
			const FCGRelic* R = G->FindRelic(G->EquippedRelicIds[i]);
			const FString RelicId = G->EquippedRelicIds[i];
			Equipped->AddSlot().AutoWidth().Padding(4, 0)
			[
				SNew(SButton)
				.ButtonStyle(&CardButton)
				.ContentPadding(FMargin(2))
				.ToolTipText(FText::FromString(R ? FString::Printf(TEXT("%s (%s)\n%s\n\nClick to unequip."), *R->name, *R->rarity, *R->description) : RelicId))
				.OnClicked_Lambda([this, RelicId]() { if (GI.IsValid()) { GI->UnequipRelic(RelicId); Rebuild(); } return FReply::Handled(); })
				[
					Framed(
						SNew(SBox).WidthOverride(58.f).HeightOverride(58.f)
						[
							SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
							[ SNew(SImage).Image(Brush(R ? R->iconSpriteId : FString())) ]
						],
						FLinearColor(0, 0, 0, 0.7f), R ? RarityColor(R->rarity) : CGColors::GoldDark, FMargin(2))
				]
			];
		}
		else
		{
			Equipped->AddSlot().AutoWidth().Padding(4, 0)
			[
				Framed(
					SNew(SBox).WidthOverride(58.f).HeightOverride(58.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
					[ Text(TEXT("+"), 22, CGColors::Muted) ],
					FLinearColor(0, 0, 0, 0.5f), FLinearColor(0.3f, 0.25f, 0.2f), FMargin(2))
			];
		}
	}

	// stored grid
	const TArray<FString> Stored = G->StoredRelicIds();
	TSharedRef<SUniformGridPanel> Grid = SNew(SUniformGridPanel).SlotPadding(FMargin(3));
	for (int32 i = 0; i < Stored.Num(); ++i)
	{
		const FCGRelic* R = G->FindRelic(Stored[i]);
		const FString RelicId = Stored[i];
		const bool bSlotFree = G->EquippedRelicIds.Num() < Slots;
		Grid->AddSlot(i % 4, i / 4)
		[
			SNew(SButton)
			.ButtonStyle(&CardButton)
			.ContentPadding(FMargin(2))
			.ToolTipText(FText::FromString(R ? FString::Printf(TEXT("%s (%s)\n%s\n\n%s"), *R->name, *R->rarity, *R->description,
				bSlotFree ? TEXT("Click to equip.") : TEXT("No free relic slots.")) : RelicId))
			.OnClicked_Lambda([this, RelicId, bSlotFree]()
			{
				if (GI.IsValid())
				{
					if (bSlotFree) { GI->EquipRelic(RelicId); Rebuild(); }
					else { ShowMessage(TEXT("No Free Slots"), TEXT("Unequip a relic first, or upgrade the Relic Vault.")); }
				}
				return FReply::Handled();
			})
			[
				Framed(
					SNew(SBox).WidthOverride(54.f).HeightOverride(54.f)
					[
						SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
						[ SNew(SImage).Image(Brush(R ? R->iconSpriteId : FString())) ]
					],
					FLinearColor(0, 0, 0, 0.7f), R ? RarityColor(R->rarity) : CGColors::GoldDark, FMargin(2))
			]
		];
	}

	return SNew(SScrollBox)
	+ SScrollBox::Slot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[ Text(TEXT("EQUIPPED RELICS"), 13, CGColors::GoldBright, true) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 6, 0, 12)
		[ Equipped ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[ Text(FString::Printf(TEXT("STORED RELICS (%d/%d)"), Stored.Num(), G->RelicStorage()), 13, CGColors::GoldBright, true) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 6)
		[ Grid ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 8)
		[ Text(TEXT("Relics passively modify party stats and expedition odds while equipped. Craft new ones in the Workshop."), 10, CGColors::Muted, false, true) ]
	];
}

// ----------------------------------------------------------------------
// Bottom bar
// ----------------------------------------------------------------------

TSharedRef<SWidget> SCGRoot::BuildBottomBar()
{
	UCGGameInstance* G = GI.Get();
	if (!G) { return SNew(SSpacer); }

	auto ActionCard = [this](const FString& Icon, const FString& Title, const FString& Caption,
		TFunction<void()> Fn, bool bEnabled, const FString& Tooltip)
	{
		TSharedRef<SButton> B = SNew(SButton)
			.ButtonStyle(&DarkButton)
			.IsEnabled(bEnabled)
			.ContentPadding(FMargin(4))
			.OnClicked_Lambda([Fn]() { Fn(); return FReply::Handled(); })
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(SBox).WidthOverride(40.f).HeightOverride(40.f)
					[
						SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
						[ SNew(SImage).Image(Brush(Icon)) ]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 2, 0, 0)
				[ Text(Title, 13, CGColors::GoldBright, true) ]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[ Text(Caption, 10, CGColors::Muted) ]
			];
		if (!Tooltip.IsEmpty()) { B->SetToolTipText(FText::FromString(Tooltip)); }
		return SNew(SBox).WidthOverride(150.f).HeightOverride(88.f)[ B ];
	};

	FString CraftReason, EmbarkReason;
	const bool bCanCraft = G->CanCraft(CraftReason);
	const bool bCanEmbark = G->CanEmbark(EmbarkReason);
	const FCGResourceBundle CraftCost = G->CraftCost();

	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot().AutoWidth().Padding(2, 0)
	[ ActionCard(TEXT("icon_recruit"), TEXT("RECRUIT"), TEXT("Find new talent"),
		[this]() { Modal = EModal::Recruit; Rebuild(); }, true, TEXT("Hire new guild members")) ]
	+ SHorizontalBox::Slot().AutoWidth().Padding(2, 0)
	[ ActionCard(TEXT("icon_craft"), TEXT("CRAFT"), TEXT("Forge relics"),
		[this]()
		{
			FString RelicId;
			if (GI.IsValid() && GI->CraftRelic(RelicId))
			{
				const FCGRelic* R = GI->FindRelic(RelicId);
				ShowMessage(TEXT("The Workshop Delivers"), R
					? FString::Printf(TEXT("Crafted: %s (%s)\n\n%s"), *R->name, *R->rarity, *R->description)
					: TEXT("Something was made. It skittered off."));
			}
		},
		bCanCraft, bCanCraft
			? FString::Printf(TEXT("Craft a random relic (%dg, %d dust)"), CraftCost.gold, CraftCost.arcaneDust)
			: CraftReason) ]
	+ SHorizontalBox::Slot().AutoWidth().Padding(2, 0)
	[ ActionCard(TEXT("icon_brew"), TEXT("BREW POTIONS"), TEXT("Mix elixirs"),
		[this]()
		{
			FString Summary;
			if (GI.IsValid())
			{
				const bool bOk = GI->BrewPotions(Summary);
				ShowMessage(bOk ? TEXT("Bottoms Up") : TEXT("The Cauldron Is Cold"), Summary);
			}
		},
		G->CanAfford(G->BrewCost()), TEXT("Heal the roster with potions (30 dust)")) ]
	+ SHorizontalBox::Slot().AutoWidth().Padding(2, 0)
	[ ActionCard(TEXT("icon_upgrade_room"), TEXT("UPGRADE ROOM"), TEXT("Improve the guild"),
		[this]() { RightTab = 1; Rebuild(); }, true, TEXT("Open the Rooms panel")) ]
	+ SHorizontalBox::Slot().AutoWidth().Padding(2, 0)
	[ ActionCard(TEXT("icon_rest"), TEXT("REST"), TEXT("Recover & heal"),
		[this]()
		{
			FString Summary;
			if (GI.IsValid())
			{
				const bool bOk = GI->RestGuild(Summary);
				ShowMessage(bOk ? TEXT("A Day of Rest") : TEXT("No Rest for the Broke"), Summary);
			}
		},
		G->Resources.supplies >= G->RestSupplyCost(),
		FString::Printf(TEXT("Rest one day (%d supplies): heal HP and mood"), G->RestSupplyCost())) ]
	+ SHorizontalBox::Slot().AutoWidth().Padding(2, 0)
	[ ActionCard(TEXT("icon_ledger"), TEXT("LEDGER"), TEXT("Review the books"),
		[this]() { Modal = EModal::Ledger; Rebuild(); }, true, TEXT("The guild's history and finances")) ]
	+ SHorizontalBox::Slot().FillWidth(1.f)[ SNew(SSpacer) ]
	+ SHorizontalBox::Slot().AutoWidth().Padding(2, 0)
	[
		SNew(SBox).WidthOverride(250.f).HeightOverride(88.f)
		[
			SNew(SButton)
			.ButtonStyle(&GoldButton)
			.IsEnabled(bCanEmbark)
			.HAlign(HAlign_Center).VAlign(VAlign_Center)
			.ToolTipText(FText::FromString(bCanEmbark ? TEXT("Send the party on the selected contract") : EmbarkReason))
			.OnClicked_Lambda([this]()
			{
				if (GI.IsValid())
				{
					LastResult = GI->Embark();
					Modal = EModal::Result;
					Rebuild();
				}
				return FReply::Handled();
			})
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[ Text(TEXT("EMBARK"), 26, CGColors::Ink, true) ]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[ Text(TEXT("Begin Expedition"), 12, CGColors::Ink) ]
			]
		]
	];
}

// ----------------------------------------------------------------------
// Modals
// ----------------------------------------------------------------------

TSharedRef<SWidget> SCGRoot::ModalShell(TSharedRef<SWidget> Content, float Width)
{
	return SNew(SBorder)
		.BorderImage(White())
		.BorderBackgroundColor(CGColors::Scrim)
		.HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(SBox).WidthOverride(Width)
			[
				Framed(Content, CGColors::Panel, CGColors::Gold, FMargin(22, 18))
			]
		];
}

TSharedRef<SWidget> SCGRoot::BuildModalOverlay()
{
	switch (Modal)
	{
	case EModal::Result:       return BuildResultModal();
	case EModal::Recruit:      return BuildRecruitModal();
	case EModal::Ledger:       return BuildLedgerModal();
	case EModal::Settings:     return BuildSettingsModal();
	case EModal::MemberDetail: return BuildMemberDetailModal();
	case EModal::Message:      return BuildMessageModal();
	default:                   return SNew(SSpacer);
	}
}

TSharedRef<SWidget> SCGRoot::BuildResultModal()
{
	UCGGameInstance* G = GI.Get();

	const bool bGood = LastResult.OutcomeTitle.Contains(TEXT("Success"));
	const FLinearColor TitleColor = LastResult.OutcomeTitle == TEXT("Great Success!") ? CGColors::GoldBright
		: bGood ? CGColors::Green
		: LastResult.OutcomeTitle == TEXT("Partial Success") ? CGColors::Orange : CGColors::Danger;

	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[ Text(LastResult.OutcomeTitle, 34, TitleColor, true) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 2, 0, 10)
		[ Text(FString::Printf(TEXT("Rolled %d against %d%% success"), LastResult.Roll, LastResult.Chance), 11, CGColors::Muted) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
		[ Text(LastResult.Flavor, 14, CGColors::Bone, false, true) ];

	// rewards
	TSharedRef<SHorizontalBox> Rewards = SNew(SHorizontalBox);
	auto AddR = [&](const FString& Icon, int32 V, const FString& Tip)
	{
		if (V <= 0) { return; }
		Rewards->AddSlot().AutoWidth().Padding(6, 0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(26.f).HeightOverride(26.f)
				[ SNew(SImage).Image(Brush(Icon)).ToolTipText(FText::FromString(Tip)) ]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[ Text(FString::Printf(TEXT("+%s"), *FString::FormatAsNumber(V)), 15, CGColors::Bone, true) ]
		];
	};
	AddR(TEXT("icon_gold"), LastResult.Rewards.gold, TEXT("Gold"));
	AddR(TEXT("icon_reputation"), LastResult.Rewards.reputation, TEXT("Reputation"));
	AddR(TEXT("icon_arcane_dust"), LastResult.Rewards.arcaneDust, TEXT("Arcane Dust"));
	if (LastResult.XPGained > 0)
	{
		Rewards->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(6, 0)
		[ Text(FString::Printf(TEXT("+%d XP"), LastResult.XPGained), 14, CGColors::Blue, true) ];
	}
	if (LastResult.ReputationLost > 0)
	{
		Rewards->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(6, 0)
		[ Text(FString::Printf(TEXT("-%d Reputation"), LastResult.ReputationLost), 14, CGColors::Danger, true) ];
	}
	Box->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 10)[ Rewards ];

	for (const FString& LineText : LastResult.InjuryLines)
	{
		const bool bLevel = LineText.Contains(TEXT("level"));
		Box->AddSlot().AutoHeight().HAlign(HAlign_Center)
		[ Text(LineText, 12, bLevel ? CGColors::Green : CGColors::Orange) ];
	}

	if (!LastResult.RelicFoundId.IsEmpty() && G)
	{
		if (const FCGRelic* R = G->FindRelic(LastResult.RelicFoundId))
		{
			Box->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 10)
			[
				Framed(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(SBox).WidthOverride(46.f).HeightOverride(46.f)
						[
							SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
							[ SNew(SImage).Image(Brush(R->iconSpriteId)) ]
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10, 0)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[ Text(FString::Printf(TEXT("Relic found: %s"), *R->name), 14, RarityColor(R->rarity), true) ]
						+ SVerticalBox::Slot().AutoHeight()
						[ Text(R->description, 11, CGColors::Muted, false, true) ]
					],
					FLinearColor(0.09f, 0.04f, 0.13f, 0.9f), RarityColor(R->rarity), FMargin(10, 8))
			];
		}
	}

	Box->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 14, 0, 0)
	[
		SNew(SBox).WidthOverride(240.f).HeightOverride(44.f)
		[ Btn(TEXT("Continue to Guild"), [this]() { Modal = EModal::None; Rebuild(); }, true, &GoldButton, 16, &CGColors::Ink) ]
	];

	return ModalShell(Box, 620.f);
}

TSharedRef<SWidget> SCGRoot::BuildRecruitModal()
{
	UCGGameInstance* G = GI.Get();
	if (!G) { return SNew(SSpacer); }

	TSharedRef<SScrollBox> List = SNew(SScrollBox);
	const TArray<const FCGMember*> Candidates = G->RecruitCandidates();
	if (Candidates.Num() == 0)
	{
		List->AddSlot()
		[ Text(TEXT("Every known monster and misfit already works for you."), 13, CGColors::Muted) ];
	}
	for (const FCGMember* M : Candidates)
	{
		FString Reason;
		const bool bCan = G->CanHire(*M, Reason);
		const FString Id = M->id;
		FString Traits;
		for (int32 i = 0; i < M->traits.Num(); ++i)
		{
			Traits += M->traits[i] + (i + 1 < M->traits.Num() ? TEXT(" · ") : TEXT(""));
		}
		List->AddSlot().Padding(0, 3)
		[
			Framed(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(52.f).HeightOverride(52.f)
					[ SNew(SImage).Image(Brush(M->portraitSpriteId)) ]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(10, 0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[ Text(FString::Printf(TEXT("%s — %s (Lv %d)"), *M->name, *M->className, M->level), 14, CGColors::Bone, true) ]
					+ SVerticalBox::Slot().AutoHeight()
					[ Text(FString::Printf(TEXT("M%d W%d Mg%d G%d S%d   %s"),
						M->stats.might, M->stats.wits, M->stats.magic, M->stats.guile, M->stats.spirit, *Traits), 11, CGColors::Muted) ]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[
						SNew(SBox).WidthOverride(80.f).HeightOverride(30.f)
						[
							Btn(TEXT("Hire"), [this, Id]()
								{
									if (GI.IsValid() && GI->Hire(Id)) { Rebuild(); }
								}, bCan, &GoldButton, 13, &CGColors::Ink, bCan ? FString() : Reason)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 3, 0, 0)
					[ Text(FString::Printf(TEXT("%dg / %d rep"), M->hireCostGold, M->hireCostRep), 10, CGColors::Gold) ]
				],
				FLinearColor(0, 0, 0, 0.45f), CGColors::GoldDark, FMargin(8))
		];
	}

	return ModalShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 8)
		[ Text(TEXT("RECRUIT HEROES"), 22, CGColors::GoldBright, true) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 8)
		[ Text(FString::Printf(TEXT("Roster %d / %d — reputation opens better doors"), G->Roster.Num(), G->RosterCapacity()), 12, CGColors::Muted) ]
		+ SVerticalBox::Slot().MaxHeight(520.f)
		[ List ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 12, 0, 0)
		[
			SNew(SBox).WidthOverride(160.f).HeightOverride(38.f)
			[ Btn(TEXT("Close"), [this]() { Modal = EModal::None; Rebuild(); }, true, &DarkButton, 14) ]
		],
		640.f);
}

TSharedRef<SWidget> SCGRoot::BuildLedgerModal()
{
	UCGGameInstance* G = GI.Get();
	if (!G) { return SNew(SSpacer); }

	TSharedRef<SScrollBox> List = SNew(SScrollBox);
	if (G->Ledger.Num() == 0)
	{
		List->AddSlot()[ Text(TEXT("The pages are blank. For now."), 13, CGColors::Muted) ];
	}
	for (const FString& LineText : G->Ledger)
	{
		List->AddSlot().Padding(0, 2)
		[ Text(LineText, 12, CGColors::Bone, false, true) ];
	}

	return ModalShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 10)
		[ Text(TEXT("THE GUILD LEDGER"), 22, CGColors::GoldBright, true) ]
		+ SVerticalBox::Slot().MaxHeight(520.f)
		[ List ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 12, 0, 0)
		[
			SNew(SBox).WidthOverride(160.f).HeightOverride(38.f)
			[ Btn(TEXT("Close"), [this]() { Modal = EModal::None; Rebuild(); }, true, &DarkButton, 14) ]
		],
		600.f);
}

TSharedRef<SWidget> SCGRoot::BuildSettingsModal()
{
	auto SetMode = [this](EWindowMode::Type Mode)
	{
		if (GEngine && GEngine->GetGameUserSettings())
		{
			UGameUserSettings* S = GEngine->GetGameUserSettings();
			S->SetFullscreenMode(Mode);
			S->ApplySettings(false);
		}
		Rebuild();
	};
	auto SetRes = [this](int32 W, int32 H)
	{
		if (GEngine && GEngine->GetGameUserSettings())
		{
			UGameUserSettings* S = GEngine->GetGameUserSettings();
			S->SetScreenResolution(FIntPoint(W, H));
			S->ApplySettings(false);
		}
		Rebuild();
	};

	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 12)
		[ Text(TEXT("SETTINGS"), 22, CGColors::GoldBright, true) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ Text(TEXT("Window Mode"), 13, CGColors::Muted, true) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(2, 0)
			[ Btn(TEXT("Fullscreen"), [SetMode]() { SetMode(EWindowMode::Fullscreen); }, true, &DarkButton, 12) ]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(2, 0)
			[ Btn(TEXT("Borderless"), [SetMode]() { SetMode(EWindowMode::WindowedFullscreen); }, true, &DarkButton, 12) ]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(2, 0)
			[ Btn(TEXT("Windowed"), [SetMode]() { SetMode(EWindowMode::Windowed); }, true, &DarkButton, 12) ]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ Text(TEXT("Resolution"), 13, CGColors::Muted, true) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(2, 0)
			[ Btn(TEXT("1920 x 1080"), [SetRes]() { SetRes(1920, 1080); }, true, &DarkButton, 12) ]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(2, 0)
			[ Btn(TEXT("1600 x 900"), [SetRes]() { SetRes(1600, 900); }, true, &DarkButton, 12) ]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(2, 0)
			[ Btn(TEXT("1280 x 720"), [SetRes]() { SetRes(1280, 720); }, true, &DarkButton, 12) ]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
		[ Text(TEXT("Audio: hook points are wired for click, embark, success, failure, relic and upgrade cues; assets ship post-MVP."), 10, CGColors::Muted, false, true) ];

	if (Screen == EScreen::Hub)
	{
		Box->AddSlot().AutoHeight().Padding(0, 0, 0, 6)
		[
			Btn(TEXT("Save & Return to Main Menu"), [this]()
			{
				if (GI.IsValid()) { GI->SaveGame(); }
				Screen = EScreen::MainMenu;
				Modal = EModal::None;
				Rebuild();
			}, true, &DarkButton, 13)
		];
	}
	Box->AddSlot().AutoHeight().Padding(0, 0, 0, 6)
	[ Btn(TEXT("Quit Game"), [this]() { DoQuit(); }, true, &DarkButton, 13, &CGColors::Danger) ];
	Box->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 8, 0, 0)
	[
		SNew(SBox).WidthOverride(160.f).HeightOverride(38.f)
		[ Btn(TEXT("Close"), [this]() { Modal = EModal::None; Rebuild(); }, true, &GoldButton, 14, &CGColors::Ink) ]
	];

	return ModalShell(Box, 460.f);
}

TSharedRef<SWidget> SCGRoot::BuildMemberDetailModal()
{
	UCGGameInstance* G = GI.Get();
	const FCGMember* M = G ? G->FindRosterMember(DetailMemberId) : nullptr;
	if (!M)
	{
		Modal = EModal::None;
		return SNew(SSpacer);
	}

	auto StatRow = [this](const FString& Name, int32 V)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)[ Text(Name, 13, CGColors::Muted) ]
			+ SHorizontalBox::Slot().AutoWidth()[ Text(FString::FromInt(V), 14, CGColors::Bone, true) ];
	};

	FString Traits = FString::Join(M->traits, TEXT(" · "));
	FString Skills = FString::Join(M->skills, TEXT(" · "));

	return ModalShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				Framed(
					SNew(SBox).WidthOverride(170.f).HeightOverride(250.f)
					[
						SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
						[ SNew(SImage).Image(Brush(M->bodySpriteId)) ]
					],
					FLinearColor(0, 0, 0, 0.6f), CGColors::GoldDark, FMargin(4))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(16, 0, 0, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[ Text(M->name, 24, CGColors::GoldBright, true) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
				[ Text(FString::Printf(TEXT("%s — Level %d (%d / %d XP)"), *M->className, M->level, M->xp, M->level * 100), 13, CGColors::Muted) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ HpBar(*M, 150.f) ]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0)
					[ Text(FString::Printf(TEXT("%d / %d HP"), M->hp, M->maxHp), 12, CGColors::Bone) ]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 8)
				[ Text(FString::Printf(TEXT("Mood: %s    Status: %s"), *M->mood, *M->status), 12, MoodColor(M->mood)) ]
				+ SVerticalBox::Slot().AutoHeight()[ StatRow(TEXT("Might"), M->stats.might) ]
				+ SVerticalBox::Slot().AutoHeight()[ StatRow(TEXT("Wits"), M->stats.wits) ]
				+ SVerticalBox::Slot().AutoHeight()[ StatRow(TEXT("Magic"), M->stats.magic) ]
				+ SVerticalBox::Slot().AutoHeight()[ StatRow(TEXT("Guile"), M->stats.guile) ]
				+ SVerticalBox::Slot().AutoHeight()[ StatRow(TEXT("Spirit"), M->stats.spirit) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
				[ Text(FString::Printf(TEXT("Traits: %s"), *Traits), 12, CGColors::Purple, false, true) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 0)
				[ Text(FString::Printf(TEXT("Skills: %s"), *Skills), 12, CGColors::Blue, false, true) ]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 14, 0, 0)
		[
			SNew(SBox).WidthOverride(160.f).HeightOverride(38.f)
			[ Btn(TEXT("Close"), [this]() { Modal = EModal::None; Rebuild(); }, true, &DarkButton, 14) ]
		],
		560.f);
}

TSharedRef<SWidget> SCGRoot::BuildMessageModal()
{
	return ModalShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 10)
		[ Text(MessageTitle, 20, CGColors::GoldBright, true) ]
		+ SVerticalBox::Slot().AutoHeight()
		[ Text(MessageBody, 14, CGColors::Bone, false, true) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 14, 0, 0)
		[
			SNew(SBox).WidthOverride(140.f).HeightOverride(38.f)
			[ Btn(TEXT("OK"), [this]() { Modal = EModal::None; Rebuild(); }, true, &GoldButton, 14, &CGColors::Ink) ]
		],
		440.f);
}

TSharedRef<SWidget> SCGRoot::BuildTutorialPopup()
{
	UCGGameInstance* G = GI.Get();
	if (!G) { return SNew(SSpacer); }

	return SNew(SBox).WidthOverride(560.f)
	[
		SNew(SBorder)
		.BorderImage(BoxBrush(TEXT("ui_parchment"), 0.18f))
		.Padding(FMargin(24, 16))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[ Text(FString::Printf(TEXT("Guildmistress's Notes  (%d/%d)"), G->TutorialStep + 1, UCGGameInstance::NumTutorialSteps), 11, CGColors::InkFaded, true) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 8)
			[ Text(G->TutorialText(), 14, CGColors::Ink, false, true) ]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[ Btn(TEXT("Next"), [this]() { if (GI.IsValid()) { GI->AdvanceTutorial(); } Rebuild(); }, true, &GoldButton, 13, &CGColors::Ink) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0)
				[ Btn(TEXT("Skip Tutorial"), [this]()
					{
						if (GI.IsValid())
						{
							while (GI->TutorialActive()) { GI->AdvanceTutorial(); }
						}
						Rebuild();
					}, true, &DarkButton, 12) ]
			]
		]
	];
}
