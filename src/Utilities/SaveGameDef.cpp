#include "SavegameDef.h"
#include <Utilities/GameUniquePointers.h>

std::unordered_map<void*, std::weak_ptr<void>> SavegameGlobal::GlobalSharedRegistry;

bool Savegame::PhobosStreamObject<CellStruct>::ReadFromStream(PhobosStreamReader& stm, CellStruct& value, bool register_for_change) const
{
	if (!Savegame::ReadPhobosStream(stm, value.X, register_for_change))
		return false;

	if (!Savegame::ReadPhobosStream(stm, value.Y, register_for_change))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<CellStruct>::WriteToStream(PhobosStreamWriter& stm, const CellStruct& value) const
{
	if (!Savegame::WritePhobosStream(stm, value.X))
		return false;

	if (!Savegame::WritePhobosStream(stm, value.Y))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<Leptons>::ReadFromStream(PhobosStreamReader& stm, Leptons& value, bool register_for_change) const
{
	return Savegame::ReadPhobosStream(stm, value.value, register_for_change);
}

bool Savegame::PhobosStreamObject<Leptons>::WriteToStream(PhobosStreamWriter& stm, const Leptons& value) const
{
	return Savegame::WritePhobosStream(stm, value.value);
}

bool Savegame::PhobosStreamObject<DirStruct>::ReadFromStream(PhobosStreamReader& stm, DirStruct& value, bool register_for_change) const
{
	int tmp_int = 0;
	if (!Savegame::ReadPhobosStream(stm, tmp_int))
		return false;

	value.Raw = static_cast<unsigned short>(tmp_int);
	// Note: Pad is not serialized as it appears to be just alignment padding
	return stm.RegisterChange(&value);
}

bool Savegame::PhobosStreamObject<DirStruct>::WriteToStream(PhobosStreamWriter& stm, const DirStruct& value) const
{
	if (!Savegame::WritePhobosStream(stm, static_cast<int>(value.Raw)))
		return false;

	// Note: Pad is not serialized as it appears to be just alignment padding
	return stm.RegisterChange(&value);
}

bool Savegame::PhobosStreamObject<CoordStruct>::ReadFromStream(PhobosStreamReader& stm, CoordStruct& value, bool register_for_change) const
{
	if (!Savegame::ReadPhobosStream(stm, value.X))
		return false;

	if (!Savegame::ReadPhobosStream(stm, value.Y))
		return false;

	if (!Savegame::ReadPhobosStream(stm, value.Z))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<CoordStruct>::WriteToStream(PhobosStreamWriter& stm, const CoordStruct& value) const
{
	if (!Savegame::WritePhobosStream(stm, value.X))
		return false;

	if (!Savegame::WritePhobosStream(stm, value.Y))
		return false;

	if (!Savegame::WritePhobosStream(stm, value.Z))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<Point2D>::ReadFromStream(PhobosStreamReader& stm, Point2D& value, bool register_for_change) const
{
	if (!Savegame::ReadPhobosStream(stm, value.X))
		return false;

	if (!Savegame::ReadPhobosStream(stm, value.Y))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<Point2D>::WriteToStream(PhobosStreamWriter& stm, const Point2D& value) const
{
	if (!Savegame::WritePhobosStream(stm, value.X))
		return false;

	if (!Savegame::WritePhobosStream(stm, value.Y))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<Point2DBYTE>::ReadFromStream(PhobosStreamReader& stm, Point2DBYTE& value, bool register_for_change) const
{
	if (!Savegame::ReadPhobosStream(stm, value.X))
		return false;

	if (!Savegame::ReadPhobosStream(stm, value.Y))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<Point2DBYTE>::WriteToStream(PhobosStreamWriter& stm, const Point2DBYTE& value) const
{
	if (!Savegame::WritePhobosStream(stm, value.X))
		return false;

	if (!Savegame::WritePhobosStream(stm, value.Y))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<Point3D>::ReadFromStream(PhobosStreamReader& stm, Point3D& value, bool register_for_change) const
{
	if (!Savegame::ReadPhobosStream(stm, value.X))
		return false;

	if (!Savegame::ReadPhobosStream(stm, value.Y))
		return false;

	if (!Savegame::ReadPhobosStream(stm, value.Z))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<Point3D>::WriteToStream(PhobosStreamWriter& stm, const Point3D& value) const
{
	if (!Savegame::WritePhobosStream(stm, value.X))
		return false;

	if (!Savegame::WritePhobosStream(stm, value.Y))
		return false;

	if (!Savegame::WritePhobosStream(stm, value.Z))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<ColorStruct>::ReadFromStream(PhobosStreamReader& stm, ColorStruct& value, bool register_for_change) const
{
	if (!Savegame::ReadPhobosStream(stm, value.R))
		return false;

	if (!Savegame::ReadPhobosStream(stm, value.G))
		return false;

	if (!Savegame::ReadPhobosStream(stm, value.B))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<ColorStruct>::WriteToStream(PhobosStreamWriter& stm, const ColorStruct& value) const
{
	if (!Savegame::WritePhobosStream(stm, value.R))
		return false;

	if (!Savegame::WritePhobosStream(stm, value.G))
		return false;

	if (!Savegame::WritePhobosStream(stm, value.B))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<WeaponStruct>::ReadFromStream(PhobosStreamReader& stm, WeaponStruct& value, bool register_for_change) const
{
	if (!Savegame::ReadPhobosStream(stm, value.WeaponType, register_for_change)) return false;
	if (!Savegame::ReadPhobosStream(stm, value.FLH, register_for_change)) return false;
	if (!Savegame::ReadPhobosStream(stm, value.BarrelLength, register_for_change)) return false;
	if (!Savegame::ReadPhobosStream(stm, value.BarrelThickness, register_for_change)) return false;
	if (!Savegame::ReadPhobosStream(stm, value.TurretLocked, register_for_change)) return false;
	return 	stm.RegisterChange(&value);;
}

bool Savegame::PhobosStreamObject<WeaponStruct>::WriteToStream(PhobosStreamWriter& stm, const WeaponStruct& value) const
{
	if (!Savegame::WritePhobosStream(stm, value.WeaponType)) return false;
	if (!Savegame::WritePhobosStream(stm, value.FLH)) return false;
	if (!Savegame::WritePhobosStream(stm, value.BarrelLength)) return false;
	if (!Savegame::WritePhobosStream(stm, value.BarrelThickness)) return false;
	if (!Savegame::WritePhobosStream(stm, value.TurretLocked)) return false;
	return 	stm.RegisterChange(&value);;
}

bool Savegame::PhobosStreamObject<TintStruct>::ReadFromStream(PhobosStreamReader& stm, TintStruct& value, bool register_for_change) const
{
	if (!Savegame::ReadPhobosStream(stm, value.Red, register_for_change)) return false;
	if (!Savegame::ReadPhobosStream(stm, value.Green, register_for_change)) return false;
	if (!Savegame::ReadPhobosStream(stm, value.Blue, register_for_change)) return false;

	return true;
}

bool Savegame::PhobosStreamObject<TintStruct>::WriteToStream(PhobosStreamWriter& stm, const TintStruct& value) const
{
	if (!Savegame::WritePhobosStream(stm, value.Red)) return false;
	if (!Savegame::WritePhobosStream(stm, value.Green)) return false;
	if (!Savegame::WritePhobosStream(stm, value.Blue)) return false;

	return true;
}

bool Savegame::PhobosStreamObject<LightingStruct>::ReadFromStream(PhobosStreamReader& stm, LightingStruct& value, bool register_for_change) const
{
	if (!Savegame::ReadPhobosStream(stm, value.Tint, register_for_change)) return false;
	if (!Savegame::ReadPhobosStream(stm, value.Ground, register_for_change)) return false;
	if (!Savegame::ReadPhobosStream(stm, value.Level, register_for_change)) return false;

	return true;
}

bool Savegame::PhobosStreamObject<LightingStruct>::WriteToStream(PhobosStreamWriter& stm, const LightingStruct& value) const
{
	if (!Savegame::WritePhobosStream(stm, value.Tint)) return false;
	if (!Savegame::WritePhobosStream(stm, value.Ground)) return false;
	if (!Savegame::WritePhobosStream(stm, value.Level)) return false;

	return true;
}

bool Savegame::PhobosStreamObject<CounterClass>::ReadFromStream(PhobosStreamReader& stm, CounterClass& value, bool register_for_change) const
{
	if (!Savegame::ReadPhobosStream<VectorClass<int>>(stm, value, register_for_change))
		return false;

	return Savegame::ReadPhobosStream(stm, value.Total, register_for_change) && stm.RegisterChange(&value);
}

bool Savegame::PhobosStreamObject<CounterClass>::WriteToStream(PhobosStreamWriter& stm, const CounterClass& value) const
{
	if (!Savegame::WritePhobosStream<VectorClass<int>>(stm, value))
		return false;

	return Savegame::WritePhobosStream(stm, value.Total) && stm.RegisterChange(&value);
}

bool Savegame::PhobosStreamObject<ScriptActionNode>::ReadFromStream(PhobosStreamReader& stm, ScriptActionNode& value, bool register_for_change) const
{

	if (!Savegame::ReadPhobosStream(stm, value.Action, register_for_change))
		return false;

	if (!Savegame::ReadPhobosStream(stm, value.Argument, register_for_change))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<ScriptActionNode>::WriteToStream(PhobosStreamWriter& stm, const ScriptActionNode& value) const
{
	if (!Savegame::WritePhobosStream(stm, value.Action))
		return false;

	if (!Savegame::WritePhobosStream(stm, value.Argument))
		return false;

	return true;
}

bool Savegame::PhobosStreamObject<BytePalette>::ReadFromStream(PhobosStreamReader& Stm, BytePalette& Value, bool RegisterForChange) const
{
	int expected = 0; //guard
	if (!Savegame::ReadPhobosStream(Stm, expected, RegisterForChange))
		return false;

	if (expected != BytePalette::EntriesCount)
		return false;

	for (int i = 0; i < BytePalette::EntriesCount; ++i)
	{
		if (!Savegame::ReadPhobosStream(Stm, Value.Entries[i], RegisterForChange))
			return false;
	}

	return true;
}

bool Savegame::PhobosStreamObject<BytePalette>::WriteToStream(PhobosStreamWriter& Stm, const BytePalette& Value) const
{
	int expected = BytePalette::EntriesCount; //guard

	if (!Savegame::WritePhobosStream(Stm, expected))
		return false;

	for (int i = 0; i < BytePalette::EntriesCount; ++i)
	{
		if (!Savegame::WritePhobosStream(Stm, Value.Entries[i]))
			return false;
	}

	return true;
}

bool Savegame::PhobosStreamObject<UniqueGamePtr<BytePalette>>::ReadFromStream(PhobosStreamReader& Stm, UniqueGamePtr<BytePalette>& Value, bool RegisterForChange) const
{
	bool hasvalue = false;
	if (!Savegame::ReadPhobosStream(Stm, hasvalue))
		return false;

	if (hasvalue)
	{
		BytePalette* ptrOld = nullptr;
		if (!Stm.Load(ptrOld))
			return false;

		auto ptrNew = GameCreate<BytePalette>();
		if (!Savegame::ReadPhobosStream(Stm, *ptrNew, RegisterForChange))
			return false;

		PHOBOS_SWIZZLE_REGISTER_POINTER((LONG)ptrOld, ptrNew, "BytePalette");
		Value.reset(ptrNew);

	}
	else
	{
		Value.reset(nullptr);
	}
	return true;
}

bool Savegame::PhobosStreamObject<UniqueGamePtr<BytePalette>>::WriteToStream(PhobosStreamWriter& Stm, const UniqueGamePtr<BytePalette>& Value) const
{
	const bool Exist = Value.get() != nullptr;
	if (!Savegame::WritePhobosStream(Stm, Exist))
		return false;

	if (Exist)
	{
		if (!Savegame::WritePhobosStream(Stm, Value.get()))
			return false;

		if (!Savegame::WritePhobosStream(Stm, *Value.get()))
			return false;
	}

	return true;
}

bool Savegame::PhobosStreamObject<SHPCaches*>::ReadFromStream(PhobosStreamReader& Stm, SHPCaches*& Value, bool RegisterForChange) const
{
	bool HasAny = false;

	if (!Savegame::ReadPhobosStream(Stm, HasAny))
		return false;

	if (!HasAny)
	{
		Value = nullptr;
		return true;
	}

	Value = nullptr;
	std::string name {};

	if (!Savegame::ReadPhobosStream(Stm, name, RegisterForChange))
		return false;

	// NOTE: a missing SHP is deliberately non-fatal -- Value stays null
	// and the load continues.
	Value = FileSystem::LoadSHPFile(name.c_str());
	return true;
}

bool Savegame::PhobosStreamObject<SHPCaches*>::WriteToStream(PhobosStreamWriter& Stm, SHPCaches* const& Value) const
{
	const bool HasAny = Value != nullptr;
	if (!Savegame::WritePhobosStream(Stm, HasAny))
		return false;

	if (!HasAny)
		return true;

	const char* filename = Value->Filename;

	if (!filename)
		Debug::FatalErrorAndExit("Invalid SHP !");

	std::string file(filename);
	return Savegame::WritePhobosStream(Stm, file);
}

bool Savegame::PhobosStreamObject<RocketStruct>::ReadFromStream(PhobosStreamReader& Stm, RocketStruct& Value, bool RegisterForChange) const
{
	if (!Savegame::ReadPhobosStream(Stm, Value.PauseFrames)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.TiltFrames)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.PitchInitial)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.PitchFinal)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.TurnRate)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.RaiseRate)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.Acceleration)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.Altitude)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.Damage)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.EliteDamage))	return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.BodyLength)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.LazyCurve)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.Type, RegisterForChange)) return false;

	return true;
}

bool Savegame::PhobosStreamObject<RocketStruct>::WriteToStream(PhobosStreamWriter& Stm, const RocketStruct& Value) const
{
	if (!Savegame::WritePhobosStream(Stm, Value.PauseFrames)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.TiltFrames)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.PitchInitial)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.PitchFinal)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.TurnRate)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.RaiseRate)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.Acceleration)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.Altitude)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.Damage)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.EliteDamage))	return false;
	if (!Savegame::WritePhobosStream(Stm, Value.BodyLength)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.LazyCurve)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.Type)) return false;

	return true;
}

bool Savegame::PhobosStreamObject<BuildType>::ReadFromStream(PhobosStreamReader& Stm, BuildType& Value, bool RegisterForChange) const
{
	if (!Savegame::ReadPhobosStream(Stm, Value.ItemIndex)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.ItemType)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.Cat)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.CurrentFactory, RegisterForChange)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.Status)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.Progress)) return false;
	if (!Savegame::ReadPhobosStream(Stm, Value.FlashEndFrame)) return false;

	return 	Stm.RegisterChange(&Value);
}

bool Savegame::PhobosStreamObject<BuildType>::WriteToStream(PhobosStreamWriter& Stm, const BuildType& Value) const
{
	if (!Savegame::WritePhobosStream(Stm, Value.ItemIndex)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.ItemType)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.Cat)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.CurrentFactory)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.Status)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.Progress)) return false;
	if (!Savegame::WritePhobosStream(Stm, Value.FlashEndFrame)) return false;

	return 	Stm.RegisterChange(&Value);
}
