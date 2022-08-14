#pragma once
#include <ParasiteClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ParasiteExt
{
public:
	static constexpr size_t Canary = 0x99954321;
	using base_type = ParasiteClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

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
	private:
		template <typename T>
		void Serialize(T& Stm);
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