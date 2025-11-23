// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Character/PlayerCharacter.h"

AMyPlayerController::AMyPlayerController()
	: DefaultMappingContext(nullptr)
	, MoveAction(nullptr)
	, JumpAction(nullptr)
	, LookAction(nullptr)
	, FireAction(nullptr) // 射击动作，由蓝图指定 InputAction
	, ToggleViewAction(nullptr)
{
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
		{
			if (DefaultMappingContext)
			{
				// 使用 Enhanced Input，将默认映射上下文添加到本地玩家
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)
		{
			EI->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnMove);
		}
		if (JumpAction)
		{
			EI->BindAction(JumpAction, ETriggerEvent::Started, this, &AMyPlayerController::OnJumpStarted);
			EI->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMyPlayerController::OnJumpStopped);
		}
		if (LookAction)
		{
			EI->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnLook);
		}

		// 绑定开火输入：Started / Completed 分别对应按下与松开
		if (FireAction)
		{
			EI->BindAction(FireAction, ETriggerEvent::Started, this, &AMyPlayerController::OnFireStarted);
			EI->BindAction(FireAction, ETriggerEvent::Completed, this, &AMyPlayerController::OnFireStopped);
		}
		// 新增：绑定视角切换输入（按下一次切换一次）
		if (ToggleViewAction)
		{
			EI->BindAction(ToggleViewAction, ETriggerEvent::Started, this, &AMyPlayerController::OnToggleView);
		}
	}
}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 只在占有 Pawn 时缓存一次，避免每帧 Cast
	CachedPlayerCharacter = Cast<APlayerCharacter>(InPawn);
}

void AMyPlayerController::OnMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	// 使用缓存的玩家角色指针，避免每次都 Cast
	if (CachedPlayerCharacter)
	{
		CachedPlayerCharacter->HandleMoveInput(Axis);
	}
}

void AMyPlayerController::OnLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	// 先根据输入更新控制器旋转
	AddYawInput(Axis.X);
	AddPitchInput(Axis.Y);

	// 如果当前控制的 Pawn 是玩家角色，则限制视角相对角色前方的偏转范围
	if (CachedPlayerCharacter)
	{
		// 控制器当前旋转
		const FRotator ControlRot = GetControlRotation();
		// 角色当前朝向
		const FRotator ActorRot   = CachedPlayerCharacter->GetActorRotation();

		// 计算控制器相对于角色的旋转差值，并归一化到 [-180, 180]
		FRotator DeltaRot = (ControlRot - ActorRot).GetNormalized();

		// === 核心限制逻辑 ===
		// 水平偏转：限制在角色正前方左右各 90 度范围内，防止视角完全转到背后
		DeltaRot.Yaw = FMath::ClampAngle(DeltaRot.Yaw, -90.0f, 90.0f);

		// 如有需要，也可以对俯仰角做限制，例如抬头/低头各 80 度：
		// DeltaRot.Pitch = FMath::ClampAngle(DeltaRot.Pitch, -80.0f, 80.0f);

		// 重新组合得到被限制后的控制器旋转：角色朝向 + 受限偏差
		FRotator ClampedControlRot = ActorRot + DeltaRot;
		ClampedControlRot.Normalize();

		SetControlRotation(ClampedControlRot);
	}
}

void AMyPlayerController::OnJumpStarted()
{
	if (CachedPlayerCharacter)
	{
		CachedPlayerCharacter->HandleJumpStarted();
	}
}

void AMyPlayerController::OnJumpStopped()
{
	if (CachedPlayerCharacter)
	{
		CachedPlayerCharacter->HandleJumpStopped();
	}
}

void AMyPlayerController::OnFireStarted(const FInputActionValue& Value)
{
	// 控制器只负责把输入事件转给角色，角色内部再决定调用武器系统/GAS
	if (CachedPlayerCharacter)
	{
		CachedPlayerCharacter->HandleFireStarted();
	}
}

void AMyPlayerController::OnFireStopped(const FInputActionValue& Value)
{
	if (CachedPlayerCharacter)
	{
		CachedPlayerCharacter->HandleFireStopped();
	}
}

void AMyPlayerController::OnToggleView(const FInputActionValue& Value)
{
	if (CachedPlayerCharacter)
	{
		CachedPlayerCharacter->ToggleViewMode();
	}
}
