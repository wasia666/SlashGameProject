// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "SlashCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class UGroomComponent;
class AItems;
class USlashOverlay;

UCLASS()
class SLASH2_API ASlashCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	
protected:
	virtual void BeginPlay() override;

	// 输入处理函数
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
    void LookUp(float Value);
	void EKeyPressed();

	/*战斗*/
	void EquipWeapon(AWeapon* Weapon);
	virtual void Attack() override;
	virtual void AttackEnd() override;//动画结束通知函数
	virtual bool CanAttack() override;//判断是否可以攻击
	// 装备处理函数
	void PlayEquipMontage(const FName& SectionName);//装备动画
	bool CanDisarm();//判断是否可以卸下
	bool CanArm();//判断是否可以装备
    void Arm();
	void Disarm();
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();

	UFUNCTION(BlueprintCallable)
    void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();
private:

	void InitializeSlashOverlay();
	/* 角色组件 */

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere , Category = Hair)
    UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere)
    UGroomComponent* Eyebrows;
	//在Items文件里面是AItems类
	UPROPERTY(VisibleInstanceOnly)
	AItems* OverlappingItem; 

	

	
	/*
	* 装备动画蒙太奇
	*/
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;//当前角色为无装备状态

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))//允许在Blueprint中访问私人变量
	EActionState ActionState = EActionState::EAS_Unoccupied;//当前角色为空闲状态

	UPROPERTY()
	USlashOverlay* SlashOverlay;

public:
	FORCEINLINE void SetOverlappingItem(AItems* Item){ OverlappingItem = Item;}//将指针的物品转换为OverlappingItem实例
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }//将CharacterState转换为public 成员变量，用于SlashAnimInstance文件使用
};
