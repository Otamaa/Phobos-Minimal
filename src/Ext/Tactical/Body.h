#pragma once

#include <Utilities/Container.h>

#include <Utilities/Iterator.h>
#include <Utilities/Template.h>

#include <Helpers/Template.h>
 
#include <TacticalClass.h>

class TacticalExt
{
public:
	static constexpr size_t Canary = 0x52DEBA12;
	using base_type = TacticalClass;

	class ExtData final : public Extension<TacticalClass>
	{
	public:

		ExtData(TacticalClass* OwnerObject) : Extension<TacticalClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;
		void Uninitialize() { }
		// void InvalidatePointer(void* ptr, bool bRemoved) { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void InitializeConstants();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static IStream* g_pStm;

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

	static void PointerGotInvalid(void* ptr, bool removed)
	{
		// Global()->InvalidatePointer(ptr, removed);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	//void DrawDebugOverlay();
	//bool DrawCurrentCell(); //TODO
	//bool DebugDrawAllCellInfo(); //TODO
	//bool DebugDrawBridgeInfo(); //TODO
	//void DebugDrawMouseCellMembers //TODO
};