#pragma once

#include <LaserDrawClass.h>
#include <TechnoClass.h>
#include <ObjectClass.h>
#include <GeneralStructures.h>

#include <Utilities/Enum.h> 

#include <unordered_map>

// ---------------------------------------------------------------------------
// LaserTrackerClass
//
// Replaces the old `namespace LaserRT` free-function + free-global design.
// Owns:
//   * the LaserDrawClass -> TrackingData record map
//   * the two reverse indices (shooter -> lasers, target -> lasers)
//   * the transient "pending creation context" that used to be loose globals
//
// Singleton by design: LaserDrawClass instances are engine-owned and there is
// exactly one game world, so a single registry is the correct ownership model.
// ---------------------------------------------------------------------------
class LaserTrackerClass final
{
public:
	// -----------------------------------------------------------------------
	// Per-laser record.
	// -----------------------------------------------------------------------
	struct TrackingData
	{
		TechnoClass* Shooter { nullptr };
		ObjectClass* Target { nullptr };
		const TechnoTypeClass* OriginalType { nullptr };

		CoordStruct SavedOffset { CoordStruct::Empty };
		CoordStruct LocalFLH { CoordStruct::Empty };

		int WeaponIndex { 0 };
		int FrozenBurstIndex { 0 };

		PositionFollow FollowMode { PositionFollow::None };
		bool StopOnFirerConvert { false };

		// A record with neither end attached can never move the beam again.
		bool IsInert() const
		{
			return !this->Shooter && !this->Target;
		}

		bool TracksShooter() const
		{
			return this->Shooter && (this->FollowMode & PositionFollow::Firer);
		}

		bool TracksTarget() const
		{
			return this->Target && (this->FollowMode & PositionFollow::Target);
		}
	};

	// -----------------------------------------------------------------------
	// Transient context, valid only between TechnoClass::CreateLaser entry
	// (0x6FD210) and the LaserDrawClass allocation site (0x6FD446).
	//
	// ORIG: LaserRT::Shooter / Target / WeaponIndex / IgnoreShooter
	//       (+ SavedLocalFLH / SavedBurstIndex - see .cpp, both were dead)
	// -----------------------------------------------------------------------
	struct PendingContext
	{
		TechnoClass* Shooter { nullptr };
		AbstractClass* Target { nullptr };
		int WeaponIndex { 0 };

		// Sticky across a whole CreateLaser call; set by the shrapnel wrapper.
		bool IgnoreShooter { false };

		void Reset()
		{
			this->Shooter = nullptr;
			this->Target = nullptr;
			this->WeaponIndex = 0;
			// IgnoreShooter is intentionally NOT reset here: it is owned by the
			// caller that raised it (RAII guard below).
		}
	};

	// RAII replacement for the manual `IgnoreShooter = true; ...; = false;` pair.
	class IgnoreShooterScope final
	{
	public:
		IgnoreShooterScope();
		~IgnoreShooterScope();

		IgnoreShooterScope(const IgnoreShooterScope&) = delete;
		IgnoreShooterScope& operator = (const IgnoreShooterScope&) = delete;

	private:
		bool Previous;
	};

	static LaserTrackerClass& Instance();

	// --- record lifetime ---------------------------------------------------
	void Assign(LaserDrawClass* pLaser, TechnoClass* pShooter, AbstractClass* pTarget,
		int weaponIdx, PositionFollow mode, bool ignoreShooter);

	void Remove(LaserDrawClass* pLaser);   // full unregister + erase (DTOR / reset)
	void Clear();                          // scenario teardown

	// --- per-frame ---------------------------------------------------------
	void Update(LaserDrawClass* pLaser);

	// --- engine object teardown -------------------------------------------
	void OnObjectRemoved(ObjectClass* pObject);

	bool IsEmpty() const { return this->TrackingMap.empty(); }

	PendingContext& Pending() { return this->Context; }

private:
	LaserTrackerClass() = default;

	LaserTrackerClass(const LaserTrackerClass&) = delete;
	LaserTrackerClass& operator = (const LaserTrackerClass&) = delete;

	void Register(LaserDrawClass* pLaser, const TrackingData& data);
	void Unregister(LaserDrawClass* pLaser, const TrackingData& data);

	// Drops only the shooter half and its reverse-index entry.
	void DetachShooter(LaserDrawClass* pLaser, TrackingData& data);

	// Frozen-burst FLH evaluation (restores CurrentBurstIndex on the way out).
	static CoordStruct GetFrozenWorldFLH(TechnoClass* pShooter, const TrackingData& data);

	static CoordStruct ResolveLocalFLH(TechnoClass* pShooter, int weaponIdx);
	static bool ResolveStopOnFirerConvert(TechnoClass* pShooter, int weaponIdx);

	std::unordered_map<LaserDrawClass*, TrackingData> TrackingMap;
	std::unordered_multimap<ObjectClass*, LaserDrawClass*> ShooterIndex;
	std::unordered_multimap<ObjectClass*, LaserDrawClass*> TargetIndex;

	PendingContext Context;
};