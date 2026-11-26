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
	PrimaryComponentTick.bCanEverTick = false;                      // 关掉 Tick，省掉每帧开销
	InteractKeyText = FText::FromString(TEXT("E"));                 // 默认交互键文字
	PromptWidgetClass = UInteractionPromptWidget::StaticClass();    // 默认用原生 Slate 提示
}

/** 开局：按 ScanInterval 起循环定时器，并立刻先扫一次（不用等第一个间隔） */
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

/** 扫描一次：从与角色重叠的 Actor 里挑出唯一目标，并把文案、可用性一起算好交给 UI */
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

	const FVector OwnerLocation = Owner->GetActorLocation();         // 提到循环外，每个候选复用
	const float MaxDistSq = FMath::Square(MaxInteractionDistance);   // 用平方距离比较，省一次开方

	AActor* BestUsable = nullptr;                     // 当前真的能交互的目标（先比优先级，再比距离）
	FText BestUsablePrompt;
	float BestUsableDistSq = TNumericLimits<float>::Max();
	int32 BestUsablePriority = TNumericLimits<int32>::Lowest();

	AActor* BestAny = nullptr;                        // 只能显示说明文字的最近目标（如满血药水）
	FText BestAnyPrompt;
	float BestAnyDistSq = TNumericLimits<float>::Max();

	for (AActor* Candidate : OverlapBuffer)
	{
		// 跳过非法对象与自身；Cast 不成功就说明它没实现交互接口
		if (!IsValid(Candidate) || Candidate == Owner)
		{
			continue;
		}

		const IInteractableInterface* Interactable = Cast<IInteractableInterface>(Candidate);
		if (!Interactable)
		{
			continue;
		}

		// 超出交互距离的候选直接丢弃
		const float DistSq = FVector::DistSquared(OwnerLocation, Candidate->GetActorLocation());
		if (DistSq > MaxDistSq)
		{
			continue;
		}

		// 每个候选只问一次文本、一次可用性；文本为空 = 现在没有可显示内容，跳过
		const FText Prompt = Interactable->GetInteractionPrompt(Owner);
		if (Prompt.IsEmpty())
		{
			continue;
		}

		// 按"能否真的交互"分两组：可用组优先；组内先比优先级（药水 > 武器），同优先级再比距离
		if (Interactable->CanInteract(Owner))
		{
			const int32 Priority = Interactable->GetInteractionPriority(Owner);
			if (Priority > BestUsablePriority || (Priority == BestUsablePriority && DistSq < BestUsableDistSq))
			{
				BestUsable = Candidate;
				BestUsablePrompt = Prompt;
				BestUsableDistSq = DistSq;
				BestUsablePriority = Priority;
			}
		}
		else if (DistSq < BestAnyDistSq)
		{
			BestAny = Candidate;
			BestAnyPrompt = Prompt;
			BestAnyDistSq = DistSq;
		}
	}

	// 可交互目标优先：站在满血药水旁时，旁边的武器依然能被它抢到目标；
	// 只有范围内没有可交互物时，才退而显示"生命值已满"这类说明文字
	if (BestUsable)
	{
		SetCurrentTarget(BestUsable, BestUsablePrompt, true);
	}
	else
	{
		SetCurrentTarget(BestAny, BestAnyPrompt, false);
	}
}

/** 切换目标：目标变化时通知旧目标失去焦点、新目标获得焦点，然后用扫描结果刷新提示 */
void UInteractionComponent::SetCurrentTarget(AActor* NewTarget, const FText& ActionText, bool bUsable)
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

	UpdatePrompt(ActionText, bUsable);
}

/** 刷新提示：文案为空就整体隐藏；文案与可用性都没变则不重设，避免反复刷 Slate */
void UInteractionComponent::UpdatePrompt(const FText& ActionText, bool bUsable)
{
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

	// 文本和可用状态都没变就不重设
	if (PromptWidget && bUsable == bLastPromptUsable && ActionText.EqualTo(LastPromptText))
	{
		return;
	}

	EnsurePromptWidget();
	if (PromptWidget)
	{
		// 不可用时传空键文本：UI 据此隐藏白底键帽，只留说明文字
		PromptWidget->ShowPrompt(bUsable ? InteractKeyText : FText::GetEmpty(), ActionText);
	}

	// 缓存放在 Widget 判断之外：即使还没创建出 Widget，按键逻辑也能直接用这份状态
	LastPromptText = ActionText;
	bLastPromptUsable = bUsable;
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
	if (!IsValid(Owner) || !IsValid(Target))
	{
		return false;       // 没有目标：按键还给角色原有逻辑
	}

	IInteractableInterface* Interactable = Cast<IInteractableInterface>(Target);
	if (!Interactable)
	{
		return false;
	}

	// 直接复用最近一次扫描缓存的文案与可用性：按键瞬间不必再问目标两次
	if (LastPromptText.IsEmpty())
	{
		return false;       // 目标已失效（如刚被别人拾取）
	}
	if (!bLastPromptUsable)
	{
		return false;       // 不可用（如满血药水）：不吃键，让 E 回落到拔刀/收刀
	}

	const bool bInteracted = Interactable->Interact(Owner);

	// 交互后立刻重扫：目标可能已销毁，提示要马上更新（重扫内部会走 SetCurrentTarget 收尾）
	ScanForInteractables();
	return bInteracted;
}
