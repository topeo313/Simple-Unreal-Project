// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "SimpleGameCharacter.h"
#include "Animation/AnimNode_StateMachine.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	APawn* PawnOwner = this->TryGetPawnOwner();
	if (PawnOwner != nullptr)
	{
		this->GameCharacter = ::Cast<ASimpleGameCharacter>(PawnOwner);
		if (this->GameCharacter != nullptr)
		{
			this->GameCharacterMovementComponent = this->GameCharacter->GetCharacterMovement();
		}
	}
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	UAnimInstance::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
	if (this->GameCharacter == nullptr || this->GameCharacterMovementComponent == nullptr) return;
	
	const FVector velocity = this->GameCharacter->GetVelocity();
	this->speed = velocity.Size2D(); // No need for upward (z) movement to get speed magnitude
	this->isInAir = this->GameCharacterMovementComponent->IsFalling();
	this->isFallingDown = velocity.Z < 0.0;
	this->isPlayerMovementInputEnabled = this->GameCharacter->IsPlayerMovementInputEnabled();
	this->currentInputMovementVector = this->GameCharacter->GetCurrentInputMovementVector();
	this->isAttackStarted = this->GameCharacter->IsAttackStarted();	
	this->isHeavyAttackStarted = this->GameCharacter->IsHeavyAttackStarted();
	this->isAttacking = this->GameCharacter->IsAttacking();
	this->isAttackBlendingOut = this->GameCharacter->IsAttackBlendingOut();
	this->isRotatingForBlock = this->GameCharacter->IsInRotationForBlock();
	this->isInBlockMode = this->GameCharacter->IsInBlockMode();
	this->isBlockAnimationEnded = this->GameCharacter->IsBlockAnimationEnded();
	this->isInDodgeMode = this->GameCharacter->IsInDodgeMode();
		
	//UE_LOG(LogTemp, Warning, TEXT("%d"), this->isRotatingForBlock);
}