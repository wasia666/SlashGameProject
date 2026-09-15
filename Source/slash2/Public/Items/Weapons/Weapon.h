// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Items.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;
/**
 * 
 */
UCLASS()
class SLASH2_API AWeapon : public AItems
{
	GENERATED_BODY()
public:
	AWeapon();
	void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);
	void DeactivateEmbers();
	void DisableSphereCollision();
	void PlayEquipSound();
	void AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName);


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
