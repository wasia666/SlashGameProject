// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Souls.h"
#include "Interfaces/PickupInterface.h"


void ASouls::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    //如果OtherActor实现了接口，即当角色触发 Souls 时，则调用接口的 AddSouls 函数，然后销毁自身
    IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
    if (PickupInterface)
    {
        PickupInterface->AddSouls(this);//如果Souls实现了接口，则添加 Souls到PickupInterface这个接口，在从这个接口到SlashCharacter中，并且添加到AttributeComponent中，然后调用AttributeComponent里的AddSouls函数
        SpawnPickupSystem();
        SpawnPickupSound();

        Destroy();
    }
   
}
