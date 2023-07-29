#pragma once

#include <Ext/SWType/Body.h>
#include "NewSWType.h"
#include <ProgressTimer.h>

#include <Ext/WeaponType/Body.h>
#include <New/Entity/ElectricBoltClass.h>

enum class SWStateMachineIdentifier : int
{
	Invalid = -1,
	UnitDelivery = 0,
	ChronoWarp = 1,
	PsychicDominator = 2,
	CloneableLighningStorm = 3 ,
	DropPod = 4,
	IonCannon = 5,
	LaserStrike = 6 ,
	count
};

// state machines - create one to use delayed effects [create a child class per NewSWType, obviously]
// i.e. start anim/sound 1 frame after clicking, fire a damage wave 25 frames later, and play second sound 50 frames after that...
class SWStateMachine
{
	static std::vector<std::unique_ptr<SWStateMachine>> Array;

public:
	SWStateMachine()
		: Type(nullptr), Super(nullptr), Coords(), Clock()
	{
	}

	SWStateMachine(int Duration, CellStruct XY, SuperClass* pSuper, NewSWType* pSWType)
		: Type(pSWType), Super(pSuper), Coords(XY)
	{
		Clock.Start(Duration);
	}

	virtual ~SWStateMachine() { }
	virtual bool Finished() { return Clock.Completed(); }
	virtual void Update() { }
	virtual void InvalidatePointer(void* ptr, bool remove) { }
	virtual SWStateMachineIdentifier GetIdentifier() const = 0;
	virtual const char* GetIdentifierStrings() const = 0;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

	int TimePassed()
	{
		return Unsorted::CurrentFrame - Clock.StartTime;
	}

	// static methods
	static void Register(std::unique_ptr<SWStateMachine> Machine)
	{
		if (Machine) {
			Array.push_back(std::move(Machine));
		}
	}

	SWTypeExt::ExtData * GetTypeExtData() {
		return SWTypeExt::ExtMap.Find(Super->Type);
	}

public:

	static void UpdateAll();
	static void PointerGotInvalid(void* ptr, bool remove);
	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

protected:
	NewSWType* Type;
	SuperClass* Super;
	CellStruct Coords;
	CDTimerClass Clock;
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
		: SWStateMachine() , Deferment { 0 } , AlreadyActivated { false }
	{
	}

	DroppodStateMachine(int Duration, int Deferment, CellStruct XY, SuperClass* pSuper, NewSWType* pSWType)
		: SWStateMachine(Duration, XY, pSuper, pSWType), Deferment { 0 }, AlreadyActivated { false }
	{
		Activate(Deferment);
	}

	virtual void Update();

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual SWStateMachineIdentifier GetIdentifier() const override
	{
		return SWStateMachineIdentifier::DropPod;
	}

	virtual const char* GetIdentifierStrings() const override
	{
		return "SWStateMachine::Droppod";
	}
	void SendDroppods();
	void Activate(int nDeferment);

	static void PlaceUnits(SuperClass* pSuper, double veterancy, Iterator<TechnoTypeClass*> const Types, int cMin, int cMax, const CellStruct& Coords ,bool retries);

protected:
	int Deferment;
	bool AlreadyActivated;
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

		ChronoWarpContainer(BuildingClass* pBld, const CellStruct& target, const CoordStruct& origin, bool isVehicle) :
			building(pBld),
			target(target),
			origin(origin),
			isVehicle(isVehicle)
		{
		}

		ChronoWarpContainer() = default;

		bool operator == (const ChronoWarpContainer& other) const
		{
			return this->building == other.building;
		}
	};

	ChronoWarpStateMachine()
		: SWStateMachine(), Buildings(), Duration(0)
	{
	}

	ChronoWarpStateMachine(int Duration, const CellStruct& XY, SuperClass* pSuper, NewSWType* pSWType, DynamicVectorClass<ChronoWarpContainer> Buildings)
		: SWStateMachine(Duration, XY, pSuper, pSWType), Buildings(std::move(Buildings)), Duration(Duration)
	{
	}

	virtual void Update();

	virtual void InvalidatePointer(void* ptr, bool remove);

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
	DynamicVectorClass<ChronoWarpContainer> Buildings;
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
		SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSuper->Type);
		this->Deferment = pData->SW_Deferment.Get(0);

		// make the game happy
		PsyDom::Owner = pSuper->Owner;
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
		SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSuper->Type);
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
		}
	}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	void Fire();
	virtual void InvalidatePointer(void* ptr, bool remove) override;

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
	static constexpr double CloudHeightFactor { 6.968466256176567 };

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

	virtual void InvalidatePointer(void* ptr, bool remove) override;

	virtual ~CloneableLighningStormStateMachine()
	{
		for (auto& CP : CloudsPresent)
		{
			if (CP)
			{
				CP->UnInit();
				CP = nullptr;
			}
		}

		CloudsPresent.Clear();

		for (auto& CM : CloudsManifest)
		{
			if (CM)
			{
				CM->UnInit();
				CM = nullptr;
			}
		}

		CloudsManifest.Clear();

		for (auto& BP : BoltsPresent)
		{
			if (BP)
			{
				BP->UnInit();
				BP = nullptr;
			}
		}

		BoltsPresent.Clear();
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
	DynamicVectorClass<AnimClass*> CloudsPresent;
	DynamicVectorClass<AnimClass*> CloudsManifest;
	DynamicVectorClass<AnimClass*> BoltsPresent;

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

	LaserStrikeStateMachine(CellStruct XY, SuperClass* pSuper, TechnoClass* pFirer, int maxcount , int deferment , NewSWType* pSWType)
		: SWStateMachine(10000, XY, pSuper, pSWType)
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
		, MaxCount { abs(maxcount) }
		, MaxCountCounter { abs(maxcount) }
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
	virtual void InvalidatePointer(void* ptr, bool remove) override;

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
			case SWStateMachineIdentifier::UnitDelivery:
				return std::make_unique<UnitDeliveryStateMachine>();
			case SWStateMachineIdentifier::ChronoWarp:
				return std::make_unique<ChronoWarpStateMachine>();
			case SWStateMachineIdentifier::PsychicDominator:
				return std::make_unique<PsychicDominatorStateMachine>();
			case SWStateMachineIdentifier::CloneableLighningStorm:
				return std::make_unique<CloneableLighningStormStateMachine>();
			case SWStateMachineIdentifier::DropPod:
				return std::make_unique<DroppodStateMachine>();
			case SWStateMachineIdentifier::IonCannon:
				return std::make_unique<IonCannonStateMachine>();
			case SWStateMachineIdentifier::LaserStrike:
				return std::make_unique<LaserStrikeStateMachine>();
			default:
				Debug::FatalErrorAndExit("SWStateMachineType %d not recognized.",
					static_cast<unsigned int>(type));
			}
		}

		return nullptr;
	}
};