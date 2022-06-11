#pragma once

#include <Utilities/Constructs.h>
#include <GeneralStructures.h>
#include "BaseType.h"

enum class MyType : int
{
	None = 0,
	Anim = 1,
	Stats = 2,
	AutoWeapon = 3,
	BlackHole = 4,
	DestroySelf = 5,
	DisableWapon = 6,
	FireSuper = 7,
	GiftBox = 8,
	OverrideWeapon = 9,
	PaintBall = 10,
	Stand = 11,
	Transform = 12
};

struct EffectIdent
{
	virtual MyType WhatAmI() const { return MyType::None; }
};

class ObjectClass;
class TechnoClass;
class TemporalClass;
class WarheadTypeClass;
class HouseClass;
struct AttachEffectManager;
class EffectsBase : public EffectIdent
{
public:

	EffectsBase(MyType nDecidedType) :
		Type { nDecidedType }
		, OwnerAEM { nullptr }
	{ }

	EffectsBase() :
		Type { MyType::None }
		, OwnerAEM { nullptr }
	{ }

	virtual MyType WhatAmI() const override{
		return Type;
	}

	virtual bool IsAlive() { return true; };
	virtual void Enable(ObjectClass* pOwner, HouseClass* pHouse, TechnoClass* pAttacker)RX;
	virtual void Disable(CoordStruct location)RX;
	virtual void ResetDuration()RX;
	virtual void OnUpdate(ObjectClass* pOwner, bool isDead)RX;
	virtual void OnTemporalUpdate(TechnoClass* ext, TemporalClass* pTemporal)RX;
	virtual void OnPut(ObjectClass* pOwner, CoordStruct* pCoord, DirStruct faceDir)RX;
	virtual void OnRemove(ObjectClass* pOwner)RX;
	virtual void OnReceiveDamage(ObjectClass* pOwner, int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH, ObjectClass* pAttacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse)RX;
	virtual void OnDestroy(ObjectClass* pOwner)RX;
	virtual void OnGuardCommand()RX;
	virtual void OnStopCommand()RX;

	void SetAEManager(ObjectClass* pOwner);

	AttachEffectManager* OwnerAEM;

protected :
	MyType Type;
};

template<typename T>
concept IsEffectType = std::is_base_of<EffectType, T>::value;

template <IsEffectType Eff>
struct Effect : public EffectsBase
{
	Eff* TypeData;


	Effect() : EffectsBase { }
		, TypeData { nullptr }
	{}

	Effect(MyType nDecidedType) : EffectsBase { nDecidedType }
		, TypeData { nullptr }
	{}

	void SetTypeData(Eff Type) {
		if (!TypeData) {
			TypeData = (GameCreate<Eff>(Type));
		}
	}

	virtual void Enable(ObjectClass* pOwner, HouseClass* pHouse, TechnoClass* pAttacker) override
	{
		SetAEManager(pOwner);
		OnEnable(pOwner, pHouse, pAttacker);
	}

	virtual void OnEnable(ObjectClass* pOwner, HouseClass* pHouse, TechnoClass* pAttacker) RX;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::Log("Processing Element From Effect ! \n");
		return Stm
			.Process(TypeData)
			.Process(OwnerAEM)
			.Success()
			;
	}
};

/*
enum class StateType : int
{
	None = 0,
	DestroySelf = 1,
	GiftBox = 2,
	OverrideWeapon = 3,
	PaintBall = 4,
};

struct StateIdent
{
	virtual StateType WhatAmI() const { return StateType::None; }
};

template <IsEffectType T>
struct StateBase : public StateIdent
{
	StateBase(StateType nDecidedType) :
		SType { nDecidedType }
	{ }

	StateBase() :
		SType { StateType::None }
	{ }

	virtual StateType WhatAmI() const override
	{ return SType; }

	virtual void Enable(int duration, const char* token, T* data) RX
	virtual void Disable(const char* token) RX
	virtual bool IsActive() { return false; }

protected:
	StateType SType;
};

template <IsEffectType T>
struct State : public StateBase<T> , public Effect<T>
{
	const char* Token;
	AttachEffectType* AEType;
protected:
	bool active;
	bool infinite;
	TimerStruct timer;
public:

	State() :
		  SType { StateType::None }
		, Effect<T> { }
		, Token { nullptr }
		, AEType { nullptr }
		, active { false }
		, infinite { false }
		, timer { }
	{ }

	State(StateType nDecidedType , MyType nDecidedType) :
		 SType { nDecidedType }
		, Effect<T> { nDecidedType }
		, Token { nullptr }
		, AEType { nullptr }
		, active { false }
		, infinite { false }
		, timer { }
	{ }

	void SetAEType(AttachEffectType* pType) {
		AEType = pType;
	}

	void Enable(int duration, const char* token, T* data) override
	{
		Token = token;
		Data = data;
		active = duration != 0;
		if (duration < 0) {
			infinite = true;
			timer.Start(0);
		} else {
			infinite = false;
			timer.Start(duration);
		}

		OnEnable();
	}

	virtual void OnEnable() RX

	void Disable(const char* token) override
	{
		if (std::strcmp(Token, token) == 0)
		{
			active = false;
			infinite = false;
			timer.stop();

			OnDisable();
		}
	}

	virtual void OnDisable() RX

	bool IsActive() override
	{
		return infinite || timer.InProgress();
	}
};*/