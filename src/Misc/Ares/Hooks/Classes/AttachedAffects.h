#pragma once

#include <New/AnonymousType/AresAttachEffectTypeClass.h>
#include <CoordStruct.h>
#include <Utilities/GameUniquePointers.h>
#include <AnimClass.h>
#include <Utilities/Handle.h>
#include <Utilities/VectorHelper.h>
#include <Utilities/GameObjectWeakPtr.h>

struct AresAEData;
class WarheadTypeClass;
struct AresAE
{
public:
	AresAttachEffectTypeClass* Type { nullptr };
	Handle<AnimClass*, MarkForDeathDeleter<AnimClass>> Anim { nullptr };
	int Duration { 0 };
	HouseClass* Invoker { nullptr };

	AresAE(const AresAE&) = delete;
	AresAE& operator=(const AresAE&) = delete;
	~AresAE() = default;
	AresAE() = default;

	AresAE(AresAE&& other) noexcept
		: Type(other.Type),
		Anim(std::move(other.Anim)),
		Duration(other.Duration),
		Invoker(other.Invoker) {
		other.Type = nullptr;
		other.Duration = 0;
		other.Invoker = nullptr;
	}

	AresAE& operator=(AresAE&& other) noexcept {
		if (this != &other) {

			// release our current anim safely
			ClearAnim();

			Type = other.Type;
			Anim = std::move(other.Anim);
			Duration = other.Duration;
			Invoker = other.Invoker;

			other.Type = nullptr;
			other.Duration = 0;
			other.Invoker = nullptr;
		}
		return *this;
	}

	void OPTIONALINLINE InvalidateAnimPointer(AnimClass* ptr) {
		if (this->Anim && ptr == this->Anim.get()) {
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
	static void applyAttachedEffect(WarheadTypeClass* pWH, const CoordStruct& coords, HouseClass* Source);

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Type, true)
			.Process(Anim, true)
			.Process(Duration)
			.Process(Invoker, true)
			.Success()
			;
	}

};

struct AresAEData
{
	HelperedVector<AresAE> Data {};
	int InitialDelay { 0 };
	BYTE NeedToRecreateAnim {};
	BYTE Isset {};

	void OPTIONALINLINE InvalidateAnimPointer(AnimClass* ptr)
	{
		for (auto& ae_ : Data) {
			ae_.InvalidateAnimPointer(ptr);
		}

	}

	AresAEData() = default;
	AresAEData(AresAEData&&) = delete;
	AresAEData& operator=(AresAEData&&) = delete;
	AresAEData(const AresAEData&) = delete;
	AresAEData& operator=(const AresAEData&) = delete;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AresAEData*>(this)->Serialize(Stm);
	}

private:
	//AresAEData(const AresAEData& other)=delete;
	//AresAEData&operator=(const AresAEData& other)=delete;
	//AresAEData&operator=(AresAEData&&)=delete;


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
