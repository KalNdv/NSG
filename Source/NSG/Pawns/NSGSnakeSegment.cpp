#include "NSGSnakeSegment.h"
#include "Components/StaticMeshComponent.h"

ANSGSnakeSegment::ANSGSnakeSegment()
{
	// Segments don't tick
	PrimaryActorTick.bCanEverTick = false;

	SegmentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SegmentMesh"));
	RootComponent = SegmentMesh;

	SegmentMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SegmentMesh->SetGenerateOverlapEvents(true);
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