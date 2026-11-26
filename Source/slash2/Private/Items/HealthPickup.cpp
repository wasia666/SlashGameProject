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
	}

	PromptText = FText::FromString(TEXT("拾取 生命药水"));
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
	if (bConsumed || !IsValid(Interactor))
	{
		return FText::GetEmpty();
	}

	const IPickupReceiverInterface* Receiver = Cast<IPickupReceiverInterface>(Interactor);
	if (!Receiver)
	{
		return FText::GetEmpty();
	}

	FPickupPayload Probe;
	Probe.Type = EPickupType::Health;
	Probe.Amount = HealAmount;
	Probe.SourceActor = const_cast<AHealthPickup*>(this);

	// 满血等不可用状态直接返回空文本，交互组件据此不显示提示、也不消耗按键
	return Receiver->CanReceivePickup(Probe) ? PromptText : FText::GetEmpty();
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
