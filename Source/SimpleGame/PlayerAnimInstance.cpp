// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "SimpleGameCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	APawn* pawnOwner = this->TryGetPawnOwner();
	if (pawnOwner != nullptr)
	{
		this->GameCharacter = ::Cast<ASimpleGameCharacter>(pawnOwner);
		if (this->GameCharacter != nullptr)
		{
			this->GameCharacterMovementComponent = this->GameCharacter->GetCharacterMovement();
		}
	}
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	UAnimInstance::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}


void UPlayerAnimInstance::UpdateAnimationProperties(float deltaTime)
{
	if (this->GameCharacter == nullptr || this->GameCharacterMovementComponent == nullptr) return;
	
	const FVector velocity = this->GameCharacter->GetVelocity();
	this->speed = velocity.Size2D(); // No need for upward (z) movement to get speed magnitude
	this->isInAir = this->GameCharacterMovementComponent->IsFalling();
	this->isPlayerMovementInputEnabled = this->GameCharacterMovementComponent->GetCurrentAcceleration().SizeSquared() > KINDA_SMALL_NUMBER;
}
