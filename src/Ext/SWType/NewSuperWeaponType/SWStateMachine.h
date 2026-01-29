#pragma once

#include "SWTypeHandler.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/House/Body.h>

#include <ProgressTimer.h>
#include <Utilities/MemoryPoolUniquePointer.h>

enum class SWStateMachineIdentifier : int
{
	Invalid = -1,
	UnitDelivery = 0,
	ChronoWarp = 1,
	PsychicDominator = 2,
	CloneableLighningStorm = 3,
	Droppod = 4,
	IonCannon = 5,
	LaserStrike = 6,
	GenericWarhead = 7,
	SonarPulse = 8,
	SpyPlane = 9,
	Reveal = 10,
	GeneticMutator = 11,
	ParaDrop = 12,
	Protect = 13,
	MeteorShower = 14,

	count
};

#define DEFINE_SWSTATEMACHINE_IDENT(n) \
virtual SWStateMachineIdentifier GetIdentifier() const override { return SWStateMachineIdentifier::##n##; }\
virtual const char* GetIdentifierStrings() const override { return "SWStateMachine::"#n; }

// ============================================================
// Base class
// state machines - create one to use delayed effects [create a child class per SWTypeHandler, obviously]
// i.e. start anim/sound 1 frame after clicking, fire a damage wave 25 frames later, and play second sound 50 frames after that...

// ============================================================
class SWStateMachine
{
public:
	static HelperedVector<std::unique_ptr<SWStateMachine>> Array;

	SWStateMachine() = default;
	SWStateMachine(int Duration, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: Super(pSuper), Clock(), Coords(XY)
	{
		Clock.Start(Duration);
	}

	virtual ~SWStateMachine() = default;

	virtual bool Finished() { return Clock.Completed(); }
	virtual void Update() = 0;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) = 0;
	virtual SWStateMachineIdentifier GetIdentifier() const = 0;
	virtual const char* GetIdentifierStrings() const = 0;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

	COMPILETIMEEVAL OPTIONALINLINE int TimePassed() const
	{
		return Unsorted::CurrentFrame - Clock.StartTime;
	}

	COMPILETIMEEVAL OPTIONALINLINE SWTypeExtData* GetTypeExtData() const
	{
		return SWTypeExtContainer::Instance.Find(Super->Type);
	}

public:
	static void UpdateAll();
	static void PointerGotInvalid(AbstractClass* ptr, bool remove);
	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

protected:
	// ============================================================
	// 8-byte aligned: Pointer
	// ============================================================
	SuperClass* Super;

	// ============================================================
	// CDTimerClass (likely 8+ bytes)
	// ============================================================
	CDTimerClass Clock;

	// ============================================================
	// 4-byte aligned: CellStruct (2 shorts = 4 bytes)
	// ============================================================
	CellStruct Coords;

private:
	SWStateMachine(const SWStateMachine&) = delete;
	SWStateMachine(SWStateMachine&&) = delete;
	SWStateMachine& operator=(const SWStateMachine& other) = delete;
};

// ============================================================
// UnitDeliveryStateMachine - no additional members
// ============================================================
class UnitDeliveryStateMachine : public SWStateMachine
{
public:
	UnitDeliveryStateMachine() : SWStateMachine() { }

	UnitDeliveryStateMachine(int Duration, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(Duration, XY, pSuper, pSWType)
	{ }

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { }

	DEFINE_SWSTATEMACHINE_IDENT(UnitDelivery)

		void PlaceUnits();
};

// ============================================================
// DroppodStateMachine - no additional members
// ============================================================
class DroppodStateMachine : public SWStateMachine
{
public:
	DroppodStateMachine() : SWStateMachine() { }

	DroppodStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
	{ }

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { }

	DEFINE_SWSTATEMACHINE_IDENT(Droppod)

		static void SendDroppods(SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, const CellStruct& loc);
	static void PlaceUnits(SuperClass* pSuper, double veterancy, Iterator<TechnoTypeClass*> const Types, int cMin, int cMax, const CellStruct& Coords, bool retries);
};

// ============================================================
// SonarPulseStateMachine - no additional members
// ============================================================
class SonarPulseStateMachine : public SWStateMachine
{
public:
	SonarPulseStateMachine() : SWStateMachine() { }

	SonarPulseStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
	{ }

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { }

	DEFINE_SWSTATEMACHINE_IDENT(SonarPulse)

	static void SendSonarPulse(SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, const CellStruct& loc);
	static void ApplySonarPulse(SuperClass* pSuper, const CellStruct& Coords, const SWRange& range);
};

// ============================================================
// SpyPlaneStateMachine
// ============================================================
class SpyPlaneStateMachine : public SWStateMachine
{
public:
	SpyPlaneStateMachine()
		: SWStateMachine()
		, target { nullptr }
	{ }

	SpyPlaneStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType, CellClass* pTarget)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
		, target { pTarget }
	{ }

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { }

	DEFINE_SWSTATEMACHINE_IDENT(SpyPlane)

	static void SendSpyPlane(SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, CellClass* target);

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override
	{
		return SWStateMachine::Load(Stm, RegisterForChange) && Stm.Process(target);
	}

	virtual bool Save(PhobosStreamWriter& Stm) const override
	{
		return SWStateMachine::Save(Stm) && Stm.Process(target);
	}

protected:
	// ============================================================
	// 8-byte aligned: Pointer
	// ============================================================
	CellClass* target;
};

// ============================================================
// ChronoWarpStateMachine
// ============================================================
class ChronoWarpStateMachine : public SWStateMachine
{
public:
	struct ChronoWarpContainer
	{
	public:
		// ============================================================
		// 8-byte aligned: Pointer
		// ============================================================
		BuildingClass* building { nullptr };

		// ============================================================
		// 12-byte: CoordStruct (3 ints)
		// ============================================================
		CoordStruct origin {};

		// ============================================================
		// 4-byte: CellStruct (2 shorts)
		// ============================================================
		CellStruct target {};

		// ============================================================
		// 1-byte: bool (at the end)
		// ============================================================
		bool isVehicle { false };

		COMPILETIMEEVAL ChronoWarpContainer(BuildingClass* pBld, const CellStruct& target, const CoordStruct& origin, bool isVehicle)
			: building(pBld)
			, origin(origin)
			, target(target)
			, isVehicle(isVehicle)
		{ }

		COMPILETIMEEVAL ChronoWarpContainer() = default;

		COMPILETIMEEVAL bool operator == (const ChronoWarpContainer& other) const
		{
			return this->building == other.building;
		}

		COMPILETIMEEVAL ~ChronoWarpContainer() = default;

		bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return Stm
				.Process(this->building, RegisterForChange)
				.Process(this->origin)
				.Process(this->target)
				.Process(this->isVehicle)
				.Success();
		}

		bool Save(PhobosStreamWriter& Stm)
		{
			return Stm
				.Process(this->building)
				.Process(this->origin)
				.Process(this->target)
				.Process(this->isVehicle)
				.Success();
		}
	};

	ChronoWarpStateMachine()
		: SWStateMachine()
		, Buildings()
		, Duration(0)
	{ }

	ChronoWarpStateMachine(int Duration, const CellStruct& XY, SuperClass* pSuper, SWTypeHandler* pSWType, HelperedVector<ChronoWarpContainer> Buildings)
		: SWStateMachine(Duration, XY, pSuper, pSWType)
		, Buildings(std::move(Buildings))
		, Duration(Duration)
	{ }

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

	DEFINE_SWSTATEMACHINE_IDENT(ChronoWarp)

		virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

protected:
	// ============================================================
	// 24-byte: Vector
	// ============================================================
	HelperedVector<ChronoWarpContainer> Buildings;

	// ============================================================
	// 4-byte: int
	// ============================================================
	int Duration;
};

// ============================================================
// PsychicDominatorStateMachine
// ============================================================
class PsychicDominatorStateMachine : public SWStateMachine
{
public:
	PsychicDominatorStateMachine()
		: SWStateMachine()
		, Deferment(0)
	{ }

	PsychicDominatorStateMachine(CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(MAXINT32, XY, pSuper, pSWType)
		, Deferment(0)
	{
		PsyDom::Status = PsychicDominatorStatus::FirstAnim;

		SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
		this->Deferment = pData->SW_Deferment.Get(0);

		if (!pSuper->Owner)
		{
			Debug::LogInfo("Psydom[{}] Firing Without Ownership!", pSuper->Type->ID);
			PsyDom::Owner = HouseExtData::FindSpecial();
		}
		else
		{
			PsyDom::Owner = pSuper->Owner;
		}

		PsyDom::Coords = XY;
		PsyDom::Anim = nullptr;
	}

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { }

	DEFINE_SWSTATEMACHINE_IDENT(PsychicDominator)

		virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

protected:
	// ============================================================
	// 4-byte: int
	// ============================================================
	int Deferment;
};

// ============================================================
// IonCannonStateMachine
// ============================================================
class IonCannonStateMachine : public SWStateMachine
{
public:
	enum class IonCannonStatus : unsigned int
	{
		Inactive = 0,
		FirstAnim = 1,
		Fire = 2,
		SecondAnim = 3,
		Reset = 4,
		Over = 5
	};

public:
	IonCannonStateMachine()
		: SWStateMachine()
		// Pointers
		, Owner(nullptr)
		, Anim(nullptr)
		, Firer(nullptr)
		// int/enum
		, Deferment(0)
		, Status(IonCannonStatus::FirstAnim)
	{ }

	IonCannonStateMachine(CellStruct XY, SuperClass* pSuper, TechnoClass* pFirer, SWTypeHandler* pSWType)
		: SWStateMachine(MAXINT32, XY, pSuper, pSWType)
		// Pointers
		, Owner(pSuper->Owner)
		, Anim(nullptr)
		, Firer(pFirer)
		// int/enum
		, Deferment(0)
		, Status(IonCannonStatus::FirstAnim)
	{
		SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
		this->Deferment = pData->SW_Deferment.Get(0);
	}

	virtual ~IonCannonStateMachine() = default;

	virtual void Update() override;

	DEFINE_SWSTATEMACHINE_IDENT(IonCannon)

		virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	void Fire();
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

protected:
	// ============================================================
	// 8-byte aligned: Pointers
	// ============================================================
	HouseClass* Owner;
	Handle<AnimClass*, UninitAnim> Anim;
	TechnoClass* Firer;

	// ============================================================
	// 4-byte: int/enum
	// ============================================================
	int Deferment;
	IonCannonStatus Status;
};

// ============================================================
// CloneableLighningStormStateMachine
// ============================================================
class CloneableLighningStormStateMachine : public SWStateMachine
{
public:
	CloneableLighningStormStateMachine()
		: SWStateMachine()
		// Vectors
		, CloudsPresent()
		, CloudsManifest()
		, BoltsPresent()
		// Pointer
		, Invoker(nullptr)
		// ints
		, ActualDuration(0)
		, StartTime(0)
		, Deferment(0)
		// bools
		, IsActive(false)
		, TimeToEnd(false)
	{ }

	CloneableLighningStormStateMachine(int Duration, int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pFirer, SWTypeHandler* pSWType)
		: SWStateMachine(Duration, XY, pSuper, pSWType)
		// Vectors
		, CloudsPresent()
		, CloudsManifest()
		, BoltsPresent()
		// Pointer
		, Invoker(pFirer)
		// ints
		, ActualDuration(0)
		, StartTime(0)
		, Deferment(0)
		// bools
		, IsActive(false)
		, TimeToEnd(false)
	{
		Start(XY, Duration, Deferment);
	}

	virtual ~CloneableLighningStormStateMachine()
	{
		for (auto& CP : CloudsPresent)
		{
			if (CP && CP->IsAlive && CP->Type)
			{
				CP->UnInit();
			}

			CP = nullptr;
		}

		for (auto& CM : CloudsManifest)
		{
			if (CM && CM->IsAlive && CM->Type)
			{
				CM->UnInit();
			}
			CM = nullptr;
		}

		for (auto& BP : BoltsPresent)
		{
			if (BP && BP->IsAlive && BP->Type)
			{
				BP->UnInit();
			}

			BP = nullptr;
		}

	}

	virtual void Update() override;

	virtual bool Finished() override
	{
		return ActualDuration <= -1 ? false : Clock.Completed() && !Deferment && TimeToEnd;
	}

	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

	DEFINE_SWSTATEMACHINE_IDENT(CloneableLighningStorm)

		virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	void Strike2(CoordStruct const& nCoord);
	bool Strike(CellStruct const& nCell);
	bool Start(CellStruct& cell, int nDuration, int nDeferment);
	void Stop() { TimeToEnd = true; }

public:
	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	HelperedVector<AnimClass*> CloudsPresent;
	HelperedVector<AnimClass*> CloudsManifest;
	HelperedVector<AnimClass*> BoltsPresent;

	// ============================================================
	// 8-byte aligned: Pointer
	// ============================================================
	TechnoClass* Invoker;

	// ============================================================
	// 4-byte: ints
	// ============================================================
	int ActualDuration;
	int StartTime;
	int Deferment;

	// ============================================================
	// 1-byte: bools (packed at the end)
	// ============================================================
	bool IsActive;
	bool TimeToEnd;
};

// ============================================================
// LaserStrikeStateMachine
// ============================================================
class LaserStrikeStateMachine : public SWStateMachine
{
public:
	LaserStrikeStateMachine()
		: SWStateMachine()
		// Pointer
		, Firer { nullptr }
		// ints
		, LaserStrikeRadius { -1 }
		, LaserStrikeStartAngle { -180 }
		, LaserStrikeRate { -1 }
		, LaserStrikeROF { 0 }
		, LaserStrikeRadiusReduce { 0 }
		, LaserStrikeAngle { 0 }
		, LaserStrikeScatter_Max { 0 }
		, LaserStrikeScatter_Min { 0 }
		, LaserStrikeDuration { 0 }
		, Deferment { -1 }
		, MaxCount { 1 }
		, MaxCountCounter { 1 }
		// bools
		, LaserStrikesetRadius { true }
		, LaserStrikeStop { false }
		, AlreadyActivated { false }
	{ }

	LaserStrikeStateMachine(CellStruct XY, SuperClass* pSuper, TechnoClass* pFirer, int maxcount, int deferment, SWTypeHandler* pSWType, int duration)
		: SWStateMachine(duration, XY, pSuper, pSWType)
		// Pointer
		, Firer { pFirer }
		// ints
		, LaserStrikeRadius { -1 }
		, LaserStrikeStartAngle { -180 }
		, LaserStrikeRate { -1 }
		, LaserStrikeROF { 0 }
		, LaserStrikeRadiusReduce { 0 }
		, LaserStrikeAngle { 0 }
		, LaserStrikeScatter_Max { 0 }
		, LaserStrikeScatter_Min { 0 }
		, LaserStrikeDuration { 0 }
		, Deferment { deferment }
		, MaxCount { Math::abs(maxcount) }
		, MaxCountCounter { Math::abs(maxcount) }
		// bools
		, LaserStrikesetRadius { true }
		, LaserStrikeStop { false }
		, AlreadyActivated { false }
	{ }

	virtual void Update() override;

	DEFINE_SWSTATEMACHINE_IDENT(LaserStrike)

		virtual bool Finished() override { return SWStateMachine::Finished() || MaxCountCounter <= 0; }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

protected:
	// ============================================================
	// 8-byte aligned: Pointer
	// ============================================================
	TechnoClass* Firer;

	// ============================================================
	// 4-byte: ints
	// ============================================================
	int LaserStrikeRadius;
	int LaserStrikeStartAngle;
	int LaserStrikeRate;
	int LaserStrikeROF;
	int LaserStrikeRadiusReduce;
	int LaserStrikeAngle;
	int LaserStrikeScatter_Max;
	int LaserStrikeScatter_Min;
	int LaserStrikeDuration;
	int Deferment;
	int MaxCount;
	int MaxCountCounter;

	// ============================================================
	// 1-byte: bools (packed at the end)
	// ============================================================
	bool LaserStrikesetRadius;
	bool LaserStrikeStop;
	bool AlreadyActivated;
	// 3 bools = 3 bytes, pads to 4
};

// ============================================================
// GenericWarheadStateMachine
// ============================================================
class GenericWarheadStateMachine : public SWStateMachine
{
public:
	GenericWarheadStateMachine()
		: SWStateMachine()
		, Firer { nullptr }
	{ }

	explicit GenericWarheadStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
		, Firer { pfirer }
	{ }

	virtual void Update() override;

	DEFINE_SWSTATEMACHINE_IDENT(GenericWarhead)

		virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

	static void SentPayload(TechnoClass* pFirer, SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, const CellStruct& loc);

protected:
	// ============================================================
	// 8-byte aligned: Pointer
	// ============================================================
	TechnoClass* Firer;
};

// ============================================================
// GeneticMutatorStateMachine
// ============================================================
class GeneticMutatorStateMachine : public SWStateMachine
{
public:
	GeneticMutatorStateMachine()
		: SWStateMachine()
		, Firer { nullptr }
		, CoordsWithBridge {}
	{ }

	explicit GeneticMutatorStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
		, Firer { pfirer }
		, CoordsWithBridge {}
	{
		this->CoordsWithBridge = MapClass::Instance->GetCellAt(XY)->GetCoordsWithBridge();
	}

	virtual void Update() override;

	DEFINE_SWSTATEMACHINE_IDENT(GeneticMutator)

		virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return SWStateMachine::Load(Stm, RegisterForChange)
			&& Stm
			.Process(Firer)
			.Process(CoordsWithBridge)
			.Success();
	}

	virtual bool Save(PhobosStreamWriter& Stm) const
	{
		return SWStateMachine::Save(Stm)
			&& Stm
			.Process(Firer)
			.Process(CoordsWithBridge)
			.Success();
	}

	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override
	{
		AnnounceInvalidPointer(Firer, ptr, remove);
	}

	static void ApplyGeneticMutator(TechnoClass* pFirer, SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, CoordStruct& coord, const CellStruct& loc, WarheadTypeClass* pWarhead, SWRange& range, int damage);

protected:
	// ============================================================
	// 8-byte aligned: Pointer
	// ============================================================
	TechnoClass* Firer;

	// ============================================================
	// 12-byte: CoordStruct (3 ints)
	// ============================================================
	CoordStruct CoordsWithBridge;
};

// ============================================================
// RevealStateMachine - no additional members
// ============================================================
class RevealStateMachine : public SWStateMachine
{
public:
	RevealStateMachine() : SWStateMachine() { }

	RevealStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
	{ }

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { }

	DEFINE_SWSTATEMACHINE_IDENT(Reveal)
};

// ============================================================
// ParaDropStateMachine
// ============================================================
class ParaDropStateMachine : public SWStateMachine
{
public:
	ParaDropStateMachine()
		: SWStateMachine()
		// Pointer
		, Target { nullptr }
		// Vectors
		, PlaneType {}
		, Types {}
		, Nums {}
	{ }

	ParaDropStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType, CellClass* pTarget)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
		// Pointer
		, Target { pTarget }
		// Vectors
		, PlaneType {}
		, Types {}
		, Nums {}
	{
		this->UpdateProperties();
	}

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { }

	void UpdateProperties();

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return SWStateMachine::Load(Stm, RegisterForChange)
			&& Stm
			.Process(Target)
			.Process(PlaneType)
			.Process(Types)
			.Process(Nums)
			.Success();
	}

	virtual bool Save(PhobosStreamWriter& Stm) const
	{
		return SWStateMachine::Save(Stm)
			&& Stm
			.Process(Target)
			.Process(PlaneType)
			.Process(Types)
			.Process(Nums)
			.Success();
	}

	DEFINE_SWSTATEMACHINE_IDENT(ParaDrop)

protected:
	// ============================================================
	// 8-byte aligned: Pointer
	// ============================================================
	CellClass* Target;

	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	std::vector<AircraftTypeClass*> PlaneType;
	std::vector<std::vector<TechnoTypeClass*>> Types;
	std::vector<std::vector<int>> Nums;
};

// ============================================================
// ProtectStateMachine - no additional members
// ============================================================
class ProtectStateMachine : public SWStateMachine
{
public:
	ProtectStateMachine() : SWStateMachine() { }

	ProtectStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
	{ }

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { }

	DEFINE_SWSTATEMACHINE_IDENT(Protect)
};

// ============================================================
// MeteorShowerStateMachine
// ============================================================
class MeteorShowerStateMachine : public SWStateMachine
{
public:
	MeteorShowerStateMachine()
		: SWStateMachine()
		, Firer { nullptr }
	{ }

	explicit MeteorShowerStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
		, Firer { pfirer }
	{ }

	virtual void Update() override;

	DEFINE_SWSTATEMACHINE_IDENT(MeteorShower)

		virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

	static void SentMeteorShower(TechnoClass* pFirer, SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, const CellStruct& loc);

protected:
	// ============================================================
	// 8-byte aligned: Pointer
	// ============================================================
	TechnoClass* Firer;
};

#define MakeStatemachine(a) \
case SWStateMachineIdentifier::## a ##:\
return std::make_unique<a##StateMachine>();\

template <>
struct Savegame::ObjectFactory<SWStateMachine>
{
	std::unique_ptr<SWStateMachine> operator() (PhobosStreamReader& Stm) const
	{
		SWStateMachineIdentifier type = SWStateMachineIdentifier::Invalid;
		if (Stm.Load(type))
		{
			switch (type)
			{
				MakeStatemachine(UnitDelivery)
					MakeStatemachine(ChronoWarp)
					MakeStatemachine(PsychicDominator)
					MakeStatemachine(CloneableLighningStorm)
					MakeStatemachine(Droppod)
					MakeStatemachine(IonCannon)
					MakeStatemachine(LaserStrike)
					MakeStatemachine(GenericWarhead)
					MakeStatemachine(SonarPulse)
					MakeStatemachine(SpyPlane)
					MakeStatemachine(Reveal)
					MakeStatemachine(GeneticMutator)
					MakeStatemachine(ParaDrop)
					MakeStatemachine(Protect)
					MakeStatemachine(MeteorShower)
			default:
				Debug::FatalErrorAndExit("SWStateMachineType %d not recognized.",
					static_cast<unsigned int>(type));
			}
		}

		return nullptr;
	}
};

#undef MakeStatemachine
#undef DEFINE_SWSTATEMACHINE_IDENT