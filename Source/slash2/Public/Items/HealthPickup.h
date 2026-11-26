// 治疗拾取物：靠近显示交互提示，按键后通过 IPickupReceiverInterface 回血。
// 本类不 include 任何角色头文件、不 Cast 具体角色，只做接口调用。

#pragma once

#include "CoreMinimal.h"
#include "Items/Items.h"
#include "Interfaces/InteractableInterface.h"
#include "HealthPickup.generated.h"

UCLASS()
class SLASH2_API AHealthPickup : public AItems, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AHealthPickup();

	//~ Begin IInteractableInterface
	virtual FText GetInteractionPrompt(AActor* Interactor) const override;
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual bool Interact(AActor* Interactor) override;
	//~ End IInteractableInterface

protected:
	/** 屏蔽基类的自动拾取：治疗物必须玩家按键才生效，也不能污染 OverlappingItem */
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	/** 一次回复的生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	float HealAmount = 30.f;

	/** 交互提示文字 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FText PromptText;

	/** 当前不可拾取（如满血）时显示的说明文字：不显示按键，只解释为什么按不动 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FText UnavailablePromptText;

private:
	/** 用探测载荷询问接收者当前能否拾取（满血返回 false）*/
	bool CanBePickedUpBy(AActor* Interactor) const;

	/** 让平面图标朝向各个方向，避免从侧面看只剩一条线 */
	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	class URotatingMovementComponent* RotateComponent;

	bool bConsumed = false;
};
