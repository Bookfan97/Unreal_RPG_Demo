// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGCharacter.h"

#include "DrawDebugHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "MagicProjectile.h"
#include "RPGGameMode.h"
#include "RPGGameModeBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

//////////////////////////////////////////////////////////////////////////
// ARPGCharacter

ARPGCharacter::ARPGCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	if (isMagic)
	{
		Health = MaxHealth;
		// static ConstructorHelpers::FObjectFinder<UAnimSequence> anim(TEXT("AnimSequence'/Game/Mannequin/Animations/ThirdPersonJump_Start.ThirdPersonJump_Start'"));
   //      Anim = anim.Object;
	}

	if (isMelee)
	{
		GetMesh()->HideBoneByName(TEXT("sword_bottom"), EPhysBodyOp::PBO_None);
		// Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("weaponSocket"));
		// Gun->SetOwner(this);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARPGCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	UE_LOG(LogTemp, Warning, TEXT("Controller: "), this->GetController());
	toAttack = false;
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARPGCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARPGCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARPGCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARPGCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ARPGCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ARPGCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ARPGCharacter::OnResetVR);

	PlayerInputComponent->BindAction(TEXT("Attack"), EInputEvent::IE_Pressed, this, &ARPGCharacter::CharacterMeleeAttack);
	PlayerInputComponent->BindAction(TEXT("Block"), EInputEvent::IE_Pressed, this, &ARPGCharacter::CharacterMeleeBlock);
}

bool ARPGCharacter::IsDead() const
{
	return Health <= 0;
}

bool ARPGCharacter::IsAttacking() const
{
	return toAttack;
}

float ARPGCharacter::GetHealthPercent() const
{
	return Health / MaxHealth;
}

void ARPGCharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
}

void ARPGCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ARPGCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ARPGCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ARPGCharacter::Attack()
{
	FVector PlayerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	FRotator PlayerRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), PlayerLoc);
	bool rotation = this->SetActorRotation(PlayerRot, ETeleportType::None);
	if (isMelee)
	{
		UE_LOG(LogTemp, Warning, TEXT("This is a melee attack"));
		//CharacterMeleeAttack();
	}
	if (isMagic)
	{
		UE_LOG(LogTemp, Warning, TEXT("This is a magic attack"));
		toAttack = true;
		CharacterMagicAttack(PlayerLoc);
	}
}

void ARPGCharacter::CharacterMeleeAttack()
{
	if (AttackAnimation)
	{
		UAnimInstance* AnimInstance = this->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			//AnimInstance->PlaySlotAnimationAsDynamicMontage(AttackAnimation, "Fire Animation", 0.0f);
		}
	}
}

bool ARPGCharacter::AimTrace(FHitResult& Hit, FVector& ShotDirection)
{
	// AController* OwnerController = GetOwnerController();
	// if (OwnerController == nullptr) return false;
	FVector Location;
	FRotator Rotation;
	this->GetController()->GetPlayerViewPoint(Location, Rotation);
	ShotDirection = Rotation.Vector();
	FVector End = Location + Rotation.Vector() * MaxRange;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	return GetWorld()->LineTraceSingleByChannel(Hit, Location, End, ECollisionChannel::ECC_GameTraceChannel1, Params);
}

void ARPGCharacter::CharacterMagicAttack(const FVector& temp)
{
	TSubclassOf<UAnimInstance> test = this->GetMesh()->AnimClass->GetSuperClass();
	UE_LOG(LogTemp, Warning, TEXT("Class : "), test);
	// GetMesh()->PlayAnimation(AttackAnimation, false);
	FVector Player = temp.ForwardVector;
	if (ProjectileClass)
	{
		FHitResult Hit;
		FVector ShotDirection;
		//FVector Player = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation().ForwardVector;
		bool bSuccess = AimTrace(Hit, ShotDirection);
		FVector traceStart = this->GetActorLocation();
		//AActor player = GetWorld()->GetFirstPlayerController()->Actor;
		//FVector Player = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
		//DrawDebugLine(GetWorld(), traceStart,ShotDirection, FColor(255, 0, 0),false, 50, 0,12.333);

		// if (bSuccess)
		// {
			// UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.Location, ShotDirection.Rotation());
			// UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ImpactSound, Hit.Location, ShotDirection.Rotation());
		AActor* HitActor = Hit.GetActor();
		//HitActor->GetActorLocation();
		//IsPlayerControlled();
		if (HitActor != nullptr) // Hit actor needs to be the character
		{
			if (AttackAnimation)
			{
				UAnimInstance* AnimInstance = this->GetMesh()->GetAnimInstance();
				GetMesh()->PlayAnimation(AttackAnimation, false);
				if (AnimInstance)
				{
					//GetMesh()->PlayAnimation(AttackAnimation, false);
					//AnimInstance->PlaySlotAnimationAsDynamicMontage(AttackAnimation, "DefaultSlot", 0.0f);
				}
			}
			FPointDamageEvent DamageEvent(Damage, Hit, ShotDirection, nullptr);
			//FPointDamageEvent DamageEvent(Damage, Hit, Player, nullptr);
			AController* OwnerController = this->GetController();//->GetOwnerController();
			HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
		}
		// }
		//UAnimInstance* AnimInstance = this->GetMesh()->GetAnimInstance();
		// GetMesh()->AnimClass->//PlayAnimation(AttackAnimation, false);
		FVector HandLocation = this->GetMesh()->GetSocketLocation("neck_01");
		FRotator HandRotation = this->GetMesh()->GetSocketRotation("neck_01");
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		ActorSpawnParams.Instigator = this;
		AMagicProjectile* Projectile = GetWorld()->SpawnActor<AMagicProjectile>(ProjectileClass, HandLocation, HandRotation, ActorSpawnParams);
		// if (Projectile)
		// {
		FVector LaunchDirection = GetOwner()->GetActorForwardVector();
		Projectile->FireInDirection(LaunchDirection);
		toAttack = false;
		//}

		//GetWorld()->SpawnActor<AMagicProjectile>(ProjectileClass, Player, HandRotation, ActorSpawnParams);
	}
	//}
   // FVector ActorLocation = GetActorLocation();
   // FVector PlayerLocation =
}

void ARPGCharacter::CharacterMeleeBlock()
{
	//Handled In Blueprint currently, bring over here
}

float ARPGCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	DamageToApply = FMath::Min(Health, DamageToApply);
	Health -= DamageToApply;
	UE_LOG(LogTemp, Warning, TEXT("Health left %f"), Health);
	if (IsDead())
	{
		ARPGGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode<ARPGGameModeBase>();
		if (GameModeBase != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("pawn Killed"), Health);
			GameModeBase->PawnKilled(this);
		}
		DetachFromControllerPendingDestroy();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	return DamageToApply;
}

void ARPGCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARPGCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ARPGCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ARPGCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}