#include "NSGSnakeSegment.h"
#include "Components/StaticMeshComponent.h"

ANSGSnakeSegment::ANSGSnakeSegment()
{
	// Segments don't tick
	PrimaryActorTick.bCanEverTick = false;

	SegmentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SegmentMesh"));
	RootComponent = SegmentMesh;
	SegmentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
}

void ANSGSnakeSegment::UpdateVisuals(bool bIsLastSegment)
{
	if (bIsLastSegment && TailMesh)
	{
		SegmentMesh->SetStaticMesh(TailMesh);
	}
	else if (!bIsLastSegment && BodyMesh)
	{
		SegmentMesh->SetStaticMesh(BodyMesh);
	}
}