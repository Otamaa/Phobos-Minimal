#pragma once

#include <TriggerTypeClass.h>
#include <Ext/Abstract/Body.h>

class TriggerClass;
class TriggerTypeExt
{
public:
	class ExtData final : public Extension<TriggerTypeClass>
	{
	public:
		using base_type = TriggerTypeClass;
		static constexpr size_t Canary = 0x2C2C2C2C;

	public:

		int HouseParam;
		ExtData(TriggerTypeClass* OwnerObject) : Extension<TriggerTypeClass>(OwnerObject) ,
			HouseParam { -1 }
		{ }

		virtual ~ExtData() override = default;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm)
		{
			Stm
				.Process(this->Initialized)
				;
		}
	};

	class ExtContainer final : public Container<TriggerTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static HouseClass* ResolveHouseParam(int const param, HouseClass* const pOwnerHouse);

};