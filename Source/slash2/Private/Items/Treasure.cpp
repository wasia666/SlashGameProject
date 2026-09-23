// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Treasure.h"
#include"Kismet/GameplayStatics.h"
#include "Interfaces/PickupInterface.h"

void ATreasure::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

    IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
    if (PickupInterface)
    {
       PickupInterface->AddGold(this);//如果Treasure实现了接口，则添加 Gold到PickupInterface这个接口，在从这个接口到SlashCharacter中，并且添加到AttributeComponent中，然后调用AttributeComponent里的AddGold函数
       SpawnPickupSound();
       Destroy();//将模型销毁
    }
}
