#include "HUD/InteractionPromptWidget.h"

#include "Brushes/SlateColorBrush.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	/** 白底刷：静态复用同一实例，避免每次重建 Slate 都新建对象 */
	FSlateColorBrush& GetWhiteBrush()
	{
		static FSlateColorBrush WhiteBrush(FLinearColor(1.f, 1.f, 1.f, 1.f));
		return WhiteBrush;
	}
}

/** 构建 Slate 树：底部居中，一行 = [白底键帽][动作文字] */
TSharedRef<SWidget> UInteractionPromptWidget::RebuildWidget()
{
	return SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0.f, 0.f, 0.f, 140.f))    // 距屏幕底部 140，避免压住 HUD 血条
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SAssignNew(KeyBoxWidget, SBox)      // 键帽外框：固定 46x46，可整体折叠
				.WidthOverride(46.f)
				.HeightOverride(46.f)
				[
					SNew(SBorder)
					.BorderImage(&GetWhiteBrush())  // 纯白底，不需要贴图
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(KeyLabel, STextBlock)
						.Text(FText::FromString(TEXT("E")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 26))   // 大号粗体，远处也看得清
						.ColorAndOpacity(FLinearColor::Black)               // 白底配黑字
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(12.f, 0.f, 0.f, 0.f))  // 与键帽留 12 像素间距
			[
				SAssignNew(ActionLabel, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
				.ColorAndOpacity(FLinearColor::White)
				.ShadowOffset(FVector2D(1.f, 1.f))                          // 黑描边：亮场景里也看得清
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.9f))
			]
		];
}

/** 显示提示：空键文本代表"当前不可用"，此时隐藏键帽只留说明 */
void UInteractionPromptWidget::ShowPrompt(const FText& KeyText, const FText& ActionText)
{
	// 空键文本 = 当前不可交互（如满血）：隐藏白底键帽，只显示说明文字
	const bool bShowKey = !KeyText.IsEmpty();
	if (KeyBoxWidget.IsValid())
	{
		KeyBoxWidget->SetVisibility(bShowKey ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (bShowKey && KeyLabel.IsValid())
	{
		KeyLabel->SetText(KeyText);
	}
	if (ActionLabel.IsValid())
	{
		ActionLabel->SetText(ActionText);
	}
	SetVisibility(ESlateVisibility::HitTestInvisible);  // 只显示不挡鼠标点击
}

/** 隐藏提示：整体折叠，不参与布局也不接收命中测试 */
void UInteractionPromptWidget::HidePrompt()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
