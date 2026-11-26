// 可交互物接口：由拾取物实现，交互组件只通过它询问提示并触发交互。

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

/** 接口的 UObject 部分：只作为反射入口，本身没有逻辑 */
UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/** 可交互物契约：提示文案 + 此刻能否交互 + 按键行为 */
class SLASH2_API IInteractableInterface
{
	GENERATED_BODY()

public:
	/** 提示文字（如"拾取 生命药水"）；返回空文本表示当前没有提示，UI 整个不显示 */
	virtual FText GetInteractionPrompt(AActor* Interactor) const { return FText::GetEmpty(); }

	/** 此刻是否真的能交互（如满血时治疗物不可拾取）；false 时 UI 仍显示说明文字但不显示按键，
	    按键不消费、回落到角色原有的装备/收刀逻辑 */
	virtual bool CanInteract(AActor* Interactor) const { return true; }

	/** 交互优先级：数值越大越优先。同一时刻有多个可交互目标时先比优先级、再比距离；
	    默认 0，即普通可交互物之间仍按距离取最近 */
	virtual int32 GetInteractionPriority(AActor* Interactor) const { return 0; }

	/** 玩家按下交互键；返回 true 表示交互成功，这次按键被消费 */
	virtual bool Interact(AActor* Interactor) { return false; }

	/** 成为当前交互目标：给需要头顶图标/高亮的物品用（默认空实现）*/
	virtual void OnFocused(AActor* Interactor) {}

	/** 失去焦点时调用（默认空实现）*/
	virtual void OnUnfocused(AActor* Interactor) {}
};
