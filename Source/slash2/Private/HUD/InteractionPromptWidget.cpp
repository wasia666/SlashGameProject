#include "HUD/InteractionPromptWidget.h"

#include "Brushes/SlateColorBrush.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	/** 白底刷：复用同一实例，避免每次重建都新建对象 */
	FSlateColorBrush& GetWhiteBrush()
	{
		static FSlateColorBrush WhiteBrush(FLinearColor(1.f, 1.f, 1.f, 1.f));
		return WhiteBrush;
	}
}

TSharedRef<SWidget> UInteractionPromptWidget::RebuildWidget()
{
	return SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0.f, 0.f, 0.f, 140.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(46.f)
				.HeightOverride(46.f)
				[
					SNew(SBorder)
					.BorderImage(&GetWhiteBrush())
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(KeyLabel, STextBlock)
						.Text(FText::FromString(TEXT("E")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 26))
						.ColorAndOpacity(FLinearColor::Black)
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(12.f, 0.f, 0.f, 0.f))
			[
				SAssignNew(ActionLabel, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
				.ColorAndOpacity(FLinearColor::White)
				.ShadowOffset(FVector2D(1.f, 1.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.9f))
			]
		];
}

void UInteractionPromptWidget::ShowPrompt(const FText& KeyText, const FText& ActionText)
{
	if (KeyLabel.IsValid())
	{
		KeyLabel->SetText(KeyText.IsEmpty() ? FText::FromString(TEXT("E")) : KeyText);
	}
	if (ActionLabel.IsValid())
	{
		ActionLabel->SetText(ActionText);
	}
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UInteractionPromptWidget::HidePrompt()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
