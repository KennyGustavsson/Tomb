#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "GP4/GP4Player.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	BulletStartLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("BulletStart"));
	BulletStartLocation->SetupAttachment(WeaponMesh);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AWeapon::Fire()
{
	if(!ProjectileActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Projectile is missing in weapon actor"));
		return;
	}

	if(!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("Couldn't find player"));
		return;
	}

	FActorSpawnParameters const SpawnInfo;
	AProjectile* const Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileActor->GetDefaultObject()->GetClass(), BulletStartLocation->GetComponentLocation(), BulletStartLocation->GetComponentRotation(), SpawnInfo);

	
	/*
	FTransform CameraTrans = Player->CameraComp->GetComponentTransform();

	FVector Start = CameraTrans.GetLocation() + (CameraTrans.GetRotation().GetForwardVector() * Player->HitBox->GetScaledCapsuleRadius());
	FVector End = Start + (CameraTrans.GetRotation().GetForwardVector() * RayLength);

	FHitResult Hit;
	
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility);

	FVector ProjectileRotation;
	
	if(Hit.bBlockingHit)
	{
		ProjectileRotation = (Hit.Location - BulletStartLocation->GetComponentLocation());
		ProjectileRotation.Normalize();
	}
	else
	{
		ProjectileRotation = (End - BulletStartLocation->GetComponentLocation());
		ProjectileRotation.Normalize();
	}

	FActorSpawnParameters const SpawnInfo;
	AProjectile* const Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileActor->GetDefaultObject()->GetClass(), BulletStartLocation->GetComponentLocation(), ProjectileRotation.Rotation(), SpawnInfo);
	*/
	OnFiredEvent.Broadcast();
}

void AWeapon::SetPlayer(AActor* OwnerPlayer)
{
	Player = Cast<AGP4Player>(OwnerPlayer);
}
