// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class SLASH2_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:

	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;
	
protected:
	
	virtual void BeginPlay() override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void Attack();
	virtual void Die(const FVector& ImpactPoint);
	void DirectionalHitReact(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	void DisableCapsule();
	virtual bool CanAttack();
	bool IsAlive();
	void DisableMeshCollision();

	/* Montage */
	void PlayHitReactMontage(const FName& SectionName);
	virtual int32 PlayAttackMontage();
	virtual void PlayDeathMontage(const FName SelectionName);
	void StopAttackMontage();

	/* 受击蒙太奇(HitReactMontage)在基类中是 private,
	   子类通过这个只读接口获取,用于判断受击动画是否在播放 */
	UAnimMontage* GetHitReactMontage() const { return HitReactMontage; }

	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();

    UFUNCTION(BlueprintCallable)
    FVector GetRotationWarpTarget();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);//武器碰撞处理函数,在蓝图中调用函数
	
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;//在蓝图中播放死亡动画蒙太奇

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose;
	/*
	组件
	*/
	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;//设置一个Actor目标

    UPROPERTY(EditAnywhere, Category = Combats)
	double WarpTargetDistance = 75.f;


private:

	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	/*
	* 攻击音效
	*/
	UPROPERTY(EditAnywhere, Category = Combats)
	USoundBase* HitSound;//攻击音效

	/*
	* 攻击粒子
	*/
	UPROPERTY(EditAnywhere, Category = Combats)
	UParticleSystem* HitParticles;

UPROPERTY(EditDefaultsOnly, Category = Combats)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combats)
	UAnimMontage* HitReactMontage;//在蓝图中播放攻击动画蒙太奇

	

	UPROPERTY(EditAnywhere, Category = Combats)
	TArray<FName> AttackMontageSections;//攻击动画蒙太奇的片段

	UPROPERTY(EditAnywhere, Category = Combats)
	TArray<FName> DeathMontageSections;//攻击动画蒙太奇的片段
public:
	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return DeathPose; }
};
