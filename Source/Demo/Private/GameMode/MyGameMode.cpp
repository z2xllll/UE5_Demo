// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode/MyGameMode.h"
#include "Character/PlayerCharacter.h"            // 自定义角色类
#include "Character/MyPlayerController.h"        // 自定义控制器类

AMyGameMode::AMyGameMode()
{
	// 设置默认 Pawn 类为我们自定义的 APlayerCharacter（可以在蓝图中进一步指定 BP_PlayerCharacter）
	DefaultPawnClass = APlayerCharacter::StaticClass();

	// 设置默认 PlayerController 类为自定义控制器，用于使用 Enhanced Input 等
	PlayerControllerClass = AMyPlayerController::StaticClass();
}