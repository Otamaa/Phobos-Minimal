#include "TestLocomotionClass.h"

#include <CellSpread.h>
#include <ScenarioClass.h>

#include <ParticleSystemClass.h>
#include <ParticleSystemTypeClass.h>

#include <AnimClass.h>
#include <AircraftClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <cmath>
#include <Utilities/EnumFunctions.h>

HRESULT __stdcall TestLocomotionClass::Load(IStream* pStm)
{
	// This loads the whole object
	HRESULT hr = LocomotionClass::Internal_Load(this, pStm);
	if (FAILED(hr))
		return hr;

	// clean up the vtable
	new (this) TestLocomotionClass(noinit_t());

	// Piggybacker handling
	bool piggybackerPresent;
	hr = pStm->Read(&piggybackerPresent, sizeof(piggybackerPresent), nullptr);
	if (!piggybackerPresent)
		return hr;

	const auto ID = __uuidof(ILocomotion);
	hr = Imports::OleLoadFromStream.get()(pStm, &ID, reinterpret_cast<LPVOID*>(&this->Piggybacker));

	//if (SUCCEEDED(hr))
	//{
	//	const auto pLoco = static_cast<LocomotionClass*>(this->Piggybacker.GetInterfacePtr());
	//	const auto Who = EnumFunctions::locomotion_toSring(pLoco);
	//	Debug::Log("Attahed To [%s - {%s}] ref[%d]\n", Who->first , Who->second, pLoco->RefCount);
	//}

	return hr;
}

HRESULT __stdcall TestLocomotionClass::Save(IStream* pStm, BOOL fClearDirty)
{
	// This saves the whole object
	HRESULT hr = LocomotionClass::Internal_Save(this, pStm, fClearDirty);
	if (FAILED(hr))
		return hr;

	// Piggybacker handling
	bool piggybackerPresent = this->Piggybacker != nullptr;
	hr = pStm->Write(&piggybackerPresent, sizeof(piggybackerPresent), nullptr);

	if (!piggybackerPresent)
		return hr;

	IPersistStreamPtr piggyPersist(this->Piggybacker);
	return Imports::OleSaveToStream.get()(piggyPersist, pStm);
}

bool TestLocomotionClass::Is_Moving()
{
	return IsMoving;
}

CoordStruct TestLocomotionClass::Destination()
{
	if (IsMoving)
	{
		return DestinationCoord;
	}

	return CoordStruct::Empty;
}

CoordStruct TestLocomotionClass::Head_To_Coord()
{
	if (IsMoving)
		return HeadToCoord;

	return LinkedTo->GetCenterCoords();
}

Move TestLocomotionClass::Can_Enter_Cell(CellStruct cell)
{
	return LinkedTo->IsCellOccupied(MapClass::Instance->GetCellAt(cell), FacingType::None, -1, nullptr, false);
}

bool TestLocomotionClass::Process()
{
	if (IsMoving)
	{
		CoordStruct coord = DestinationCoord;

		/**
		 *  Rotate the object around the center coord.
		 */
		int radius = Unsorted::LeptonsPerCell * 2;
		coord.X += int(radius * std::sin(Angle));
		coord.Y += int(radius * std::cos(Angle));
		//coord.Z // No need to adjust the height of the object.

		// Pickup the object the game world before we set the new coord.
		LinkedTo->UpdatePlacement(PlacementType::Remove);

		if (Can_Enter_Cell(CellClass::Coord2Cell(coord)) == Move::OK)
		{
			LinkedTo->SetLocation(coord);

			// Increase the angle, wrapping if full circle is complete.
			double scale = 360.0;
			Angle += Math::deg2rad(360.0) / scale;
			if (Angle > 360.0)
				Angle -= 360.0;
		}

		LinkedTo->UpdatePlacement(PlacementType::Put);
	}

	return Is_Moving();
}

void TestLocomotionClass::Move_To(CoordStruct to)
{
	DestinationCoord = to;

	IsMoving = HeadToCoord != CoordStruct::Empty
		|| DestinationCoord != CoordStruct::Empty;
}

void TestLocomotionClass::Stop_Moving()
{
	HeadToCoord = CoordStruct::Empty;
	DestinationCoord = CoordStruct::Empty;

	Angle = 0;

	IsMoving = false;
}

void TestLocomotionClass::Do_Turn(DirStruct coord)
{
	LinkedTo->PrimaryFacing.Set_Current(coord);
}

// void TestLocomotionClass::Unlimbo()
// {
// 	Force_New_Slope(LinkedTo->Get_Cell_Ptr()->Ramp);
// }

void TestLocomotionClass::Force_Immediate_Destination(CoordStruct coord)
{
	DestinationCoord = coord;
}

bool TestLocomotionClass::Is_Moving_Now()
{
	if (LinkedTo->PrimaryFacing.Is_Rotating())
		return true;

	if (Is_Moving())
		return HeadToCoord != CoordStruct::Empty && Apparent_Speed() > 0;

	return false;
}

void TestLocomotionClass::Mark_All_Occupation_Bits(int mark)
{
	CoordStruct headTo = Head_To_Coord();
	if (mark != 0)
		LinkedTo->MarkAllOccupationBits(headTo);
	else
		LinkedTo->UnmarkAllOccupationBits(headTo);
}

bool TestLocomotionClass::Is_Moving_Here(CoordStruct to)
{
	CoordStruct headTo = Head_To_Coord();
	return CellClass::Coord2Cell(headTo) == CellClass::Coord2Cell(to)
		&& std::abs(headTo.Z - to.Z) <= Unsorted::CellHeight;
}

bool TestLocomotionClass::Is_Really_Moving_Now()
{
	return IsMoving;
}

void TestLocomotionClass::Clear_Coords()
{
	TestLocomotionClass::Stop_Moving();
}


HRESULT TestLocomotionClass::Begin_Piggyback(ILocomotion* pointer)
{
	if (!pointer)
		return E_POINTER;

	if (this->Piggybacker)
		return E_FAIL;

	this->Piggybacker = pointer;

	return S_OK;
}

HRESULT TestLocomotionClass::End_Piggyback(ILocomotion** pointer)
{
	if (!pointer)
		return E_POINTER;

	if (!this->Piggybacker)
		return S_FALSE;

	*pointer = this->Piggybacker.Detach();

	return 0;
}

bool TestLocomotionClass::Is_Ok_To_End()
{
	// determines when to end piggybacking automatically
	// we don't want to end piggybacking automatically for this loco
	return false; // this->Piggybacker && !this->LinkedTo->IsAttackedByLocomotor;
}

HRESULT TestLocomotionClass::Piggyback_CLSID(GUID* classid)
{
	HRESULT hr;

	if (classid == nullptr)
		return E_POINTER;

	if (this->Piggybacker)
	{
		IPersistStreamPtr piggyAsPersist(this->Piggybacker);

		hr = piggyAsPersist->GetClassID(classid);
	}
	else
	{
		if (reinterpret_cast<IPiggyback*>(this) == nullptr)
			return E_FAIL;

		IPersistStreamPtr thisAsPersist(this);

		if (thisAsPersist == nullptr)
			return E_FAIL;

		hr = thisAsPersist->GetClassID(classid);
	}

	return hr;
}

bool TestLocomotionClass::Is_Piggybacking()
{
	return this->Piggybacker != nullptr;
}