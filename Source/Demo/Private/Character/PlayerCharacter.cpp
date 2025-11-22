// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerCharacter::APlayerCharacter()
{
	// 创建弹簧臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; // 视距
	CameraBoom->bUsePawnControlRotation = true; // 允许玩家控制旋转
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;

	// 创建摄像机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // 摄像机自己不跟随臂的控制器旋转 (由臂处理)

	// 角色不直接使用控制器旋转，旋转交给 Movement 根据速度方向处理
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// 让 CharacterMovement 朝着移动方向自动旋转角色
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;      // 朝移动方向转身
	MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f); // 转身速度
	MoveComp->MaxWalkSpeed = 600.f;
}
