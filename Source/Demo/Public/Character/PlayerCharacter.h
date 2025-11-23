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
	// 第三人称摄像机组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* FollowCamera;

	// 当前视角模式（第一人称 / 第三人称）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	EViewMode CurrentViewMode = EViewMode::ThirdPerson;

	// ========= 玩家专用：最近一帧输入方向 =========
	// 敌人不依赖玩家输入，因此不放在 CharacterBase，而是只在玩家类中维护
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement|State")
	FVector LastMovementInput = FVector::ZeroVector;

	// ========= 简易射击实现（当前阶段无武器类，无 GAS） =========

	// 射线射击的最大射程；
	// 注意：这是一个「临时的角色级别配置」，将来正式武器系统上线后，
	// 建议从武器配置（DataAsset / DataTable）或 GAS Attribute 中读取。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Debug")
	float SimpleFireMaxRange = 10000.0f; // 默认 10000 cm（100 m），可在蓝图中调整，用于测试

	// 服务器 RPC：客户端请求进行一次简单开火（射线检测）
	UFUNCTION(Server, Reliable)
	void Server_PerformSimpleFire(); // 将来可以被武器类或 Ability 替代

	// 实际执行射线检测与伤害（仅在服务器内调用）
	void PerformSimpleFire_Internal(); // 射击骨架，后续可迁移到 Weapon/Ability

	// ========= 武器占位：当前暂未真正使用 =========
	// 默认枪的蓝图类，在编辑器中指定（例如 BP_Gun）
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<AGun> DefaultGunClass;

	// 当前装备的枪实例
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	AGun* CurrentGun = nullptr;

	// 在 BeginPlay 中生成并附加默认枪
	virtual void BeginPlay() override;

public:
	// ========= 玩家专用：移动 / 跳跃输入接口 =========
	// 处理从控制器传入的移动输入（X: 右/左, Y: 前/后）
	void HandleMoveInput(const FVector2D& InputAxis);

	// 跳跃开始
	void HandleJumpStarted();

	// 跳跃结束
	void HandleJumpStopped();

	// ========= 射击输入接口（未来对接武器系统 / Gameplay Ability System） =========

	// 开火开始（按键按下）
	UFUNCTION(BlueprintCallable, Category="Weapon|Input")
	void HandleFireStarted(); // 将触发当前武器/Ability 的射击开始

	// 开火结束（按键松开）
	UFUNCTION(BlueprintCallable, Category="Weapon|Input")
	void HandleFireStopped(); // 将触发当前武器/Ability 的射击停止

	// 为了后续从外部（比如其他系统）直接触发当前武器开火，提供一个简单封装
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void StartFireCurrentGun();

	// 视角切换（由控制器调用）
	UFUNCTION(BlueprintCallable, Category="Camera")
	void ToggleViewMode();
};
