
#include "Breakable/BreakableActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Items/Treasure.h"
#include "Components/CapsuleComponent.h"
#include"Kismet/GameplayStatics.h"

ABreakableActor::ABreakableActor()
{

	PrimaryActorTick.bCanEverTick = false;

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));//创建几何集合
	SetRootComponent(GeometryCollection);//UGeometryCollectionComponent从USenceComponent派生而来，所以可以将GeometryCollection设置为根目录

	GeometryCollection->SetGenerateOverlapEvents(true);//打开重叠事件
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//将相机通道设置为忽略
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);//将Pawn通道设置为忽略

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));//创建胶囊体
    Capsule->SetupAttachment(RootComponent);//将胶囊体接到根目录下
    Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);//将Pawn通道设置为拦截
}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	
}




void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	//确保只执行一次，避免重复执行
	if (bBroken) return;//已经被击打过，将不会在c++端继续执行
    bBroken = true;


	UWorld* World = GetWorld();
    if (World && TreasureClasses.Num() > 0)
    {
		FVector Location = GetActorLocation();//获取Treasure位置
        Location.Z += 80.f;//添加Treasure高度

		const int32 Selection = FMath::RandRange(0, TreasureClasses.Num() - 1);//随机选择一个Treasure

		World->SpawnActor<ATreasure>(TreasureClasses[Selection], Location, GetActorRotation());//在breakable打击之后创建Treasure物品

    }

}
