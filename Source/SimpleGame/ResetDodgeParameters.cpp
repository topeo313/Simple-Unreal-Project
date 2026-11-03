// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleGameCharacter.h"
#include "ResetDodgeParameters.h"

void UResetDodgeParameters::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// Always call the Super first
	Super::Notify(MeshComp, Animation, EventReference);
	
	ASimpleGameCharacter* Character = ::Cast<ASimpleGameCharacter>(MeshComp->GetOwner());
	if (Character != nullptr)
	{
		Character->ResetDodgeParameters();
	}
}