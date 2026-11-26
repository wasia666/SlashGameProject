// 交互组件：挂在玩家角色上，周期性扫描与自身重叠的可交互物，
// 选出一个目标显示提示，并把交互键转发给它（通过 IInteractableInterface）。
// 组件不认识任何具体物品类，物品也不认识本组件。

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UInteractionPromptWidget;

/** 玩家侧的交互探测器：负责"选目标 -> 显示提示 -> 转发按键"三件事 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SLASH2_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 构造函数：关闭 Tick、写入默认按键文字与提示 Widget 类 */
	UInteractionComponent();

	/** 开局：启动扫描定时器并立刻扫一次，保证进游戏就有提示 */
	virtual void BeginPlay() override;

	/** 结束：停掉定时器并移除提示 Widget，避免残留到下一个关卡 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 玩家按下交互键：返回 true 表示这次按键被交互消费（角色侧据此跳过装备/收刀） */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryInteract();

	/** 当前交互目标（没有目标时为 nullptr，供蓝图/调试查询） */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

protected:
	/** 扫描一次重叠：过滤出可交互者，按"可用优先 -> 优先级 -> 距离"选出唯一目标 */
	void ScanForInteractables();

	/** 切换目标并刷新提示：旧目标收 OnUnfocused、新目标收 OnFocused；
	    ActionText / bUsable 直接用扫描时算好的结果，避免再问目标一遍 */
	void SetCurrentTarget(AActor* NewTarget, const FText& ActionText, bool bUsable);

	/** 按给定文案与可用性刷新 UI；内容没变就跳过，避免反复刷 Slate */
	void UpdatePrompt(const FText& ActionText, bool bUsable);

	/** 首次需要显示提示时才创建 Widget（全程没提示就不占内存） */
	void EnsurePromptWidget();

	/** 扫描间隔（秒）：越小越灵敏、开销越高 */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0.02"))
	float ScanInterval = 0.1f;

	/** 额外距离上限（厘米）：防止物品 Sphere 半径过大导致隔墙能交互 */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0.0"))
	float MaxInteractionDistance = 320.f;

	/** 提示里显示的按键文字（默认 "E"） */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	FText InteractKeyText;

	/** 提示 Widget 类型，默认使用原生 UInteractionPromptWidget */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TSubclassOf<UInteractionPromptWidget> PromptWidgetClass;

	/** 运行时创建的提示 Widget（懒创建，结束游戏时移除） */
	UPROPERTY(Transient)
	UInteractionPromptWidget* PromptWidget;

private:
	/** 当前选中的交互目标（弱引用：目标被销毁后自动变空） */
	TWeakObjectPtr<AActor> CurrentTarget;

	/** 重叠查询复用缓冲：避免每次扫描都重新分配数组 */
	TArray<AActor*> OverlapBuffer;

	/** 周期扫描的定时器句柄 */
	FTimerHandle ScanTimerHandle;

	/** 最近一次显示的提示文字：既用于 UI 去重，也供按键时复用（省掉一次接口调用） */
	FText LastPromptText;

	/** 最近一次目标是否可用（可交互=true，满血等=false），与提示文字一起做刷新判断 */
	bool bLastPromptUsable = false;
};
