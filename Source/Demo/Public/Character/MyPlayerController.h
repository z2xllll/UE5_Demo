// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

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

	// 新增：鼠标/摇杆查看输入
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* LookAction;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// 对应角色上的移动与跳跃逻辑
	void OnMove(const struct FInputActionValue& Value);
	void OnJumpStarted();
	void OnJumpStopped();
	void OnLook(const struct FInputActionValue& Value);
};
