#pragma once
#include <SuperClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SuperExt
{
public:
	static constexpr size_t Canary = 0x12311111;
	using base_type = SuperClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public Extension<SuperClass>
	{
	public:

		ExtData(SuperClass* OwnerObject) : Extension<SuperClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;
		// void InvalidatePointer(void* ptr, bool bRemoved) { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SuperExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};