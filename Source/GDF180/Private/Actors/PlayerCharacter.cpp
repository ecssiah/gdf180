#include "PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	if (auto* CharacterMovementComponent { GetCharacterMovement() }) 
	{
		CharacterMovementComponent->AirControl = 1.0f;
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetMode(MOVE_Flying);
	
	if (const auto* PlayerController { Cast<APlayerController>(GetController()) })
	{
		if (
			auto* InputLocalPlayerSubsystem {
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
					PlayerController->GetLocalPlayer()
				)
			}
		) {
			InputLocalPlayerSubsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (
		auto* EnhancedInputComponent {
			CastChecked<UEnhancedInputComponent>(PlayerInputComponent)
		}
	) {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(ModeAction, ETriggerEvent::Started, this, &APlayerCharacter::SwitchMode);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}
	
	const FVector MoveInput { Value.Get<FVector>() };
	const FRotator ControlRotator { Controller->GetControlRotation() };

	if (
		const auto* CharacterMovementComponent { GetCharacterMovement() };
		CharacterMovementComponent->IsFlying()
	) {
		const FRotationMatrix ControlRotatorMatrix { ControlRotator };

		const FRotator YawRotator { 0.0f, ControlRotator.Yaw, 0.0f };
		const FRotationMatrix ControlYawRotatorMatrix { YawRotator };
		
		const FVector Forward { ControlRotatorMatrix.GetUnitAxis(EAxis::X) };
		const FVector Right { ControlRotatorMatrix.GetUnitAxis(EAxis::Y) };
		const FVector Up { ControlYawRotatorMatrix.GetUnitAxis(EAxis::Z) };
		
		AddMovementInput(Forward, MoveInput.Y);
		AddMovementInput(Right, MoveInput.X);
		AddMovementInput(Up, MoveInput.Z);
	}
	else
	{
		const FRotator YawRotator { 0.0f, ControlRotator.Yaw, 0.0f };
		const FRotationMatrix ControlYawRotatorMatrix { YawRotator };
		
		const FVector Forward { ControlYawRotatorMatrix.GetUnitAxis(EAxis::X) };
		const FVector Right { ControlYawRotatorMatrix.GetUnitAxis(EAxis::Y) };
		
		AddMovementInput(Forward, MoveInput.Y);
		AddMovementInput(Right, MoveInput.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookInput { Value.Get<FVector2D>() };
	
	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void APlayerCharacter::SwitchMode(const FInputActionValue& Value)
{
	const auto* CharacterMovementComponent { GetCharacterMovement() };
	
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	if (CharacterMovementComponent->MovementMode == MOVE_Flying)
	{
		SetMode(MOVE_Walking);
	}
	else
	{
		SetMode(MOVE_Flying);
	}
}

void APlayerCharacter::SetMode(const EMovementMode& Mode)
{
	auto* CharacterMovementComponent { GetCharacterMovement() };
	
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	if (Mode == MOVE_Flying)
	{
		CharacterMovementComponent->SetMovementMode(MOVE_Flying);
		
		CharacterMovementComponent->Velocity = FVector::ZeroVector;
		CharacterMovementComponent->MaxFlySpeed = 1600.0f;
		CharacterMovementComponent->MaxAcceleration = 999999.0f;
		CharacterMovementComponent->BrakingFrictionFactor = 0.0f;
		CharacterMovementComponent->BrakingDecelerationFlying = 999999.0f;
		CharacterMovementComponent->bApplyGravityWhileJumping = false;

		GetCapsuleComponent()->SetEnableGravity(false);
	}
	else
	{
		CharacterMovementComponent->SetMovementMode(MOVE_Walking);
		
		CharacterMovementComponent->MaxWalkSpeed = 800.0f;
		CharacterMovementComponent->MaxAcceleration = 2048.0f;
		CharacterMovementComponent->JumpZVelocity = 1200.0f;
		CharacterMovementComponent->BrakingFrictionFactor = 1.0f;
		CharacterMovementComponent->BrakingDecelerationWalking = 2048.0f;
	
		GetCapsuleComponent()->SetEnableGravity(true);
	}
}