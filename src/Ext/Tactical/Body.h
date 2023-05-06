#pragma once

#include <Utilities/Container.h>

#include <Utilities/Iterator.h>
#include <Utilities/Template.h>

#include <Helpers/Template.h>
 
#include <TacticalClass.h>

class TacticalExt
{
public:
	static IStream* g_pStm;

	class ExtData final : public Extension<TacticalClass>
	{
	public:
		static constexpr size_t Canary = 0x52DEBA12;
		using base_type = TacticalClass;

	public:

		ExtData(TacticalClass* OwnerObject) : Extension<TacticalClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;
public:

	static void Allocate(TacticalClass* pThis);
	static void Remove(TacticalClass* pThis);

	static ExtData* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(TacticalClass::Instance);
	}

	//void DrawDebugOverlay();
	//bool DrawCurrentCell(); //TODO
	//bool DebugDrawAllCellInfo(); //TODO
	//bool DebugDrawBridgeInfo(); //TODO
	//void DebugDrawMouseCellMembers //TODO
};