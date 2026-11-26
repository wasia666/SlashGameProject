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
	if (IsValid(Target) && Owner)
	{
		if (const IInteractableInterface* Interactable = Cast<IInteractableInterface>(Target))
		{
			ActionText = Interactable->GetInteractionPrompt(Owner);
		}
	}

	if (ActionText.IsEmpty())
	{
		if (PromptWidget)
		{
			PromptWidget->HidePrompt();
		}
		LastPromptText = FText::GetEmpty();
		return;
	}

	// 文本没变就不重设，避免每帧无意义地刷新 Slate
	if (PromptWidget && ActionText.EqualTo(LastPromptText))
	{
		return;
	}

	EnsurePromptWidget();
	if (PromptWidget)
	{
		PromptWidget->ShowPrompt(InteractKeyText, ActionText);
		LastPromptText = ActionText;
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

	// 目标当前不可用（如刚被别人拾取）时不吃掉按键，让 E 键回落到原有逻辑
	if (Interactable->GetInteractionPrompt(Owner).IsEmpty())
	{
		return false;
	}

	// 交互失败（如刚好满血）同样不吃按键
	const bool bInteracted = Interactable->Interact(Owner);

	// 交互后立即刷新，避免提示多显示一次扫描间隔
	SetCurrentTarget(nullptr);
	ScanForInteractables();
	return bInteracted;
}
