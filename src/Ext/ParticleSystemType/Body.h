#pragma once
#include <ParticleSystemTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ParticleSystemTypeExtData final
{
public:
	static constexpr size_t Canary = 0xEAAEEEEE;
	using base_type = ParticleSystemTypeClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	Valueable<bool> ApplyOptimization { true };
	std::array<Point2D, (size_t)FacingType::Count> FacingMult {};
	Valueable<bool> AdjustTargetCoordsOnRotation { true };

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(ParticleSystemTypeExtData) -
			(4u //AttachedToObject
			 );
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleSystemTypeExtContainer final : public Container<ParticleSystemTypeExtData>
{
public:
	static ParticleSystemTypeExtContainer Instance;

	//CONSTEXPR_NOCOPY_CLASSB(ParticleSystemTypeExtContainer, ParticleSystemTypeExtData, "ParticleSystemTypeClass");
};

class FakeParticleSystemTypeClass : public ParticleSystemTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	ParticleSystemTypeExtData* _GetExtData() {
		return *reinterpret_cast<ParticleSystemTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeParticleSystemTypeClass) == sizeof(ParticleSystemTypeClass), "Invalid Size !");