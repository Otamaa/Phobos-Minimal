#include "Body.h"

#include <Ext/Tactical/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

void __fastcall Draw_Radial_Indicator(bool draw_line, bool adjust_color, CoordStruct coord, ColorStruct rgb, float line_mult, bool a8, bool a9)
{
	JMP_STD(0x456980);
}

void __fastcall FakeObjectClass::_DrawRadialIndicator(ObjectClass* pThis, discard_t, int val)
{
	if (auto pTechno = flag_cast_to<TechnoClass*, false>(pThis))
	{
		auto pType = GET_TECHNOTYPE(pTechno);
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (pType->HasRadialIndicator && pTypeExt->AlwayDrawRadialIndicator.Get(!pTechno->Deactivated))
		{
			if (HouseClass::IsCurrentPlayerObserver()
				|| pTechno->Owner->ControlledByCurrentPlayer())
			{
				int nRadius = 0;

				if (pTypeExt->RadialIndicatorRadius.isset())
					nRadius = pTypeExt->RadialIndicatorRadius.Get();
				else if (pType->GapGenerator)
					nRadius = pTypeExt->GapRadiusInCells.Get();
				else
				{
					const auto pWeapons = pTechno->GetPrimaryWeapon();
					if (!pWeapons || !pWeapons->WeaponType)
						return;

					const int range_ = WeaponTypeExtData::GetRangeWithModifiers(pWeapons->WeaponType, pTechno);
					if (range_ <= 0)
						return;

					nRadius = range_ / Unsorted::LeptonsPerCell;;
				}

				if (nRadius > 0)
				{
					const auto Color = pTypeExt->RadialIndicatorColor.Get(pTechno->Owner->Color);

					if (Color != ColorStruct::Empty)
					{
						auto nCoord = pTechno->GetCoords();
						FakeTacticalClass::__DrawRadialIndicator(false, true, nCoord, Color, (nRadius * 1.0f), false, true);
					}
				}
			}
		}
	}
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5DA0, FakeObjectClass::_DrawRadialIndicator)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB188, FakeObjectClass::_DrawRadialIndicator)

FORCEDINLINE COMPILETIMEEVAL int cell_Distance_Squared(CoordStruct& our_coord, CoordStruct& their_coord)
{
	const int64_t dx = int64_t(our_coord.X) - int64_t(their_coord.X);
	const int64_t dy = int64_t(our_coord.Y) - int64_t(their_coord.Y);
	const int64_t d2 = dx * dx + dy * dy;

	return d2 > INT_MAX ? INT_MAX : int(d2);
}

int __fastcall FakeObjectClass::_GetDistanceOfObj(ObjectClass* pThis, discard_t, AbstractClass* pThat)
{
	int nResult = 0;
	if (pThat)
	{
		auto nThisCoord = pThis->GetCoords();
		auto nThatCoord = pThat->GetCoords();
		nResult = //(int)nThisCoord.DistanceFromXY(nThatCoord)
			cell_Distance_Squared(nThisCoord, nThatCoord);
		;
	}

	return nResult;
}

int __fastcall FakeObjectClass::_GetDistanceOfCoord(ObjectClass* pThis, discard_t, CoordStruct* pThat)
{
	auto nThisCoord = pThis->GetCoords();
	return cell_Distance_Squared(nThisCoord, *pThat);
}

CellClass* __fastcall FakeObjectClass::_GetCell(ObjectClass* pThis, discard_t)
{
	return MapClass::Instance->GetCellAt(pThis->Location);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x5F6500, FakeObjectClass::_GetDistanceOfObj);
DEFINE_FUNCTION_JUMP(CALL, 0x6EB2DC, FakeObjectClass::_GetDistanceOfObj);
DEFINE_FUNCTION_JUMP(CALL, 0x4DEFF4, FakeObjectClass::_GetDistanceOfObj);

DEFINE_FUNCTION_JUMP(LJMP, 0x5F6560, FakeObjectClass::_GetDistanceOfCoord);
DEFINE_FUNCTION_JUMP(CALL, 0x6EABCB, FakeObjectClass::_GetDistanceOfCoord);
DEFINE_FUNCTION_JUMP(CALL, 0x6EAC96, FakeObjectClass::_GetDistanceOfCoord);
DEFINE_FUNCTION_JUMP(CALL, 0x6EAD4B, FakeObjectClass::_GetDistanceOfCoord);
DEFINE_FUNCTION_JUMP(CALL, 0x741801, FakeObjectClass::_GetDistanceOfCoord);