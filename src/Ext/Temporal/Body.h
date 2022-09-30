#pragma once
#include <TemporalClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class TemporalExt
{
public:
	static constexpr size_t Canary = 0x82229781;
	using base_type = TemporalClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif
	class ExtData final : public Extension<TemporalClass>
	{
	public:
		WeaponTypeClass* Weapon;
		ExtData(TemporalClass* OwnerObject) : Extension<TemporalClass>(OwnerObject)
			, Weapon { nullptr }
		{ }

		virtual ~ExtData() override = default;
		void InvalidatePointer(void* ptr, bool bRemoved) { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void InitializeConstants() { }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TemporalExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};