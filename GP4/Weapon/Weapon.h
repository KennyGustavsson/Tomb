#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFiredEvent);

UCLASS()
class GP4_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void BeginPlay() override;

// Components
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* WeaponMesh;
	UPROPERTY(EditDefaultsOnly, Category=Weapon)
	TSubclassOf<AProjectile> ProjectileActor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Weapon)
	class UArrowComponent* BulletStartLocation;

// Functions
	UFUNCTION()
	void Fire();
	UFUNCTION()
	void SetPlayer(AActor* OwnerPlayer);

// Events
	UPROPERTY(BlueprintAssignable)
	FOnFiredEvent OnFiredEvent;
	
// Variables
	UPROPERTY(EditDefaultsOnly, Category=Weapon)
	float RayLength = 10000.0f;
	UPROPERTY(Transient)
	class AGP4Player* Player;
	
};
