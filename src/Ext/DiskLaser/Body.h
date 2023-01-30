#pragma once
#include <DiskLaserClass.h>

#include <Ext/Abstract/Body.h>

class DiskLaserExt
{
public:
	static constexpr size_t Canary = 0x87659771;
	using base_type = DiskLaserClass;

	class ExtData final : public TExtension<DiskLaserClass>
	{
	public:
		
		ExtData(DiskLaserClass* OwnerObject) : TExtension<DiskLaserClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void InitializeConstants() override { }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<DiskLaserExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};