#include "Items/HealthPickup.h"
#include "Components/SphereComponent.h"
#include "Interfaces/PickupReceiverInterface.h"
#include "Interfaces/PickupTypes.h"
#include "GameFramework/RotatingMovementComponent.h"

/** 构造函数：配置交互球体、图标自转和两条提示文案 */
AHealthPickup::AHealthPickup()
{
	// 平面图标缓慢自转：任何角度靠近都能看清它
	RotateComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotateComponent"));
	RotateComponent->RotationRate = FRotator(0.f, 60.f, 0.f);

	// 交互检测复用"物品 Sphere <-> 角色 Mesh"的重叠，这里显式保证参数正确
	if (Sphere)
	{
		Sphere->SetSphereRadius(180.f);                              // 判定半径约 1.8m
		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);   // 只做查询，不参与物理
		Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		Sphere->SetGenerateOverlapEvents(true);                      // 必须开，否则不产生重叠事件

		// 蓝图把 ItemMesh 缩到 0.45，若球体跟随缩放，180 的半径只剩 81cm；
		// 让本组件的缩放独立于父级，判定范围才等于配置值
		Sphere->SetAbsolute(false, false, true);
	}

	PickupDisplayName = FText::FromString(TEXT("生命药水"));
	UnavailablePromptText = FText::FromString(TEXT("生命值已满"));
}

/** 进入范围：故意什么都不做（治疗物要按键才生效，不走基类的"碰到即拾取"） */
void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* /*OverlappedComponent*/, AActor* /*OtherActor*/, UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	// 故意不调用基类：治疗物不走"碰到即拾取"，也不写入 OverlappingItem（否则 E 键会走拔刀分支）
}

/** 离开范围：同样不调用基类，保持与其他物品的重叠状态互不干扰 */
void AHealthPickup::OnSphereEndOverlap(UPrimitiveComponent* /*OverlappedComponent*/, AActor* /*OtherActor*/, UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	// 同上：不调用基类，避免把别的物品的 OverlappingItem 清空
}

/** 提示文案：可用显示"拾取 <名字>"；满血显示"生命值已满"（组件会隐藏白底键帽） */
FText AHealthPickup::GetInteractionPrompt(AActor* Interactor) const
{
	// 已消耗或对方不是接收者：没有可显示内容，交互组件据此跳过本物品
	if (bConsumed || !GetPickupReceiver(Interactor))
	{
		return FText::GetEmpty();
	}

	return CanBePickedUpBy(Interactor) ? MakePickupPrompt(PickupDisplayName) : UnavailablePromptText;
}

/** 此刻能否真的拾取：没被消耗、且对方愿意收（满血为 false） */
bool AHealthPickup::CanInteract(AActor* Interactor) const
{
	return !bConsumed && CanBePickedUpBy(Interactor);
}

/** 交互优先级：药水(100) 始终高于武器(默认 0)，缺血时按 E 一定先回血 */
int32 AHealthPickup::GetInteractionPriority(AActor* /*Interactor*/) const
{
	return 100;
}

/** 取交互者身上的接收者接口：无效或没实现接口都返回 nullptr */
const IPickupReceiverInterface* AHealthPickup::GetPickupReceiver(AActor* Interactor) const
{
	return IsValid(Interactor) ? Cast<IPickupReceiverInterface>(Interactor) : nullptr;
}

/** 构造本物品的载荷：探测与真正拾取共用，避免两处重复赋值 */
FPickupPayload AHealthPickup::MakePayload() const
{
	FPickupPayload Payload;
	Payload.Type = EPickupType::Health;                        // 本物品只产出生命值
	Payload.Amount = HealAmount;                               // 一次回复的点数
	Payload.SourceActor = const_cast<AHealthPickup*>(this);    // 弱引用只用指针，const 方法里去掉 const
	return Payload;
}

/** 询问接收者"现在能收吗"：纯只读判断，不改任何状态 */
bool AHealthPickup::CanBePickedUpBy(AActor* Interactor) const
{
	const IPickupReceiverInterface* Receiver = GetPickupReceiver(Interactor);
	return Receiver && Receiver->CanReceivePickup(MakePayload());
}

/** 真正拾取：接收成功才播特效音效并销毁自己；失败时物品原样保留 */
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

	// 只调 ReceivePickup：接收方内部会再判断一次能否接收（满血会拒绝），
	// 所以不需要在这里先问一次，物品也不会被白白吃掉
	if (!Receiver->ReceivePickup(MakePayload()))
	{
		return false;
	}

	bConsumed = true;
	SpawnPickupSystem();    // 拾取特效（AItems 基类提供）
	SpawnPickupSound();     // 拾取音效（AItems 基类提供）
	Destroy();
	return true;
}
