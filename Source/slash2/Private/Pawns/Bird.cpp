// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Bird.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
ABird::ABird()
{
	PrimaryActorTick.bCanEverTick = true;//允许Tick

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));//创建胶囊体组件

	Capsule->SetCapsuleHalfHeight(20.0f);//设置胶囊体高度

    Capsule->SetCapsuleRadius(15.0f);//设置胶囊体半径

	SetRootComponent(Capsule);//添加胶囊体组件到根目录

	BirdMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BirdMesh"));//添加骨架网格组件
	BirdMesh->SetupAttachment(GetRootComponent());//添加到根组件

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));//添加弹簧臂组件
    SpringArm->SetupAttachment(GetRootComponent());//添加到根组件
	SpringArm->TargetArmLength = 300.0f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
    ViewCamera->SetupAttachment(SpringArm);

	AutoPossessPlayer = EAutoReceiveInput::Player0;//添加玩家控制
}

void ABird::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABird::MoveForward(float Value)
{
	if (Controller && (Value != 0.f))
	{
		FVector Forward = GetActorForwardVector();//获取Forward
		AddMovementInput(Forward, Value);//添加移动输入
	}

}
void ABird::Turn(float Value)
{
	AddControllerYawInput(Value);///添加Yaw输入(绕y轴旋转)
}
void ABird::LookUp(float Value)
{
	AddControllerPitchInput(Value);//添加Pitch输入(绕x轴旋转)
}

void ABird::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABird::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ABird::MoveForward);//绑定ws输入
    PlayerInputComponent->BindAxis(FName("Turn"), this, &ABird::Turn);//绑定鼠标x轴输入
    PlayerInputComponent->BindAxis(FName("LookUp"), this, &ABird::LookUp);//绑定鼠标y轴输入
}

