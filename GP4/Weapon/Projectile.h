#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

USTRUCT(BlueprintType)
struct FOnProjectileCollideInfo
{
	GENERATED_BODY()
	
	UPROPERTY(Transient)
	FVector CollideLocation;
	UPROPERTY(Transient)
	FVector HitNormal;
	UPROPERTY(Transient)
	AActor* CollisionActor = nullptr;
	UPROPERTY(Transient)
	bool bHitInteractTarget = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileCollide, FOnProjectileCollideInfo, InteractInfo);

UCLASS()
class GP4_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

// Components
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	class USphereComponent* Collider;
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	UStaticMeshComponent* ProjectileMesh;

// Events
	UPROPERTY(BlueprintAssignable)
	FOnProjectileCollide OnCollide;
	
//Variables
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	float ProjectileSpeed = 1000.0f;
	UPROPERTY(EditDefaultsOnly)
	float LifeTime = 10.0f;
	UPROPERTY(Transient)
	float Timer = 0;
};
