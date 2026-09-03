#include "Character/ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/AnimInstance.h"

AShooterCharacter::AShooterCharacter()
{
	// Don't rotate character with camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 600.0f;

	// Create camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Initialize movement states
	bIsSprinting = false;
	bIsAiming = false;

	// Initialize health and shield
	CurrentHealth = MaxHealth;
	CurrentShield = MaxShield;

	PrimaryActorTick.TickInterval = 0.0f;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Get animation instance
	AnimInstance = GetMesh()->GetAnimInstance();
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update aiming offset
	UpdateAimingOffset();
}

void AShooterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Move);
		}

		// Looking
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Look);
		}

		// Jumping
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		// Firing
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Fire);
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopFire);
		}

		// Aiming
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Aim);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopAim);
		}

		// Sprinting
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Sprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopSprint);
		}

		// Reloading
		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Reload);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AShooterCharacter::SetupPlayerInputComponent: EnhancedInputComponent not found!"));
	}
}

void AShooterCharacter::Move(const FInputActionValue& Value)
{
	// Input is a Value type. It stores 2D vector
	if (Value.GetMagnitude() != 0.0f)
	{
		const FVector2D MovementVector = Value.Get<FVector2D>();

		// Find forward direction
		if (Controller != nullptr)
		{
			// Add movement in forward direction
			if (MovementVector.Y != 0.f)
			{
				// Find forward direction
				const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				const FVector Direction = FRotmat(YawRotation).GetUnitAxis(EAxis::X);
				AddMovementInput(Direction, MovementVector.Y);
			}

			// Add movement in right direction
			if (MovementVector.X != 0.f)
			{
				// Find right direction
				const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				const FVector Direction = FRotmat(YawRotation).GetUnitAxis(EAxis::Y);
				AddMovementInput(Direction, MovementVector.X);
			}
		}
	}
}

void AShooterCharacter::Look(const FInputActionValue& Value)
{
	// Input is a Value type. It stores 2D vector
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AShooterCharacter::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("Fire action triggered"));
	// TODO: Implement firing logic
}

void AShooterCharacter::StopFire()
{
	UE_LOG(LogTemp, Warning, TEXT("Fire action stopped"));
	// TODO: Implement stop firing logic
}

void AShooterCharacter::Aim()
{
	if (CurrentWeapon)
	{
		bIsAiming = true;
		GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed;
		UE_LOG(LogTemp, Warning, TEXT("Aiming started"));
	}
}

void AShooterCharacter::StopAim()
{
	bIsAiming = false;
	if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
	UE_LOG(LogTemp, Warning, TEXT("Aiming stopped"));
}

void AShooterCharacter::Sprint()
{
	if (!bIsAiming)
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		UE_LOG(LogTemp, Warning, TEXT("Sprinting started"));
	}
}

void AShooterCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	UE_LOG(LogTemp, Warning, TEXT("Sprinting stopped"));
}

void AShooterCharacter::Reload()
{
	if (CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reload triggered"));
		// TODO: Implement reload logic
	}
}

void AShooterCharacter::AddWeapon(AWeapon* Weapon, EWeaponSlot Slot)
{
	if (Weapon)
	{
		WeaponInventory.Add(Slot, Weapon);
	}
}

void AShooterCharacter::SwitchWeapon(EWeaponSlot NewSlot)
{
	if (WeaponInventory.Contains(NewSlot))
	{
		CurrentWeapon = WeaponInventory[NewSlot];
		CurrentWeaponSlot = NewSlot;
	}
}

void AShooterCharacter::EquipWeapon(EWeaponSlot Slot)
{
	SwitchWeapon(Slot);
}

void AShooterCharacter::TakeDamage(float Damage, AActor* DamageCauser)
{
	float RemainingDamage = Damage;

	// Damage shield first
	if (CurrentShield > 0.0f)
	{
		if (CurrentShield >= RemainingDamage)
		{
			CurrentShield -= RemainingDamage;
			RemainingDamage = 0.0f;
		}
		else
		{
			RemainingDamage -= CurrentShield;
			CurrentShield = 0.0f;
		}
	}

	// Remaining damage goes to health
	if (RemainingDamage > 0.0f)
	{
		CurrentHealth -= RemainingDamage;
		if (CurrentHealth <= 0.0f)
		{
			Die();
		}
	}
}

void AShooterCharacter::Die()
{
	CurrentHealth = 0.0f;
	UE_LOG(LogTemp, Warning, TEXT("Character died"));
	// TODO: Implement death logic
}

FVector AShooterCharacter::GetAimDirection() const
{
	if (Controller)
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
		return CameraRotation.Vector();
	}
	return GetActorForwardVector();
}

void AShooterCharacter::UpdateAimingOffset()
{
	if (AnimInstance)
	{
		// Update aiming animation blend space
		if (bIsAiming)
		{
			AimAlpha = FMath::FInterpTo(AimAlpha, 1.0f, GetWorld()->DeltaTimeSeconds, 5.0f);
		}
		else
		{
			AimAlpha = FMath::FInterpTo(AimAlpha, 0.0f, GetWorld()->DeltaTimeSeconds, 5.0f);
		}

		// Set animation values
		AnimInstance->Montage_IsPlaying(nullptr); // Placeholder for animation updates
	}
}

void AShooterCharacter::RegenerateShield()
{
	if (CurrentShield < MaxShield)
	{
		CurrentShield = FMath::Min(CurrentShield + ShieldRegenRate, MaxShield);
	}
}
