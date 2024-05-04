#pragma once

#include <New/AnonymousType/AresAttachEffectTypeClass.h>
#include <CoordStruct.h>
#include <Utilities/GameUniquePointers.h>

struct AresAEData;
class WarheadTypeClass;
struct AresAE
{
public:
	AresAttachEffectTypeClass* Type { nullptr };
	Handle<AnimClass*, UninitAnim> Anim { nullptr };
	int Duration { 0 };
	HouseClass* Invoker { nullptr };

	void InvalidatePointer(AnimClass* ptr, bool bDetach)
	{
		if (ptr == this->Anim.get()) {
			this->Anim.release();
		}
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AresAE*>(this)->Serialize(Stm);
	}

	void ClearAnim();
	void ReplaceAnim(TechnoClass* pTechno, AnimClass* pNewAnim);
	void CreateAnim(TechnoClass* pTechno);

	static void UpdateTempoal(AresAEData* ae, TechnoClass* pTechno);
	static void Update(AresAEData* ae, TechnoClass* pTechno);
	static bool Remove(AresAEData* ae);
	static void Remove(AresAEData* ae, TechnoClass* pTechno);
	static void RemoveSpecific(AresAEData* ae, TechnoClass* pTechno, AbstractTypeClass* pRemove);
	static bool Attach(AresAttachEffectTypeClass* pType, TechnoClass* pTargetTechno, int duration, HouseClass* pInvokerOwner);
	static void TransferAttachedEffects(TechnoClass* From, TechnoClass* To);
	static void RecalculateStat(AresAEData* ae, TechnoClass* pThis);
	static void applyAttachedEffect(WarheadTypeClass* pWH, const CoordStruct& coords, HouseClass* Source);

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Type)
			.Process(Anim)
			.Process(Duration)
			.Process(Invoker)
			.Success()
			&& Stm.RegisterChange(this) // announce this type
			;
	}

};

struct AresAEData
{
	HelperedVector<AresAE> Data {};
	int InitialDelay { 0 };
	BYTE NeedToRecreateAnim {};
	BYTE Isset {};

	void InvalidatePointer(AnimClass* ptr, bool bDetach)
	{
		for (auto& ae_ : Data)
			ae_.InvalidatePointer(ptr, bDetach);
	}

	~AresAEData() = default;
	AresAEData() = default;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AresAEData*>(this)->Serialize(Stm);
	}

private:
	AresAEData(const AresAEData& other)=delete;
	AresAEData&operator=(const AresAEData& other)=delete;
	AresAEData&operator=(AresAEData&&)=delete;

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Data)
			.Process(InitialDelay)
			.Process(NeedToRecreateAnim)
			.Process(Isset)
			.Success()
			//&& Stm.RegisterChange(this)
			; // announce this type
	}
};
