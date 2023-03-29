#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

PhobosMap<LineTrail*, CoordStruct> LineTrailExt::LineTrailMap;

bool LineTrailExt::DeallocateLineTrail(TechnoClass* pTech)
{
	auto const pTechExt = TechnoExt::ExtMap.Find(pTech);
	{
		if (pTechExt->TechnoLineTrail.Count > 0)
		{
			for (auto& pItems : pTechExt->TechnoLineTrail)
			{
				if (pItems)
				{
					pItems->~LineTrail();
					pItems = nullptr;
				}
			}

			return true;
		}
	}

	return false;
}

bool LineTrailExt::DeallocateLineTrail(BulletClass* pBullet)
{
	auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
	{
		if (pBulletExt->BulletTrails.Count > 0)
		{
			for (auto& pItems : pBulletExt->BulletTrails)
			{
				if (pItems)
				{
					pItems->~LineTrail();
					pItems = nullptr;
				}
			}
			return true;
		}
	}

	return false;
}

bool LineTrailExt::DeallocateLineTrail(ObjectClass* pObject)
{
	switch ((((DWORD*)pObject)[0]))
	{
	case InfantryClass::vtable:
	case AircraftClass::vtable:
	case UnitClass::vtable:
		return LineTrailExt::DeallocateLineTrail(static_cast<FootClass*>(pObject));
	case BulletClass::vtable:
		return LineTrailExt::DeallocateLineTrail(static_cast<BulletClass*>(pObject));
	}

	return false;
}

void LineTrailExt::DetachLineTrails(ObjectClass* pThis)
{
	switch ((((DWORD*)pThis)[0]))
	{
	case InfantryClass::vtable:
	case AircraftClass::vtable:
	case UnitClass::vtable:
		LineTrailExt::DeallocateLineTrail(static_cast<FootClass*>(pThis));
	case BulletClass::vtable:
			LineTrailExt::DeallocateLineTrail(static_cast<BulletClass*>(pThis));
	}

	if (pThis->LineTrailer)
	{
		pThis->LineTrailer->~LineTrail();
		pThis->LineTrailer = nullptr;
	}
}

void LineTrailExt::ConstructLineTrails(TechnoClass* pTech)
{
	{
		auto const pTechExt = TechnoExt::ExtMap.Find(pTech);
		auto const pType = pTech->GetTechnoType();
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (!pTypeExt->LineTrailData.empty())
		{
			for (auto& pTraildata : pTypeExt->LineTrailData)
			{
				LineTrailExt::Construct(pTechExt->TechnoLineTrail, pTech, pTraildata.LineTrailColor, pTraildata.LineTrailColorDecrement, pTraildata.LineTrailFLH);
			}
		}
	}
}

void LineTrailExt::ConstructLineTrails(BulletClass* pBullet)
{
	{
		auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
		auto const pType = pBullet->Type;
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pType);

		if (!pTypeExt->LineTrailData.empty())
		{
			for (auto& pTraildata : pTypeExt->LineTrailData)
			{
				LineTrailExt::Construct(pBulletExt->BulletTrails, pBullet, pTraildata.LineTrailColor, pTraildata.LineTrailColorDecrement, pTraildata.LineTrailFLH);
			}
		}
	}
}

DEFINE_HOOK(0x5F515D, ObjectClass_UnLimbo_ConstructLineTrail, 0x6)
{
	enum { ConstructVanilla = 0x5F5163, EndFunc = 0x5F5210 };

	if (R->CL())
	{
		//GET(ObjectClass* const, pThis, ESI);

		//if (auto pFoot = abstract_cast<FootClass*>(pThis))
		//	LineTrailExt::ConstructLineTrails(pFoot);
		//else
		//	if (auto pBullet = specific_cast<BulletClass*>(pThis))
		//		LineTrailExt::ConstructLineTrails(pBullet);


		return ConstructVanilla;
	}

	return EndFunc;
}

DEFINE_HOOK(0x556B39, LineTrail_DTOR, 0x6)
{
	GET(LineTrail*, pThis, ECX);
	GET(ObjectClass*, pObject, EAX);
	LineTrailExt::RemoveLineTrail(pThis);
	pObject->LineTrailer = nullptr;
	pThis->Owner = nullptr;

	return 0x556B42;
}

DEFINE_HOOK(0x5F5284, ObjectClass_Detach_RemoveLineTrail, 0x6)
{
	GET(ObjectClass* const, pThis, ESI);

	LineTrailExt::DetachLineTrails(pThis);

	return 0x5F529D;
}

DEFINE_HOOK(0x5F3D93, ObjectClass_Destroy_LineTrail, 0x6)
{
	GET(ObjectClass*, pThis, ESI);

	LineTrailExt::DetachLineTrails(pThis);

	return 0x5F3DAC;
}

DEFINE_HOOK(0x556ADA, LineTrailClass_Destroy_Check, 0x6)
{
	GET(LineTrail*, pThis, ECX);
	GET(ObjectClass*, pObject, EAX);
	LineTrailExt::RemoveLineTrail(pThis);
	pObject->LineTrailer = nullptr;
	pThis->Owner = nullptr;

	return 0x556AE3;
}

DEFINE_HOOK(0x556B7F, LineTrail_556B70_DrawCoords, 0x5)
{
	GET(LineTrail*, pThis, ECX);
	GET(CoordStruct const*, pCoord, EAX);

	auto nCoord = *pCoord;
	auto nOutput = LineTrailExt::GetCoord(pThis);
	nCoord += nOutput;

	R->EDX(nCoord.X);
	R->ESI(nCoord.Y);
	R->EDI(nCoord.Z);

	return 0x556B87;
}
