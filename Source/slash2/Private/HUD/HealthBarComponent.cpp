// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/HealthBarComponent.h"
#include "HUD/HealthBar.h"
#include "Components/ProgressBar.h"

void UHealthBarComponent::SetHealthPercent(float Percent)
{
    if (HealthBarWidget == nullptr)
    {
        //如果子类（HealthBarWidget）不存在，就将父类（HealthBar）转换为子类
	    HealthBarWidget = Cast<UHealthBar>(GetUserWidgetObject());//先获取UserWidget，再将父类转换为子类
      
    }
	if (HealthBarWidget && HealthBarWidget->HealthBar)
    {
        HealthBarWidget->HealthBar->SetPercent(Percent);//子类访问父类的属性
    }

}
