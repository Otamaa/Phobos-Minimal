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
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual size_t Size() const { return sizeof(*this); }
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<OverlayTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};