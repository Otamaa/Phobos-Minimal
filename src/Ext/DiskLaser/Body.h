#pragma once
#include <DiskLaserClass.h>

#include <Ext/Abstract/Body.h>

class DiskLaserExt
{
public:
	class ExtData final : public Extension<DiskLaserClass>
	{
	public:
		static constexpr size_t Canary = 0x87659771;
		using base_type = DiskLaserClass;

	public:

		ExtData(DiskLaserClass* OwnerObject) : Extension<DiskLaserClass>(OwnerObject) { }
		virtual ~ExtData() override = default;

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<DiskLaserExt::ExtData>
	{
	public:
		CONSTEXPR_NOCOPY_CLASS(DiskLaserExt::ExtData, "DiskLaserClass");
	};

	static ExtContainer ExtMap;
};