#pragma once

#include <OverlayTypeClass.h>

#include <Ext/Abstract/Body.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/Debug.h>
#include <Helpers/Macro.h>

class OverlayTypeExt
{
public:
	using base_type = OverlayTypeClass;
	static constexpr size_t Canary = 0x414B4B4A;

	class ExtData final : public TExtension<OverlayTypeClass>
	{
	public:

		ExtData(OverlayTypeClass* OwnerObject) : TExtension<OverlayTypeClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}
		virtual bool InvalidateIgnorable(void* const ptr) const { return true; };
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<OverlayTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};