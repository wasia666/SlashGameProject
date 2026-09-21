// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Items.h"
#include"slash2/DebugMacros.h"
#include "Components/SphereComponent.h"
#include "Interfaces/PickupInterface.h"
#include "NiagaraComponent.h"

// Sets default values
AItems::AItems()
{
	PrimaryActorTick.bCanEverTick = true;
    
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//忽略碰撞
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//禁用碰撞
    SetRootComponent(ItemMesh);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    Sphere->SetupAttachment(GetRootComponent());

	ItemEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Embers"));
	ItemEffect->SetupAttachment(GetRootComponent());
}

void AItems::BeginPlay()
{
	Super::BeginPlay();
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItems::OnSphereOverlap);//回调函数绑定委托函数的方式
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItems::OnSphereEndOverlap);
	UWorld* World = GetWorld();//获取世界对象,防止返回空指针
	/*
	if (World)
	{
		FVector Location = GetActorLocation();//获取当前物品的坐标
		
		DrawDebugSphere(World,Location, 50.f, 24, FColor::Red, true, 30.0f);//绘制一个球体
		//先获取当前世界
		//第一个参数：位置
        //第二个参数：半径
        //第三个参数：分段数
        //第四个参数：颜色
        //第五个参数：是否显示
		//第六个参数：显示时间
	}
	*/
	
	/*
	if (World)
	{
		DrawDebugLine(World, Location, Location + Forward * 100.0f, FColor::Blue, true,-1.f,0 ,1.0f);
		//第一个参数：位置
        //第二个参数：终点
        //第三个参数：颜色
        //第四个参数：是否显示
		//第五个参数：生命周期，因为线段没有生命周期，所以这里设置-1
		//第六个参数：线段优先级，值域为0-255，值越小越优先显示
		//第七个参数：线段宽度，值域为0-10，值越小越细
	}
	*/

	//SetActorLocation(FVector(0.0f, 0.0f, 50.0f));//设置物品的位置
	//SetActorRotation(FRotator(0.0f,45.0f, 0.0f));//设置物品的旋转
	
	/*
	FVector Location = GetActorLocation();
	FVector Forward = GetActorForwardVector();//获取当前物品的朝向,默认长度为1

	DRAW_SPHERE(Location);//绘制一个球体
	//DRAW_LINE(Location, Location + Forward * 100.0f);//绘制一条线段

	/*
	if (World)
	{
		DrawDebugPoint(World, Location + Forward * 100.0f, 15.0f ,FColor::Red, true);
	}

	//DRAW_POINT(Location + Forward * 100.0f);//绘制一个点
	DRAW_VECTOR(Location , Location + Forward * 100.0f);
	*/


	

}
float AItems::TransformedSin()
{
	return Amplitude * FMath::Sin(RunningTime * TimeConstant);//0.25 * sin(5 * RunningTime)
}

float AItems::TransformedCos()
{
	return Amplitude * FMath::Cos(RunningTime * TimeConstant);//0.25 * cos(5 * RunningTime)
}

void AItems::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//将actor的类型转换为SlashCharacter
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
    if (PickupInterface)
    {
		PickupInterface->SetOverlappingItem(this);
    }
}

void AItems::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		PickupInterface->SetOverlappingItem(nullptr);
	}
}

void AItems::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*
	UE_LOG(LogTemp, Display, TEXT("DeltaTime: %f"),DeltaTime);

	if (GEngine)
	{
		FString Name = GetName();//string类型，需要转换成FString对象，才能在UE中显示
		FString Message = FString::Printf(TEXT("Item Name: %s"), *Name);//需要加*号，这里传入的不是F字符串对象，是c字符串对象
		GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Red, Message);

		UE_LOG(LogTemp, Warning, TEXT("Item Name is : %s"),*Name);
	}
	*/

	RunningTime += DeltaTime;//运行时间

	//float DeltaZ =Amplitude * FMath::Sin(RunningTime * TimeConstant);//0.25 * sin(5 * RunningTime)
	
	//AddActorWorldOffset(FVector(0.0f, 0.0f, DeltaZ));

	/*
	DRAW_SPHERE_SINGLEFRAME(GetActorLocation());
	DRAW_VECTOR_SINGLEFRAME(GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 100.f);


	FVector AvgVector = Avg<FVector>(GetActorLocation(), FVector::ZeroVector);
	DRAW_POINT_SINGLEFRAME(AvgVector);
	*/

	if (ItemState == EItemState::EIS_Hovering)//物品处于悬停状态
	{
		AddActorLocalOffset(FVector(0.0f, 0.0f,TransformedSin() ));
	}
}

