// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "Gun.h"
#include "SimpleShooterGameMode.h"
#include "Components/CapsuleComponent.h"
// #include "ShooterAIController.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	Health = MaxHealth;

	if(!ensure(!GunClassArray.IsEmpty())) return;
	
	for(TSubclassOf<AGun> GunClass : GunClassArray)
	{
		Gun = GetWorld()->SpawnActor<AGun>(GunClass);
		GetMesh()->HideBoneByName(TEXT("weapon_r"), EPhysBodyOp::PBO_None);
		Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponSocket"));
		Gun->SetOwner(this);
		Gun->SetActorHiddenInGame(true);
		GunArray.Add(Gun);
	}
	
	GunArray[WeaponActiveIndex]->SetActorHiddenInGame(false);
	// AIController = CreateDefaultSubobject<AShooterAIController>(TEXT("AIController"));
}

bool AShooterCharacter::IsDead() const
{
	// 체력이 0보다 작으면 true를 반환.
	return Health <= 0;
}

float AShooterCharacter::GetHealthPercent() const
{
	return Health / MaxHealth;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward/Backwards"), this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight/Left"), this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("UE_LOGLookUp/Down"), this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(TEXT("LookLeft/Right"), this, &APawn::AddControllerYawInput);
	// PlayerInputComponent->BindAxis(TEXT("LookUpRate"), this, &AShooterCharacter::LookUpRate);
	// PlayerInputComponent->BindAxis(TEXT("LookRightRate"), this, &AShooterCharacter::LookRightRate);

	PlayerInputComponent->BindAction(TEXT("JumpAction"), EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Shoot"), EInputEvent::IE_Pressed, this, &AShooterCharacter::Shoot);
	PlayerInputComponent->BindAction(TEXT("DrawWeapon1"), EInputEvent::IE_Pressed, this, &AShooterCharacter::ChangeWeapon1);
	PlayerInputComponent->BindAction(TEXT("DrawWeapon2"), EInputEvent::IE_Pressed, this, &AShooterCharacter::ChangeWeapon2);
}

float AShooterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	DamageToApplied = FMath::Min(Health, DamageToApplied);
	Health -= DamageToApplied;
	
	UE_LOG(LogTemp, Warning, TEXT("Health Left : %f"), Health);

	if(IsDead())
	{
		ASimpleShooterGameMode *GameMode = GetWorld()->GetAuthGameMode<ASimpleShooterGameMode>();
		if(GameMode != nullptr)
		{
			GameMode->PawnKilled(this);
		}
		DetachFromControllerPendingDestroy();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	return DamageToApplied;
}

void AShooterCharacter::MoveForward(float AxisValue)
{
	AddMovementInput(GetActorForwardVector() * AxisValue);
}

void AShooterCharacter::MoveRight(float AxisValue)
{
	AddMovementInput(GetActorRightVector() * AxisValue);
}

// 왜 AddMovementInput으로 구현하지 않는 거임?
// AddMovementInput의 경우에는 우리가 입력을 넣으면 이미 그게 속도라는 걸 가정하기 때문임
// 실제 이동한 양이 아님
void AShooterCharacter::LookUpRate(float AxisValue)
{
	AddControllerPitchInput(AxisValue * RotationRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookRightRate(float AxisValue)
{
	AddControllerYawInput(AxisValue * RotationRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Shoot()
{
	GunArray[WeaponActiveIndex]->PullTrigger();
	UE_LOG(LogTemp, Warning, TEXT("You use the Weapon : %s!"), *GunArray[WeaponActiveIndex]->GetName());
}

void AShooterCharacter::ChangeWeapon1()
{
	WeaponActiveIndex = 0;
	ChangeWeapon();
}

void AShooterCharacter::ChangeWeapon2()
{
	WeaponActiveIndex = 1;
	ChangeWeapon();
}

void AShooterCharacter::ChangeWeapon()
{
	for(int32 WeaponIndex = 0 ; WeaponIndex < GunArray.Num(); WeaponIndex++)
	{
		if(WeaponIndex == WeaponActiveIndex)
		{
			GunArray[WeaponIndex]->SetActorHiddenInGame(false);
		}
		else
		{
			GunArray[WeaponIndex]->SetActorHiddenInGame(true);
		}
	}
}