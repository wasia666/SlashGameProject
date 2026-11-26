// 交互提示 UI：纯 C++ 构建 Slate，不需要额外 WBP 资源。
// 屏幕空间渲染 => 尺寸恒定、天然正对镜头；白底键帽 + 大写交互键字母。

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptWidget.generated.h"

class STextBlock;

UCLASS()
class SLASH2_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 显示/更新提示 */
	void ShowPrompt(const FText& KeyText, const FText& ActionText);

	/** 隐藏提示 */
	void HidePrompt();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	TSharedPtr<STextBlock> KeyLabel;
	TSharedPtr<STextBlock> ActionLabel;
};
