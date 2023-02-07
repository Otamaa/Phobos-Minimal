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

		virtual ~ExtData() override = default;
		void Uninitialize() { }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;

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
		if (auto pGlobal = Global())
		{
			if (pGlobal->InvalidateIgnorable(ptr))
				return;

			pGlobal->InvalidatePointer(ptr, removed);
		}
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	//void DrawDebugOverlay();
	//bool DrawCurrentCell(); //TODO
	//bool DebugDrawAllCellInfo(); //TODO
	//bool DebugDrawBridgeInfo(); //TODO
	//void DebugDrawMouseCellMembers //TODO
};