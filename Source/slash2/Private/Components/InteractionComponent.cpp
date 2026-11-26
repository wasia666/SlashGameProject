#include "Components/InteractionComponent.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HUD/InteractionPromptWidget.h"
#include "Interfaces/InteractableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InteractKeyText = FText::FromString(TEXT("E"));
	PromptWidgetClass = UInteractionPromptWidget::StaticClass();
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ScanTimerHandle, this, &UInteractionComponent::ScanForInteractables,
			FMath::Max(ScanInterval, 0.02f), true);
	}

	ScanForInteractables();
}

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

	const float MaxDistSq = FMath::Square(MaxInteractionDistance);
	AActor* BestTarget = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (AActor* Candidate : OverlapBuffer)
	{
		if (!IsValid(Candidate) || Candidate == Owner || !Candidate->Implements<UInteractableInterface>())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Candidate->GetActorLocation());
		if (DistSq > MaxDistSq || DistSq >= BestDistSq)
		{
			continue;
		}

		const IInteractableInterface* Interactable = Cast<IInteractableInterface>(Candidate);
		if (!Interactable || Interactable->GetInteractionPrompt(Owner).IsEmpty())
		{
			continue;   // 当前不可用（如满血），不参与选中
		}

		BestTarget = Candidate;
		BestDistSq = DistSq;
	}

	SetCurrentTarget(BestTarget);
}

void UInteractionComponent::SetCurrentTarget(AActor* NewTarget)
{
	AActor* OldTarget = CurrentTarget.Get();
	if (OldTarget != NewTarget)
	{
		if (IsValid(OldTarget))
		{
			if (IInteractableInterface* OldInteractable = Cast<IInteractableInterface>(OldTarget))
			{
				OldInteractable->OnUnfocused(GetOwner());
			}
		}

		CurrentTarget = NewTarget;

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

void UInteractionComponent::UpdatePrompt()
{
	AActor* Owner = GetOwner();
	AActor* Target = CurrentTarget.Get();

	FText ActionText = FText::GetEmpty();
	bool bUsable = false;
	if (IsValid(Target) && Owner)
	{
		if (const IInteractableInterface* Interactable = Cast<IInteractableInterface>(Target))
		{
			ActionText = Interactable->GetInteractionPrompt(Owner);
			bUsable = Interactable->CanInteract(Owner);
		}
	}

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
		PromptWidget->AddToViewport(0);
		PromptWidget->HidePrompt();
	}
}

bool UInteractionComponent::TryInteract()
{
	AActor* Owner = GetOwner();
	AActor* Target = CurrentTarget.Get();
	if (!IsValid(Target) || !IsValid(Owner))
	{
		return false;
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

	// 目标还在但当前不可用（如满血）：提示文字已经说明了原因，
	// 这次按键直接吃掉，避免顺势触发装备/收刀
	if (!Interactable->CanInteract(Owner))
	{
		return true;
	}

	const bool bInteracted = Interactable->Interact(Owner);

	// 交互后立即刷新，避免提示多显示一次扫描间隔
	SetCurrentTarget(nullptr);
	ScanForInteractables();
	return bInteracted;
}
