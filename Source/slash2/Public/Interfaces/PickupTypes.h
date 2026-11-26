// 拾取数据层：纯数据，不依赖任何角色/物品类，供拾取物与接收方共用。

#pragma once

#include "CoreMinimal.h"
#include "PickupTypes.generated.h"

/** 统一的交互提示文案：固定"拾取 "前缀 + 蓝图里配置的物品名，治疗物与武器共用，避免各写一套 */
FORCEINLINE FText MakePickupPrompt(const FText& DisplayName)
{
	return FText::FromString(FString::Printf(TEXT("拾取 %s"), *DisplayName.ToString()));
}

/** 拾取类型：接收方按类型决定加哪个属性（要加新道具时先在这里扩展）*/
UENUM(BlueprintType)
enum class EPickupType : uint8
{
	Health  UMETA(DisplayName = "Health"),   // 生命值
	Stamina UMETA(DisplayName = "Stamina"),  // 耐力
	Gold    UMETA(DisplayName = "Gold"),     // 金币
	Souls   UMETA(DisplayName = "Souls")     // 灵魂
};

/** 一次拾取请求的数据包：拾取物只描述"给什么"，不关心谁来收 */
USTRUCT(BlueprintType)
struct FPickupPayload
{
	GENERATED_BODY()

	/** 给的是什么资源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	EPickupType Type = EPickupType::Health;

	/** 给多少：生命/耐力是点数，金币/灵魂是个数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float Amount = 0.f;

	/** 来源 Actor（弱引用：仅用于接收方播特效/记日志，不影响 GC）*/
	TWeakObjectPtr<AActor> SourceActor;
};
