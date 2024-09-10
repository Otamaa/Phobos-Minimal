#pragma once
#include <InfantryTypeClass.h>

#include <Ext/TechnoType/Body.h>

struct Phobos_DoControls
{
	static void ReadSequence(DoInfoStruct* Desig, InfantryTypeClass* pInf, CCINIClass* pINI);
};

class InfantryTypeExtData final
{
public:
	static constexpr size_t Canary = 0xAAAAACCA;
	using base_type = InfantryTypeClass;
	static constexpr size_t ExtOffset = 0xECC;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	TechnoTypeExtData* Type { nullptr };
	Valueable<bool> Is_Deso { false };
	Valueable<bool> Is_Cow { false };
	Nullable<double> C4Delay {};
	Nullable<int> C4ROF {};
	Nullable<int> C4Damage {};
	Nullable<WarheadTypeClass*> C4Warhead {};

	Valueable<bool> HideWhenDeployAnimPresent { false };
	Valueable<bool> DeathBodies_UseDieSequenceAsIndex { false };
	WeaponStruct CrawlingWeaponDatas[4] {};
	//std::vector<DoInfoStruct> Sequences {};

	ValueableIdxVector<VocClass> VoiceGarrison {};

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
	void Initialize();

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(InfantryTypeExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class InfantryTypeExtContainer final : public Container<InfantryTypeExtData>
{
public:
	static InfantryTypeExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(InfantryTypeExtContainer, InfantryTypeExtData, "InfantryTypeClass");
};
