#include "Projectile.h"
#include "Components/SphereComponent.h"
#include "GP4/Components/InteractionComponent.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
	RootComponent = Collider;
	Collider->SetCollisionProfileName("BlockAll");

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionProfileName("NoCollision");
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult Hit;
	AddActorWorldOffset(GetActorForwardVector() * ProjectileSpeed * DeltaTime, true, &Hit);

	if(Hit.bBlockingHit)
	{
		FOnProjectileCollideInfo CollideInfo;
		CollideInfo.CollideLocation = Hit.ImpactPoint;
		CollideInfo.HitNormal = Hit.ImpactNormal;
		
		TArray<UInteractionComponent*> InteractionComponents;
		CollideInfo.CollisionActor = Hit.Actor.Get();

		if(Hit.Actor != nullptr)
		{
			Hit.Actor->GetComponents(InteractionComponents);

			for (const UInteractionComponent* InteractionComponent : InteractionComponents)
			{
				CollideInfo.bHitInteractTarget = true;

				FOnInteractInfo const InteractInfo;
				InteractionComponent->OnInteract.Broadcast(InteractInfo);
			}
		}

		OnCollide.Broadcast(CollideInfo);
		Destroy();
	}

	if(Timer >= LifeTime)
	{
		Destroy();
	}

	Timer += DeltaTime;
}
