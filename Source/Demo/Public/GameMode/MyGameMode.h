// File: Source/Demo/Public/GameMode/MyGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyGameMode.generated.h"

UCLASS()
class DEMO_API AMyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMyGameMode();
	// 备注：通过构造函数设置默认 Pawn 和 PlayerController，便于在 C++ 层面保证默认类型并支持蓝图覆盖
};
