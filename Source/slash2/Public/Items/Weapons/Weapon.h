// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Items.h"
#include "Interfaces/InteractableInterface.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;
class IPickupReceiverInterface;   // 只前置声明：头文件里只用指针，完整定义在 cpp
/**
 * 
 */
UCLASS()
class SLASH2_API AWeapon : public AItems, public IInteractableInterface
{
	GENERATED_BODY()
public:
	AWeapon();
	void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);
	void DeactivateEmbers();
	void DisableSphereCollision();
	void PlayEquipSound();
	void AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName);

	//~ Begin IInteractableInterface：让武器像治疗物一样走"靠近显示 E"的交互流程
	/** 靠近时的提示文案：默认 "拾取 " + 蓝图里填的武器名；已装备或无法接收时返回空 */
	virtual FText GetInteractionPrompt(AActor* Interactor) const override;
	/** 此刻能否按 E：武器未被装备，且交互者愿意接收 */
	virtual bool CanInteract(AActor* Interactor) const override;
	/** 按 E：把"装备这把武器"转交给接收者执行，成功返回 true */
	virtual bool Interact(AActor* Interactor) override;
	//~ End IInteractableInterface

	/** 蓝图里可改的武器显示名，用于拼出"拾取 XXX"提示 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FText PickupDisplayName;

	/** 角色当前无法接收时的提示文案 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FText UnavailablePromptText;

	/** 是否已经被装备（装备后球体碰撞会被关掉，提示也随之消失） */
	FORCEINLINE bool IsEquipped() const { return ItemState == EItemState::EIS_Equipped; }


	TArray<AActor*> IgnoreActors;//用于忽略已经碰撞过的物体
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override; // 修复:逐帧命中检测,解决攻击时偶发打不碎物体

	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool ActorIsSameType(AActor* OtherActor);

	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);//创建Fields,从cpp文件调用函数，传入位置，调用BlueprintImplementableEvent给蓝图实现函数
private:

	/** 取交互者身上的接收者接口：提示、可用性、交互三处共用，避免重复 Cast */
	const IPickupReceiverInterface* GetPickupReceiver(AActor* Interactor) const;

	/** 构造一次探测请求问接收者"你现在能拿这把武器吗"，不改变任何状态 */
	bool CanBeEquippedBy(AActor* Interactor) const;
	void BoxTrace(FHitResult& BoxHit);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FVector BoxTraceExtent = FVector(20.f);

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bShowBoxDebug = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USoundBase* EquipSound;//装备声音(sfx_EquipSound)


	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UBoxComponent* WeaponBox;//武器碰撞体积

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceStart;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceEnd;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage = 20.f;

	void TraceHit(); // 修复:命中检测逻辑(重叠回调与Tick共用)

	void ExecuteGetHit(FHitResult& BoxHit);

	
public:
	FORCEINLINE UBoxComponent* GetWeaponBox() const{return WeaponBox;}

};
