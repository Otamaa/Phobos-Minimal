#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Misc/DamageArea.h>

#include <Utilities/Macro.h>

#include <Phobos.SaveGame.h>

template <typename T>
void BombExtData::Serialize(T& Stm) {

	Stm
		.Process(this->Weapon)
		;
}

// =============================
// container
BombExtContainer BombExtContainer::Instance;

bool BombExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(BombExtContainer::ClassName))
	{
		auto& container = root[BombExtContainer::ClassName];

		for (auto& entry : container[BombExtData::ClassName])
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

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, BombExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;
}

bool BombExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[BombExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : BombExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer); // write all data to stream

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[BombExtData::ClassName] = std::move(_extRoot);
	return true;
}
// =============================
// container hooks

// not initEd :
// Ownerhouse
// target
// state
// ticksound

ASMJIT_PATCH(0x4385FC, BombClass_CTOR, 0x6)
{
	GET(BombClass*, pItem, ESI);
	BombExtContainer::Instance.Allocate(pItem);
	return 0;
}ASMJIT_PATCH_AGAIN(0x438EE9, BombClass_CTOR, 0x6)

ASMJIT_PATCH(0x4393F2, BombClass_SDDTOR, 0x5)
{
	GET(BombClass *, pItem, ECX);
	BombExtContainer::Instance.Remove(pItem);
	return 0;
}

void FakeBombClass::__Detonate() { 
	static COMPILETIMEEVAL reference<int, 0xABAD1C> WoodBridgeSet {};

	auto const pTarget = this->Target;

	if (!pTarget || this->State != BombState::Planted)
		return;

	bool const wasInLimbo = pTarget->InLimbo;
	pTarget->AttachedBomb = nullptr;

	if (wasInLimbo) {
		pTarget->BombVisible = false;
		this->State = BombState::Removed ;
	} else {
		pTarget->BombVisible = false;
		this->State = BombState::Removed;

		// Also adjust detonation coordinate.
		auto pExt = this->_GetExtData();

		CoordStruct coords = pExt->Weapon->Ivan_AttachToCenter.Get(RulesExtData::Instance()->IvanBombAttachToCenter) ?
			pTarget->GetCenterCoords() : pTarget->Location;

		const auto pBombWH = pExt->Weapon->Ivan_WH.Get(RulesClass::Instance->IvanWarhead);
		const auto nDamage = pExt->Weapon->Ivan_Damage.Get(RulesClass::Instance->IvanDamage);
		const auto OwningHouse = this->GetOwningHouse();

		/*WarheadTypeExtData::DetonateAt(pBombWH, pTarget, coords, pThis->Owner, nDamage);*/
		DamageArea::Apply(&coords, nDamage, this->Owner, pBombWH, pBombWH->Tiberium, OwningHouse);
		MapClass::Instance->FlashbangWarheadAt(nDamage, pBombWH, coords);
		const auto pDetonateCoords = MapClass::Instance->GetCellAt(coords);

		if (auto pAnimType = MapClass::Instance->SelectDamageAnimation(nDamage, pBombWH, pDetonateCoords->LandType, coords)) {
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coords, 0, 1, AnimFlag::AnimFlag_2600, -15, false),
				OwningHouse,
				this->Target ? this->Target->GetOwningHouse() : nullptr,
				this->Owner,
				false, false
			);
		}

		if(pExt->Weapon->Ivan_KillsBridges) {
			// Bridge Repair Hut special handling
			auto const pTargetBuilding = cast_to<BuildingClass*, false>(this->Target);

			if (pTargetBuilding && pTargetBuilding->Type->BridgeRepairHut) {

				bool foundBridge = false;
				auto buildingMapCoords = pTargetBuilding->GetMapCoords();

				// Scan 5x5 area around target
				for (int y = -2; y < 3; ++y) {
					for (int x = -2; x < 3; ++x) {

						CellStruct targetCell = buildingMapCoords;
						targetCell.X += static_cast<short>(x);
						targetCell.Y += static_cast<short>(y);

						auto const pCell = MapClass::Instance->GetCellAt(targetCell);
						int const tileType = pCell->IsoTileTypeIndex;
						int const overlay = pCell->OverlayTypeIndex;

						// Check for wood bridge tiles (WoodBridgeSet to WoodBridgeSet+15)
						// or bridge overlay types (74-101 aka 0x4A-0x65)
						bool const isWoodBridge = (tileType >= WoodBridgeSet && tileType < WoodBridgeSet + 16);
						bool const isBridgeOverlay = (overlay >= 74 && overlay <= 101);

						if (isWoodBridge || isBridgeOverlay) {
							foundBridge = true;
						}
					}
				}

				(MapClass::Instance->*(foundBridge ? 
					&MapClass::DestroyWoodBridgeAt :
					&MapClass::DestroyConcreteBridgeAt))
				(buildingMapCoords);
			}
		}
	}

	this->Owner = nullptr;
	this->Target = nullptr;
	this->OwnerHouse = nullptr;
	this->TickAudioController.AudioEventHandleEnd();
	this->ShouldPlayTickingSound = 0;
}

int FakeBombClass::__GetBombFrame()
{
	const auto ext = this->_GetExtData();
	const auto pData = ext->Weapon;

	const SHPStruct* shp = pData->Ivan_Image.Get(RulesClass::Instance->BOMBCURS_SHP);
	const int frames = shp->Frames;

	int result = 0;

	if (frames >= 2) {
		if (this->Type != BombType::NormalBomb) {
			// DeathBomb → last frame
			result = frames - 1;
		} else {
			const int delay = pData->Ivan_Delay.Get(RulesClass::Instance->IvanTimedDelay);
			const int flickerRate = pData->Ivan_FlickerRate.Get(RulesClass::Instance->IvanIconFlickerRate);
			const int elapsed = Unsorted::CurrentFrame - this->PlantingFrame;

			const int half = frames / 2;
			int capped = half - 1;

			if (flickerRate <= 0) {
				// no flicker: use only half the frames
				int frame = elapsed / (delay / (2 * half));
				if (frame > capped) frame = capped;
				result = frame;
			} else {
				// flicker: use full even/odd pattern
				int frame = elapsed / (delay / half);
				if (frame > capped) frame = capped;

				int even = frame * 2;
				int odd = even + 1;

				bool flick = (Unsorted::CurrentFrame % (2 * flickerRate)) < flickerRate;
				result = flick ? even : odd;
			}
		}
	}

	return result;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x438720, FakeBombClass::__Detonate)
DEFINE_FUNCTION_JUMP(CALL, 0x4C7849, FakeBombClass::__Detonate)

DEFINE_FUNCTION_JUMP(LJMP, 0x438A00, FakeBombClass::__GetBombFrame)
DEFINE_FUNCTION_JUMP(CALL, 0x6F5230, FakeBombClass::__GetBombFrame)