#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

class WarheadTypeClass;
class DamageSelfType
{
public:

	int Damage;
	int ROF;
	WarheadTypeClass* Warhead;
	bool PlayWarheadAnim;
	bool IgnoreArmor;
	bool Decloak;
	KillMethod Type;

	DamageSelfType() : Damage { 1 }
		, ROF { 0 }
		, Warhead { nullptr }
		, PlayWarheadAnim { false }
		, IgnoreArmor { true }
		, Decloak { true }
		, Type { KillMethod::Explode }
	{}

	virtual ~DamageSelfType() = default;

	DamageSelfType(const DamageSelfType& nData) : Damage { nData.Damage }
		, ROF { nData.ROF }
		, Warhead { nData.Warhead }
		, PlayWarheadAnim { nData.PlayWarheadAnim }
		, IgnoreArmor { nData.IgnoreArmor }
		, Decloak { nData.Decloak }
		, Type { nData.Type }
	{}

	DamageSelfType(DamageSelfType& nData) : Damage { nData.Damage }
		, ROF { nData.ROF }
		, Warhead { nData.Warhead }
		, PlayWarheadAnim { nData.PlayWarheadAnim }
		, IgnoreArmor { nData.IgnoreArmor }
		, Decloak { nData.Decloak }
		, Type { nData.Type }
	{}

	void Read(INI_EX& parser, const char* pSection);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<DamageSelfType*>(this)->Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::Log("Processing Element From DamageSelfType ! \n");

		return Stm
			.Process(Damage)
			.Process(ROF)
			.Process(Warhead)
			.Process(PlayWarheadAnim)
			.Process(IgnoreArmor)
			.Process(Decloak)
			.Process(Type)
			.Success()
			;
	}
};

class DamageSelfState
{
public:

	virtual ~DamageSelfState() = default;

	virtual void Disable(WarheadTypeClass* pAffector)
	{
		if (Token == pAffector)
		{
			DelayTimer.Stop();
		}
	}

	virtual void Disable(bool bForce)
	{
		if ((!Token) || bForce)
		{
			DelayTimer.Stop();
			Token = nullptr;
			Data.reset();
		}
	}


	virtual bool IsActive() { return DelayTimer.InProgress(); }

	void Enable(int nDuration, DamageSelfType data, WarheadTypeClass* pAffector)
	{
		Enable(nDuration, pAffector, data);
	}

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
		, Token { }
		, delay { }
		, Data { }
		, DelayTimer { }
	{ }


	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<DamageSelfState*>(this)->Serialize(Stm); }


	bool Hit;
	WarheadTypeClass* Token;
	int delay;
	std::unique_ptr<DamageSelfType> Data;

private:
	TimerStruct DelayTimer;
public:

	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::Log("Processing Element From DamageSelfState ! \n");
		return Stm
			.Process(Hit)
			.Process(Token)
			.Process(delay)
			.Process(Data)
			.Process(DelayTimer)
			.Success()
			;
	}
};
#endif
