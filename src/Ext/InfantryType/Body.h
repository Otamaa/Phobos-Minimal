#pragma once
#include <InfantryTypeClass.h>

#include <Ext/TechnoType/Body.h>

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
		TechnoTypeExt::ExtData* Type;
		Valueable<bool> Is_Deso;
		Valueable<bool> Is_Cow;
		Nullable<double> C4Delay;
		Nullable<int> C4ROF;
		Nullable<int> C4Damage;
		Nullable<WarheadTypeClass*> C4Warhead;

		Valueable<bool> HideWhenDeployAnimPresent;
		Valueable<bool> DeathBodies_UseDieSequenceAsIndex;
		WeaponStruct CrawlingWeaponDatas[4];

		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
			, Type { nullptr }
			, Is_Deso { false }
			, Is_Cow { false }
			, C4Delay {}
			, C4ROF {}
			, C4Damage {}
			, C4Warhead {}
			, HideWhenDeployAnimPresent { false }
			, DeathBodies_UseDieSequenceAsIndex { false }
			, CrawlingWeaponDatas {}
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
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
