#pragma once
#include <InfantryTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class InfantryTypeExt
{
public:
	static constexpr size_t Canary = 0xAAAAACCA;
	using base_type = InfantryTypeClass;
	static constexpr size_t ExtOffset = 0xECC;

	class ExtData final : public Extension<base_type>
	{
	public:

		Nullable<double> C4Delay;
		Nullable<int> C4ROF;

		Valueable<bool> HideWhenDeployAnimPresent;
		Valueable<bool> DeathBodies_UseDieSequenceAsIndex;
		WeaponStruct CrawlingWeaponDatas[4];
		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
			, C4Delay { }
			, C4ROF { }
			, HideWhenDeployAnimPresent { false }
			, DeathBodies_UseDieSequenceAsIndex { false }
			, CrawlingWeaponDatas { }
		{ }

		virtual ~ExtData() override = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InitializeConstants() override;
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<InfantryTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};
