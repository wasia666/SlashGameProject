// 治疗拾取物：靠近显示交互提示，按键后通过 IPickupReceiverInterface 回血。
// 本类不 include 任何角色头文件、不 Cast 具体角色，只做接口调用。

#pragma once

#include "CoreMinimal.h"
#include "Items/Items.h"
#include "Interfaces/InteractableInterface.h"
#include "HealthPickup.generated.h"

/** 治疗药水：继承 AItems 复用模型/球体/音效特效，额外实现"按键拾取"的交互接口 */
UCLASS()
class SLASH2_API AHealthPickup : public AItems, public IInteractableInterface
{
	GENERATED_BODY()

public:
	/** 构造函数：配置球体判定范围、初始提示文案与图标自转 */
	AHealthPickup();

	//~ Begin IInteractableInterface
	/** 返回当前应显示的文字：可用="拾取 <名字>"，满血="生命值已满" */
	virtual FText GetInteractionPrompt(AActor* Interactor) const override;

	/** 此刻能否真的拾取（已消耗/无接收者/满血 都返回 false）*/
	virtual bool CanInteract(AActor* Interactor) const override;

	/** 交互优先级：只要药水可用就高于武器，缺血时按 E 一定是回血而不是切武器 */
	virtual int32 GetInteractionPriority(AActor* Interactor) const override;

	/** 玩家按下 E：问接收者能否收，能收才加血并销毁自己；返回 true 表示按键被消费 */
	virtual bool Interact(AActor* Interactor) override;
	//~ End IInteractableInterface

protected:
	/** 屏蔽基类的自动拾取：治疗物必须玩家按键才生效，也不能污染 OverlappingItem */
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	/** 同上：离开范围时也不调用基类，避免清掉别的物品的重叠记录 */
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	/** 一次回复的生命值（点数）*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	float HealAmount = 30.f;

	/** 物品显示名：蓝图里改这里即可，提示会固定拼成"拾取 <名字>" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FText PickupDisplayName;

	/** 当前不可拾取（如满血）时显示的说明文字：不显示按键，只解释为什么按不动 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FText UnavailablePromptText;

private:
	/** 用探测载荷询问接收者当前能否拾取（满血返回 false）*/
	bool CanBePickedUpBy(AActor* Interactor) const;

	/** 让平面图标朝向各个方向，避免从侧面看只剩一条线 */
	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	class URotatingMovementComponent* RotateComponent;

	/** 是否已经被拾取过（保证一次物品只生效一次）*/
	bool bConsumed = false;
};
