// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitInterface.generated.h"

//这个类是接口类，允许接口参与反射
UINTERFACE(MinimalAPI)
class UHitInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
// 这个类是接口类，允许在类中利用多重继承时实现接口，也是为我们声明函数的类
//当从其他cpp文件定义这个接口的时候（从其他头文件定义该类时），必须要包含这个接口的头文件
class SLASH2_API IHitInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	void GetHit(const FVector& ImpactPoint, AActor* Hitter);//可以在其他cpp文件中定义,如果想要在蓝图中调用这个函数，必须改为BlueprintNativeEvent
	//一般来说纯虚函数在接口类定义就无法在蓝图中调用
	//但是在接口类中定义为BlueprintNativeEvent后，可以在其他cpp文件中定义的同时，在蓝图中也看可以覆盖定义函数，但是无法是纯虚函数
	//本质上，这使得一个函数可以在蓝图中实现，另一个隐藏函数在名称中添加实现，我们可以用c++实现，我们可以在这里做任何我们想做的事情
};
