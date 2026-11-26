// 拾取接收者接口：拾取物只认这个接口，不 Cast 任何具体角色类。

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interfaces/PickupTypes.h"
#include "PickupReceiverInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPickupReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 任何"能收东西的 Actor"实现本接口即可接收拾取物，
 * 效果由接收方按 FPickupPayload 自行决定（加血/加钱/加灵魂...）。
 */
class SLASH2_API IPickupReceiverInterface
{
	GENERATED_BODY()

public:
	/** 能否接收：例如满血时拒绝治疗物，拾取物据此不消耗自己 */
	virtual bool CanReceivePickup(const FPickupPayload& Payload) const { return true; }

	/** 真正生效；返回 true 表示已消费，拾取物据此播特效并销毁 */
	virtual bool ReceivePickup(const FPickupPayload& Payload) { return false; }
};
