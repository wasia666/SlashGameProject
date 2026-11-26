// 交互组件：挂在玩家角色上，周期性扫描与自身重叠的可交互物，
// 选最近的一个显示提示，并把交互键转发给目标（通过 IInteractableInterface）。
// 组件不认识任何具体物品类，物品也不认识本组件。

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UInteractionPromptWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SLASH2_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 玩家按下交互键：返回 true 表示这次按键被交互消费 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

protected:
	void ScanForInteractables();
	void SetCurrentTarget(AActor* NewTarget);
	void UpdatePrompt();
	void EnsurePromptWidget();

	/** 扫描间隔（秒）：越小越灵敏、开销越高 */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0.02"))
	float ScanInterval = 0.1f;

	/** 额外距离上限（厘米），防止物品 Sphere 半径过大 */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0.0"))
	float MaxInteractionDistance = 320.f;

	/** 提示中显示的交互键文字 */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	FText InteractKeyText;

	/** 提示 Widget 类型，默认使用原生 UInteractionPromptWidget */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TSubclassOf<UInteractionPromptWidget> PromptWidgetClass;

	UPROPERTY(Transient)
	UInteractionPromptWidget* PromptWidget;

private:
	TWeakObjectPtr<AActor> CurrentTarget;
	TArray<AActor*> OverlapBuffer;
	FTimerHandle ScanTimerHandle;
	FText LastPromptText;
	bool bLastPromptUsable = false;
};
