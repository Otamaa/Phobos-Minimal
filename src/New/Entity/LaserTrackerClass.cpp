#include "LaserTrackerClass.h"

#include <BuildingClass.h>
#include <DiskLaserClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Rules/Body.h>

#include <Utilities/Macro.h>

#include <utility>

// ===========================================================================
// Singleton
// ===========================================================================
LaserTrackerClass& LaserTrackerClass::Instance()
{
	static LaserTrackerClass instance;
	return instance;
}

// ===========================================================================
// IgnoreShooterScope
// ===========================================================================
LaserTrackerClass::IgnoreShooterScope::IgnoreShooterScope()
	: Previous(LaserTrackerClass::Instance().Pending().IgnoreShooter)
{
	LaserTrackerClass::Instance().Pending().IgnoreShooter = true;
}

LaserTrackerClass::IgnoreShooterScope::~IgnoreShooterScope()
{
	LaserTrackerClass::Instance().Pending().IgnoreShooter = this->Previous;
}

// ===========================================================================
// Static helpers
// ===========================================================================

// ORIG: inlined twice in LaserRT (TrackingData::Initialize + LaserDrawClass_Update_Tracking).
CoordStruct LaserTrackerClass::GetFrozenWorldFLH(TechnoClass* pShooter, const TrackingData& data)
{
	const int savedBurstIndex = pShooter->CurrentBurstIndex;
	pShooter->CurrentBurstIndex = data.FrozenBurstIndex;
	const CoordStruct worldFLH = pShooter->GetFLH(data.WeaponIndex, data.LocalFLH.X , data.LocalFLH.Y , data.LocalFLH.Z);
	pShooter->CurrentBurstIndex = savedBurstIndex;
	return worldFLH;
}

CoordStruct LaserTrackerClass::ResolveLocalFLH(TechnoClass* pShooter, int weaponIdx)
{
	auto[flhFound, localFLH] = TechnoExtData::GetBurstFLH(pShooter, weaponIdx);

	if (!flhFound) {
		// BUGFIX: original did `pShooter->GetWeapon(weaponIdx)->FLH` with no null guard.
		if (const auto pWeaponStruct = pShooter->GetWeapon(weaponIdx))
			localFLH = pWeaponStruct->FLH;
		else
			localFLH = CoordStruct::Empty;


	}

	// SUSPECT: the old hook at 0x6FD210 additionally mirrored Y for odd burst
	// indices (`if (SavedBurstIndex % 2 != 0) FLH.Y = -FLH.Y;`), but that value
	// was written into LaserRT::SavedLocalFLH and then thrown away at 0x6FD446
	// via std::exchange without ever being read. SetLaserTrackingData always
	// recomputed the FLH itself, unmirrored. Behaviour preserved as-is
	// (unmirrored) because that is what actually ran. TechnoClass::GetFLH is
	// believed to apply burst mirroring internally, so mirroring here would
	// double-flip - VERIFY against 0x6F3300 before "fixing" this.
	return localFLH;
}

bool LaserTrackerClass::ResolveStopOnFirerConvert(TechnoClass* pShooter, int weaponIdx)
{
	const auto pWeaponStruct = pShooter->GetWeapon(weaponIdx);
	const auto pWeapon = pWeaponStruct ? pWeaponStruct->WeaponType : nullptr;

	// SUSPECT: vanilla path returned false (not the Rules default) when the
	// weapon slot was empty. Preserved verbatim.
	if (!pWeapon)
		return false;

	return WeaponTypeExtContainer::Instance.Find(pWeapon)->LaserPositionUpdate_StopOnFirerConvert
		.Get(FakeRulesClass::Instance()->LaserPositionUpdate_StopOnFirerConvert);
}

// ===========================================================================
// Reverse index maintenance
// ===========================================================================
void LaserTrackerClass::Register(LaserDrawClass* pLaser, const TrackingData& data)
{
	if (data.TracksShooter())
		this->ShooterIndex.emplace(data.Shooter, pLaser);

	if (data.TracksTarget())
		this->TargetIndex.emplace(data.Target, pLaser);
}

void LaserTrackerClass::Unregister(LaserDrawClass* pLaser, const TrackingData& data)
{
	if (data.Shooter)
	{
		const auto range = this->ShooterIndex.equal_range(data.Shooter);
		for (auto it = range.first; it != range.second; ++it)
		{
			if (it->second == pLaser)
			{
				this->ShooterIndex.erase(it);
				break;
			}
		}
	}

	if (data.Target)
	{
		const auto range = this->TargetIndex.equal_range(data.Target);
		for (auto it = range.first; it != range.second; ++it)
		{
			if (it->second == pLaser)
			{
				this->TargetIndex.erase(it);
				break;
			}
		}
	}
}

// DIFF: the old Update hook set `data.Shooter = nullptr` on firer conversion but
// left the stale ShooterToLasers entry behind. This detaches both halves.
void LaserTrackerClass::DetachShooter(LaserDrawClass* pLaser, TrackingData& data)
{
	if (!data.Shooter)
		return;

	const auto range = this->ShooterIndex.equal_range(data.Shooter);
	for (auto it = range.first; it != range.second; ++it)
	{
		if (it->second == pLaser)
		{
			this->ShooterIndex.erase(it);
			break;
		}
	}

	data.Shooter = nullptr;
	data.OriginalType = nullptr;
}

// ===========================================================================
// Record lifetime
// ===========================================================================
void LaserTrackerClass::Assign(LaserDrawClass* pLaser, TechnoClass* pShooter, AbstractClass* pTarget,
	int weaponIdx, PositionFollow mode, bool ignoreShooter)
{
	if (!pLaser)
		return;

	if (ignoreShooter)
		pShooter = nullptr;

	// Garrisonable buildings move their firing origin per occupant slot, so a
	// frozen FLH would desync from the muzzle - firer follow is dropped.
	if (const auto pBuilding = cast_to<BuildingClass*>(pShooter))
	{
		if (pBuilding->Type->MaxNumberOccupants > 0)
			mode &= ~PositionFollow::Firer;
	}

	TrackingData data;
	data.WeaponIndex = weaponIdx;
	data.FollowMode = mode;

	// DIFF (perf only): the old SetLaserTrackingData resolved FLH / burst index /
	// StopOnFirerConvert unconditionally and then discarded them unless Firer was
	// set. Same result, fewer ExtMap lookups.
	if (pShooter && (mode & PositionFollow::Firer))
	{
		data.Shooter = pShooter;
		data.LocalFLH = LaserTrackerClass::ResolveLocalFLH(pShooter, weaponIdx);
		if (pShooter->CurrentBurstIndex % 2 != 0)
			data.LocalFLH.Y = -data.LocalFLH.Y;

		data.FrozenBurstIndex = pShooter->CurrentBurstIndex;
		data.StopOnFirerConvert = LaserTrackerClass::ResolveStopOnFirerConvert(pShooter, weaponIdx);

		if (data.StopOnFirerConvert)
			data.OriginalType = pShooter->GetTechnoType();

		data.SavedOffset = pLaser->Source - LaserTrackerClass::GetFrozenWorldFLH(pShooter, data);
	}

	if (mode & PositionFollow::Target)
		data.Target = flag_cast_to<ObjectClass*>(pTarget);

	// Always drop any prior record first - the laser pointer may be recycled.
	this->Remove(pLaser);

	// Nothing to follow (e.g. Target-only mode aimed at a cell): do not pay for
	// a map entry we would only skip over every frame.
	if (data.IsInert())
		return;

	auto& stored = this->TrackingMap[pLaser];
	stored = data;
	this->Register(pLaser, stored);
}

void LaserTrackerClass::Remove(LaserDrawClass* pLaser)
{
	const auto it = this->TrackingMap.find(pLaser);

	if (it == this->TrackingMap.end())
		return;

	this->Unregister(pLaser, it->second);
	this->TrackingMap.erase(it);
}

void LaserTrackerClass::Clear()
{
	this->TrackingMap.clear();
	this->ShooterIndex.clear();
	this->TargetIndex.clear();
	this->Context = PendingContext {};
}

// ===========================================================================
// Per-frame update
// ===========================================================================
void LaserTrackerClass::Update(LaserDrawClass* pLaser)
{
	if (this->TrackingMap.empty())
		return;

	const auto it = this->TrackingMap.find(pLaser);

	if (it == this->TrackingMap.cend())
		return;

	auto& data = it->second;

	if (data.Shooter && data.StopOnFirerConvert && data.OriginalType)
	{
		if (data.Shooter->GetTechnoType() != data.OriginalType)
			this->DetachShooter(pLaser, data);
	}

	if (const auto pShooter = data.Shooter)
		pLaser->Source = LaserTrackerClass::GetFrozenWorldFLH(pShooter, data) + data.SavedOffset;

	if (const auto pTarget = data.Target)
		pLaser->Target = pTarget->GetTargetCoords();

	// DIFF: reap dead records here instead of waiting for the DTOR. `data` is not
	// touched after this point, so erasing through `it` is safe.
	if (data.IsInert())
		this->TrackingMap.erase(it);
}

// ===========================================================================
// Engine object teardown
// ===========================================================================
void LaserTrackerClass::OnObjectRemoved(ObjectClass* pObject)
{
	if (!pObject)
		return;

	// NOTE on iterator safety: inside each loop the matching half is nulled
	// *before* the inert check, so Unregister() can never touch the index we are
	// currently iterating. The bulk erase of the key happens after the loop.
	{
		const auto range = this->ShooterIndex.equal_range(pObject);
		for (auto it = range.first; it != range.second; ++it)
		{
			LaserDrawClass* const pLaser = it->second;
			const auto dataIt = this->TrackingMap.find(pLaser);

			if (dataIt == this->TrackingMap.end())
				continue;

			auto& data = dataIt->second;

			if (data.Shooter == pObject)
			{
				data.Shooter = nullptr;
				data.OriginalType = nullptr;
			}

			if (data.IsInert())
			{
				this->Unregister(pLaser, data);   // no-op: both halves already null
				this->TrackingMap.erase(dataIt);
			}
		}

		this->ShooterIndex.erase(pObject);
	}

	{
		const auto range = this->TargetIndex.equal_range(pObject);
		for (auto it = range.first; it != range.second; ++it)
		{
			LaserDrawClass* const pLaser = it->second;
			const auto dataIt = this->TrackingMap.find(pLaser);

			if (dataIt == this->TrackingMap.end())
				continue;

			auto& data = dataIt->second;

			if (data.Target == pObject)
				data.Target = nullptr;

			if (data.IsInert())
			{
				this->Unregister(pLaser, data);
				this->TrackingMap.erase(dataIt);
			}
		}

		this->TargetIndex.erase(pObject);
	}
}
