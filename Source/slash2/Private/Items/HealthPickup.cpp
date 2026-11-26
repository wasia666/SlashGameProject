#include "Items/HealthPickup.h"
#include "Components/SphereComponent.h"
#include "Interfaces/PickupReceiverInterface.h"
#include "Interfaces/PickupTypes.h"
#include "GameFramework/RotatingMovementComponent.h"

/** 构造函数：配置交互球体、图标自转和两条提示文案 */
AHealthPickup::AHealthPickup()
{
	// 平面图标缓慢自转：任何角度靠近都能看到它
	RotateComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotateComponent"));
	RotateComponent->RotationRate = FRotator(0.f, 60.f, 0.f);

	// 交互检测复用"物品 Sphere <-> 角色 Mesh"的重叠，这里显式保证参数正确
	if (Sphere)
	{
		Sphere->SetSphereRadius(180.f);                              // 判定半径约 1.8m
		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);   // 只查询、不参与物理
		Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		Sphere->SetGenerateOverlapEvents(true);                      // 必须开，否则不产生重叠事件

		// 蓝图把 ItemMesh 缩到 0.45，若球体跟随缩放，180 的半径只剩 81cm；
		// 让本组件的缩放独立于父级，判定范围才等于配置值
		Sphere->SetAbsolute(false, false, true);
	}

	PickupDisplayName = FText::FromString(TEXT("生命药水"));
	UnavailablePromptText = FText::FromString(TEXT("生命值已满"));
}

/** 进入范围：故意什么都不做（治疗物要按键才生效，不走基类的"碰到即拾取"）*/
void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* /*OverlappedComponent*/, AActor* /*OtherActor*/, UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	// 故意不调用基类：治疗物不走"碰到即拾取"，也不写入 OverlappingItem（避免 E 键走拔刀分支）
}

/** 离开范围：同样不调用基类，保持与其他物品的重叠状态互不干扰 */
void AHealthPickup::OnSphereEndOverlap(UPrimitiveComponent* /*OverlappedComponent*/, AActor* /*OtherActor*/, UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	// 同上：不调用基类，避免把别的物品的 OverlappingItem 清空
}

/** 提示文案：没被消耗、交互者能接收时才给文字；满血给"不可用"说明 */
FText AHealthPickup::GetInteractionPrompt(AActor* Interactor) const
{
	if (bConsumed || !IsValid(Interactor) || !Cast<IPickupReceiverInterface>(Interactor))
	{
		return FText::GetEmpty();
	}

	// 可用：显示"E 拾取 生命药水"；满血：只显示"生命值已满"
	// （交互组件看到 CanInteract()==false 会隐藏白底键帽，所以玩家能看出为什么按不动）
	return CanBePickedUpBy(Interactor) ? MakePickupPrompt(PickupDisplayName) : UnavailablePromptText;
}

/** 能否交互：没消耗、交互者有效、且接收者当前愿意收 */
bool AHealthPickup::CanInteract(AActor* Interactor) const
{
	return !bConsumed && IsValid(Interactor) && CanBePickedUpBy(Interactor);
}

/** 交互优先级：药水（100）永远高于武器（默认 0），缺血时按键优先回血 */
int32 AHealthPickup::GetInteractionPriority(AActor* /*Interactor*/) const
{
	return 100;
}

/** 用一次探测请求问接收者"你现在能收这个药水吗"，不改动任何状态 */
bool AHealthPickup::CanBePickedUpBy(AActor* Interactor) const
{
	const IPickupReceiverInterface* Receiver = Cast<IPickupReceiverInterface>(Interactor);
	if (!Receiver)
	{
		return false;
	}

	FPickupPayload Probe;                        // 探测载荷：只用于询问，不真的加血
	Probe.Type = EPickupType::Health;
	Probe.Amount = HealAmount;
	Probe.SourceActor = const_cast<AHealthPickup*>(this);

	return Receiver->CanReceivePickup(Probe);
}

/** 真正拾取：接收者确认能收后加血，然后播特效音效并销毁自己 */
bool AHealthPickup::Interact(AActor* Interactor)
{
	if (bConsumed || !IsValid(Interactor))
	{
		return false;
	}

	IPickupReceiverInterface* Receiver = Cast<IPickupReceiverInterface>(Interactor);
	if (!Receiver)
	{
		return false;
	}

	FPickupPayload Payload;                      // 正式载荷：交给接收者生效
	Payload.Type = EPickupType::Health;
	Payload.Amount = HealAmount;
	Payload.SourceActor = this;

	// 先问再给：接收者拒绝（如满血）时物品保留，不白白消耗
	if (!Receiver->CanReceivePickup(Payload) || !Receiver->ReceivePickup(Payload))
	{
		return false;
	}

	bConsumed = true;
	SpawnPickupSystem();                         // 拾取特效（AItems 基类提供）
	SpawnPickupSound();                          // 拾取音效（AItems 基类提供）
	Destroy();
	return true;
}
