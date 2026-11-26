// 拾取接收者接口：拾取物只认这个接口，不 Cast 任何具体角色类。

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interfaces/PickupTypes.h"
#include "PickupReceiverInterface.generated.h"

class AWeapon;

/** 接口的 UObject 部分：只作为反射入口，本身没有逻辑 */
UINTERFACE(MinimalAPI)
class UPickupReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 任何"能收东西的 Actor"实现本接口即可接收拾取物，
 * 效果由接收方按 FPickupPayload 自行决定（加血/加钱/加灵魂...）。
 * 武器不是数据载荷，而是"把这个 Actor 交给接收方处理"，所以单独一对方法。
 */
class SLASH2_API IPickupReceiverInterface
{
	GENERATED_BODY()

public:
	/** 能否接收：例如满血时拒绝治疗物；拾取物据此不消耗自己 */
	virtual bool CanReceivePickup(const FPickupPayload& Payload) const { return true; }

	/** 真正生效；返回 true 表示已消费，拾取物据此播特效并销毁 */
	virtual bool ReceivePickup(const FPickupPayload& Payload) { return false; }

	/** 能否接收这把武器（活着、武器未被装备、角色当前状态允许）*/
	virtual bool CanReceiveWeapon(AWeapon* Weapon) const { return false; }

	/** 接过武器：由接收方决定怎么处理（本项目中 = 装备到手上），返回 true 表示成功 */
	virtual bool ReceiveWeapon(AWeapon* Weapon) { return false; }
};