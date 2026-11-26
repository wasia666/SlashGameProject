// 拾取数据层：纯数据，不依赖任何角色/物品类，供拾取物与接收方共用。

#pragma once

#include "CoreMinimal.h"
#include "PickupTypes.generated.h"

/** 拾取类型（要加新道具类型时先在这里扩展）*/
UENUM(BlueprintType)
enum class EPickupType : uint8
{
	Health  UMETA(DisplayName = "Health"),
	Stamina UMETA(DisplayName = "Stamina"),
	Gold    UMETA(DisplayName = "Gold"),
	Souls   UMETA(DisplayName = "Souls")
};

/** 一次拾取请求的数据包：拾取物只描述"给什么"，不关心谁来收 */
USTRUCT(BlueprintType)
struct FPickupPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	EPickupType Type = EPickupType::Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float Amount = 0.f;

	/** 来源 Actor（弱引用，仅用于接收方播特效/记日志，不参与 GC）*/
	TWeakObjectPtr<AActor> SourceActor;
};
