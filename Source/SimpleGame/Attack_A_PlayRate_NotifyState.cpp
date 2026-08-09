// Fill out your copyright notice in the Description page of Project Settings.


#include "Attack_A_PlayRate_NotifyState.h"

void UAttack_A_PlayRate_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();	
	if (AnimInstance != nullptr)
	{
		UAnimMontage* AnimMontage = AnimInstance->GetCurrentActiveMontage();
	
		if (AnimMontage != nullptr)
		{
			// Store DefaultAttackA_PlayRate for future use if we're seeing it for the first time
			if (this->DefaultAttackA_PlayRate < 0.0f)
			{
				this->DefaultAttackA_PlayRate = AnimInstance->Montage_GetPlayRate(AnimMontage); 
			}
		
			AnimInstance->Montage_SetPlayRate(AnimMontage, this->IncreasedAttackA_PlayRate);
		}	
	}
}

void UAttack_A_PlayRate_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();	
	if (AnimInstance != nullptr)
	{
		UAnimMontage* AnimMontage = AnimInstance->GetCurrentActiveMontage();
	
		if (AnimMontage != nullptr)
		{
			AnimInstance->Montage_SetPlayRate(AnimMontage, this->DefaultAttackA_PlayRate);
		}	
	}
}
