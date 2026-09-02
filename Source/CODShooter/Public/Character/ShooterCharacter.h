#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ShooterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AWeapon;
class UAnimMontage;

UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	Primary,
	Secondary,
	Melee,
	Equipment
};

UCLASS()
class CODSHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();

protected:
	// Enhanced Input System
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UEnhancedInputComponent* EnhancedInputComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	// Weapon System
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TMap<EWeaponSlot, AWeapon*> WeaponInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponSlot CurrentWeaponSlot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AWeapon* CurrentWeapon;

	// Movement Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AimWalkSpeed = 400.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsAiming;

	// Combat Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxShield = 50.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float CurrentShield;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ShieldRegenDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ShieldRegenRate = 10.0f;

	// Animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimInstance* AnimInstance;

	// Aiming offset
	FRotator AimingRotation;
	float AimAlpha = 0.0f;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// Callbacks for Enhanced Input
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Fire();
	void StopFire();
	void Aim();
	void StopAim();
	void Sprint();
	void StopSprint();
	void Reload();
	void EquipWeapon(EWeaponSlot Slot);

	// Weapon Management
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AddWeapon(AWeapon* Weapon, EWeaponSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SwitchWeapon(EWeaponSlot NewSlot);

	// Health & Damage
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeDamage(float Damage, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetShield() const { return CurrentShield; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsAlive() const { return CurrentHealth > 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Die();

	// State Getters
	UFUNCTION(BlueprintCallable, Category = "Character")
	bool IsAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	FVector GetAimDirection() const;

	UFUNCTION(BlueprintCallable, Category = "Character")
	void RegenerateShield();

private:
	FTimerHandle ShieldRegenTimerHandle;
	void UpdateAimingOffset();
};
