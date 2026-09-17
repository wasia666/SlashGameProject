
#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/WidgetComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Items/Weapons/Weapon.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);//将碰撞预设改为WorldDynamic
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);//将网格体Visibilty(可视通道)的碰撞响应改为Block
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//将网格体的Camera(相机)的碰撞相应改为Ignore
	GetMesh()->SetGenerateOverlapEvents(true);//开启生成重叠事件
	

	

    HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));//创建一个健康hud组件
    HealthBarWidget->SetupAttachment(GetRootComponent());
	GetCharacterMovement()->bOrientRotationToMovement = true;//角色的移动方向与加速方向或朝向一致
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;//关闭角色的Y轴旋转
	bUseControllerRotationRoll = false;


	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));//创建一个感知组件
	PawnSensing->SightRadius = 9000.f;//感知距离
    PawnSensing->SetPeripheralVisionAngle(45.f);//感知角度
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (IsDead())return;
	if (bHitReacting) return;// 受击硬直中:不评估 AI,等受击动画播完再行动
	if (EnemyState > EEnemyState::EES_Patrolling)//敌人状态大于EES_Patrolling
	{
		//检测战斗目标
		CheckCombatTarget();
	}
	else
	{
		//检测巡逻目标
		CheckPatrolTarget();
	}
}

float AEnemy::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();//获取对Enemy造成伤害的Pawn赋给CombatTarget，便于Enemy对该pawn进行动作
	if (IsInsideAttackRadius())
	{
		EnemyState = EEnemyState::EES_Attacking;
		UE_LOG(LogTemp, Warning, TEXT("Enemy Attacking"));
	}
	else if (IsOutsideAttackRadius())
	{
		ChaseTarget();
	}

	return DamageAmount;
}

void AEnemy::Destroyed()
{
	//在游戏过程中或在编辑器中被明确销毁时调用，而不是在关卡流或游戏结束时调用。
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{

	if(!IsDead()) ShowHealthBar();
	Super::GetHit_Implementation(ImpactPoint,Hitter);
	if (IsAlive()) //如果有生命值而且还活着
	{
		DirectionalHitReact(ImpactPoint);//受击方向
	}
	else
	{
		Die(ImpactPoint);
	}
	ClearPatrolTimer();
	ClearAttackTimer();
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	StopAttackMontage();

	if (IsAlive())
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && GetHitReactMontage() && AnimInstance->Montage_IsPlaying(GetHitReactMontage()))
		{
			/* 受击动画正在播放:进入硬直并停住移动,
			   等受击动画播完(OnHitReactMontageEnded)再恢复 AI,
			   避免攻击蒙太奇被中断后状态卡死,以及受击时滑步 */
			bHitReacting = true;
			if (EnemyController) EnemyController->StopMovement();
		}
		else
		{
			/* 没有可播放的受击动画:立即恢复状态机,
			   防止攻击蒙太奇被中断后 AttackEnd 通知永不触发导致敌人卡死 */
			EnemyState = EEnemyState::EES_NoState;
			CheckCombatTarget();
		}
	}
}

/* 受击动画播放结束回调(由 AnimInstance 的 OnMontageEnded 委托触发)
   受击动画完整播完后解除硬直,并按 AttackEnd 相同的逻辑重新评估战斗,
   使敌人恢复攻击/追击/巡逻 */
void AEnemy::OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != GetHitReactMontage()) return;// 只处理受击蒙太奇,忽略攻击/死亡等其他蒙太奇

	if (bInterrupted)
	{
		/* 受击动画被中断(例如又挨了一刀):新的受击动画正在播放,
		   继续保持硬直,等最新一次受击动画播完 */
		return;
	}
	if (!IsAlive()) return;// 已死亡则不恢复

	/* 受击动画正常播完:解除硬直,恢复状态机并重新评估战斗目标(与 AttackEnd 一致) */
	bHitReacting = false;
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);//当敌人被感知时，调用PawnSeen
	}
	InitializeEnemy();
	Tags.Add(FName("Enemy"));

	/* 绑定受击动画结束回调:受击动画播完后解除硬直、恢复 AI(见 OnHitReactMontageEnded) */
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &AEnemy::OnHitReactMontageEnded);
	}
}

void AEnemy::Die(const FVector& ImpactPoint)
{
	EnemyState = EEnemyState::EES_Dead;//设置状态为死亡
	Super::Die(ImpactPoint);
	ClearAttackTimer();
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);//敌人在死亡三秒后销毁
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AEnemy::Attack()
{
	EnemyState = EEnemyState::EES_Engaged;
	UE_LOG(LogTemp, Warning, TEXT("Enemy Engaging"));
	Super::Attack();
	PlayAttackMontage();

}

bool AEnemy::CanAttack()
{
	bool CanAttack =
		IsInsideAttackRadius()
		&& !IsAttacking()
		&& !IsEngaged()
		&& !IsDead();
	return CanAttack;
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;//设置状态为无状态

	CheckCombatTarget();//检测战斗目标
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	if (Attributes && HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());//通过Weapon的Damage的伤害更新健康hud
	}

}

void AEnemy::InitializeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());//将敌人的控制器保存在EnemyController中
	HideHealthBar();
	MoveToTarget(PatrolTarget);//移动到目标点
	SpawnDefultWeapon();
}

void AEnemy::CheckPatrolTarget()
{
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();//随机选择一个巡逻目标点
		const float WaitTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);//创建一个计时器，计时结束后调用PatrolTimerFinished
	}
}

void AEnemy::CheckCombatTarget()
{
	if (IsOutsideCombatRadius())//如果Enemy距离CombatTarget大于检测半径，也就是目标在战斗检测范围外
	{
		ClearAttackTimer();
		LostInterest();//失去兴趣函数
		if (!IsEngaged())
		{
			//如果不在交战，则开始巡逻
			StartPatrolling();//开始巡逻
		}
	}
	else if (IsOutsideAttackRadius() && !IsChasing())//如果不在攻击范围且没有在追踪角色
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			//如果在交战，则开始追击，如果开始攻击，则无法开始追踪目标(防滑步)
			ChaseTarget();//追击主角
		}
	}
	else if (CanAttack())
	{
		ClearAttackTimer();
		StartAttackTimer();//设置攻击间隔
	}

}

void AEnemy::PatrolTimerFinished()
{
	//当轮询时间结束时，开始移动
	MoveToTarget(PatrolTarget);
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);//如果有Enemy血条，则隐藏
	}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);//如果有Enemy血条，则显示
	}
}

void AEnemy::LostInterest()
{
	CombatTarget = nullptr;//重置CombatTarget,使Enemy血条再次消失
	HideHealthBar();//隐藏血条
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;//敌人进入巡逻状态
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;//敌人移动速度改为124
	MoveToTarget(PatrolTarget);//移动到PatrolTarget
}

void AEnemy::ChaseTarget()
{
	//敌人不在攻击范围内而且没有追击目标,则追击目标
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutsideCombatRadius()
{
	//如果Enemy距离CombatTarget大于检测半径，也就是目标在战斗检测范围外
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	//如果在追击范围内且不在攻击范围内
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
	//在攻击范围内
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
	//正在攻击
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsDead()
{
	//检查是否死亡
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged()
{
	//检查是否在参战
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);//如果我们现在处于追逐状态，我们不希望该计时器响起，因此我们可以清除该计时器
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	//设置一个定时器(AttackTimer)，在设定的时间间隔(AttackTime)调用指定的本地函数(&AEnemy::Attack)。如果这个句柄已经有一个定时器，它会替换当前的定时器。
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr)return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();//从Enemy到Target的向量。
	return DistanceToTarget <= Radius;
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr|| Target == nullptr)return;
	FAIMoveRequest MoveRequest;//创建一个移动请求
	MoveRequest.SetGoalActor(Target);//设置移动的目标
	MoveRequest.SetAcceptanceRadius(50.f);//设置移动的半径
	EnemyController->MoveTo(MoveRequest);//移动到目标
	
}

AActor* AEnemy::ChoosePatrolTarget()
{
	TArray<AActor*> ValidTargets;//这个数组用于存储剩下的PatrolTargets(除了当前跟踪的PatrolTargets)
	for (AActor* Target : PatrolTargets)
	{
		if (Target != PatrolTarget)//如果PatrolTargets不是当前正在跟踪的PatrolTarget
		{
			ValidTargets.AddUnique(Target);//将PatrolTargets添加到ValidTargets中
		}
	}

	const int32 NumPatrolTargets = ValidTargets.Num();//获取巡逻目标数量
	if (NumPatrolTargets > 0)//如果巡逻目标数量大于0
	{
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);//随机选择一个PatrolTargets的索引
		return ValidTargets[TargetSelection];//获取TargetSelection,赋给Target
	}

	return nullptr;
}

void AEnemy::SpawnDefultWeapon()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefultWeapon = World->SpawnActor<AWeapon>(WeaponClass);//在世界生成一个武器给敌人
		DefultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);//通过函数将武器装到插槽上面
		EquippedWeapon = DefultWeapon;//设置已经装备过的武器
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	//判断是否追踪目标
	if (bHitReacting) return;// 受击硬直中不重新索敌,避免受击动画期间跑去追目标
	const bool bShouldChaseTarget =
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Chasing &&
		EnemyState < EEnemyState::EES_Attacking &&
		SeenPawn->ActorHasTag(FName("EngageableTarget"));
	if (bShouldChaseTarget)
	{
		CombatTarget = SeenPawn;//战斗目标为SeenPawn所看到的玩家
		ShowHealthBar();// 只有存活且真正要追击时才显示血条,死亡后不再显示
		ClearPatrolTimer();
		ChaseTarget();
	}
	
}


