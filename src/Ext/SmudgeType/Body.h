#pragma once
#include <SmudgeTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>

class SmudgeTypeExt
{
public:
	static constexpr size_t Canary = 0xBEE75008;
	using base_type = SmudgeTypeClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public Extension<SmudgeTypeClass>
	{
	public:

		Valueable<bool> Clearable;
		ExtData(SmudgeTypeClass* OwnerObject) : Extension<SmudgeTypeClass>(OwnerObject)
			, Clearable { true }

		{ }

		virtual ~ExtData() = default;
		void LoadFromINIFile(CCINIClass* pINI);
		// void InvalidatePointer(void* ptr, bool bRemoved) { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SmudgeTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};