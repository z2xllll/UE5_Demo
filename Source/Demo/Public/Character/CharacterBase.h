// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

UCLASS()
class DEMO_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// ========= 通用运动状态：玩家和敌人都可以使用 =========

	// 当前水平速度（只在 C++ 中更新），驱动 Idle/Walk/Run 等动画
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement|State")
	float GroundSpeed = 0.0f;

	// 是否在空中（跳跃/下落），供动画状态机和逻辑判断
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement|State")
	bool bIsInAir = false;

public:
	// Blueprint 只读访问接口，方便 AnimBP 或其他蓝图读取状态
	UFUNCTION(BlueprintPure, Category="Movement|State")
	float GetGroundSpeed() const { return GroundSpeed; }

	UFUNCTION(BlueprintPure, Category="Movement|State")
	bool IsInAir() const { return bIsInAir; }
};
