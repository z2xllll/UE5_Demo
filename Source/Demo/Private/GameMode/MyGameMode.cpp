// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode/MyGameMode.h"
#include "Character/PlayerCharacter.h"
#include "Character/MyPlayerController.h"

AMyGameMode::AMyGameMode()
{
	// 设置默认 Pawn 类为我们自定义的 APlayerCharacter（可以在蓝图中进一步指定 BP_PlayerCharacter）
	DefaultPawnClass = APlayerCharacter::StaticClass();

	// 设置默认 PlayerController 类为自定义控制器，用于使用 Enhanced Input 等
	PlayerControllerClass = AMyPlayerController::StaticClass();

	// 说明：武器/射击逻辑完全在 PlayerController + PlayerCharacter 层处理，
	// GameMode 不参与具体战斗逻辑，保持单一职责，便于扩展联机规则。
}
