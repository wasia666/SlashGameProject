
#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Components/AttributeComponent.h"
#include"Components/CapsuleComponent.h"
#include"Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"// 修复:校验蒙太奇 section 名,避免 JumpToSection 静默失败
#include "Enemy/Enemy.h"// 修复:在SetWeaponCollisionEnabled中区分敌人与玩家


ABaseCharacter::ABaseCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));//创建属性组件
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//将胶囊Camera(相机)的碰撞相应改为Ignore

}
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}
void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{ 
	if (IsAlive() && Hitter) //如果有生命值而且还活着
	{
		DirectionalHitReact(Hitter->GetActorLocation());//受击方向
	}
	PlayHitSound(ImpactPoint);//播攻击音效

	SpawnHitParticles(ImpactPoint);//特效
}

void ABaseCharacter::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr;//如果目标死亡，则将CombatTarget设置为空
	}
}

void ABaseCharacter::Die(const FVector& ImpactPoint, AActor* Hitter)
{
	Tags.Add(FName("Dead"));
	if (Hitter)
	{
		PlayerDeadFromDirctionalImpactPiont(Hitter->GetActorLocation());
	}
}

void ABaseCharacter::PlayerDeadFromDirctionalImpactPiont(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();//获取角色当前的朝向单位向量
	//将撞击点降低到actor的Z轴，也就是投影到actor的Z轴
	const FVector ImpactLower(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);

	const FVector ToHit = (ImpactLower - GetActorLocation()).GetSafeNormal();//获取角色和 ImpactLower之间的向量,并且将这个向量单位化

	//a*b = |a||b|cos(角度)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);//获取Forward和ToHit之间的点积
	//通过反三角函数获取弧度
	double Theta = FMath::Acos(CosTheta);//弧度
	//弧度转换为角度
	Theta = FMath::RadiansToDegrees(Theta);

	//如果叉积的Z轴方向为正，则角度为正，否则为负
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);//获取Forward和ToHit之间的叉积
	if (CrossProduct.Z < 0)//ue引擎的坐标系Z轴方向为正，所以需要乘以-1
	{
		Theta *= -1.f;
	}
	/*
	/根据角度判断角色被攻击的方向
	*/
	FName Section("DeathFromBehind");
	DeathPose = EDeathPose::EDP_DeathFromBehind;
	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("DeathFromFront");//角色被正面攻击
		DeathPose = EDeathPose::EDP_DeathFromFront;
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("DeathFromLeft");//角色被左侧攻击
		DeathPose = EDeathPose::EDP_DeathFromLeft;
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("DeathFromRight");//角色被右侧攻击
		DeathPose = EDeathPose::EDP_DeathFromRight;
	}
	PlayDeathMontage(Section);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ::ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();//获取动画实例
	if (AnimInstance && HitReactMontage)//动画实例和HitReactMontage都不为空
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

int32 ABaseCharacter::PlayAttackMontage()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->IgnoreActors.Empty();// 修复:命中列表改在攻击开始时清空一次,不再每次开启武器盒都清空;避免蒙太奇第二个判定窗口对同一敌人二次命中
	}
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);//随机播放攻击蒙太奇
}

void ABaseCharacter::PlayDeathMontage(const FName SelectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();//获取动画实例
	if (AnimInstance && DeathMontage)//动画实例和HitReactMontage都不为空
	{
		if (!DeathMontage->IsValidSectionName(SelectionName))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Death] Montage %s has no section '%s' - JumpToSection will be ignored and the first section will play."), *DeathMontage->GetName(), *SelectionName.ToString());
		}
		AnimInstance->Montage_Play(DeathMontage);
		AnimInstance->Montage_JumpToSection(SelectionName, DeathMontage);
	}
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Stop(0.4f, AttackMontage);
    }
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
    if (CombatTarget == nullptr) return FVector();

	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();

	const FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal() * WarpTargetDistance;
	return CombatTargetLocation + TargetToMe;
}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}


void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);//停止胶囊体碰撞
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();//获取角色当前的朝向单位向量
	//将撞击点降低到actor的Z轴，也就是投影到actor的Z轴
	const FVector ImpactLower(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);

	const FVector ToHit = (ImpactLower - GetActorLocation()).GetSafeNormal();//获取角色和 ImpactLower之间的向量,并且将这个向量单位化

	//a*b = |a||b|cos(角度)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);//获取Forward和ToHit之间的点积
	//通过反三角函数获取弧度
	double Theta = FMath::Acos(CosTheta);//弧度
	//弧度转换为角度
	Theta = FMath::RadiansToDegrees(Theta);

	//如果叉积的Z轴方向为正，则角度为正，否则为负
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);//获取Forward和ToHit之间的叉积
	if (CrossProduct.Z < 0)//ue引擎的坐标系Z轴方向为正，所以需要乘以-1
	{
		Theta *= -1.f;
	}
	/*
	/根据角度判断角色被攻击的方向
	*/
	FName Section("FromBack");

	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("FromFront");//角色被正面攻击
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("FromLeft");//角色被左侧攻击
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("FromRight");//角色被右侧攻击
	}



	PlayHitReactMontage(Section);
}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)//受击声源
	{
		UGameplayStatics::PlaySoundAtLocation
		(this,
			HitSound,
			ImpactPoint
		);//在受击点播放受击声音
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles && GetWorld())//受击粒子
	{
		UGameplayStatics::SpawnEmitterAtLocation
		(
			GetWorld(),
			HitParticles,
			ImpactPoint
		);//在ImpactPoint处播放受击粒子
	}
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);//接受来自Weapon的Damage的伤害,然后将伤害扣除角色的生命
	}
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();//获取动画
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);//播放攻击动画
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;//如果攻击动画蒙太奇的片段有问题，则无法播放
	const int32 MaxSectionIndex = SectionNames.Num() - 1;//防止超过索引范围
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);//随机选择一个数组的数字
	PlayMontageSection(Montage, SectionNames[Selection]);//我们可以将攻击蒙太奇和攻击蒙太奇索引传入。

	return Selection;
}


bool ABaseCharacter::CanAttack()
{
	return false;
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

void ABaseCharacter::DisableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);// 修复:命中列表清空时机——敌人每次开盒清空(见下方判断),玩家攻击开始时清空(PlayAttackMontage)

		// 修复:敌人每次开启判定窗口都清空命中列表,多段攻击动画每段都能命中玩家
		// 玩家不在此清空(攻击开始时PlayAttackMontage已清空一次),保持一次攻击最多命中一次
		if (Cast<AEnemy>(this))
		{
			EquippedWeapon->IgnoreActors.Empty();
		}
	}
}


