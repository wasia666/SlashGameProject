// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "HealthBarComponent.generated.h"

/**
 * 
 */
UCLASS()
class SLASH2_API UHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
    void SetHealthPercent(float Percent);
	
private:
	UPROPERTY()
    class UHealthBar* HealthBarWidget;//现在我们有一个成员变量，而不必每次都进行强制转换，我们可以选择仅在以下情况下进行强制转换：此HealthBar尚未设置，换句话说，如果它为空。
};
