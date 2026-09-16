#include "Characters/SlashCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GroomComponent.h" 
#include "Components/AttributeComponent.h"
#include "Items/Items.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"

ASlashCharacter::ASlashCharacter()
{

	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;//禁止控制器旋转
	bUseControllerRotationRoll = false;
    bUseControllerRotationPitch = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;//角色的旋转由输入控制
	GetCharacterMovement()->RotationRate = FRotator(0.f, 850.f, 0.f);//角色旋转速度

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);//设置碰撞类型为动态
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//忽略所有碰撞
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);//设置可见通道为阻挡
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);//将WorldDynamic设置为可以触发重叠事件
	GetMesh()->SetGenerateOverlapEvents(true);//开启生成重叠事件

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));//创建弹簧杆
    SpringArm->SetupAttachment(GetRootComponent());//弹簧杆挂载在根节点
	SpringArm->TargetArmLength = 300.f;


	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
    ViewCamera->SetupAttachment(SpringArm);


    Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));//创建头发
    Hair->SetupAttachment(GetMesh()); //头发挂载在角色的Mesh上
    Hair->AttachmentName = FString("head"); 

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
    Eyebrows->SetupAttachment(GetMesh());
    Eyebrows->AttachmentName = FString("head"); 
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ASlashCharacter::MoveForward);//绑定移动
	PlayerInputComponent->BindAxis(FName("Turn"), this, &ASlashCharacter::Turn);//绑定旋转
    PlayerInputComponent->BindAxis(FName("LookUp"), this, &ASlashCharacter::LookUp);//绑定抬头
    PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ASlashCharacter::MoveRight);

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);//绑定跳跃键(空格键)
	PlayerInputComponent->BindAction(FName("Equip"), IE_Pressed, this, &ASlashCharacter::EKeyPressed);//绑定装备键(E键)
	PlayerInputComponent->BindAction(FName("Attack"), IE_Pressed, this, &ASlashCharacter::Attack);//绑定攻击键(鼠标左键)
}

float ASlashCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	ActionState = EActionState::EAS_HitReaction;//角色被攻击状态
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	Tags.Add(FName("EngageableTarget"));

	InitializeSlashOverlay();
}



void ASlashCharacter::MoveForward(float Value)
{
	if (ActionState != EActionState::EAS_Unoccupied)//角色正在进行动作且停止移动
	{
		return;
	}
	if (Controller && Value != 0.0f)
	{	
		const FRotator ControlRotation = GetControlRotation();//获取控制器的旋转方向
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);//只取控制器的“Yaw”（水平朝向），把“Pitch”（上下俯仰）和“Roll”（翻滚）都设为零。
		//因为前后移动时，我们只想让角色在地面上水平移动，不希望角色因为抬头或低头而改变移动方向

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);//EAxis::X代表这个旋转的“前”方向
		//玩家相机水平 facing 的方向（角色应该朝这个方向移动）
        AddMovementInput(Direction, Value);//按 Direction 方向移动角色，移动的强度是 Value。
	}
}

void ASlashCharacter:: MoveRight(float Value)//移动左右
{
	if (ActionState != EActionState::EAS_Unoccupied)//角色正在攻击将停止转向
	{
		return;
	}
	if (Controller && Value != 0.0f)
	{
		//获取控制器的旋转方向
        const FRotator ControlRotation = GetControlRotation();
        const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);//只取控制器的“Yaw”（水平朝向），把“Pitch”（上下俯仰）和“Roll”（翻滚）都设为零。
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);//EAxis::Y代表这个旋转的“右”方向
        AddMovementInput(Direction, Value);
	}
}

void ASlashCharacter::Turn(float Value)//旋转
{
	AddControllerYawInput(Value);
}

void ASlashCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ASlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);//将OverlappingItem转换为AWeapon类型赋值给OverlappingWeapon
	if (OverlappingWeapon)
    {
		EquipWeapon(OverlappingWeapon);
    }
	else
	{
		if (CanDisarm())//角色空闲且正在装备武器
		{
			Disarm();
		}
		else if (CanArm())//角色空闲且未装备武器
		{
			Arm();
		}
	}
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	//装备OverlappingWeapon给到角色的RightHandSocket插槽上面
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	OverlappingItem = nullptr;//移除OverlappingItem,这样就角色就不会再次触发装备已经在身上的武器
	EquippedWeapon = Weapon;//将OverlappingWeapon赋给EquippedWeapon来存储装备过的武器变量
}

void ASlashCharacter::Attack()
{
	Super::Attack();
	if (CanAttack())
    {
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
        
    }

}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;

}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;//角色处于空闲状态且装备武器
}

void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
    {
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
    }
}

bool ASlashCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;//角色空闲且装备武器
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState == ECharacterState::ECS_Unequipped && EquippedWeapon;//角色空闲且未装备武器
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));//播放武器装备动画
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;//角色装备武器
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));//播放武器卸载动画
	CharacterState = ECharacterState::ECS_Unequipped;//角色未装备武器
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon)
	{
        EquippedWeapon->AttachMeshToSocket(GetMesh(),FName("SpineSocket"));//武器挂载在角色的SpineSocket插槽上面
	}
}

void ASlashCharacter::AttachWeaponToHand()
{
    if (EquippedWeapon)
    {
        EquippedWeapon->AttachMeshToSocket(GetMesh(),FName("RightHandSocket"));//武器挂载在角色的RightHandSocket插槽上面
    }
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;//角色空闲
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}
void ASlashCharacter::InitializeSlashOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());//GetController()返回的是AController类型的指针，所以需要强转为APlayerController来获取HUD
	if (PlayerController)
	{
		ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());//获取角色的HUD
		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();//获取HUD的Overlay
			if (SlashOverlay)
			{
				SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());//设置当前生命值
				SlashOverlay->SetStaminaBarPercent(1.f);
				SlashOverlay->SetGold(1);
				SlashOverlay->SetSouls(1);
			}
		}
	}
}





