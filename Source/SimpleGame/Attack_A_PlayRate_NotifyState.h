// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Attack_A_PlayRate_NotifyState.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLEGAME_API UAttack_A_PlayRate_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
// Operations
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

// Members
private:
	float DefaultAttackA_PlayRate = -1.0f;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters", meta = (AllowPrivateAccess = true))	
	float IncreasedAttackA_PlayRate = 3.0f;
};
