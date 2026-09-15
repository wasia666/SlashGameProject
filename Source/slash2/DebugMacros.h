#pragma once
#include"DrawDebugHelpers.h"

#define DRAW_SPHERE(Location) if(GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.f, 24, FColor::Red, true);
#define DRAW_SPHERE_COLOR(Location,Color) if(GetWorld()) DrawDebugSphere(GetWorld(), Location, 8.f, 24, Color, false, 5.f);
#define DRAW_SPHERE_SINGLEFRAME(Location) if(GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.f, 24, FColor::Red, false, -1.F);
#define DRAW_LINE(StartLocation, EndLocation) if(GetWorld()) DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, true, -1.f , 0 , 1.0f );
#define DRAW_LINE_SINGLEFRAME(StartLocation, EndLocation) if(GetWorld()) DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, false, -1.f , 0 , 1.0f );
#define DRAW_POINT(Location) if(GetWorld()) DrawDebugPoint(GetWorld(), Location, 15.f, FColor::Red, true);
#define DRAW_POINT_SINGLEFRAME(Location) if(GetWorld()) DrawDebugPoint(GetWorld(), Location, 15.f, FColor::Red, false, -1.f);
#define DRAW_VECTOR(StartLocation,EndLocation) if(GetWorld())\
	{\
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, true, -1.f , 0 , 1.0f );\
		DrawDebugPoint(GetWorld(), EndLocation, 15.f, FColor::Red, true);\
	}
#define DRAW_VECTOR_SINGLEFRAME(StartLocation,EndLocation) if(GetWorld())\
	{\
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, false, -1.f , 0 , 1.0f );\
		DrawDebugPoint(GetWorld(), EndLocation, 15.f, FColor::Red, false , -1.f);\
	}