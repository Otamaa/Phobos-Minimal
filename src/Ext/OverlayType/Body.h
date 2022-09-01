#pragma once

#include <OverlayTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/Debug.h>
#include <Helpers/Macro.h>

class OverlayTypeExt
{
public:
	using base_type = OverlayTypeClass;

	class ExtData final : public Extension<OverlayTypeClass>
	{
	public:

		ExtData(OverlayTypeClass* OwnerObject) : Extension<OverlayTypeClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;
		void LoadFromINIFile(CCINIClass* pINI);
		void InvalidatePointer(void* ptr, bool bRemoved) {}
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<OverlayTypeExt>
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