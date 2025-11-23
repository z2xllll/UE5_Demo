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
	// 鼠标/摇杆控制视角：X 控制 Yaw，Y 控制 Pitch
	AddYawInput(Axis.X);
	AddPitchInput(Axis.Y);
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
