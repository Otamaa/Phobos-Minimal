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
	CloneableLighningStorm = 3 ,
	Droppod = 4,
	IonCannon = 5,
	LaserStrike = 6 ,
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

// state machines - create one to use delayed effects [create a child class per SWTypeHandler, obviously]
// i.e. start anim/sound 1 frame after clicking, fire a damage wave 25 frames later, and play second sound 50 frames after that...
class SWStateMachine
{
public:
	static HelperedVector<std::unique_ptr<SWStateMachine>> Array;

	SWStateMachine() = default;
	SWStateMachine(int Duration, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: Super(pSuper), Coords(XY)
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

	COMPILETIMEEVAL OPTIONALINLINE int TimePassed() const {
		return Unsorted::CurrentFrame - Clock.StartTime;
	}

	COMPILETIMEEVAL OPTIONALINLINE SWTypeExtData * GetTypeExtData() const {
		return SWTypeExtContainer::Instance.Find(Super->Type);
	}

public:

	static void UpdateAll();
	static void PointerGotInvalid(AbstractClass* ptr, bool remove);
	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

protected:
	SuperClass* Super;
	CellStruct Coords;
	CDTimerClass Clock;

private:
	SWStateMachine(const SWStateMachine&) = delete;
	SWStateMachine(SWStateMachine&&) = delete;
	SWStateMachine& operator=(const SWStateMachine& other) = delete;
};

class UnitDeliveryStateMachine : public SWStateMachine
{
public:
	UnitDeliveryStateMachine()
		: SWStateMachine()
	{
	}

	UnitDeliveryStateMachine(int Duration, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(Duration, XY, pSuper, pSWType)
	{
	}

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { };

	DEFINE_SWSTATEMACHINE_IDENT(UnitDelivery)

	void PlaceUnits();
};

class DroppodStateMachine : public SWStateMachine
{
public:
	DroppodStateMachine()
		: SWStateMachine()
	{
	}

	DroppodStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
	{
	}

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { };

	DEFINE_SWSTATEMACHINE_IDENT(Droppod)

	static void SendDroppods(SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, const CellStruct& loc);
	static void PlaceUnits(SuperClass* pSuper, double veterancy, Iterator<TechnoTypeClass*> const Types, int cMin, int cMax, const CellStruct& Coords ,bool retries);

};

class SonarPulseStateMachine : public SWStateMachine
{
public:
	SonarPulseStateMachine()
		: SWStateMachine()
	{
	}

	SonarPulseStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
	{
	}

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { };

	DEFINE_SWSTATEMACHINE_IDENT(SonarPulse)

	static void SendSonarPulse(SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, const CellStruct& loc);
	static void ApplySonarPulse(SuperClass* pSuper, const CellStruct& Coords , const SWRange& range);

};

class SpyPlaneStateMachine : public SWStateMachine
{
public:
	SpyPlaneStateMachine()
		: SWStateMachine(), target { nullptr }
	{
	}

	SpyPlaneStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType , CellClass* pTarget)
		: SWStateMachine(Deferment, XY, pSuper, pSWType) , target { pTarget }
	{
	}

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { };

	DEFINE_SWSTATEMACHINE_IDENT(SpyPlane)

	static void SendSpyPlane(SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, CellClass* target);

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override
	{
		return SWStateMachine::Load(Stm, RegisterForChange) &&
			Stm.Process(target);
	}

	virtual bool Save(PhobosStreamWriter& Stm) const override
	{
		return SWStateMachine::Save(Stm) &&
			Stm.Process(target);
	}

protected :
	CellClass* target;
};

class ChronoWarpStateMachine : public SWStateMachine
{
public:
	struct ChronoWarpContainer
	{
	public:
		BuildingClass* building { nullptr };
		CellStruct target {};
		CoordStruct origin {};
		bool isVehicle { false };

		COMPILETIMEEVAL ChronoWarpContainer(BuildingClass* pBld, const CellStruct& target, const CoordStruct& origin, bool isVehicle) :
			building(pBld),
			target(target),
			origin(origin),
			isVehicle(isVehicle)
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
				.Process(this->target)
				.Process(this->origin)
				.Process(this->isVehicle)
				.Success();
		}

		bool Save(PhobosStreamWriter& Stm)
		{
			return Stm
				.Process(this->building)
				.Process(this->target)
				.Process(this->origin)
				.Process(this->isVehicle)
				.Success();
		}
	};

	ChronoWarpStateMachine()
		: SWStateMachine(), Buildings(), Duration(0)
	{
	}

	ChronoWarpStateMachine(int Duration, const CellStruct& XY, SuperClass* pSuper, SWTypeHandler* pSWType, HelperedVector<ChronoWarpContainer> Buildings)
		: SWStateMachine(Duration, XY, pSuper, pSWType), Buildings(std::move(Buildings)), Duration(Duration)
	{
	}

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

	DEFINE_SWSTATEMACHINE_IDENT(ChronoWarp)

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;

	virtual bool Save(PhobosStreamWriter& Stm) const override;

protected:
	HelperedVector<ChronoWarpContainer> Buildings;
	int Duration;
};

class PsychicDominatorStateMachine : public SWStateMachine
{
public:
	PsychicDominatorStateMachine()
		: SWStateMachine(), Deferment(0)
	{
	}

	PsychicDominatorStateMachine(CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(MAXINT32, XY, pSuper, pSWType), Deferment(0)
	{
		PsyDom::Status = PsychicDominatorStatus::FirstAnim;

		// the initial deferment
		SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
		this->Deferment = pData->SW_Deferment.Get(0);

		// make the game happy
		if (!pSuper->Owner) {
			Debug::LogInfo("Psydom[{}] Firing Without Ownership!", pSuper->Type->ID);
			PsyDom::Owner = HouseExtData::FindSpecial();
		}else{
			PsyDom::Owner = pSuper->Owner;
		}

		PsyDom::Coords = XY;
		PsyDom::Anim = nullptr;
	};

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { };

	DEFINE_SWSTATEMACHINE_IDENT(PsychicDominator)

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;

	virtual bool Save(PhobosStreamWriter& Stm) const override;

protected:
	int Deferment;
};

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
		: SWStateMachine(),
		Deferment(0),
		Status(IonCannonStatus::FirstAnim),
		Owner(nullptr),
		Anim(nullptr),
		Firer(nullptr)
	{
	}

	IonCannonStateMachine(CellStruct XY, SuperClass* pSuper,TechnoClass* pFirer , SWTypeHandler* pSWType)
		: SWStateMachine(MAXINT32, XY, pSuper, pSWType),
		Deferment(0) ,
		Status(IonCannonStatus::FirstAnim) ,
		Owner (pSuper->Owner) ,
		Anim(nullptr) ,
		Firer(pFirer)
	{
		// the initial deferment
		SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
		this->Deferment = pData->SW_Deferment.Get(0);
	};

	~IonCannonStateMachine()
	{
		if (this->Anim)
		{
			this->Anim->TimeToDie = true;
			this->Anim->UnInit();
			this->Anim = nullptr;
		}
	}

	virtual void Update() override;

	DEFINE_SWSTATEMACHINE_IDENT(IonCannon)

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	void Fire();
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

protected:
	int Deferment;
	IonCannonStatus Status;
	HouseClass* Owner;
	AnimClass* Anim;
	TechnoClass* Firer;
};

class CloneableLighningStormStateMachine : public SWStateMachine
{
public:

	CloneableLighningStormStateMachine()
		: SWStateMachine(), ActualDuration(0), StartTime(0), Deferment(0), IsActive(false), TimeToEnd(false) , Invoker(nullptr)
	{
	}

	CloneableLighningStormStateMachine(int Duration, int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pFirer, SWTypeHandler* pSWType)
		: SWStateMachine(Duration, XY, pSuper, pSWType), ActualDuration(0), StartTime(0), Deferment(0), IsActive(false), TimeToEnd(false) , Invoker(pFirer)
	{
		Start(XY, Duration, Deferment);
	}

	~CloneableLighningStormStateMachine()
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
	HelperedVector<AnimClass*> CloudsPresent;
	HelperedVector<AnimClass*> CloudsManifest;
	HelperedVector<AnimClass*> BoltsPresent;

	int ActualDuration;
	int StartTime; //storing current frame
	int Deferment;
	bool IsActive;
	bool TimeToEnd;
	TechnoClass* Invoker;
};

class LaserStrikeStateMachine : public SWStateMachine
{
public:

	LaserStrikeStateMachine()
		: SWStateMachine()
		, Firer { nullptr }
		, LaserStrikesetRadius { true }
		, LaserStrikeRadius { -1 }
		, LaserStrikeStartAngle { -180 }
		, LaserStrikeStop { false }
		, LaserStrikeRate { -1 }
		, LaserStrikeROF { 0 }
		, LaserStrikeRadiusReduce { 0 }
		, LaserStrikeAngle { 0 }
		, LaserStrikeScatter_Max { 0 }
		, LaserStrikeScatter_Min { 0 }
		, LaserStrikeDuration { 0 }
		, AlreadyActivated { false }
		, Deferment { -1 }
		, MaxCount { 1 }
		, MaxCountCounter { 1 }
	{ }

	LaserStrikeStateMachine(CellStruct XY, SuperClass* pSuper, TechnoClass* pFirer, int maxcount , int deferment , SWTypeHandler* pSWType, int duration)
		: SWStateMachine(duration, XY, pSuper, pSWType)
		, Firer { pFirer }
		, LaserStrikesetRadius { true }
		, LaserStrikeRadius { -1 }
		, LaserStrikeStartAngle { -180 }
		, LaserStrikeStop { false }
		, LaserStrikeRate { -1 }
		, LaserStrikeROF { 0 }
		, LaserStrikeRadiusReduce { 0 }
		, LaserStrikeAngle { 0 }
		, LaserStrikeScatter_Max { 0 }
		, LaserStrikeScatter_Min { 0 }
		, LaserStrikeDuration { 0 }
		, AlreadyActivated { false }
		, Deferment { deferment }
		, MaxCount { Math::abs(maxcount) }
		, MaxCountCounter { Math::abs(maxcount) }
	{ }

	virtual void Update() override;

	DEFINE_SWSTATEMACHINE_IDENT(LaserStrike)

	virtual bool Finished() override { return SWStateMachine::Finished() || MaxCountCounter <= 0; }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

protected:
	TechnoClass* Firer;
	bool LaserStrikesetRadius;
	int LaserStrikeRadius;
	int LaserStrikeStartAngle;
	bool LaserStrikeStop;
	int LaserStrikeRate;
	int LaserStrikeROF;
	int LaserStrikeRadiusReduce;
	int LaserStrikeAngle;
	int LaserStrikeScatter_Max;
	int LaserStrikeScatter_Min;
	int LaserStrikeDuration;
	bool AlreadyActivated;
	int Deferment;
	int MaxCount;
	int MaxCountCounter;
};

class GenericWarheadStateMachine : public SWStateMachine
{
public:
	GenericWarheadStateMachine()
		: SWStateMachine() , Firer { nullptr }
	{
	}

	explicit GenericWarheadStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer ,  SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType), Firer { pfirer }
	{
	}

	virtual void Update() override;

	DEFINE_SWSTATEMACHINE_IDENT(GenericWarhead)

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

	static void SentPayload(TechnoClass* pFirer ,SuperClass* pSuper , SWTypeExtData* pData , SWTypeHandler* pNewType, const CellStruct& loc);

protected:
	TechnoClass* Firer;
};

class GeneticMutatorStateMachine : public SWStateMachine
{
public:
	GeneticMutatorStateMachine()
		: SWStateMachine(), Firer { nullptr }, CoordsWithBridge {}
	{
	}

	explicit GeneticMutatorStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType), Firer { pfirer }, CoordsWithBridge {}
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

	static void ApplyGeneticMutator(TechnoClass* pFirer, SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, CoordStruct& coord, const CellStruct& loc, WarheadTypeClass* pWarhead, SWRange& range , int damage);

protected:
	TechnoClass* Firer;
	CoordStruct CoordsWithBridge;
};

class RevealStateMachine : public SWStateMachine
{
public:
	RevealStateMachine()
		: SWStateMachine()
	{
	}

	RevealStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
	{
	}

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { };

	DEFINE_SWSTATEMACHINE_IDENT(Reveal)
};

class ParaDropStateMachine : public SWStateMachine
{
public:
	ParaDropStateMachine()
		: SWStateMachine(), Target { nullptr }, PlaneType { }, Types {}, Nums {}
	{
	}

	ParaDropStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType ,CellClass* pTarget)
		: SWStateMachine(Deferment, XY, pSuper, pSWType), Target { pTarget }, PlaneType { }, Types {}, Nums {}
	{
		this->UpdateProperties();
	}

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { };

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
	CellClass* Target;
	std::vector<AircraftTypeClass*> PlaneType;
	std::vector<std::vector<TechnoTypeClass*>> Types;
	std::vector<std::vector<int>> Nums;
};

class ProtectStateMachine : public SWStateMachine
{
public:
	ProtectStateMachine()
		: SWStateMachine()
	{ }

	ProtectStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
	{ }

	virtual void Update() override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override { };

	DEFINE_SWSTATEMACHINE_IDENT(Protect)

};

class MeteorShowerStateMachine : public SWStateMachine
{
public:
	MeteorShowerStateMachine()
		: SWStateMachine(), Firer { nullptr }
	{ }

	explicit MeteorShowerStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer, SWTypeHandler* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType), Firer { pfirer }
	{ }

	virtual void Update() override;

	DEFINE_SWSTATEMACHINE_IDENT(MeteorShower)

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

	static void SentMeteorShower(TechnoClass* pFirer, SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, const CellStruct& loc);

protected:
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