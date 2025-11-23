// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AGun;

UENUM(BlueprintType)
enum class EViewMode : uint8
{
	ThirdPerson,
	FirstPerson
};

/**
 * 玩家角色：负责移动、基础状态和射击入口（未来会接入武器系统/GAS）。
 * 当前阶段：仅实现极简的「单发射线射击」，用于验证输入链路与联机逻辑。
 */
UCLASS()
class DEMO_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	APlayerCharacter();
	virtual void Tick(float DeltaTime) override;

protected:
	// 摄像机与视角
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	EViewMode CurrentViewMode = EViewMode::ThirdPerson;

	// ========= 玩家专用：输入与视角状态 =========
	// 最近一帧输入方向（用于移动/动画）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement|State")
	FVector LastMovementInput = FVector::ZeroVector;

	// 水平视角角度（Yaw）：左右转头/转身角度（单位：度）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera|State")
	float HorizontalAngle = 0.0f;

	// 垂直视角角度（Pitch）：抬头/低头角度（单位：度）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera|State")
	float VerticalAngle = 0.0f;

	// ========= 简易射击实现 =========
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Debug")
	float SimpleFireMaxRange = 10000.0f;

	UFUNCTION(Server, Reliable)
	void Server_PerformSimpleFire();
	void PerformSimpleFire_Internal();

	// ========= 武器占位 =========
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<AGun> DefaultGunClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	AGun* CurrentGun = nullptr;

	virtual void BeginPlay() override;

public:
	// 输入处理
	void HandleMoveInput(const FVector2D& InputAxis);
	void HandleJumpStarted();
	void HandleJumpStopped();

	UFUNCTION(BlueprintCallable, Category="Weapon|Input")
	void HandleFireStarted();

	UFUNCTION(BlueprintCallable, Category="Weapon|Input")
	void HandleFireStopped();

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void StartFireCurrentGun();

	UFUNCTION(BlueprintCallable, Category="Camera")
	void ToggleViewMode();

	// 视角角度的 Blueprint 访问接口（可用于动画蓝图或 UI）
	UFUNCTION(BlueprintPure, Category="Camera|State")
	float GetHorizontalAngle() const { return HorizontalAngle; }

	UFUNCTION(BlueprintPure, Category="Camera|State")
	float GetVerticalAngle() const { return VerticalAngle; }
};
