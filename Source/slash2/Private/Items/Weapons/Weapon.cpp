// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Interfaces/HitInterface.h"
#include "NiagaraComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true; // 修复:开启Tick,用于攻击窗口内逐帧命中检测
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponBox"));//创建一个碰撞盒子组件
    WeaponBox->SetupAttachment(GetRootComponent());
    WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);//设置武器初始碰撞检测为无碰撞
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);//设置所有碰撞响应为重叠
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);//设置对Pawn的碰撞响应为忽略

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));//创建起点
    BoxTraceStart->SetupAttachment(GetRootComponent());//添加到武器网格体

	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));//创建终点
    BoxTraceEnd->SetupAttachment(GetRootComponent());//添加到武器网格体
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();//继承父类

	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);//添加重叠事件
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
    ItemState = EItemState::EIS_Equipped;
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
	AttachMeshToSocket(InParent, InSocketName);
	DisableSphereCollision();
	PlayEquipSound();
	DeactivateEmbers();
}

void AWeapon::DeactivateEmbers()
{
	if (EmbersEffect)
	{
		EmbersEffect->Deactivate();//当装备上武器的时候禁用粒子效果
	}
}

void AWeapon::DisableSphereCollision()
{
	if (Sphere)
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//当武器被装备上时禁用碰撞，以免进行其他动作时被触发了重叠事件
	}
}

void AWeapon::PlayEquipSound()
{
	if (EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquipSound,
			GetActorLocation()
		);
	}
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);//创建一个挂载规则
	ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);//将武器挂载到角色
}



void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ActorIsSameType(OtherActor)) return;
	// 修复:命中判定统一走 TraceHit(),与 Tick 逐帧检测共用同一逻辑
	TraceHit();
}

bool AWeapon::ActorIsSameType(AActor* OtherActor)
{
	return GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}



void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 修复核心:挥砍窗口内(武器盒开启时)每帧做一次命中检测,不再只依赖一次重叠事件
	// 解决:快速挥砍穿过物体(隧穿)、重叠瞬间角度不对导致的"打不碎"
	if (WeaponBox && WeaponBox->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
	{
		TraceHit();
	}
}

void AWeapon::TraceHit()
{
	
	FHitResult BoxHit;//创建一个命中结果
    BoxTrace(BoxHit);
	if (BoxHit.GetActor()) //在被击中的actor上调用接口函数->#include "Interfaces/HitInterface.h"
	{
		if (ActorIsSameType(BoxHit.GetActor())) return;
		//在武器上设置伤害
		UGameplayStatics::ApplyDamage(
			BoxHit.GetActor(),//命中的actor
			Damage,//伤害值，会继承到Enemy.cpp中的TakeDamge函数中的DamageAmount参数
			GetInstigator()->GetController(),//造成伤害的控制器Controller(slashCharacter)
			this,//造成伤害的actor(Weapon)
			UDamageType::StaticClass()//伤害类型
		);
		/*
		* 1.获取命中的actor，确保确实击中了1个actor
		* 2.需要查看是否可以将其转换为命中接口
		* 3.获取命中actor，将它转换为命中接口，如果转换失败，则返回null
		*/
		ExecuteGetHit(BoxHit);
		CreateFields(BoxHit.ImpactPoint);	
	}
}
void AWeapon::ExecuteGetHit(FHitResult& BoxHit)
{
	IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());//将命中actor转换为命中接口
	if (HitInterface)//如果转换成功
	{
		HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());//调用Enemy.cpp中的GetHit命中接口的函数
		//为什么不用Box.GetActor(),因为使用Box.GetActor()执行GetHit的actor将根据其自身位置执行命中反应，这并不是我们想要的方式
		//我们想要命中actor自身，而不是使用其自身位置，防止出现攻击到背面导致的敌人往前倒的现象
	}
}
void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	// 修复:追踪长度由BoxTraceEnd组件的114cm加长到200cm,覆盖整个挥砍弧线,避免距离稍远就打不中
	const FVector End = Start + BoxTraceStart->GetComponentRotation().Vector() * 200.f;
	TArray<AActor*> ActorsToIgnore;//创建一个用于忽略的数组
	ActorsToIgnore.Add(this);//盒子碰撞检测忽略武器自身，以免自己与自己发生碰撞，确保不会击中武器本身

	/*
		/添加忽略的actor
	*/
	if (GetInstigator())
	{
		ActorsToIgnore.AddUnique(GetInstigator());// 修复:玩家自身加入忽略列表,避免自己的胶囊体挡住追踪
	}

	for (AActor* Actor : IgnoreActors)
	{
		ActorsToIgnore.AddUnique(Actor);//添加忽略的actor,如果actor已经添加过，则不会重复添加两次，不会得到指向同一对象的指针
	}

	UKismetSystemLibrary::BoxTraceSingle(
		this,//当前组件
		Start,//起点
		End,//终点
		BoxTraceExtent,// 修复:追踪范围由5x5x5加宽到20x20x20,提高容错
		BoxTraceStart->GetComponentRotation(),//盒子旋转
		ETraceTypeQuery::TraceTypeQuery1,//检测类型
		false,//忽略物理体
		ActorsToIgnore,//忽略的武器自身
		bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,//绘制追踪时长
		BoxHit,//命中结果
		true//绘制追踪
	);
	IgnoreActors.AddUnique(BoxHit.GetActor());//添加命中的actor,如果已经添加过，则不会重复添加两次，不会得到指向同一对象的指针
}
