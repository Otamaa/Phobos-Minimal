#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

// AbstractClass_Distance2DSquared_1
DEFINE_PATCH(0x5F6515, 0x8B, 0x01, 0x56, 0x8D, 0x54, 0x24, 0x08 , 0x57);

DEFINE_HOOK(0x5F6515 , AbstractClass_Distance2DSquared_1, 0x8)
{
	GET(AbstractClass*, pThis, ECX);
	GET(AbstractClass*, pThat, EBX);

	auto const nThisCoord = pThis->GetCoords();
	auto const nThatCoord = pThat->GetCoords();
	auto const nXY = 
	((nThisCoord.Y - nThatCoord.Y) * (nThisCoord.Y - nThatCoord.Y)) +
	((nThisCoord.X - nThatCoord.X) * (nThisCoord.X - nThatCoord.X));

	R->EAX(nXY >= INT_MAX ? INT_MAX : nXY);
	return 0x5F6559;
}