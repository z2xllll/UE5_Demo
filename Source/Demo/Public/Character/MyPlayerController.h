// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class APlayerCharacter; // 前向声明玩家角色，避免循环包含

/**
 *
 */
UCLASS()
class DEMO_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

protected:
	// Enhanced Input 资产，由蓝图赋值
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* JumpAction;

	// 鼠标/摇杆查看输入
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* LookAction;

	// 开火输入（按住触发/点击触发均可在 InputAction 配置）
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* FireAction; // 只负责转发给角色的 HandleFire* 接口

	// 视角切换输入（第一/第三人称）
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* ToggleViewAction;

	// 缓存当前控制的玩家角色，避免每次都 Cast
	UPROPERTY()
	APlayerCharacter* CachedPlayerCharacter = nullptr;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override; // 新增：当控制器占有 Pawn 时更新缓存

	// 对应角色上的移动与跳跃逻辑
	void OnMove(const struct FInputActionValue& Value);
	void OnJumpStarted();
	void OnJumpStopped();
	void OnLook(const struct FInputActionValue& Value);

	// 新增：射击开始（按键按下）
	void OnFireStarted(const struct FInputActionValue& Value); // 将调用角色的 HandleFireStarted

	// 新增：射击结束（按键松开）
	void OnFireStopped(const struct FInputActionValue& Value); // 将调用角色的 HandleFireStopped

	// 视角切换回调
	void OnToggleView(const struct FInputActionValue& Value);
};
