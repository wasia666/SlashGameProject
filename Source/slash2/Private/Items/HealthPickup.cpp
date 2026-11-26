#include "Items/HealthPickup.h"
#include "Components/SphereComponent.h"
#include "Interfaces/PickupReceiverInterface.h"
#include "Interfaces/PickupTypes.h"
#include "GameFramework/RotatingMovementComponent.h"

AHealthPickup::AHealthPickup()
{
	// 平面图标缓慢自转：任何角度靠近都能看到它
	RotateComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotateComponent"));
	RotateComponent->RotationRate = FRotator(0.f, 60.f, 0.f);

	// 交互检测复用"物品 Sphere <-> 角色 Mesh"的重叠，这里显式保证参数正确
	if (Sphere)
	{
		Sphere->SetSphereRadius(180.f);
		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		Sphere->SetGenerateOverlapEvents(true);

		// 蓝图把 ItemMesh 缩到 0.45，若球体跟随缩放，180 的半径只剩 81cm；
		// 让本组件的缩放独立于父级，判定范围才等于配置值
		Sphere->SetAbsolute(false, false, true);
	}

	PromptText = FText::FromString(TEXT("拾取 生命药水"));
	UnavailablePromptText = FText::FromString(TEXT("生命值已满"));
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* /*OverlappedComponent*/, AActor* /*OtherActor*/, UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	// 故意不调用基类：治疗物不走"碰到即拾取"，也不写入 OverlappingItem（避免 E 键走拔刀分支）
}

void AHealthPickup::OnSphereEndOverlap(UPrimitiveComponent* /*OverlappedComponent*/, AActor* /*OtherActor*/, UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	// 同上：不调用基类，避免把别的物品的 OverlappingItem 清空
}

FText AHealthPickup::GetInteractionPrompt(AActor* Interactor) const
{
	if (bConsumed || !IsValid(Interactor) || !Cast<IPickupReceiverInterface>(Interactor))
	{
		return FText::GetEmpty();
	}

	// 可用：显示"E 拾取 生命药水"；满血：只显示"生命值已满"
	// （交互组件看到 CanInteract()==false 会隐藏白底键帽，所以玩家能看出为什么按不动）
	return CanBePickedUpBy(Interactor) ? PromptText : UnavailablePromptText;
}

bool AHealthPickup::CanInteract(AActor* Interactor) const
{
	return !bConsumed && IsValid(Interactor) && CanBePickedUpBy(Interactor);
}

bool AHealthPickup::CanBePickedUpBy(AActor* Interactor) const
{
	const IPickupReceiverInterface* Receiver = Cast<IPickupReceiverInterface>(Interactor);
	if (!Receiver)
	{
		return false;
	}

	FPickupPayload Probe;
	Probe.Type = EPickupType::Health;
	Probe.Amount = HealAmount;
	Probe.SourceActor = const_cast<AHealthPickup*>(this);

	return Receiver->CanReceivePickup(Probe);
}

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

	FPickupPayload Payload;
	Payload.Type = EPickupType::Health;
	Payload.Amount = HealAmount;
	Payload.SourceActor = this;

	if (!Receiver->CanReceivePickup(Payload) || !Receiver->ReceivePickup(Payload))
	{
		return false;
	}

	bConsumed = true;
	SpawnPickupSystem();
	SpawnPickupSound();
	Destroy();
	return true;
}
