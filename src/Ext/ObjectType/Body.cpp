#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/TechnoType/Body.h>

#include <TechnoTypeClass.h>
#include <UnitTypeClass.h>
#include <FileFormats/VXL.h>

#include <Utilities/Cast.h>
#include <Utilities/Debug.h>

#include <CRT.h>

BuildingClass* __fastcall FakeObjectTypeClass::WhoCanBuildMe(ObjectTypeClass* pThis, discard_t, bool intheory, bool bool2, bool legal, HouseClass* house)
{
	if(auto pTechno = type_cast<TechnoTypeClass*>(pThis)) {
		const auto nBuffer = HouseExtData::HasFactory(
			house,
			pTechno,
			intheory,
			bool2,
			legal,
		false);

		return nBuffer.first >= NewFactoryState::Available_Alternative ?
			nBuffer.second : nullptr;
	}

	return nullptr;
}

// Backport of ObjectTypeClass::LoadVoxel (0x5F8110–0x5F8CDB).
// Loads VXL/HVA art assets (main body, turrets, barrels) for a TechnoType.
// Integrates Phobos extensions: NoSpawnAlt→SpawnAltData, multi-turret, multi-barrel.
/*
void __fastcall FakeObjectTypeClass::_LoadVoxel(ObjectTypeClass* pThis, discard_t)
{
	bool bLoadFailed = false;

	// ===== Step 1: Load main body voxel =====
	{
		ImageStatusses status = ImageStatusses::ReadVoxel(pThis->ImageFile);
		if (!status.Loaded)
			bLoadFailed = true;
		else
			status.swap(pThis->MainVoxel);
	}

	// ===== Step 2: Load turret/special voxels =====
	const auto pUnitType = type_cast<UnitTypeClass*>(pThis);
	const bool bIsUnitType = (pUnitType != nullptr);
	auto pTechnoType = static_cast<TechnoTypeClass*>(pThis);

	if (bIsUnitType && !pTechnoType->Turret)
	{
		// Carrier/APC path: no gun turret. May have WO (NoSpawnAlt) or W (APC) art.

		// Phobos extension: if NoSpawnAlt=yes, load "<ImageFile>WO" into SpawnAltData.
		if (pThis->NoSpawnAlt)
		{
			auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
			std::string woName = pThis->ImageFile;
			woName += "WO";
			ImageStatusses status = ImageStatusses::ReadVoxel(woName.c_str());
			if (!status.Loaded)
			{
				Debug::LogInfo("{} Techno NoSpawnAlt Image[{}] cannot be loaded ,returning load failed ! ",
					pThis->ID, woName.c_str());
				bLoadFailed = true;
			}
			else
			{
				status.swap(pTypeExt->SpawnAltData);
			}
		}

		// Vanilla: APC gets "<ImageFile>W" (open-top wheel variant) loaded into TurretVoxel.
		if (CRT::strcmp(pThis->ID, "APC") == 0)
		{
			std::string wName = pThis->ImageFile;
			wName += "W";
			ImageStatusses status = ImageStatusses::ReadVoxel(wName.c_str());
			if (!status.Loaded)
				bLoadFailed = true;
			else
				status.swap(pThis->TurretVoxel);
		}
	}
	else
	{
		// Has turret flag or is not a UnitType: load "<ImageFile>TUR[N]" variants.
		const bool bMultiTurret = bIsUnitType
			&& pTechnoType->HasTurret()
			&& !pTechnoType->IsGattling;

		if (bMultiTurret && pTechnoType->TurretCount > 0)
		{
			// Phobos: extended multi-turret loop into ChargerTurrets + TurretImageData.
			auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
			const int nRemaining = std::max(0, pTechnoType->TurretCount - TechnoTypeClass::MaxWeapons);
			pTypeExt->TurretImageData.resize(nRemaining);

			for (int i = 0; i < pTechnoType->TurretCount; ++i)
			{
				std::string turName = pThis->ImageFile;
				turName += (i == 0) ? "TUR" : (std::string("TUR") + std::to_string(i));

				auto& slot = (i < TechnoTypeClass::MaxWeapons)
					? pThis->ChargerTurrets[i]
					: pTypeExt->TurretImageData[i - TechnoTypeClass::MaxWeapons];

				ImageStatusses status = ImageStatusses::ReadVoxel(turName.c_str());
				if (!status.Loaded)
				{
					Debug::LogInfo("{} Techno Turret [{}] at[{}] cannot be loaded , breaking the loop ! ",
						pThis->ID, turName.c_str(), i);
					Debug::RegisterParserError();
					continue;
				}
				status.swap(slot);
			}
		}
		else
		{
			// Vanilla: single "<ImageFile>TUR" into TurretVoxel.
			std::string turName = pThis->ImageFile;
			turName += "TUR";
			ImageStatusses status = ImageStatusses::ReadVoxel(turName.c_str());
			if (!status.Loaded)
				bLoadFailed = true;
			else
				status.swap(pThis->TurretVoxel);
		}
	}

	// ===== Step 3: Load barrel voxels =====
	// UnitType with no turret flag (carriers/APCs) have no barrel voxel.
	if (!(bIsUnitType && !pTechnoType->Turret))
	{
		const bool bMultiBarrel = bIsUnitType
			&& pTechnoType->HasTurret()
			&& !pTechnoType->IsGattling;

		if (bMultiBarrel && pTechnoType->TurretCount > 0)
		{
			// Phobos: extended multi-barrel loop into ChargerBarrels + BarrelImageData.
			auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
			const int nRemaining = std::max(0, pTechnoType->TurretCount - TechnoTypeClass::MaxWeapons);
			pTypeExt->BarrelImageData.resize(nRemaining);

			for (int i = 0; i < pTechnoType->TurretCount; ++i)
			{
				std::string brlName = pThis->ImageFile;
				brlName += (i == 0) ? "BARL" : (std::string("BARL") + std::to_string(i));

				auto& slot = (i < TechnoTypeClass::MaxWeapons)
					? pThis->ChargerBarrels[i]
					: pTypeExt->BarrelImageData[i - TechnoTypeClass::MaxWeapons];

				ImageStatusses status = ImageStatusses::ReadVoxel(brlName.c_str());
				if (!status.Loaded)
				{
					Debug::LogInfo("{} Techno Barrel [{}] at[{}] cannot be loaded ! ",
						pThis->ID, brlName.c_str(), i);
					Debug::RegisterParserError();
					continue;
				}
				status.swap(slot);
			}
		}
		else
		{
			// Vanilla: single "<ImageFile>BARL" into BarrelVoxel.
			std::string brlName = pThis->ImageFile;
			brlName += "BARL";
			ImageStatusses status = ImageStatusses::ReadVoxel(brlName.c_str());
			if (!status.Loaded)
				bLoadFailed = true;
			else
				status.swap(pThis->BarrelVoxel);
		}
	}

	// ===== Step 4: Error cleanup =====
	if (bLoadFailed)
	{
		GameDelete<true, true>(pThis->MainVoxel.VXL);
		pThis->MainVoxel.VXL = nullptr;
		GameDelete<true, true>(pThis->MainVoxel.HVA);
		pThis->MainVoxel.HVA = nullptr;

		if (bIsUnitType && pTechnoType->HasTurret() && !pTechnoType->IsGattling)
		{
			// Multi-turret path: DeallocTurrents clears ChargerTurrets, ChargerBarrels,
			// TurretImageData, BarrelImageData, and SpawnAltData (via Phobos-patched sub_5F8080).
			pThis->sub_5F8080();
		}
		else
		{
			// Single voxel path: free TurretVoxel and BarrelVoxel directly.
			GameDelete<true, true>(pThis->TurretVoxel.VXL);
			pThis->TurretVoxel.VXL = nullptr;
			GameDelete<true, true>(pThis->TurretVoxel.HVA);
			pThis->TurretVoxel.HVA = nullptr;
			// Bug fix: vanilla omits BarrelVoxel cleanup here, causing a potential leak
			// if barrel loaded successfully but an earlier step had set bLoadFailed.
			GameDelete<true, true>(pThis->BarrelVoxel.VXL);
			pThis->BarrelVoxel.VXL = nullptr;
			GameDelete<true, true>(pThis->BarrelVoxel.HVA);
			pThis->BarrelVoxel.HVA = nullptr;
		}
		return;
	}

	// ===== Step 5: Compute MaxDimension from main VXL layer extents =====
	// Finds the largest of SizeX/SizeY/SizeZ across all VXL sections, clamped to 8 minimum.
	{
		VoxLib* pVXL = pThis->MainVoxel.VXL;
		int maxSize = static_cast<unsigned char>(pVXL->leaSectionTailer(0, 0)->size.X);
		for (int layer = 0; layer < static_cast<int>(pVXL->CountHeaders); ++layer)
		{
			auto* pTailer = pVXL->leaSectionTailer(layer, 0);
			const unsigned char sx = static_cast<unsigned char>(pTailer->size.X);
			const unsigned char sy = static_cast<unsigned char>(pTailer->size.Y);
			const unsigned char sz = static_cast<unsigned char>(pTailer->size.Z);
			if (maxSize < sx) maxSize = sx;
			if (maxSize < sy) maxSize = sy;
			if (maxSize < sz) maxSize = sz;
		}
		if (maxSize < 8) maxSize = 8;
		pThis->MaxDimension = maxSize;
	}

	// ===== Step 6: Reset VoxelCaches =====
	for (auto& cache : pThis->VoxelCaches_)
		cache.Clear();
}
*/