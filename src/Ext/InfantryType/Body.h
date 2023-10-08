#pragma once
#include <InfantryTypeClass.h>

#include <Ext/TechnoType/Body.h>

struct Phobos_DoControls
{
	static void ReadSequence(std::vector<DoInfoStruct>& Desig, InfantryTypeClass* pInf, CCINIClass* pINI);
};

class InfantryTypeExt
{
public:
	class ExtData final : public Extension<InfantryTypeClass>
	{
	public:
		static constexpr size_t Canary = 0xAAAAACCA;
		using base_type = InfantryTypeClass;
		static constexpr size_t ExtOffset = 0xECC;

	public:
		TechnoTypeExt::ExtData* Type { nullptr };
		Valueable<bool> Is_Deso { false };
		Valueable<bool> Is_Cow { false };
		Nullable<double> C4Delay {};
		Nullable<int> C4ROF {};
		Nullable<int> C4Damage {};
		Nullable<WarheadTypeClass*> C4Warhead {};

		Valueable<bool> HideWhenDeployAnimPresent { false };
		Valueable<bool> DeathBodies_UseDieSequenceAsIndex { false };
		WeaponStruct CrawlingWeaponDatas[4] {};
		std::vector<DoInfoStruct> Sequences{};

		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr) ;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
		void Initialize();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<InfantryTypeExt::ExtData>
	{
	public:
		CONSTEXPR_NOCOPY_CLASS(InfantryTypeExt::ExtData, "InfantryTypeClass");
	};

	static ExtContainer ExtMap;
};
