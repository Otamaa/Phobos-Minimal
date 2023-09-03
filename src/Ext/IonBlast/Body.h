#pragma once
#include <IonBlastClass.h>
#include <ScenarioClass.h>
#include <Helpers/Macro.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Ext/WarheadType/Body.h>

// container for various function and static Map that attach the IonBlast to the ExtData
//class IonBlastExt
//{
//public:
//	~IonBlastExt() = default;
//	IonBlastExt() noexcept { IonBlastExt::ExtData(); }
//	IonBlastExt(IonBlastClass* pIon, WarheadTypeExt::ExtData* pWhExt, HouseClass* pOwner, TechnoClass* pTechno) noexcept
//	{ IonExtMap.insert(pIon, std::make_unique<IonBlastExt::ExtData>(pWhExt, pOwner, pTechno)); }
//
//	//similar to ExtDataContainer, handle the real data
//	class ExtData final
//	{
//	public:
//
//		WarheadTypeExt::ExtData* AttachedWarheadData;
//		HouseClass* IonHouseOwner;
//		TechnoClass* TechnoOwner;
//
//		ExtData() noexcept : 
//			TechnoOwner(nullptr),
//			IonHouseOwner(nullptr),
//			AttachedWarheadData(nullptr)
//		{ }
//
//		ExtData(WarheadTypeExt::ExtData* pWhExt, HouseClass* pOwner, TechnoClass* pTechno) noexcept :
//			TechnoOwner(pTechno),
//			IonHouseOwner(pOwner),
//			AttachedWarheadData(pWhExt)
//		{ }
//
//		virtual ~ExtData() = default;
//		virtual size_t Size() const { return sizeof(*this); }
//
//		virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved)
//		{
//			AnnounceInvalidPointer(AttachedWarheadData, ptr);
//			AnnounceInvalidPointer(IonHouseOwner, ptr);
//			AnnounceInvalidPointer(TechnoOwner, ptr);		
//		}
//
//		virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
//		{ return Serialize(Stm); }
//
//		virtual bool Save(PhobosStreamWriter& Stm) const
//		{ return const_cast<IonBlastExt::ExtData*>(this)->Serialize(Stm); }
//
//	private:
//		template <typename T>
//		bool Serialize(T& Stm)
//		{
//			return Stm
//				.Process(AttachedWarheadData)
//				.Process(IonHouseOwner)
//				.Process(TechnoOwner)
//				.Success();
//		}
//	};
//
//	static bool DoAffects(IonBlastClass* pIon);
//	static void RemoveFromMap(IonBlastClass* pIon)
//	{
//		if (IonExtMap[pIon].get())
//		{
//			IonExtMap[pIon] = nullptr;
//			IonExtMap.erase(pIon);
//		}
//	}
//
//	static IonBlastExt::ExtData* Find(IonBlastClass* pIon)
//	{
//		if (auto pData = IonExtMap[pIon].get())
//			return pData;
//		else
//			Debug::Log(__FUNCTION__" [%x] Failed  \n", pIon);
//
//		return nullptr;
//	}
//
//	static bool LoadGlobals(PhobosStreamReader& Stm) 
//	{ 
//		return Stm
//			.Process(IonExtMap)
//			.Success();
//	}
//
//	static bool SaveGlobals(PhobosStreamWriter& Stm) 
//	{ 
//		return Stm
//			.Process(IonExtMap)
//			.Success();
//	}
//
//	static void Clear() { IonExtMap.clear(); }
//
//	virtual size_t Size() const { return sizeof(*this); }
//
//private:
//
//	static PhobosMap<IonBlastClass*, std::unique_ptr<IonBlastExt::ExtData>> IonExtMap;
//};