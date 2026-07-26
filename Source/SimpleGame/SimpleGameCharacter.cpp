// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleGameCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
ASimpleGameCharacter::ASimpleGameCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create CameraArm component
	this->CameraArm = this->CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	this->CameraArm->SetupAttachment(this->RootComponent);
	this->CameraArm->TargetArmLength = 425.0f;
	
	this->Camera = this->CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	this->Camera->SetupAttachment(this->CameraArm, USpringArmComponent::SocketName);
	
	this->bUseControllerRotationPitch = true;
	this->bUseControllerRotationYaw = true;
	this->bUseControllerRotationRoll = false;
}

// Called when the game starts or when spawned
void ASimpleGameCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PlayerController = ::Cast<APlayerController>(this->Controller);
	if (PlayerController != nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* PcSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (PcSubsystem != nullptr)
		{
			PcSubsystem->AddMappingContext(this->PlayerMappingContext, 0);
		}
	}
}

// Called every frame
void ASimpleGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASimpleGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EnhancedInputComponent = ::CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	EnhancedInputComponent->BindAction(this->MoveAction, ETriggerEvent::Triggered, this, &ASimpleGameCharacter::Move);
	EnhancedInputComponent->BindAction(this->TurnLeftRightAction, ETriggerEvent::Triggered, this, &ASimpleGameCharacter::TurnLeftRight);
	EnhancedInputComponent->BindAction(this->LookUpDownAction, ETriggerEvent::Triggered, this, &ASimpleGameCharacter::LookUpDown);
}

void ASimpleGameCharacter::Move(const FInputActionValue& Value)
{
	if (this->Controller != nullptr)
	{
		// Extract 2D axis data (x and y) 
		FVector2D MovementVector = Value.Get<FVector2D>();
		
		// Extract yaw rotation info
		FRotator Rotation = this->Controller->GetControlRotation();
		FRotator YawRotation(0, Rotation.Yaw, 0);
		
		// Create rotation matrix based on the yaw
		FRotationMatrix RotationMatrix = FRotationMatrix(YawRotation);
		FVector ForwardDirection = RotationMatrix.GetUnitAxis(EAxis::X); // In Unreal, X is forward/backward
		FVector RightDirection = RotationMatrix.GetUnitAxis(EAxis::Y); // In Unreal, Y is left/right
		
		this->AddMovementInput(ForwardDirection, MovementVector.Y); // In Unreal, Y is where we store W/S movement (hence why we swizzle in the InputMappingContext) 
		this->AddMovementInput(RightDirection, MovementVector.X); // In Unreal, X is where we store A/D movement
	}
}

void ASimpleGameCharacter::TurnLeftRight(const FInputActionValue& Value)
{
	FVector2D TurnVector = Value.Get<FVector2D>();
	
	this->AddControllerYawInput(TurnVector.X);
}

void ASimpleGameCharacter::LookUpDown(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	
	this->AddControllerPitchInput(LookVector.Y);
}

