#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HitInterface.h"
#include "BreakableActor.generated.h"

class UGeometryCollectionComponent;

UCLASS()
class SLASH2_API ABreakableActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	

	ABreakableActor();

	virtual void Tick(float DeltaTime) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	UGeometryCollectionComponent* GeometryCollection;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCapsuleComponent* Capsule;
private:

	bool bBroken = false;
	


	UPROPERTY(EditAnywhere,Category = "Breakable Properties")
	TArray<TSubclassOf<class ATreasure>> TreasureClasses;
	/*
	TSubclass是模板类，是一个包装器，包装器是一种结构体，通常是一种模板类型，旨在包装指针，它为我们存储该指针，返回一个UClass指针但它被包装在一个TSubclasss指针中，这强制执行它只能从选中的类和其派生类（这里选中的类是ATreasure）
	这样在蓝图里面的TreasureClass变量中只能存储类 BP_Treasure以及其派生的类
	UClass是Unreal引擎中用于描述Unreal引擎中的类和类的属性的类。UClass对象可以创建实例，并访问实例的属性。

	模板函数一般需要传入参数类<ATreasure>(c++类)，但是如果想要传入蓝图类，可以借助UClass，通过蓝图里面传入的类名，获取类BP_Treasure
	然后我们将在c++中有一个变量，它代表我们在编辑器中创建的蓝图，并且它附带该蓝图上的任何数据，例如我们可能在其上设置的任何静态网格。
	*/
};
