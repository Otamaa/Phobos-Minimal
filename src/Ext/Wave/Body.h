#pragma once
#include <WaveClass.h>

#include <Ext/Abstract/Body.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>
#include <Helpers/Macro.h>

class WeaponTypeClass;
class WaveExt
{
public:

	static constexpr size_t Canary = 0xAABAAAAC;
	using base_type = WaveClass;

	class ExtData final : public Extension<WaveClass>
	{
	public:

		WeaponTypeClass* Weapon;
		int WeaponIdx;
		bool ReverseAgainstTarget;

		ExtData(WaveClass* OwnerObject) : Extension<WaveClass>(OwnerObject)
			, Weapon { nullptr }
			, WeaponIdx { 0 }
			, ReverseAgainstTarget { false }

		{ }

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		virtual void Initialize() override;
		
		void InitWeaponData();
		void SetWeaponType(WeaponTypeClass* pWeapon, int nIdx);
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WaveExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
