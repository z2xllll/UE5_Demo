// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/Character.h"

AMyPlayerController::AMyPlayerController()
	: DefaultMappingContext(nullptr)
	, MoveAction(nullptr)
	, JumpAction(nullptr)
	, LookAction(nullptr)
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
	}
}

void AMyPlayerController::OnMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	ACharacter* Char = Cast<ACharacter>(GetPawn());
	if (Char && !Axis.IsNearlyZero())
	{
		// 使用控制器的 Yaw 计算基于摄像机的前/右方向
		const FRotator ControlRot = GetControlRotation();
		const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);
		const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		const FVector RightDir   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

		if (!FMath::IsNearlyZero(Axis.Y))
		{
			Char->AddMovementInput(ForwardDir, Axis.Y);
		}
		if (!FMath::IsNearlyZero(Axis.X))
		{
			Char->AddMovementInput(RightDir, Axis.X);
		}
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
	if (ACharacter* Char = Cast<ACharacter>(GetPawn()))
	{
		Char->Jump();
	}
}

void AMyPlayerController::OnJumpStopped()
{
	if (ACharacter* Char = Cast<ACharacter>(GetPawn()))
	{
		Char->StopJumping();
	}
}
