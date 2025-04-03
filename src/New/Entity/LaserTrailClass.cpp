#include "LaserTrailClass.h"

#include <Utilities/TemplateDef.h>

// Draws LaserTrail if the conditions are suitable.
// Returns true if drawn, false otherwise.
bool LaserTrailClass::Update(CoordStruct const& location)
{
	bool result = false;

	if (!this->LastLocation.has_value() || !this->LastLocation.get().IsValid())
	{
		// The trail was just inited
		this->LastLocation = location;
	}
	else if (location.DistanceFrom(this->LastLocation.get()) > this->Type->SegmentLength) // TODO reimplement IgnoreVertical properly?
	{
		if (LaserTrailClass::AllowDraw(location))
		{
			// We spawn new laser segment if the distance is long enough, the game will do the rest - Kerbiter
			LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(
				this->LastLocation.get(), location,
				this->CurrentColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 },
				this->Type->FadeDuration.Get());

			pLaser->Thickness = this->Type->Thickness;
			pLaser->IsHouseColor = true;
			pLaser->IsSupported = this->Type->IsIntense;

			result = true;
		}

		this->LastLocation = location;
	}

	return result;
}

void LaserTrailClass::FixZLoc(bool forWho)
{
	if (forWho && LastLocation.has_value())
	{
		CoordStruct nLastLoc = LastLocation.get();
		nLastLoc.Z = MapClass::Instance->GetCellFloorHeight(nLastLoc);
		LastLocation = nLastLoc;
	}
}

#pragma region Save/Load

template <typename T>
bool LaserTrailClass::Serialize(T& stm)
{
	//Debug::LogInfo("Processing Element From LaserTrailClass ! ");

	return stm
		.Process(this->Type)
		.Process(this->Visible)
		.Process(this->FLH)
		.Process(this->IsOnTurret)
		.Process(this->CurrentColor)
		.Process(this->LastLocation)
		.Process(this->InitialDelayTimer)
		.Process(this->CanDraw)
		.Process(this->Cloaked)
		.Process(this->InitialDelay)
		.Success();
};

bool LaserTrailClass::Load(PhobosStreamReader& stm, bool RegisterForChange)
{
	return Serialize(stm);
}

bool LaserTrailClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<LaserTrailClass*>(this)->Serialize(stm);
}

#pragma endregion
