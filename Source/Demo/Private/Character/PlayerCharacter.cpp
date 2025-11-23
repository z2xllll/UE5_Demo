// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"
#include "Weapon/Gun.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

APlayerCharacter::APlayerCharacter()
{
	// 创建弹簧臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; // 视距
	CameraBoom->bUsePawnControlRotation = true; // 允许玩家控制旋转
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;

	// 创建摄像机，默认附着在 SpringArm 末端（第三人称）
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // 摄像机自己不跟随臂的控制器旋转 (由臂处理)
	FollowCamera->SetRelativeLocation(FVector::ZeroVector); // 第三人称默认相对位置为 0，偏移由 SpringArm 控制
	// 相机组件本身通常不参与物理阻挡，关闭 SpringArm 的 bDoCollisionTest 已足够避免被墙体推挤

	// 角色不直接使用控制器旋转，旋转交给 Movement 根据速度方向处理
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// 让 CharacterMovement 朝着移动方向自动旋转角色
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;      // 朝移动方向转身
	MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f); // 转身速度
	MoveComp->MaxWalkSpeed = 500.f;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 如果在蓝图中指定了默认枪类，则在游戏开始时生成并附加一把枪
	if (DefaultGunClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		AGun* SpawnedGun = GetWorld()->SpawnActor<AGun>(DefaultGunClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (SpawnedGun)
		{
			CurrentGun = SpawnedGun;
			CurrentGun->InitializeOwner(this);

			// 将枪附加到角色 Mesh 上的武器插槽（需在 SkeletalMesh 上预先创建名为 "WeaponSocket" 的 Socket）
			if (USkeletalMeshComponent* MeshComp = GetMesh())
			{
				CurrentGun->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
			}
		}
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 注意：通用运动状态（GroundSpeed、bIsInAir）已经在 ACharacterBase::Tick 中统一更新，
	// 这里无需重复计算；如果玩家有额外的 per-frame 逻辑，可以在此处补充。
}

void APlayerCharacter::HandleMoveInput(const FVector2D& InputAxis)
{
	// 没有控制器则不处理
	if (!Controller)
	{
		LastMovementInput = FVector::ZeroVector;
		return;
	}

	if (InputAxis.IsNearlyZero())
	{
		LastMovementInput = FVector::ZeroVector;
		return;
	}

	// 使用控制器 Yaw 计算与相机对齐的前/右方向
	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);
	const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector RightDir   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	FVector WorldInput = FVector::ZeroVector;

	if (!FMath::IsNearlyZero(InputAxis.Y))
	{
		AddMovementInput(ForwardDir, InputAxis.Y);
		WorldInput += ForwardDir * InputAxis.Y;
	}

	if (!FMath::IsNearlyZero(InputAxis.X))
	{
		AddMovementInput(RightDir, InputAxis.X);
		WorldInput += RightDir * InputAxis.X;
	}

	LastMovementInput = WorldInput;
}

void APlayerCharacter::HandleJumpStarted()
{
	// 目前直接使用引擎自带跳跃，将来可以在这里触发 Jump Ability
	Jump();
}

void APlayerCharacter::HandleJumpStopped()
{
	StopJumping();
}

void APlayerCharacter::HandleFireStarted()
{
	// 如果已经装备了枪，优先通过枪来处理开火逻辑
	if (CurrentGun)
	{
		CurrentGun->StartFire();
		return;
	}

	// ===== 以下为原有的简易射线实现，作为无武器情况下的回退逻辑 =====
	if (HasAuthority())
	{
		PerformSimpleFire_Internal();
	}
	else
	{
		Server_PerformSimpleFire();
	}
}

void APlayerCharacter::HandleFireStopped()
{
	// 当前的简易射击是“单发”行为，按下就打一枪，不需要处理松开逻辑。
	// 将来如果要实现“长按连发”：
	// - 可以在 HandleFireStarted 中开启一个定时器（或激活一个持续 Ability）
	// - 在 HandleFireStopped 中清理定时器（或结束 Ability）

	// 预留 GAS / 武器系统扩展点：
	// if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	// {
	//     ASC->CancelAbilitiesWithTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Weapon.Fire")));
	// }
}

// ========= 简易射击：RPC 与内部实现 =========

void APlayerCharacter::Server_PerformSimpleFire_Implementation()
{
	// 安全起见，再次确认是服务器上下文
	if (!HasAuthority())
	{
		return;
	}

	PerformSimpleFire_Internal();
}

void APlayerCharacter::PerformSimpleFire_Internal()
{
	// 这里只实现一个简单的射线射击，用于测试输入链路与联机同步。
	// 未来你可以把这部分逻辑迁移到：
	// - 某个 AWeaponBase::PerformFire()
	// - 或某个 UGameplayAbility::ActivateAbility() 中

	// 1. 计算射线起点和方向：这里使用摄像机位置与朝向
	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation); // 获取“眼睛位置”和视线方向

	const FVector ShotDirection = ViewRotation.Vector();

	// 2. 定义射线长度（避免硬编码，未来可以从配置或武器数据资产中读取）
	constexpr float SimpleFireRange = 10000.0f; // 10 米射程，后续可提取为常量或配置

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd   = TraceStart + ShotDirection * SimpleFireRange;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PerformSimpleFire), /*bTraceComplex=*/true, this);
	QueryParams.bReturnPhysicalMaterial = false;

	FHitResult HitResult;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	// 3. 调试可视化：在服务器上画一条射线，方便观察方向与命中（联机时只在服务器可见）
#if ENABLE_DRAW_DEBUG
	const FColor LineColor = bHit ? FColor::Red : FColor::Green;
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, LineColor, false, 1.0f, 0, 1.0f);
#endif

	// 4. 如果命中目标，这里先简单打印日志，未来可替换为伤害应用
	if (bHit && HitResult.GetActor())
	{
		UE_LOG(LogTemp, Log, TEXT("SimpleFire hit actor: %s at location %s"),
			*HitResult.GetActor()->GetName(),
			*HitResult.ImpactPoint.ToString());

		// TODO（未来）：在这里应用伤害（推荐用 GAS 的 GameplayEffect）
		// 例如暂时可以用原生伤害系统：
		// UGameplayStatics::ApplyPointDamage(
		//     HitResult.GetActor(),
		//     BaseDamage,              // 从武器配置读取
		//     ShotDirection,
		//     HitResult,
		//     GetController(),
		//     this,
		//     UDamageType::StaticClass()
		// );
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("SimpleFire did not hit any actor."));
	}
}

void APlayerCharacter::StartFireCurrentGun()
{
	// 当前尚未真正接入武器对象，这里作为占位实现：
	// 先直接复用角色自身的射击入口，确保蓝图/输入绑定不会触发链接错误。
	HandleFireStarted();
}

void APlayerCharacter::ToggleViewMode()
{
	// 在第一/第三人称之间切换
	if (CurrentViewMode == EViewMode::ThirdPerson)
	{
		CurrentViewMode = EViewMode::FirstPerson;

		// 第一人称：缩短 SpringArm 长度，但主要依赖插槽来确定相机精确位置
		if (CameraBoom)
		{
			CameraBoom->TargetArmLength = 0.f;
			CameraBoom->bUsePawnControlRotation = false; // 让相机直接使用控制器旋转
		}

		if (FollowCamera)
		{
			FollowCamera->bUsePawnControlRotation = true;

			// 将相机从 SpringArm 上脱离，附着到角色 Mesh 的 "Camera" 插槽上
			if (USkeletalMeshComponent* MeshComp = GetMesh())
			{
				// 这里假定你已经在角色头部的骨骼上创建了名为 "Camera" 的 Socket
				FollowCamera->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Camera"));
				// 清掉相对偏移，强制使用 Socket 的位置和旋转
				FollowCamera->SetRelativeLocation(FVector::ZeroVector);
				FollowCamera->SetRelativeRotation(FRotator::ZeroRotator);
			}
		}

		// 切入第一人称时，强制让角色朝向对齐到当前控制器 Yaw，避免身朝向与视角不同步
		if (Controller)
		{
			const FRotator ControlRot = Controller->GetControlRotation();
			const FRotator NewYawRot(0.f, ControlRot.Yaw, 0.f);
			SetActorRotation(NewYawRot);
		}

		// 第一人称：角色旋转由控制器控制，不再根据移动方向自动转身
		bUseControllerRotationYaw = true;
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->bOrientRotationToMovement = false;
		}
	}
	else
	{
		CurrentViewMode = EViewMode::ThirdPerson;

		// 第三人称：恢复 SpringArm 长度与旋转设置
		if (CameraBoom)
		{
			CameraBoom->TargetArmLength = 300.f; // 可以根据需要调节，后续可提为配置
			CameraBoom->bUsePawnControlRotation = true;
		}
		if (FollowCamera)
		{
			FollowCamera->bUsePawnControlRotation = false;

			// 将相机重新附着回 SpringArm 末端，由 SpringArm 控制第三人称位置
			FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::SnapToTargetNotIncludingScale, USpringArmComponent::SocketName);
			FollowCamera->SetRelativeLocation(FVector::ZeroVector);
			FollowCamera->SetRelativeRotation(FRotator::ZeroRotator);
		}

		// 第三人称：角色朝移动方向自动旋转，控制器只负责摄像机
		bUseControllerRotationYaw = false;
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->bOrientRotationToMovement = true;
		}
	}
}
