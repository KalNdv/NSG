#include "NSGSnakeSegment.h"
#include "Components/StaticMeshComponent.h"

ANSGSnakeSegment::ANSGSnakeSegment()
{
	// Segments don't tick
	PrimaryActorTick.bCanEverTick = false;

	SegmentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SegmentMesh"));
	RootComponent = SegmentMesh;
	SegmentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Stop segments from bumping into each other
}