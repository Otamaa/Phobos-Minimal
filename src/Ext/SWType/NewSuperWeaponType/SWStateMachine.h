#pragma once

#include <Ext/SWType/Body.h>
#include "NewSWType.h"
#include <ProgressTimer.h>

enum class SWStateMachineIdentifier : int
{
	Invalid = -1,
	UnitDelivery = 0,
	ChronoWarp = 1,
	PsychicDominator = 2,
	EMPulse = 3
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

	void PlaceUnits();
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

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;

	virtual bool Save(PhobosStreamWriter& Stm) const override;

protected:
	int Deferment;
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
			default:
				Debug::FatalErrorAndExit("SWStateMachineType %d not recognized.",
					static_cast<unsigned int>(type));
			}
		}

		return nullptr;
	}
};