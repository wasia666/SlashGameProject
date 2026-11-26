#include "Components/InteractionComponent.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HUD/InteractionPromptWidget.h"
#include "Interfaces/InteractableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

/** 构造函数：交互靠定时扫描驱动，不需要每帧 Tick */
UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;          // 关掉 Tick，节省每帧开销
	InteractKeyText = FText::FromString(TEXT("E"));     // 默认交互键文字
	PromptWidgetClass = UInteractionPromptWidget::StaticClass();  // 默认用原生 Slate 提示
}

/** 开局：按 ScanInterval 起循环定时器，并立刻先扫一次（不用等第一个间隔）*/
void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ScanTimerHandle, this, &UInteractionComponent::ScanForInteractables,
			FMath::Max(ScanInterval, 0.02f), true);     // 下限 0.02s，防止配置成 0 变成每帧扫描
	}

	ScanForInteractables();
}

/** 结束：清定时器 + 移除提示 Widget，避免关卡切换后残留 UI */
void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScanTimerHandle);
	}

	if (PromptWidget)
	{
		PromptWidget->RemoveFromParent();
		PromptWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

/** 扫描一次：从与角色重叠的 Actor 里挑出最近、且当前真的有提示的那个作为目标 */
void UInteractionComponent::ScanForInteractables()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	// 复用"物品 Sphere <-> 角色 Mesh"的重叠结果，不额外增加碰撞开销
	OverlapBuffer.Reset();
	Owner->GetOverlappingActors(OverlapBuffer);

	const float MaxDistSq = FMath::Square(MaxInteractionDistance);  // 用平方距离比较，省一次开方

	AActor* BestUsable = nullptr;                        // 现在真的能交互的最近目标（没满血/没被装备）
	float BestUsableDistSq = TNumericLimits<float>::Max();
	AActor* BestAny = nullptr;                           // 只能显示说明文字的最近目标（如满血药水）
	float BestAnyDistSq = TNumericLimits<float>::Max();

	for (AActor* Candidate : OverlapBuffer)
	{
		// 跳过非法对象、自身，以及没实现交互接口的 Actor
		if (!IsValid(Candidate) || Candidate == Owner || !Candidate->Implements<UInteractableInterface>())
		{
			continue;
		}

		// 超出交互距离的候选直接丢弃
		const float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Candidate->GetActorLocation());
		if (DistSq > MaxDistSq)
		{
			continue;
		}

		// 提示为空 = 当前没有可显示内容（如目标已失效），不参与选中
		const IInteractableInterface* Interactable = Cast<IInteractableInterface>(Candidate);
		if (!Interactable || Interactable->GetInteractionPrompt(Owner).IsEmpty())
		{
			continue;
		}

		// 按"能否真的交互"分两组各自取最近，可交互的那组永远优先
		if (Interactable->CanInteract(Owner))
		{
			if (DistSq < BestUsableDistSq)
			{
				BestUsable = Candidate;
				BestUsableDistSq = DistSq;
			}
		}
		else if (DistSq < BestAnyDistSq)
		{
			BestAny = Candidate;
			BestAnyDistSq = DistSq;
		}
	}

	// 可交互目标优先：站在满血药水旁时，旁边的武器依然能被它抢到目标；
	// 只有范围内没有可交互物时，才退而显示"生命值已满"这类说明文字
	SetCurrentTarget(BestUsable ? BestUsable : BestAny);
}

/** 切换目标：目标变化时通知旧目标失去焦点、新目标获得焦点，然后刷新提示 */
void UInteractionComponent::SetCurrentTarget(AActor* NewTarget)
{
	AActor* OldTarget = CurrentTarget.Get();
	if (OldTarget != NewTarget)
	{
		// 旧目标先收 Unfocused（它可能已经被销毁，所以要判 IsValid）
		if (IsValid(OldTarget))
		{
			if (IInteractableInterface* OldInteractable = Cast<IInteractableInterface>(OldTarget))
			{
				OldInteractable->OnUnfocused(GetOwner());
			}
		}

		CurrentTarget = NewTarget;

		// 新目标收 Focused，方便物品做高亮/头顶图标
		if (IsValid(NewTarget))
		{
			if (IInteractableInterface* NewInteractable = Cast<IInteractableInterface>(NewTarget))
			{
				NewInteractable->OnFocused(GetOwner());
			}
		}
	}

	UpdatePrompt();
}

/** 刷新提示：取当前目标的文案与可用状态，决定显示"键帽+文字"还是只显示文字 */
void UInteractionComponent::UpdatePrompt()
{
	AActor* Owner = GetOwner();
	AActor* Target = CurrentTarget.Get();

	FText ActionText = FText::GetEmpty();
	bool bUsable = false;       // 目标此刻是否真的能被交互（满血治疗物为 false）
	if (IsValid(Target) && Owner)
	{
		if (const IInteractableInterface* Interactable = Cast<IInteractableInterface>(Target))
		{
			ActionText = Interactable->GetInteractionPrompt(Owner);
			bUsable = Interactable->CanInteract(Owner);
		}
	}

	// 没有文案：整个提示隐藏
	if (ActionText.IsEmpty())
	{
		if (PromptWidget)
		{
			PromptWidget->HidePrompt();
		}
		LastPromptText = FText::GetEmpty();
		bLastPromptUsable = false;
		return;
	}

	// 文本和可用状态都没变就不重设，避免每帧无意义地刷新 Slate
	if (PromptWidget && bUsable == bLastPromptUsable && ActionText.EqualTo(LastPromptText))
	{
		return;
	}

	EnsurePromptWidget();
	if (PromptWidget)
	{
		// 不可用时传空键文本：UI 据此隐藏白底键帽，只留说明文字
		PromptWidget->ShowPrompt(bUsable ? InteractKeyText : FText::GetEmpty(), ActionText);
		LastPromptText = ActionText;
		bLastPromptUsable = bUsable;
	}
}

/** 懒创建提示 Widget：优先用角色自己的 PlayerController，兜底用 0 号玩家的 */
void UInteractionComponent::EnsurePromptWidget()
{
	if (PromptWidget)
	{
		return;
	}

	APlayerController* PC = nullptr;
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		PC = Cast<APlayerController>(OwnerPawn->GetController());
	}
	if (!PC)
	{
		PC = UGameplayStatics::GetPlayerController(this, 0);
	}
	if (!PC)
	{
		return;
	}

	const TSubclassOf<UInteractionPromptWidget> WidgetClass =
		PromptWidgetClass ? PromptWidgetClass : UInteractionPromptWidget::StaticClass();

	PromptWidget = CreateWidget<UInteractionPromptWidget>(PC, WidgetClass);
	if (PromptWidget)
	{
		PromptWidget->AddToViewport(0);     // 加进视口即显示，先统一隐藏等提示调用
		PromptWidget->HidePrompt();
	}
}

/** 玩家按下交互键：把按键转发给当前目标，并返回"这次按键是否被消费" */
bool UInteractionComponent::TryInteract()
{
	AActor* Owner = GetOwner();
	AActor* Target = CurrentTarget.Get();
	if (!IsValid(Target) || !IsValid(Owner))
	{
		return false;       // 没有目标：按键还给角色原有逻辑
	}

	IInteractableInterface* Interactable = Cast<IInteractableInterface>(Target);
	if (!Interactable)
	{
		return false;
	}

	// 目标已消失/失效（如刚被别人拾取）时不吃掉按键，让 E 键回落到原有逻辑
	if (Interactable->GetInteractionPrompt(Owner).IsEmpty())
	{
		return false;
	}

	// 目标还在但此刻不可用（如满血药水）：提示继续显示说明文字，
	// 但这次按键不消费，让 E 回落到角色原有的拔刀/收刀逻辑
	if (!Interactable->CanInteract(Owner))
	{
		return false;
	}

	const bool bInteracted = Interactable->Interact(Owner);

	// 交互后立即刷新，避免提示多显示一次扫描间隔
	SetCurrentTarget(nullptr);
	ScanForInteractables();
	return bInteracted;
}
