// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"

void ASlashHUD::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* Controller = World->GetFirstPlayerController();//先获取玩家控制器
		if (Controller && SlashOverlayClass)
		{
			SlashOverlay = CreateWidget<USlashOverlay>(Controller, SlashOverlayClass);//创造游戏布局在游戏开始的时候
			SlashOverlay->AddToViewport();//通过SlashOverlayClass获取到的SlashOverlay布局添加到游戏ui界面中
		}
	}

}
