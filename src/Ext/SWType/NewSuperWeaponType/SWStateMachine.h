#pragma once

#include "NewSWType.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/House/Body.h>

#include <ProgressTimer.h>

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
	count
};

// state machines - create one to use delayed effects [create a child class per NewSWType, obviously]
// i.e. start anim/sound 1 frame after clicking, fire a damage wave 25 frames later, and play second sound 50 frames after that...
class SWStateMachine
{
	OPTIONALINLINE static HelperedVector<std::unique_ptr<SWStateMachine>> Array;

public:

	SWStateMachine() = default;
	SWStateMachine(int Duration, CellStruct XY, SuperClass* pSuper, NewSWType* pSWType)
		: Type(pSWType), Super(pSuper), Coords(XY)
	{
		Clock.Start(Duration);
	}

	virtual ~SWStateMachine() = default;

	virtual bool Finished() { return Clock.Completed(); }
	virtual void Update() { }
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) { }
	virtual SWStateMachineIdentifier GetIdentifier() const = 0;
	virtual const char* GetIdentifierStrings() const = 0;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

	COMPILETIMEEVAL OPTIONALINLINE int TimePassed() const {
		return Unsorted::CurrentFrame - Clock.StartTime;
	}

	// static methods
	static OPTIONALINLINE
#if _HAS_CXX23 == 1
		COMPILETIMEEVAL
#endif
	SWStateMachine* Register(std::unique_ptr<SWStateMachine> Machine) {
		if (Machine) {
			Array.push_back(std::move(Machine));
			return Array.back().get();
		}

		return nullptr;
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
	NewSWType* Type;
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

	UnitDeliveryStateMachine(int Duration, CellStruct XY, SuperClass* pSuper, NewSWType* pSWType)
		: SWStateMachine(Duration, XY, pSuper, pSWType)
	{
	}

	virtual void Update();

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::UnitDelivery;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::UnitDelivery";
	}

	void PlaceUnits();
};

class DroppodStateMachine : public SWStateMachine
{
public:
	DroppodStateMachine()
		: SWStateMachine()
	{
	}

	DroppodStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, NewSWType* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
	{
	}

	virtual void Update() override;

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::Droppod;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::Droppod";
	}

	static void SendDroppods(SuperClass* pSuper, SWTypeExtData* pData, NewSWType* pNewType, const CellStruct& loc);
	static void PlaceUnits(SuperClass* pSuper, double veterancy, Iterator<TechnoTypeClass*> const Types, int cMin, int cMax, const CellStruct& Coords ,bool retries);

};

class SonarPulseStateMachine : public SWStateMachine
{
public:
	SonarPulseStateMachine()
		: SWStateMachine()
	{
	}

	SonarPulseStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, NewSWType* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
	{
	}

	virtual void Update() override;

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::SonarPulse;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::SonarPulse";
	}

	static void SendSonarPulse(SuperClass* pSuper, SWTypeExtData* pData, NewSWType* pNewType, const CellStruct& loc);
	static void ApplySonarPulse(SuperClass* pSuper, const CellStruct& Coords , const SWRange& range);

};

class SpyPlaneStateMachine : public SWStateMachine
{
public:
	SpyPlaneStateMachine()
		: SWStateMachine(), target { nullptr }
	{
	}

	SpyPlaneStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, NewSWType* pSWType , CellClass* pTarget)
		: SWStateMachine(Deferment, XY, pSuper, pSWType) , target { pTarget }
	{
	}

	virtual void Update() override;

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::SpyPlane;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::SpyPlane";
	}

	static void SendSpyPlane(SuperClass* pSuper, SWTypeExtData* pData, NewSWType* pNewType, CellClass* target);

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
		{
		}

		COMPILETIMEEVAL ChronoWarpContainer() = default;

		COMPILETIMEEVAL bool operator == (const ChronoWarpContainer& other) const {
			return this->building == other.building;
		}

		COMPILETIMEEVAL ~ChronoWarpContainer() = default;
	};

	ChronoWarpStateMachine()
		: SWStateMachine(), Buildings(), Duration(0)
	{
	}

	ChronoWarpStateMachine(int Duration, const CellStruct& XY, SuperClass* pSuper, NewSWType* pSWType, HelperedVector<ChronoWarpContainer> Buildings)
		: SWStateMachine(Duration, XY, pSuper, pSWType), Buildings(std::move(Buildings)), Duration(Duration)
	{
	}

	virtual void Update();

	virtual void InvalidatePointer(AbstractClass* ptr, bool remove);

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::ChronoWarp;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::ChronoWarp";
	}

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

	PsychicDominatorStateMachine(CellStruct XY, SuperClass* pSuper, NewSWType* pSWType)
		: SWStateMachine(MAXINT32, XY, pSuper, pSWType), Deferment(0)
	{
		PsyDom::Status = PsychicDominatorStatus::FirstAnim;

		// the initial deferment
		SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
		this->Deferment = pData->SW_Deferment.Get(0);

		// make the game happy
		if (!pSuper->Owner) {
			Debug::Log("Psydom[%s] Firing Without Ownership!\n", pSuper->Type->ID);
			PsyDom::Owner = HouseExtData::FindSpecial();
		}else{
			PsyDom::Owner = pSuper->Owner;
		}

		PsyDom::Coords = XY;
		PsyDom::Anim = nullptr;
	};

	virtual void Update();

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::PsychicDominator;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::PsyhicDominator";
	}

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

	IonCannonStateMachine(CellStruct XY, SuperClass* pSuper,TechnoClass* pFirer , NewSWType* pSWType)
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

	virtual void Update();

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::IonCannon;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::IonCannon";
	}

	virtual ~IonCannonStateMachine()
	{
		if (this->Anim)
		{
			this->Anim->TimeToDie = true;
			this->Anim->UnInit();
			this->Anim = nullptr;
		}
	}

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
	static COMPILETIMEEVAL double CloudHeightFactor { 6.968466256176567 };

	CloneableLighningStormStateMachine()
		: SWStateMachine(), ActualDuration(0), StartTime(0), Deferment(0), IsActive(false), TimeToEnd(false) , Invoker(nullptr)
	{
	}

	CloneableLighningStormStateMachine(int Duration, int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pFirer, NewSWType* pSWType)
		: SWStateMachine(Duration, XY, pSuper, pSWType), ActualDuration(0), StartTime(0), Deferment(0), IsActive(false), TimeToEnd(false) , Invoker(pFirer)
	{
		Start(XY, Duration, Deferment);
	}

	virtual void Update() override;

	virtual bool Finished() override
	{
		return ActualDuration <= -1 ? false : Clock.Completed() && !Deferment && TimeToEnd;
	}

	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

	virtual ~CloneableLighningStormStateMachine()
	{
		for (auto& CP : CloudsPresent)
		{
			if (CP && CP->IsAlive && CP->Type) {
				CP->UnInit();
			}

			CP = nullptr;
		}

		for (auto& CM : CloudsManifest)
		{
			if (CM && CM->IsAlive && CM->Type) {
				CM->UnInit();
			}
			CM = nullptr;
		}

		for (auto& BP : BoltsPresent)
		{
			if (BP && BP->IsAlive && BP->Type) {
				BP->UnInit();
			}

			BP = nullptr;
		}

	}

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::CloneableLighningStorm;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::CloneableLighningStorm";
	}

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

	LaserStrikeStateMachine(CellStruct XY, SuperClass* pSuper, TechnoClass* pFirer, int maxcount , int deferment , NewSWType* pSWType, int duration)
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

	virtual void Update();

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::LaserStrike;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::LaserStrike";
	}

	virtual ~LaserStrikeStateMachine() = default;
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

	GenericWarheadStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer ,  NewSWType* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType), Firer { pfirer }
	{
	}

	virtual void Update() override;

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::GenericWarhead;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::GenericWarhead";
	}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual void InvalidatePointer(AbstractClass* ptr, bool remove) override;

	static void SentPayload(TechnoClass* pFirer ,SuperClass* pSuper , SWTypeExtData* pData , NewSWType* pNewType, const CellStruct& loc);

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

	GeneticMutatorStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, TechnoClass* pfirer, NewSWType* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType), Firer { pfirer }, CoordsWithBridge {}
	{
		this->CoordsWithBridge = MapClass::Instance->GetCellAt(XY)->GetCoordsWithBridge();
	}

	virtual void Update() override;

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::GeneticMutator;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::GeneticMutator";
	}

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

	virtual void InvalidatePointer(AbstractClass* ptr, bool remove)
	{
		AnnounceInvalidPointer(Firer, ptr, remove);
	}

	static void ApplyGeneticMutator(TechnoClass* pFirer, SuperClass* pSuper, SWTypeExtData* pData, NewSWType* pNewType, CoordStruct& coord, const CellStruct& loc, WarheadTypeClass* pWarhead, SWRange& range , int damage);

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

	RevealStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, NewSWType* pSWType)
		: SWStateMachine(Deferment, XY, pSuper, pSWType)
	{
	}

	virtual void Update() override;

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::Reveal;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::Reveal";
	}

};

class ParaDropStateMachine : public SWStateMachine
{
public:
	ParaDropStateMachine()
		: SWStateMachine(), Target { nullptr }, PlaneType { }, Types {}, Nums {}
	{
	}

	ParaDropStateMachine(int Deferment, CellStruct XY, SuperClass* pSuper, NewSWType* pSWType ,CellClass* pTarget)
		: SWStateMachine(Deferment, XY, pSuper, pSWType), Target { pTarget }, PlaneType { }, Types {}, Nums {}
	{
		this->UpdateProperties();
	}

	virtual void Update() override;
	void UpdateProperties();

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::ParaDrop;
	}

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

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::ParaDrop";
	}

protected:
	CellClass* Target;
	std::vector<AircraftTypeClass*> PlaneType;
	std::vector<Iterator<TechnoTypeClass*>> Types;
	std::vector<Iterator<int>> Nums;
};

#define MakeStatemachine(a) \
case SWStateMachineIdentifier::## a ##:\
return std::make_unique<## a ##StateMachine>();\

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
			default:
				Debug::FatalErrorAndExit("SWStateMachineType %d not recognized.",
					static_cast<unsigned int>(type));
			}
		}

		return nullptr;
	}
};
#undef MakeStatemachine