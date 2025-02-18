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
		Debug::LogInfo("Processing Element From Effect ! ");
		return Stm
			.Process(TypeData)
			.Process(OwnerAEM)
			.Success()
			;
	}
};