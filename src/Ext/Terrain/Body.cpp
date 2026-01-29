#include "Body.h"

#include <Ext/TerrainType/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>

#include <Phobos.SaveGame.h>

bool TerrainExtData::CanMoveHere(TechnoClass* pThis, TerrainClass* pTerrain) {
	const auto pExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);

	if (pExt->IsPassable)
		return true;

	if (auto pUnit = cast_to<UnitClass*, false>(pThis)) {
		if (pTerrain->Type->Crushable) {
			if (TechnoTypeExtContainer::Instance.Find(pUnit->Type)
					->CrushLevel.Get(pThis) > pExt->CrushableLevel) {
				return true;
			}
		}
	}

	return false;
}

void TerrainExtData::InitializeLightSource()
{
	if (!this->LightSource && This()->Type)
	{
		auto const TypeData = TerrainTypeExtContainer::Instance.Find(This()->Type);

		if (!TypeData->LightEnabled || !TypeData->LightIntensity.isset())
			return;

		auto const nVisibility = TypeData->LightVisibility.Get();

		if (!nVisibility)
			return;

		auto Tint = TypeData->GetLightTint();
		auto Coords = This()->GetCoords();
		const auto light = GameCreate<LightSourceClass>(Coords, nVisibility, TypeData->GetLightIntensity(), Tint);
		light->Activate();
		this->LightSource.reset(light);
	}
}

void TerrainExtData::InitializeAnim()
{
	if (!AttachedAnim && This()->Type)
	{
		auto const TypeData = TerrainTypeExtContainer::Instance.Find(This()->Type);

		if (TypeData->AttachedAnim.empty())
			return;

		AnimTypeClass* pAnimType = nullptr;
		if (TypeData->AttachedAnim.size() == 1)
			pAnimType = TypeData->AttachedAnim[0];
		else
			pAnimType =
			TypeData->
			AttachedAnim[ScenarioClass::Instance->Random.RandomFromMax(TypeData->AttachedAnim.size() - 1)];

		if (pAnimType)
		{
			auto const Coords = this->AttachedAnim->GetCoords();

			AttachedAnim.reset(GameCreate<AnimClass>(pAnimType, Coords));
		}
	}
}

void TerrainExtData::Unlimbo(TerrainClass* pThis, CoordStruct* pCoord)
{
	if (!pThis || !pThis->Type)
		return;

	auto const TerrainExt = TerrainExtContainer::Instance.FindOrAllocate(pThis);

	//if (auto const CellExt = CellExt::ExtMap.Find<true>(Map[*pCoord]))
	//{
	//	auto const iter = std::find_if(CellExt->AttachedTerrain.begin(), CellExt->AttachedTerrain.end(),
	//		[&](auto const pCellTerrain) { return pCellTerrain == pThis; });

	//	if (iter != CellExt->AttachedTerrain.end())
	//		CellExt->AttachedTerrain.push_back(pThis);
	//}

	{
		TerrainExt->InitializeLightSource();
		TerrainExt->InitializeAnim();
	}

}

// =============================
// load / save
template <typename T>
void TerrainExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->LightSource, true)
		.Process(this->AttachedAnim, true)
		.Process(this->AttachedFireAnim, true)
		.Process(this->AdjacentCells)
		;
}

// =============================
// container
TerrainExtContainer TerrainExtContainer::Instance;
bool TerrainExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(TerrainExtContainer::ClassName))
	{
		auto& container = root[TerrainExtContainer::ClassName];

		for (auto& entry : container[TerrainExtData::ClassName])
		{
			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, TerrainExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool TerrainExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[TerrainExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : TerrainExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer);

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[TerrainExtData::ClassName] = std::move(_extRoot);

	return true;
}

// container hooks
#include <Notifications.h>

DEFINE_JUMP(LJMP, 0x71BC31 , 0x71BC86);

ASMJIT_PATCH(0x71BE74, TerrainClass_CTOR, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	TerrainExtContainer::Instance.Allocate(pItem);
	//PointerExpiredNotification::NotifyInvalidObject->Add(pItem);
	return 0;
}

ASMJIT_PATCH(0x71BCA5, TerrainClass_CTOR_MoveAndAllocate, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	GET_STACK(CellStruct*, pCoord, 0x24);

	auto pExt = TerrainExtContainer::Instance.FindOrAllocate(pItem);

	if (pCoord->IsValid()) {
		//vtable may not instantiated
		if (!pItem->TerrainClass::Unlimbo(CellClass::Cell2Coord(*pCoord), static_cast<DirType>(0))) {
			pItem->ObjectClass::UnInit();
		}

		if(pItem->Type){
			GeneralUtils::AdjacentCellsInRange(pExt->AdjacentCells, (short)TerrainTypeExtContainer::Instance.Find(pItem->Type)->SpawnsTiberium_Range);
		}
	}

	return 0x0;
}

//Remove Ext later , dont do it to early otherwise some stuffs broke
ASMJIT_PATCH(0x71B824, TerrainClass_DTOR, 0x5)
{
	GET(TerrainClass*, pItem, ESI);

	if(Unsorted::WTFMode() || pItem->Type)
	{
		pItem->IsAlive = true;
		if (!pItem->Limbo())
			pItem->AnnounceExpiredPointer();
	}

	if(auto pExt = TerrainExtContainer::Instance.TryFind(pItem)) {
		delete pExt;
		TerrainExtContainer::Instance.ClearExtAttribute(pItem);
		//PointerExpiredNotification::NotifyInvalidObject->Remove(pItem);
	}

	return 0x71B845;
}


#include <Misc/Hooks.Otamaa.h>

void FakeTerrainClass::_AI()
{
	this->ObjectClass::Update();
	if (this->Type->IsAnimated) {
		if (!this->Animation.Stage) {
			auto v2 =ScenarioClass::Instance->Random.Random();

			if ((double)((int)Math::abs(v2) % 1000000) * 0.000001 < this->Type->AnimationProbability) {
				this->Animation.Stage = 0;
				this->Animation.Start(this->Type->AnimationRate);
			}
		}
	}

	if (this->Animation.Timer.GetTimeLeft() || !this->Animation.Timer.Rate)
	{
		// timer is still running or hasn't been set yet.
		this->Animation.HasChanged = false;
	}
	else
	{
		// timer expired. move one step forward.
		this->Animation.Stage += this->Animation.Step;
		this->Animation.HasChanged = true;
		this->Animation.Timer.Restart();

		//auto const pTypeExt = this->_GetTypeExtData();

		//not sure what here ,..
	}
}

//ASMJIT_PATCH(0x71CFD0, TerrainClass_Detach, 0x5)
//{
//	GET(TerrainClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pObj, 0x4);
//	GET_STACK(bool, bRemoved, 0x8);
//
//	pThis->ObjectClass::PointerExpired(pObj, bRemoved);
//	TerrainExtContainer::Instance.InvalidatePointerFor(pThis, pObj, bRemoved);
//
//	if (pThis->Type == pObj)
//		pThis->Type = nullptr;
//
//	return 0x71CFF7;
//}

void FakeTerrainClass::_Detach(AbstractClass* target, bool all)
{
	if(auto pExt = this->_GetExtData())
		pExt->InvalidatePointer(target, all);
	this->TerrainClass::PointerExpired(target, all);
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5254, FakeTerrainClass::_Detach);


void FakeTerrainClass::_AnimPointerExpired(AnimClass* pAnim) {

	auto pExt = this->_GetExtData();

	if (pExt->AttachedFireAnim.get() == pAnim) {
		pExt->AttachedFireAnim.release();
	}
}
DEFINE_FUNCTION_JUMP(VTABLE ,0x7F528C, FakeTerrainClass::_AnimPointerExpired)
