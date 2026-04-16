#include "NSGPellet.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../Pawns/NSGSnakeBase.h"
// ^ included so we can check if snake is touched


// Sets default values
ANSGPellet::ANSGPellet()
{
	// Save performance, pellets do not tick
	PrimaryActorTick.bCanEverTick = false;

	// Collision! A sphere in this case
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(40.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // Overlapping instead of blocking
	RootComponent = CollisionComponent;

	// Pellet mesh
	PelletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PelletMesh"));
	PelletMesh->SetupAttachment(RootComponent);
	PelletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // The mesh is just for looks, the sphere handles the math

	// Overlap function bound to overlap
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ANSGPellet::OnOverlapBegin);
}

// Called when the game starts or when spawned
void ANSGPellet::BeginPlay()
{
	Super::BeginPlay();
}

// This runs when the collision sphere is touched, previously mentioned on touch function
void ANSGPellet::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if touched thing is not snake itself
	if (OtherActor && (OtherActor != this))
	{
		// Is snake? If no, bad. 
		ANSGSnakeBase* Snake = Cast<ANSGSnakeBase>(OtherActor);

		if (Snake)
		{
			// Snake ate pellet
			// UE_LOG(LogTemp, Warning, TEXT("Snake ate a pellet"));

			// Snake growth function
			Snake->AddTailSegment();
			
			// DESTROY!!!
			Destroy();
		}
	}
}