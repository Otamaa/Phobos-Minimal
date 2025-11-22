#include "Body.h"

#include <Ext/WarheadType/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/Techno/Body.h>

#include <New/Type/RadTypeClass.h>
#include <LightSourceClass.h>
#include <Utilities/Macro.h>
#include <Notifications.h>

void RadSiteExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(TechOwner, ptr , bRemoved);
	AnnounceInvalidPointer(HouseOwner, ptr);

	damageCounts.erase((BuildingClass*)ptr);
}

void RadSiteExtData::CreateInstance(CellClass* pCell , int spread, int amount, WeaponTypeExtData* pWeaponExt, TechnoClass* const pTech)
{
	// use real ctor
	const auto pRadExt = RadSiteExtContainer::Instance.Find(GameCreate<RadSiteClass>());

	//Adding Owner to RadSite, from bullet
	if (pWeaponExt)
	{
		pRadExt->Weapon = pWeaponExt->This();
		pRadExt->Type = pWeaponExt->RadType.Get(0);
		pRadExt->NoOwner = pWeaponExt->Rad_NoOwner.Get();
	}
	else
	{
		pRadExt->Type = RadTypeClass::FindOrAllocate(GameStrings::Radiation());
	}

	if (pTech && pRadExt->Type->GetHasInvoker() && !pRadExt->NoOwner && pRadExt->Type->GetHasOwner())
	{
		pRadExt->HouseOwner = pTech->GetOwningHouse();
		pRadExt->TechOwner = pTech;
	}

	pRadExt->CreationFrame = Unsorted::CurrentFrame;
	CellExtContainer::Instance.Find(pCell)->RadSites.push_back(pRadExt->This());
	pRadExt->This()->BaseCell = pCell->MapCoords;
	pRadExt->This()->SetSpread(spread);
	pRadExt->SetRadLevel(amount);
	pRadExt->CreateLight();
}

//RadSiteClass Activate , Rewritten
void RadSiteExtData::CreateLight()
{
	const auto pThis = this->This();
	const auto nLevelDelay = this->Type->GetLevelDelay();
	const auto nLightDelay = this->Type->GetLightDelay();
	const auto nRadcolor = this->Type->GetColor();

	//if(Phobos::Otamaa::IsAdmin)
	//	Debug::LogInfo("RadSite [%s] CreateLight With Color [%d , %d , %d] ", Type->Name.data(), nRadcolor.R, nRadcolor.G, nRadcolor.B);

	const auto nTintFactor = this->Type->GetTintFactor();
	const auto nRadLevelFactor = pThis->RadLevel * this->Type->GetLightFactor();
	const auto nLightFactor = std::clamp(nRadLevelFactor , 1.0 , 2000.0 );
	const auto nDuration = (double)pThis->RadDuration;

	pThis->RadLevelTimer.Start(nLevelDelay);
	pThis->RadLightTimer.Start(nLightDelay);
	pThis->Intensity = int(nLightFactor);
	pThis->LevelSteps = int(nDuration / (double)nLevelDelay);
	pThis->IntensitySteps = int(nDuration / (double)nLightDelay);
	pThis->IntensityDecrement = int(nLightFactor / (nDuration / (double)nLightDelay));
	const TintStruct nTintBuffer { nRadcolor  , nTintFactor };

	pThis->Tint = nTintBuffer;

	if (pThis->LightSource)
	{
		pThis->LightSource->ChangeLevels(static_cast<int>(nLightFactor), nTintBuffer, false);
		pThis->Radiate();
	}
	else
	{
		auto const pCell = MapClass::Instance->GetCellAt(pThis->BaseCell);
		{
			auto const pLight = GameCreate<LightSourceClass>(pCell->GetCoordsWithBridge(), pThis->SpreadInLeptons, static_cast<int>(nLightFactor), nTintBuffer);
			pThis->LightSource = pLight;
			pLight->DetailLevel = 0;
			pLight->Activate(false);
			pThis->Radiate();
		}
	}
}

// Rewrite because of crashing craziness
void RadSiteExtData::Add(int amount)
{
	const auto pThis = this->This();
	pThis->Deactivate();
	const auto nInput = int(double(pThis->RadLevel * pThis->RadTimeLeft) / (double)pThis->RadDuration) + amount;
	pThis->RadLevel = nInput;
	const auto nInput_2 = nInput * this->Type->GetDurationMultiple();
	pThis->RadDuration = nInput_2;
	pThis->RadTimeLeft = nInput_2;
	this->CreateLight();
	this->CreationFrame = Unsorted::CurrentFrame;
}

void RadSiteExtData::SetRadLevel(int amount)
{
	const auto pThis = this->This();
	const auto nMax = this->Type->GetLevelMax();
	const auto nDecidedamount = MinImpl(amount,  nMax);
	const int mult = this->Type->GetDurationMultiple();
	pThis->RadLevel = nDecidedamount;
	pThis->RadDuration = mult * nDecidedamount;
	pThis->RadTimeLeft = mult * nDecidedamount;
}

// helper function provided by AlexB
const double RadSiteExtData::GetRadLevelAt(CellStruct const& cell)
{
	return this->GetRadLevelAt(this->This()->BaseCell.DistanceFrom(cell));
}

bool NOINLINE IsFiniteNumber(double x) {
	return (x <= DBL_MAX && x >= -DBL_MAX);
}

const double RadSiteExtData::GetRadLevelAt(double distance)
{
	const auto pThis = this->This();
	const auto nMax = static_cast<double>(pThis->Spread);
	double radLevel = pThis->RadLevel;

	//  will produce `-nan(ind)` result if both dist and max is zero
	// and used on formula below this check
	// ,.. -Otamaa
	if (distance && nMax) {

		//distance is too far
		if (distance > nMax) {
			return 0.0;
		}
		else {
			radLevel = (nMax - distance) / nMax * pThis->RadLevel;
		}
	}

	// Vanilla YR stores & updates the decremented RadLevel on CellClass.
	// Because we're not storing multiple radiation site data on CellClass (yet?)
	// we need to fully recalculate this stuff every time we need the radiation level for a cell coord - Starkku
	const auto frame_Step = (Unsorted::CurrentFrame - this->CreationFrame);
	const int stepCount = frame_Step ? frame_Step / this->Type->GetLevelDelay() : 0;

	if(radLevel && pThis->LevelSteps)
		radLevel -= (radLevel / pThis->LevelSteps) * stepCount;

	return radLevel;
}

//return false mean it is already death
const RadSiteExtData::DamagingState RadSiteExtData::ApplyRadiationDamage(TechnoClass* pTarget, int damage, int distance)
{
	const auto pWarhead = this->Type->GetWarhead();
	if (!pTarget->IsAlive || pTarget->InLimbo || !pTarget->Health || pTarget->IsSinking || pTarget->IsCrashing)
		return RadSiteExtData::DamagingState::Dead;

	auto const pUnit = cast_to<UnitClass*, false>(pTarget);

	if ((pUnit && pUnit->DeathFrameCounter > 0))
		return RadSiteExtData::DamagingState::Ignore;

	{
		if (!this->Type->GetWarheadDetonate())
		{
			HouseClass* const pOwner = this->TechOwner ? this->TechOwner->Owner : this->HouseOwner;
			const auto result = pTarget->ReceiveDamage(&damage, distance, pWarhead, this->TechOwner, false, true, pOwner);

			if (result != DamageState::NowDead && result != DamageState::PostMortem)
				return RadSiteExtData::DamagingState::Continue;
			else if (result == DamageState::PostMortem)
				return RadSiteExtData::DamagingState::Ignore;
		}
		else
		{
			auto const coords = pTarget->GetCoords();
			HouseClass* const pOwner = this->TechOwner ? this->TechOwner->Owner : this->HouseOwner;
			WarheadTypeExtData::DetonateAt(pWarhead, pTarget, coords , this->TechOwner, damage , pOwner);

			if ((pUnit && pUnit->DeathFrameCounter > 0))
				return RadSiteExtData::DamagingState::Ignore;
		}
	}

	const auto res =  pTarget->IsAlive && !pTarget->InLimbo && pTarget->Health > 0 && !pTarget->IsSinking && !pTarget->IsCrashing;

	return res ? RadSiteExtData::DamagingState::Continue : RadSiteExtData::DamagingState::Dead;
}

// =============================
// load / save

template <typename T>
void RadSiteExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Weapon, true)
		.Process(this->Type, true)
		.Process(this->TechOwner, true)
		.Process(this->HouseOwner, true)
		.Process(this->NoOwner)
		.Process(this->CreationFrame)
		.Process(this->damageCounts)
		;
}

// =============================
// container
RadSiteExtContainer RadSiteExtContainer::Instance;
std::vector<RadSiteExtData*> Container<RadSiteExtData>::Array;

void Container<RadSiteExtData>::Clear()
{
	Array.clear();
}

bool RadSiteExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool RadSiteExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

// =============================
// container hooks

ASMJIT_PATCH(0x65B243, RadSiteClass_CTOR, 0x6)
{

	GET(RadSiteClass*, pThis, ESI);
	RadSiteExtContainer::Instance.Allocate(pThis);

	return 0;
}

ASMJIT_PATCH(0x65B344, RadSiteClass_DTOR, 0x6)
{
	GET(RadSiteClass*, pThis, ESI);
	const auto pBaseCell = MapClass::Instance->TryGetCellAt(pThis->BaseCell);

	if (pBaseCell) {
		CellExtContainer::Instance.Find(pBaseCell)->RadSites.remove(pThis);
	}

	for (CellRangeEnumerator it(pThis->BaseCell, pThis->Spread + 0.5); it; it++) {
		if (const auto pCell = MapClass::Instance->TryGetCellAt(*it)) {
			CellExtContainer::Instance.Find(pCell)->RadLevels.remove_all_if([pThis](auto& level) { return level.Rad == pThis; });
		}
	}

	RadSiteExtContainer::Instance.Remove(pThis);

	return 0;
}

void FakeRadSiteClass::_Detach(AbstractClass* pTarget, bool bRemove)
{
	RadSiteExtContainer::Instance.InvalidatePointerFor(this, pTarget, bRemove);
	//this->RadSiteClass::PointerExpired(pTarget, bRemove);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0838, FakeRadSiteClass::_Detach);

HouseClass* FakeRadSiteClass::_GetOwningHouse()
{
	return RadSiteExtContainer::Instance.Find(this)->HouseOwner;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F084C, FakeRadSiteClass::_GetOwningHouse);

HRESULT __stdcall FakeRadSiteClass::_Load(IStream* pStm)
{
	HRESULT hr = this->RadSiteClass::Load(pStm);
	if (SUCCEEDED(hr))
		hr = RadSiteExtContainer::Instance.LoadKey(this, pStm);

	return hr;
}

HRESULT __stdcall FakeRadSiteClass::_Save(IStream* pStm, BOOL clearDirty)
{
	HRESULT hr = this->RadSiteClass::Save(pStm, clearDirty);
	if (SUCCEEDED(hr))
		hr = RadSiteExtContainer::Instance.SaveKey(this, pStm);

	return hr;
}

// DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0824, FakeRadSiteClass::_Load)
// DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0828, FakeRadSiteClass::_Save)
template<bool reduce = false>
void PopulateCellRadVector(FakeRadSiteClass* pRad, CellStruct* cell, int distance, int timeParam)
{
	const auto max = pRad->SpreadInLeptons;

	if (distance <= max)
	{
		if (auto pCell = MapClass::Instance->TryGetCellAt(cell))
		{
			const auto pCellExt = CellExtContainer::Instance.Find(pCell);
			auto it = pCellExt->RadLevels.find_if([pRad](auto& pair) { return pair.Rad == pRad; });

			if constexpr (!reduce)
			{
				const int amount = int(static_cast<double>(max - distance) / max * pRad->RadLevel);


				if (it != pCellExt->RadLevels.end())
					it->Level += MinImpl(it->Level + amount, RadSiteExtContainer::Instance.Find(pRad)->Type->GetLevelMax());
				else
					pCellExt->RadLevels.emplace_back(pRad, amount);
			}
			else
			{
				if (it != pCellExt->RadLevels.end())
				{
					it->Level -= int(static_cast<double>(max - distance) / max * pRad->RadLevel / pRad->LevelSteps * timeParam);
				}
			}
		}
	}
}

static NOINLINE int CalculateRadiationDamage(
	int baseLevel,
	double levelFactor,
	double distanceInCells,
	int cellSpreadInCells,
	double minFalloff = 0.1
)
{
	double maxDistance = std::max(1.0, static_cast<double>(cellSpreadInCells));

	// Linear falloff, clamped between minFalloff and 1.0
	double falloff = std::clamp(1.0 - (distanceInCells / maxDistance), minFalloff, 1.0);

	// Base damage before falloff
	double rawDamage = static_cast<double>(baseLevel) * levelFactor;

	// Final damage (may be negative for healing)
	return static_cast<int>(rawDamage * falloff);
}

static NOINLINE void ApplyRadDamage(RadSiteClass* pRad, FootClass* pObj, CellClass* pCell, int distance)
{
	if (pObj->IsAlive && !pObj->InLimbo && pObj->Health > 0 && !pObj->TemporalTargetingMe && !TechnoExtData::IsRadImmune(pObj))
	{
		const auto pRadExt = RadSiteExtContainer::Instance.Find(pRad);
		RadTypeClass* pRadType = pRadExt->Type;

		const int RadApplicationDelay = RulesExtData::Instance()->UseGlobalRadApplicationDelay ? pRadType->GetApplicationDelay() : RulesClass::Instance->RadApplicationDelay;
		if ((RadApplicationDelay <= 0)
			|| (Unsorted::CurrentFrame % RadApplicationDelay))
			return;

		if (pObj->GetTechnoType()->Immune || !pRadType->GetWarhead())
			return;

		auto it = CellExtContainer::Instance.Find(pCell)->RadLevels.find_if([pRad](auto& pair) { return pair.Rad == pRad; });

		if (it == CellExtContainer::Instance.Find(pCell)->RadLevels.end() || it->Level <= 0)
			return;

		const auto damage = CalculateRadiationDamage(it->Level, pRadType->GetLevelFactor(), distance, pRad->Spread);

		if (damage == 0)
			return;

		UnitClass* pUnit = cast_to<UnitClass*, false>(pObj);
		FootClass* pFoot = pObj;

		if ((pUnit && pUnit->DeathFrameCounter > 0) || !RadSiteClass::Array->Count)
			return;

		if (pObj->IsSinking || pObj->IsCrashing)
			return;

		if (pObj->IsInAir())
			return;

		if (pObj->IsBeingWarpedOut() || TechnoExtData::IsChronoDelayDamageImmune(pFoot))
			return;

		if (pRadExt->ApplyRadiationDamage(pObj, damage, static_cast<int>(distance)) == RadSiteExtData::DamagingState::Dead)
			return;

	}
}

struct BuildingRadiationExposure
{
	int Damage;
	int BestRadLevel;
	double BestDistance;
	CellStruct SourceCell;
};

static NOINLINE BuildingRadiationExposure CalculateBuildingRadiationDamage(
	RadSiteClass* pRad,
	RadTypeClass* pRadType,
	BuildingClass* pBld
)
{
	BuildingRadiationExposure result {};

	const auto baseCell = pRad->BaseCell;
	const auto nCurCoord = pBld->InlineMapCoords();

	for (auto* pFoundation = pBld->GetFoundationData(false); *pFoundation != CellStruct::EOL; ++pFoundation)
	{

		const auto nLoc = nCurCoord + (*pFoundation);
		auto pCell = MapClass::Instance->TryGetCellAt(nLoc);

		if (!pCell) continue;

		auto* pCellExt = CellExtContainer::Instance.Find(pCell);

		auto it = pCellExt->RadLevels.find_if([pRad](auto& pair)
 {
	 return pair.Rad == pRad;
		});

		if (it == pCellExt->RadLevels.end() || it->Level <= 0)
			continue;

		const int radLevel = it->Level;
		double distance = static_cast<double>(baseCell.DistanceFrom(nLoc));

		// Use best exposed cell logic (max level or min distance)
		if (radLevel > result.BestRadLevel || result.BestRadLevel == 0)
		{
			result.BestRadLevel = radLevel;
			result.BestDistance = distance;
			result.SourceCell = nLoc;
		}
	}

	if (result.BestRadLevel > 0)
	{
		result.Damage = CalculateRadiationDamage(result.BestRadLevel, pRadType->GetLevelFactor(), result.BestDistance, pRad->Spread);
	}

	return result;
}

static NOINLINE void ApplyRadDamage(RadSiteClass* pRad, BuildingClass* pObj, CellClass* pCell, int distance)
{
	if (pObj->IsAlive && !pObj->InLimbo && pObj->Health > 0 && !pObj->TemporalTargetingMe && !TechnoExtData::IsRadImmune(pObj))
	{

		const auto pRadExt = RadSiteExtContainer::Instance.Find(pRad);

		RadTypeClass* pRadType = pRadExt->Type;

		const int delay = RulesExtData::Instance()->UseGlobalRadApplicationDelay ? pRadType->GetBuildingApplicationDelay() : RulesExtData::Instance()->RadApplicationDelay_Building;

		if ((delay <= 0) || (Unsorted::CurrentFrame % delay))
			return;

		if (pObj->GetTechnoType()->Immune || !pRadType->GetWarhead() || pObj->IsBeingWarpedOut())
			return;

		auto& damageCount = pRadExt->damageCounts[pObj];
		const int maxDamageCount = pRadType->GetBuildingDamageMaxCount();
		if (maxDamageCount > 0 && damageCount >= maxDamageCount)
			return;

		const auto damage = CalculateBuildingRadiationDamage(pRad, pRadType, pObj);

		if (damage.Damage == 0)
			return;

		if (pRadExt->ApplyRadiationDamage(pObj, damage.Damage, static_cast<int>(damage.BestDistance)) == RadSiteExtData::DamagingState::Dead)
			return;
	}
}

template<typename Func>
void FakeRadSiteClass::ForEachCellInRadiationArea(Func&& callback)
{
	// Calculate the bounding box for the radiation area
	short spread = this->Spread;
	short minX = this->BaseCell.X - spread;
	short maxX = minX + (2 * spread) + 1;
	short minY = this->BaseCell.Y - spread;
	short maxY = minY + (2 * spread) + 1;

	// Get the center cell for distance calculations
	CellClass* centerCell = MapClass::Instance->GetCellAt(this->BaseCell);
	CoordStruct centerCoord = centerCell->GetCoords();

	// Iterate through all cells in the radiation area
	for (short y = minY; y < maxY; ++y) {
		for (short x = minX; x < maxX; ++x) {
			CellStruct currentCell(x,y);

			// Get the current cell
			CellClass* cell = MapClass::Instance->GetCellAt(currentCell);

			// Get cell center coordinate
			CoordStruct cellCoord = cell->GetCoords();

			// Calculate 3D distance between radiation center and current cell
			int distance = int((centerCoord - cellCoord).Length());

			// Calculate base radiation strength at this distance
			double radiationAmount = 0.0;

			if (distance <= this->SpreadInLeptons)
			{
				// Radiation decreases linearly with distance from center
				int distanceFromEdge = this->SpreadInLeptons - distance;
				double normalizedDistance = static_cast<double>(distanceFromEdge) / this->SpreadInLeptons;
				radiationAmount = normalizedDistance * this->RadLevel;
			}

			// Call the callback with the cell and calculated radiation amount
			callback(cell, radiationAmount, distance);
		}
	}
}

void FakeRadSiteClass::__Reduce_In_Area()
{
	// Calculate reduction multiplier based on frames elapsed
	auto pExt = this->_GetExtData();
	int reductionMultiplier = (this->RadTimeLeft / pExt->Type->GetLevelDelay()) + 1;

	ForEachCellInRadiationArea([this, reductionMultiplier](CellClass* cell, double radiationAmount, int distance) {
		// Apply reduction multiplier and current level decrement

		if (radiationAmount <= 0)
			radiationAmount = 1;

		double reductionAmount = (radiationAmount / this->LevelSteps) * reductionMultiplier;
		PopulateCellRadVector<true>(this, &cell->MapCoords, distance, reductionMultiplier);
		cell->RadLevel_Decrease(reductionAmount);
	});
}

void FakeRadSiteClass::__Increase_In_Area()
{
	ForEachCellInRadiationArea([this](CellClass* cell, double radiationAmount, int distance) {
		// Simply increase radiation by the calculated amount
		PopulateCellRadVector<false>(this, &cell->MapCoords, distance, 0);
		cell->RadLevel_Increase(radiationAmount);
	});
}

void FakeRadSiteClass::__Reduce_Radiation() {
	ForEachCellInRadiationArea([this](CellClass* cell, double radiationAmount, int distance) {
		// Apply current level decrement to calculate reduction amount
		// This makes radiation fade faster as time goes on
		PopulateCellRadVector<true>(this, &cell->MapCoords, distance, 0);
		double reductionAmount = radiationAmount / this->LevelSteps;
		cell->RadLevel_Decrease(reductionAmount);
	});
}

// Radiation_At remains unchanged as it has different logic
double FakeRadSiteClass::__Radiation_At(CellStruct* cell) const
{
	// Get the radiation center cell
	CellClass* centerCell = MapClass::Instance->GetCellAt(this->BaseCell);
	CoordStruct centerCoord = centerCell->GetCoords();

	// Get the query cell
	CellClass* queryCell = MapClass::Instance->GetCellAt(cell);
	CoordStruct queryCoord = queryCell->GetCoords();

	// Calculate 3D distance between radiation center and query cell
	const int distance = (centerCoord - queryCoord).Length();

	// Check if cell is within radiation range
	if (distance > this->SpreadInLeptons)
	{
		return 0.0; // No radiation outside the spread radius
	}

	// Calculate radiation strength at this distance
	// Radiation decreases linearly with distance from center
	int distanceFromEdge = this->SpreadInLeptons - distance;
	double normalizedDistance = static_cast<double>(distanceFromEdge) / this->SpreadInLeptons;

	return normalizedDistance * this->RadLevel;
}

void FakeRadSiteClass::__AI()
{
	--this->RadTimeLeft;
	auto pExt = RadSiteExtContainer::Instance.Find(this);

	// Check radiation level reduction timer
	// Time to reduce radiation
	if (this->RadLevelTimer.GetTimeLeft() == 0)
	{
		this->DecreaseRadiation();

		// Reset radiation level timer
		this->RadLevelTimer.Start(pExt->Type->GetLevelDelay());
	}

	// Check radiation light update timer
	// Time to update radiation light
	if (this->RadLightTimer.GetTimeLeft() == 0)
	{
		// Calculate current light intensity based on remaining radiation
		// Light fades proportionally as radiation decays
		TintStruct tintIntensity(
			this->RadTimeLeft * this->Tint.Red / this->RadDuration,
			this->RadTimeLeft * this->Tint.Green / this->RadDuration,
			this->RadTimeLeft * this->Tint.Blue / this->RadDuration
		);

		// Update light source with new color values
		// CurrentLightStage is subtracted from the base color
		this->LightSource->ChangeLevels(
			this->LightSource->LightIntensity - this->IntensityDecrement,
			tintIntensity, 0);

		// Reset radiation light timer
		this->RadLightTimer.Start(pExt->Type->GetLightDelay());
	}

	// Self-destruct when radiation is fully depleted
	if (this->RadTimeLeft <= 0 || this->RadLevel <= 0) {
		// Call destructor and deallocate (the 1 parameter means delete memory)
		this->_scalar_dtor(1);
	}

	ForEachCellInRadiationArea([this](CellClass* pCell, double radiationAmount, int distance) {
		if (auto pObj = pCell->Cell_Occupier()) {
			if (auto pFoot = flag_cast_to<FootClass*, false>(pObj))
				ApplyRadDamage(this, pFoot, pCell, distance);
			else if (auto pBld = cast_to<BuildingClass*, false>(pObj))
				ApplyRadDamage(this, pBld, pCell, distance);
		}
	});
}