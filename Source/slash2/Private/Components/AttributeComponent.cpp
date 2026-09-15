
#include "Components/AttributeComponent.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();


	
}

void UAttributeComponent::ReceiveDamage(float Damage)
{
    Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);//如果传入一个值，然后将其限制在最小值0到最大值MaxHealth之间

}

float UAttributeComponent::GetHealthPercent()
{
    return Health / MaxHealth;//返回当前生命值百分比
}

bool UAttributeComponent::IsAlive()
{
    return Health > 0.f;//如果当前生命值大于0，则返回真
}


void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

