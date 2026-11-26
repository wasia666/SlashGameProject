// 交互组件：挂在玩家角色上，周期性扫描与自身重叠的可交互物，
// 选最近的一个显示提示，并把交互键转发给目标（通过 IInteractableInterface）。
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
	/** 构造函数：关闭 Tick、写入默认交互键文字与提示 Widget 类 */
	UInteractionComponent();

	/** 开局启动扫描定时器并立刻扫一次，保证进游戏就能看到提示 */
	virtual void BeginPlay() override;

	/** 结束时停掉定时器并移除提示 Widget，避免残留到下一个关卡 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 玩家按下交互键：返回 true 表示这次按键被交互消费（角色侧据此跳过装备/收刀） */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryInteract();

	/** 当前交互目标（没有目标时为 nullptr，供蓝图/调试查询）*/
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

protected:
	/** 扫描一次重叠：过滤出可交互者，取距离最近且"有提示"的那个作为目标 */
	void ScanForInteractables();

	/** 切换目标：旧目标收 OnUnfocused、新目标收 OnFocused，再刷新提示 */
	void SetCurrentTarget(AActor* NewTarget);

	/** 按当前目标刷新提示文字与键帽显隐；内容没变就跳过，避免每帧刷 Slate */
	void UpdatePrompt();

	/** 首次需要显示提示时懒创建 Widget 并挂到视口 */
	void EnsurePromptWidget();

	/** 扫描间隔（秒）：越小越灵敏、开销越高 */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0.02"))
	float ScanInterval = 0.1f;

	/** 额外距离上限（厘米）：防止物品 Sphere 半径过大导致隔墙能交互 */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0.0"))
	float MaxInteractionDistance = 320.f;

	/** 提示里显示的按键文字（默认 "E"）*/
	UPROPERTY(EditAnywhere, Category = "Interaction")
	FText InteractKeyText;

	/** 提示 Widget 类型，默认使用原生 UInteractionPromptWidget */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TSubclassOf<UInteractionPromptWidget> PromptWidgetClass;

	/** 运行时创建的提示 Widget（懒加载，结束游戏时移除）*/
	UPROPERTY(Transient)
	UInteractionPromptWidget* PromptWidget;

private:
	/** 当前选中的交互目标（弱引用：目标被销毁后自动变空）*/
	TWeakObjectPtr<AActor> CurrentTarget;

	/** 重叠查询复用缓冲，避免每次扫描都分配数组 */
	TArray<AActor*> OverlapBuffer;

	/** 周期扫描的定时器句柄 */
	FTimerHandle ScanTimerHandle;

	/** 上一次显示的提示文字（用于判断是否需要刷新 UI）*/
	FText LastPromptText;

	/** 上一次目标是否可用（可交互=true，满血等=false），与提示文字一起做刷新判断 */
	bool bLastPromptUsable = false;
};
