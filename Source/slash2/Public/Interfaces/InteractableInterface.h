// 可交互物接口：由拾取物实现，交互组件只通过它询问提示并触发交互。

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class SLASH2_API IInteractableInterface
{
	GENERATED_BODY()

public:
	/** 提示文字（如"拾取 生命药水"）；返回空文本表示当前不可交互，UI 不显示 */
	virtual FText GetInteractionPrompt(AActor* Interactor) const { return FText::GetEmpty(); }

	/** 当前是否真的能交互（如满血时治疗物不可拾取）；false 时 UI 仍显示说明文字但不显示按键，按键被吃掉不落到其他逻辑 */
	virtual bool CanInteract(AActor* Interactor) const { return true; }

	/** 玩家按下交互键；返回 true 表示交互成功，这次按键被消费 */
	virtual bool Interact(AActor* Interactor) { return false; }

	/** 成为当前交互目标 / 失去焦点（给需要头顶图标的物品预留）*/
	virtual void OnFocused(AActor* Interactor) {}
	virtual void OnUnfocused(AActor* Interactor) {}
};
