// Fill out your copyright notice in the Description page of Project Settings.


#include "GP4Hinge.h"

#include "CableComponent.h"
#include "GP4Player.h"
#include "CharacterComponents/GP4GrapplingComp.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

AGP4Hinge::AGP4Hinge()
{
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	PhysicsConstraintComponent =  CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Physical Constraint"));
	PhysicsConstraintComponent->SetupAttachment(RootScene);

	FixMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fixed Mesh"));
	FixMesh->SetupAttachment(RootScene);
	
	MobileMesh  = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mobile Mesh"));
	MobileMesh->SetupAttachment(RootScene);

	BoxCollision  = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	BoxCollision->SetupAttachment(MobileMesh);
	
	Cable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable Component"));
	Cable->SetupAttachment(RootScene);
	
}


// Called when the game starts or when spawned
void AGP4Hinge::BeginPlay()
{
	Super::BeginPlay();
	HingeLocation = GetActorLocation();
	MobileMesh->SetPhysicsMaxAngularVelocityInDegrees(180);
}


// Called every frame
void AGP4Hinge::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
// Can not hook if Hinge is just created , At least 0.3 secs . Bypassing Engine Issues
bool AGP4Hinge::IsAvailable() const
{
	if(GetGameTimeSinceCreation() > HookRestPeriod) return  true;
	return  false;
}

void AGP4Hinge::ResetHinge()
{
	GetWorld()->GetTimerManager().ClearTimer(SwingLimitTimer);
	GetWorld()->GetTimerManager().ClearTimer(CollisionTimer);
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AGP4Hinge *Temp = GetWorld()->SpawnActor<AGP4Hinge>(NewHinge,HingeLocation, FRotator::ZeroRotator, SpawnInfo);
	Temp->CableLength = GetCableLength();
	Temp->HookDistance = GetHookDistance();
	Temp = nullptr;
	Destroy();
}

void AGP4Hinge::ChangeAngularLimit() const
{
	PhysicsConstraintComponent->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited,100);
}

float AGP4Hinge::GetPlayerRange() const
{
	AGP4Player* PlayerCharacter = Cast<AGP4Player>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if(PlayerCharacter)
	{
		return PlayerCharacter->GrapplingComp->GetPlayerRange();
	}
	return 0;
}


void AGP4Hinge::SetWingLimit()
{
	GetWorld()->GetTimerManager().SetTimer(SwingLimitTimer, this, &AGP4Hinge::ChangeAngularLimit, 0.01f, true);
}


