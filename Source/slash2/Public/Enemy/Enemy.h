// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"



class UHealthBarComponent;
class UPawnSensingComponent;
class UAnimMontage;//受击动画结束回调的参数类型前置声明

UCLASS()
class SLASH2_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	/* <AActor> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	/* </AActor> */

	/* <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/* </IHitInterface> */

protected:

	/* <AActor> */
	virtual void BeginPlay() override;
	/* </AActor> */

	/* <ABaseCharacter> */
	virtual void Die(const FVector& ImpactPoint, AActor* Hitter) override;
	void SpawnSoul();
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void HandleDamage(float DamageAmount) override;
	virtual void AttackEnd() override;
	/* </ABaseCharacter> */
	

	

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;//设置敌人默认状态为巡逻

	

private:

	/* AI 行为 */
	void InitializeEnemy();
	void CheckPatrolTarget();
	void CheckCombatTarget();
	void PatrolTimerFinished();
	void HideHealthBar();
	void ShowHealthBar();
	void LostInterest();
	void StartPatrolling();
	void ChaseTarget();
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	bool IsChasing();
	bool IsAttacking();
	bool IsDead();
	bool IsEngaged();
	void ClearPatrolTimer();
	void StartAttackTimer();
	void ClearAttackTimer();
	bool InTargetRange(AActor* Target, double Radius);
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
	void SpawnDefultWeapon();
	

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);//感知回调委托，需要绑定UFUNCTION()

	/* 受击动画播完的回调(绑定到 AnimInstance 的 OnMontageEnded 委托):
	   正常播完后解除硬直并恢复 AI;被中断(如连击)则继续硬直,
	   等最新一次受击动画播完 */
	UFUNCTION()
	void OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<class AWeapon> WeaponClass;//敌人武器类



	UPROPERTY(EditAnywhere)
	double CombatRadius = 1000.f;//敌人检测半径

	UPROPERTY(EditAnywhere)
	double AttackRadius = 200.f;//敌人战斗半径

	/* 导航 */
	UPROPERTY()
	class AAIController* EnemyController;

	/* 受击硬直标志:受击动画播放期间为 true,
	   期间 Tick 与 AI 恢复逻辑被挂起,直到受击动画播完 */
	bool bHitReacting = false;

	//当前巡逻目标
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 500.f;//敌人巡逻检测半径


	FTimerHandle PatrolTimer;//巡逻计时器

	UPROPERTY(EditAnywhere , Category = "AI Navigation")
	float PatrolWaitMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
    float PatrolWaitMax = 10.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float PatrollingSpeed = 124.f;

	FTimerHandle AttackTimer;//攻击计时器

    UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMin = 0.1f;

    UPROPERTY(EditAnywhere, Category = Combat)
    float AttackMax = 1.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ChasingSpeed = 300.f;


	UPROPERTY(EditAnywhere, Category = Combat)
	float DeathLifeSpan = 8.f;

    UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class ASouls> SoulClass;

};
