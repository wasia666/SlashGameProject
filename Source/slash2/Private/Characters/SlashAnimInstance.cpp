// Fill out your copyright notice in the Description page of Project Settings.



#include "Characters/SlashAnimInstance.h"
#include "Characters/SlashCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void USlashAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();//调用父类NativeInitializeAnimation函数

	SlashCharacter = Cast<ASlashCharacter>(TryGetPawnOwner());//将父类指针转换为ASlashCharacter
	if (SlashCharacter)
	{
		SlashCharacterMovement = SlashCharacter->GetCharacterMovement();//获取角色的移动组件
	}
}

void USlashAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (SlashCharacterMovement)
    {
		GroundSpeed = UKismetMathLibrary::VSizeXY(SlashCharacterMovement->Velocity);//获取水平速度
		IsFalling = SlashCharacterMovement->IsFalling();
		CharacterState = SlashCharacter->GetCharacterState();//获取角色状态
        ActionState = SlashCharacter->GetActionState();//获取动作状态
		DeathPose = SlashCharacter->GetDeathPose();
    }
}
