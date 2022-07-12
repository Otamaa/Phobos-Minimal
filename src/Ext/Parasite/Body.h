#pragma once
#include <ParasiteClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ParasiteExt
{
public:

	using base_type = ParasiteClass;

    class ExtData final : public Extension<ParasiteClass>
    {
    public:

		ExtData(ParasiteClass* OwnerObject) : Extension<ParasiteClass>(OwnerObject)
        { }

        virtual ~ExtData() override = default;
		//virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) {}
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override { }

	};


	class ExtContainer final : public Container<ParasiteExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};