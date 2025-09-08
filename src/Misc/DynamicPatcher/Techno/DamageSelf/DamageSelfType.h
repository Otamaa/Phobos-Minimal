#pragma once
#include <Utilities/TemplateDef.h>

class WarheadTypeClass;
class DamageSelfType
{
public:

	bool Enable { false };
	bool DeactiveWhenCivilian { true };
	int Damage { 1 };
	int ROF { 0 };
	WarheadTypeClass* Warhead { nullptr };
	bool PlayWarheadAnim { false };
	bool IgnoreArmor { true };
	bool Decloak { true };
	KillMethod Type { KillMethod::Explode };

	/*
	DamageSelfType() : Enable { false }
		, DeactiveWhenCivilian { true }
		, Damage { 1 }
		, ROF { 0 }
		, Warhead { nullptr }
		, PlayWarheadAnim { false }
		, IgnoreArmor { true }
		, Decloak { true }
		, Type { KillMethod::Explode }
	{}

	~DamageSelfType() = default;

	DamageSelfType(const DamageSelfType& nData) : Enable { nData.Enable }
		, DeactiveWhenCivilian { nData.DeactiveWhenCivilian }
		, Damage { nData.Damage }
		, ROF { nData.ROF }
		, Warhead { nData.Warhead }
		, PlayWarheadAnim { nData.PlayWarheadAnim }
		, IgnoreArmor { nData.IgnoreArmor }
		, Decloak { nData.Decloak }
		, Type { nData.Type }
	{}

	DamageSelfType(DamageSelfType& nData) : Enable { nData.Enable }
		, DeactiveWhenCivilian { nData.DeactiveWhenCivilian }
		, Damage { nData.Damage }
		, ROF { nData.ROF }
		, Warhead { nData.Warhead }
		, PlayWarheadAnim { nData.PlayWarheadAnim }
		, IgnoreArmor { nData.IgnoreArmor }
		, Decloak { nData.Decloak }
		, Type { nData.Type }
	{}
	*/

	void Read(INI_EX& parser, const char* pSection);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<DamageSelfType*>(this)->Serialize(Stm); }

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		//Debug::LogInfo("Processing Element From DamageSelfType ! ");

		return Stm
			.Process(Enable)
			.Process(DeactiveWhenCivilian)
			.Process(Damage)
			.Process(ROF)
			.Process(Warhead)
			.Process(PlayWarheadAnim)
			.Process(IgnoreArmor)
			.Process(Decloak)
			.Process(Type)
			.Success()
			//&& Stm.RegisterChange(this)
			;
	}
};

class DamageSelfState
{
public:

	virtual ~DamageSelfState() = default;

	virtual bool IsActive() { return DelayTimer.InProgress(); }

	void Reset()
	{
		Hit = false;
		delay = Data->ROF;
		if (delay > 0)
		{
			DelayTimer.Start(delay);
		}
	}

	bool CanHitSelf()
	{
		return IsActive() && !Hit && Timeup();
	}

	bool Timeup()
	{
		Hit = delay <= 0 || DelayTimer.Expired();
		return Hit;
	}


	DamageSelfState() : Hit { }
		, delay { }
		, Data { }
		, DelayTimer { }
	{ }

	DamageSelfState(int Delay , const DamageSelfType& DData) : Hit { false }
		, delay { Delay }
		, Data { }
		, DelayTimer { }
	{
		Data = std::make_unique<DamageSelfType>(DData);
	}

	DamageSelfState(const DamageSelfState& other) = default;
	DamageSelfState& operator=(const DamageSelfState& other) = default;

	static void OnPut(std::unique_ptr<DamageSelfState>& pState, const DamageSelfType& DData);

	int GetRealDamage(ObjectClass* pObj, int damage, bool ignoreArmor, WarheadTypeClass* pWH);

	void PlayWHAnim(ObjectClass* pObj, int realDamage, WarheadTypeClass* pWH);

	void TechnoClass_Update_DamageSelf(TechnoClass* pTechno);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<DamageSelfState*>(this)->Serialize(Stm); }


	bool Hit;
	int delay;
	std::unique_ptr<DamageSelfType> Data;

private:
	CDTimerClass DelayTimer;
public:

	template <typename T>
	bool Serialize(T& Stm)
	{
	//	Debug::LogInfo("Processing Element From DamageSelfState ! ");
		return Stm
			.Process(Hit)
			.Process(delay)
			.Process(Data)
			.Process(DelayTimer)
			.Success()
			//&& Stm.RegisterChange(this)
			;
	}
};

template <>
struct Savegame::ObjectFactory<DamageSelfType>
{
	std::unique_ptr<DamageSelfType> operator() (PhobosStreamReader& Stm) const
	{
		return std::make_unique<DamageSelfType>();
	}
};
