// 交互提示 UI：纯 C++ 构建 Slate，不需要额外 WBP 资源。
// 屏幕空间渲染 => 尺寸恒定、天然正对镜头；白底键帽 + 大写交互键字母。

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptWidget.generated.h"

class STextBlock;

/** 底部居中的交互提示：左边白底键帽（如 E），右边动作文字（如"拾取 生命药水"）*/
UCLASS()
class SLASH2_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 显示/更新提示：KeyText 为空时折叠键帽，只显示 ActionText（用于"生命值已满"这类说明）*/
	void ShowPrompt(const FText& KeyText, const FText& ActionText);

	/** 隐藏提示（整块 Widget 折叠，不占布局）*/
	void HidePrompt();

protected:
	/** 构建 Slate 树：外层 SBox 负责底部居中，内层是"键帽 + 文字"横向条 */
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	/** 白底键帽的容器（固定 46x46）：不可交互时整体折叠，只留说明文字 */
	TSharedPtr<SWidget> KeyBoxWidget;

	/** 键帽里的字符（默认 "E"）*/
	TSharedPtr<STextBlock> KeyLabel;

	/** 键帽右侧的动作说明文字 */
	TSharedPtr<STextBlock> ActionLabel;
};
