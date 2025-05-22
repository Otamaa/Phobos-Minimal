#include "Body.h"
#include <Utilities/Macro.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/AnimType/Body.h>

#include <Misc/Hooks.Otamaa.h>
#include <Ext/House/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/Cell/Body.h>
#include <Utilities/AnimHelpers.h>
#include <Utilities/Helpers.h>

#include <Misc/Ares/Hooks/Header.h>

#include <SmudgeClass.h>
#include <SmudgeTypeClass.h>

#include <Memory.h>


ASMJIT_PATCH(0x685078, Generate_OreTwinkle_Anims, 0x7)
{
	GET(FakeCellClass* const, location, ESI);

	const int tib_idx = location->_GetTiberiumType();
	const int value = tib_idx == -1 ? 0 : TiberiumClass::Array->Items[tib_idx]->Value;

	if (value > 0)
	{
		const auto pTibExt = ((FakeTiberiumClass*)TiberiumClass::Array->Items[tib_idx])->_GetExtData();

		if (!ScenarioClass::Instance->Random.RandomFromMax(pTibExt->GetTwinkleChance() - 1))
		{
			if (auto pAnimtype = pTibExt->GetTwinkleAnim())
			{
				GameCreate<AnimClass>(pAnimtype, location->GetCoords(), 1);
			}
		}
	}

	return 0x6850E5;
}


/// replae this entirely since the function using lea for getting int and seems broke everyone else stacks
void NOINLINE FakeAnimClass::_Middle()
{
	auto center = this->GetCoords();
	auto centercell = MapClass::Instance->GetCellAt(center);
	int width = 30;
	int height = 30;
	if (auto pSHP = this->Type->GetImage())
	{
		RectangleStruct bounds {};
		if (this->Type->MiddleFrameWidth == -1 || this->Type->MiddleFrameHeight == -1)
			bounds = pSHP->GetFrameBounds(this->Type->MiddleFrameIndex);

		if (this->Type->MiddleFrameWidth == -1)
		{
			this->Type->MiddleFrameWidth = bounds.Width;
		}

		if (this->Type->MiddleFrameHeight == -1)
		{
			this->Type->MiddleFrameHeight = bounds.Height;
		}

		width = this->Type->MiddleFrameWidth;
		height = this->Type->MiddleFrameHeight;
	}

	AnimExtData::OnMiddle(this);
	AnimExtData::OnMiddle_SpawnSmudge(this, centercell, { width ,height });
	this->_GetExtData()->OnMiddle();
}

void NOINLINE FakeAnimClass::_Start()
{
	this->Mark(MarkType::Change);

	auto pTypeExt = this->_GetTypeExtData();
	auto _gCoords = this->GetCoords();

	if (pTypeExt->AdditionalHeight > 0)
		_gCoords.Z += pTypeExt->AdditionalHeight;

	if (pTypeExt->DetachedReport.isset())
	{
		VocClass::PlayAt(pTypeExt->DetachedReport.Get(), _gCoords);
	}

	if (pTypeExt->AltReport.isset())
	{
		VocClass::PlayIndexAtPos(pTypeExt->AltReport, _gCoords, nullptr);
	}

	if (!this->IsPlaying && this->Type->Report != -1)
	{
		VocClass::PlayIndexAtPos(this->Type->Report, _gCoords, &this->Audio3);
	}
	else
	{
		this->Audio3.AudioEventHandleStop();
	}

	this->Audio4.AudioEventHandleStop();

	if (!this->Type->MiddleFrameIndex)
	{
		this->_Middle();
	}

	if (!this->IsPlaying && this->Type->TiberiumChainReaction)
	{
		auto gCoords = this->GetCoords();
		CellClass* cptr = MapClass::Instance->GetCellAt(gCoords);
		int tib = cptr->GetContainedTiberiumIndex();

		if (tib != -1)
		{
			TiberiumClass* tiberium = TiberiumClass::Array->Items[tib];
			auto pExt = TiberiumExtContainer::Instance.Find(tiberium);

			cptr->ReduceTiberium(cptr->OverlayData + 1);


			if (tiberium->Debris.size() > 0)
			{
				int chance = pExt->GetDebrisChance();
				if (ScenarioClass::Instance->Random.RandomFromMax(99) < chance)
				{
					auto SpawnLoc = gCoords;
					SpawnLoc.Z += 10;

					auto pSpawn = GameCreate<AnimClass>(tiberium->Debris[ScenarioClass::Instance->Random.RandomFromMax(tiberium->Debris.size() - 1)], SpawnLoc);
					pSpawn->LightConvert = ColorScheme::Array->Items[tiberium->Color]->LightConvert;
					pSpawn->TintColor = cptr->Intensity_Normal;
				}
			}

			int damage = pExt->GetExplosionDamage();
			auto pWarhead = pExt->GetExplosionWarhead();

			DamageArea::Apply(&gCoords, damage, nullptr, pWarhead, false, nullptr);

			cptr->RecalcAttributes(-1);
			MapClass::Instance->ResetZones(cptr->MapCoords);
			MapClass::Instance->RecalculateSubZones(cptr->MapCoords);
		}
	}

	this->_GetExtData()->OnStart();
}

bool __fastcall Is_Visible_To_Psychic(HouseClass* house, CellClass* cell)
{
	JMP_STD(0x43B4C0);
}

void NOINLINE AnimExtData::OnTypeChange()
{
	const auto pTypeExt = AnimTypeExtContainer::Instance.Find(this->AttachedToObject->Type);

	if (this->AttachedSystem && this->AttachedSystem->Type != pTypeExt->AttachedSystem.Get())
		this->AttachedSystem = nullptr;

	if (!this->AttachedSystem && pTypeExt->AttachedSystem)
		this->CreateAttachedSystem();
}

#include <GameOptionsClass.h>
#include <OverlayClass.h>
#include <OverlayTypeClass.h>
#include <TacticalClass.h>

void NOINLINE FakeAnimClass::_ApplySpawns(CoordStruct& nCoord)
{
	if (!this->Type->Spawns || this->Type->SpawnCount <= 0)
		return;

	const auto nMax = ScenarioClass::Instance->Random.RandomFromMax((this->Type->SpawnCount)) +
		ScenarioClass::Instance->Random.RandomFromMax((this->Type->SpawnCount));

	if (nMax <= 0)
		return;

	const auto pAnimTypeExt = this->_GetTypeExtData();
	TechnoClass* pTech = AnimExtData::GetTechnoInvoker(this);
	HouseClass* pOwner = this->Owner ? this->Owner : pTech ? pTech->GetOwningHouse() : nullptr;
	auto nDelay = pAnimTypeExt->Spawns_Delay.Get();

	for (int i = nMax; i > 0; --i)
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(this->Type->Spawns, nCoord, nDelay, 1, AnimFlag(0x600), 0, false),
		pOwner,
		nullptr,
		pTech,
		false
		);
	}
}

void NOINLINE FakeAnimClass::_ApplyVeinsDamage()
{
	if (this->Type->IsVeins && RulesExtData::Instance()->Veinhole_Warhead && RulesExtData::Instance()->VeinsAttack_interval)
	{
		auto coord = this->GetCoords();
		auto pCoorCell = MapClass::Instance->GetCellAt(coord);
		auto pFirst = pCoorCell->FirstObject;

		if (!pFirst
			|| pFirst->GetHeight() > 0
			|| pCoorCell->OverlayTypeIndex != 126
			|| pCoorCell->OverlayData < 0x30u
			|| pCoorCell->SlopeIndex
			)
		{
			this->__ToDelete_197 = true; // wut
			return;
		}

		if (Unsorted::CurrentFrame % RulesExtData::Instance()->VeinsAttack_interval == 0)
		{
			while (pFirst != nullptr)
			{
				ObjectClass* pNext = pFirst->NextObject;

				if (auto pTechno = flag_cast_to<TechnoClass*, false>(pFirst))
				{
					const auto pType = pTechno->GetTechnoType();
					if (!TechnoTypeExtContainer::Instance.Find(pType)->IsDummy && pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo)
					{
						if (pTechno->WhatAmI() != UnitClass::AbsID || ((UnitClass*)pTechno)->DeathFrameCounter <= 0)
						{
							if ((!RulesExtData::Instance()->VeinsDamagingWeightTreshold.isset() || pType->Weight >= RulesExtData::Instance()->VeinsDamagingWeightTreshold)
								&& !pType->ImmuneToVeins
								&& !pTechno->HasAbility(AbilityType::VeinProof)
								&& pTechno->GetHeight() <= 5
								)
							{
								int dmg = RulesClass::Instance->VeinDamage;
								pFirst->ReceiveDamage(&dmg, 0, RulesExtData::Instance()->Veinhole_Warhead, nullptr, false, false, nullptr);
							}
						}
					}
				}

				pFirst = pNext;
			}
		}
	}
}

void FakeAnimClass::_ApplyDeformTerrrain()
{

}

void NOINLINE FakeAnimClass::_SpreadTiberium(CoordStruct& coords, bool isOnbridge)
{
	RectangleStruct updaterect(0, 0, 0, 0);

	if (this->Type->IsTiberium && !isOnbridge && this->Type->TiberiumSpawnType) {
		const int _radius = this->Type->TiberiumSpreadRadius;

		for (int x = -_radius; x <= _radius; ++x) {
			for (int y = -_radius; y <= _radius; y++) {
				if ((int)Math::sqrt((double)x * (double)x + (double)y * (double)y) <= _radius) {
					CellClass* cellptr = MapClass::Instance->GetCellAt(CellClass::Coord2Cell(coords) + CellSpread::AdjacentCell[(int)x & 7]);

					if (cellptr->CanTiberiumGerminate(nullptr)) {
						if (auto pType = OverlayTypeClass::Array->GetItemOrDefault(this->Type->TiberiumSpawnType->ArrayIndex + ScenarioClass::Instance->Random.RandomFromMax(3))) {

							if (!pType->Tiberium)
								return; // not valid

							GameCreate<OverlayClass>(
								pType,
								cellptr->MapCoords,
								-1);

							cellptr->OverlayData = ScenarioClass::Instance->Random.RandomFromMax(2);
							RectangleStruct overlayrect = cellptr->GetOverlayShapeRect();
							overlayrect.Y -= TacticalClass::view_bound->Y;
							updaterect = RectangleStruct::Union(updaterect, overlayrect);
						}
						else {
							return; // not valid
						}
					}
				}
			}
		}

		TacticalClass::Instance->RegisterDirtyArea(updaterect, false);
	}
}

void NOINLINE FakeAnimClass::_PlayExtraAnims(bool onWater, bool onBridge)
{
	auto coords = this->GetCoords();

	if (!onWater || onBridge)
	{
		if (this->Type->ExpireAnim)
		{
			GameCreate<AnimClass>(this->Type->ExpireAnim,
				coords,
				0,
				1,
				AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000,
				-30
			);
		}

		auto coord_ = this->Bounce.GetCoords();
		DamageArea::Apply(&coord_, this->Type->Damage, nullptr, this->Type->Warhead, true, nullptr);
		MapClass::FlashbangWarheadAt(this->Type->Damage, this->Type->Warhead, coord_);
	}
	else if (this->Type->IsMeteor)
	{
		GameCreate<AnimClass>(RulesClass::Instance->SplashList[0],
			coords,
			0,
			1,
			AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200,
			0
		);
	}
	else
	{
		GameCreate<AnimClass>(RulesClass::Instance->Wake,
			coords,
			0,
			1,
			AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200,
			0
		);

		CoordStruct _splashCoord = coords;
		_splashCoord.Z += 3;
		GameCreate<AnimClass>(RulesClass::Instance->SplashList[0],
			_splashCoord,
			0,
			1,
			AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200,
			0
		);
	}
}

void NOINLINE FakeAnimClass::_DrawTrailerAnim() {
	if (this->Type->TrailerAnim && this->Type->TrailerSeperation >= 1) {

		CoordStruct _coord = this->GetCoords();
		if (!_coord.IsValid()) //yeah , fuck , lagging the game
			return;

		TechnoClass* const pTech = AnimExtData::GetTechnoInvoker(this);
		HouseClass* const pOwner = !this->Owner && pTech ? this->Owner : this->Owner;

		if(this->Type->TrailerSeperation == 1 || !(Unsorted::CurrentFrame() % this->Type->TrailerSeperation)) {

			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(this->Type->TrailerAnim, _coord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0), pOwner, nullptr, pTech, false);
		}
	}
}

void NOINLINE FakeAnimClass::_ApplyHideIfNoOre()
{
	if (!this->Type->HideIfNoOre)
		return;

	auto pCell = MapClass::Instance->GetCellAt(this->GetCoords());
	this->Invisible = !pCell || pCell->GetContainedTiberiumValue()
		<= Math::abs(this->_GetTypeExtData()->HideIfNoOre_Threshold.Get());
}

void NOINLINE FakeAnimClass::_CreateFootApplyOccupyBits()
{

	if (this->Type->MakeInfantry != -1) {
		if (!this->Location.IsValid())
			return;

		this->MarkAllOccupationBits(this->Location);
		this->_GetExtData()->CreateUnitLocation = this->Location;
	}
	else
	{
		AnimTypeExtData::CreateUnit_MarkCell(this);
	}
}

void NOINLINE FakeAnimClass::_CreateFoot()
{
	if (this->Type->MakeInfantry != -1)
	{
		if (!this->Location.IsValid())
			return;

		this->UnmarkAllOccupationBits(this->_GetExtData()->CreateUnitLocation);

		if (this->Location != this->_GetExtData()->CreateUnitLocation)
			return;

		const auto& toInf = RulesClass::Instance->AnimToInfantry;

		if ((size_t)this->Type->MakeInfantry < toInf.size()) {


			if (HouseClass* pInfOwner = !this->Owner || this->Owner->Defeated ?
				HouseExtData::FindFirstCivilianHouse() : this->Owner) {

				auto pInf = (InfantryClass*)toInf.Items[this->Type->MakeInfantry]->CreateObject(pInfOwner);

				if (!pInf->Unlimbo(this->Location, DirType::SouthEast))
				{
					--this->Animation.Stage;
					pInf->UnInit();
					return;
				}

				auto pAnimCoords = this->GetCoords();
				auto pCell = MapClass::Instance->GetCellAt(pAnimCoords);

				if(pCell->ContainsBridge()) {
					if (this->Location.Z > pAnimCoords.Z + Unsorted::BridgeHeight) {
						pInf->Mark(MarkType::Remove);
						pInf->OnBridge = true;
						pInf->Mark(MarkType::Put);
					}
				}

				if (!pInfOwner->Type->MultiplayPassive)
				{
					const auto Is_AI = !pInfOwner->IsControlledByHuman();

					if (!this->_GetTypeExtData()->ScatterAnimToInfantry(Is_AI))
						pInf->QueueMission(this->_GetTypeExtData()->GetAnimToInfantryMission(Is_AI), false);
					else
						pInf->Scatter(CoordStruct::Empty, true, false);
				}
			}
		}
	}
	else
	{
		AnimTypeExtData::CreateUnit_Spawn(this);
	}
}

bool NOINLINE PingPong(FakeAnimClass* pThis) {
	if (pThis->Type->PingPong)
	{
		if ((pThis->RemainingIterations <= 1 && (pThis->Animation.Stage >= pThis->Type->End || pThis->Animation.Stage == 0))
			|| (pThis->RemainingIterations > 1 && (pThis->Animation.Stage >= pThis->Type->LoopEnd - pThis->Type->Start || pThis->Animation.Stage == pThis->Type->Start))
			)
		{
			pThis->Animation.Step = -pThis->Animation.Step;
			return true;
		}
	}

	return false;
}

bool NOINLINE StageLoops(FakeAnimClass* pThis)
{
	const auto remaining = pThis->RemainingIterations;
	if ((remaining > 1u && pThis->Animation.Stage >= pThis->Type->LoopEnd - pThis->Type->Start)
		|| (remaining <= 1u && pThis->Animation.Stage >= pThis->Type->End))
	{
		return false;
	}

	return true;
}

bool NOINLINE ReverseAndShadow(FakeAnimClass* pThis) {

	if ((!pThis->Type->Shadow && (!pThis->Type->Reverse && !pThis->Reverse || pThis->Animation.Stage > 0))
		|| (pThis->Type->Reverse && pThis->Animation.Stage > 0)
		|| (pThis->Reverse || pThis->Animation.Stage < pThis->Type->LoopEnd - pThis->Type->Start) && (
			!pThis->Type->Reverse && !pThis->Reverse || pThis->Animation.Stage > 0))
	{
			return true;
	}

	return false;
}

void NOINLINE SwapType(FakeAnimClass* pThis, AnimTypeClass* pNewType) {
	pThis->Type = pNewType;
}

bool NOINLINE UpdateLoopDelay(FakeAnimClass* pThis) {
	if (pThis->LoopDelay > 0) {
		const int delay = pThis->LoopDelay - 1;
		pThis->LoopDelay = delay;
		if (delay == 0)
			pThis->_Start();

		return true;
	}

	return false;
}

void FakeAnimClass::_AI()
{
	if (this->IsAlive) {

		if (!this->GetCoords().IsValid() || !MapClass::Instance->IsValid(this->GetCoords())) {
			this->__ToDelete_197 = true;
			this->TimeToDie = true;
			this->UnInit();
			return;
		}

		if (!this->IsPlaying && this->Type->Report != -1) {
			VocClass::PlayIndexAtPos(this->Type->Report, this->GetCoords(), &this->Audio3);
		}

		if (this->Type->IsFlamingGuy) {
			this->FlamingGuy_AI();
			this->ObjectClass::Update();
		} else {
			TechnoExt_ExtData::UpdateAlphaShape(this);
		}

		if (this->Type->PsiWarning) {
			this->Invisible = !Is_Visible_To_Psychic(this->Owner, MapClass::Instance->GetCellAt(this->GetCoords()));
		}

		if (this->Type == RulesClass::Instance->Behind) {
			this->Invisible = !GameOptionsClass::Instance->ShowHidden;
		}

		this->_ApplyHideIfNoOre();
		this->_CreateFootApplyOccupyBits();

		if (this->Unpaused && this->PausedAnimFrame == this->Animation.Stage) {
			this->Unpaused = false;
		}

		if (this->HasExtras) {
			const auto bounceStatus = this->AnimExtras();

			if (bounceStatus == BounceClass::Status::Impact || bounceStatus == BounceClass::Status::Bounce)
			{
				auto pAnimCoords = this->GetCoords();
				auto pAnimCell = MapClass::Instance->GetCellAt(pAnimCoords);
				const bool water = pAnimCell->LandType == LandType::Water;
				const bool isBridge = pAnimCoords.Z >= CellClass::BridgeHeight + MapClass::Instance->GetCellFloorHeight(this->Location);

				AnimExtData::OnExpired(this, water, isBridge);

				if (!water || isBridge)
				{
					auto _bounceCoords = this->Bounce.GetCoords();
					this->_ApplySpawns(_bounceCoords);
					this->_ApplyDeformTerrrain();
					this->_SpreadTiberium(_bounceCoords, isBridge);
				}

				this->TimeToDie = true;
				this->UnInit();
				return;
			}
		}

		if (this->IsAlive && !this->__ToDelete_197) {
			this->_DrawTrailerAnim();
		}

		if (this->Type == RulesClass::Instance->DropZoneAnim) {
			this->__ToDelete_197 = MapClass::Instance->GetCellAt(this->GetCoords())->GetBuilding() != nullptr;
		}

		if (this->__ToDelete_197) {
			this->TimeToDie = true;
			this->UnInit();
			return;
		}

		if (this->SkipProcessOnce) {
			this->SkipProcessOnce = false;
			return;
		}

		if (UpdateLoopDelay(this)) {
			return;
		}

		if (this->IsAlive)
		{
			if (this->Type->IsVeins) {
				this->_ApplyVeinsDamage();
			}

			if (this->Type->IsAnimatedTiberium) {
				auto _animated_coords = this->GetCoords() - CoordStruct(384, 384, 0);
				auto pCell = MapClass::Instance->GetCellAt(_animated_coords);

				if (pCell->OverlayTypeIndex == -1 || OverlayTypeClass::Array->Items[pCell->OverlayTypeIndex]->CellAnim != this->Type) {
					this->__ToDelete_197 = true;
				}
			}

			if (this->Type->End == -1)
			{
				if(auto pImage = this->Type->GetImage())
					this->Type->End = pImage->Frames;

				if (this->Type->Shadow)
					this->Type->End /= 2;
			}

			if (this->Type->LoopEnd == -1)
			{
				this->Type->LoopEnd = this->Type->End;
			}

			this->Mark(MarkType::Change);

			if (!this->PowerOff && !this->Paused)
			{
				if (!this->Animation.Update())
					return;

				if (this->Type->Damage > 0.0 && !this->HasExtras && !this->InLimbo) {
					AnimExtData::DealDamageDelay(this);
					if (!this->IsAlive)
						return;
				}

				if (this->Type->MiddleFrameIndex
					&& this->Animation.Stage + this->Type->Start == this->Type->MiddleFrameIndex
					&& !this->HasExtras) {
					this->_Middle();
				}

				if (PingPong(this)) {
					return;
				}

				if (StageLoops(this)){
					if (ReverseAndShadow(this)) {
						return;
					}
				}

				if (this->RemainingIterations && this->RemainingIterations != UCHAR_MAX) this->RemainingIterations--;
				if (this->RemainingIterations)
				{
					if (this->Type->Reverse || this->Reverse)
					{
						this->Animation.Stage = this->Type->LoopEnd;
					}
					else
					{
						this->Animation.Stage = this->Type->LoopStart - this->Type->Start;
					}

					if (this->Type->RandomLoopDelay.IsValid())
					{
						this->LoopDelay = ScenarioClass::Instance->Random.RandomRanged(this->Type->RandomLoopDelay);
					}

					return;
				}

				this->_GetExtData()->OnEnd();

				if (const auto pNext = this->Type->Next)
				{
					SwapType(this, pNext);

					this->_GetExtData()->OnTypeChange();

					if (this->Type->End == -1)
					{
						if (auto pImage = this->Type->GetImage())
							this->Type->End = pImage->Frames;

						if (this->Type->Shadow)
							this->Type->End /= 2;
					}

					if (this->Type->LoopEnd == -1)
					{
						this->Type->LoopEnd = this->Type->End;
					}

					this->__ToDelete_197 = false;
					this->RemainingIterations = this->Type->LoopCount;
					this->Accum = 0.0;
					int delay = this->Type->Rate;
					if (this->Type->RandomRate.IsValid())
					{
						delay = ScenarioClass::Instance->Random.RandomRanged(this->Type->RandomRate);
					}

					if (this->Type->Normalized)
					{
						delay = GameOptionsClass::Instance->GetAnimSpeed(delay);
					}

					this->Animation.Start(delay);
					this->Animation.Stage = this->Type->Start;
					if(this->_GetTypeExtData()->Damaging_UseSeparateState){
						int damagedelay = this->_GetTypeExtData()->Damaging_Rate == -1 ? this->Animation.Step : this->_GetTypeExtData()->Damaging_Rate;
						this->_GetExtData()->DamagingState.Start(damagedelay);
					}
					this->_Start();
					return;
				}

				this->_CreateFoot();

				this->TimeToDie = true;
				this->UnInit();
			}
		}
	}
}

int FakeAnimClass::_BounceAI()
{
	BounceClass* pBounce = &this->Bounce;
	this->_GetExtData()->CreateAttachedSystem();

	const BounceClass::Status status  = pBounce->Update();

	if (this->Type->IsMeteor) {
		pBounce->Velocity.Z += pBounce->Gravity;
	}

	auto _coord = pBounce->GetCoords();

	if (status == BounceClass::Status::Bounce)
	{
		TechnoClass* pTechnoInvoker = AnimExtData::GetTechnoInvoker(this);

		if (auto pBounceAnim = this->Type->BounceAnim)
		{
			HouseClass* pHouse = this->Owner ? this->Owner : (pTechnoInvoker ? pTechnoInvoker->GetOwningHouse() : nullptr);
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pBounceAnim, _coord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0),
				pHouse,
				nullptr,
				pTechnoInvoker,
				false
			);
		}

		if (this->Type->DamageRadius < 0 || CLOSE_ENOUGH(this->Type->Damage, 0.0) || !this->Type->Warhead) {
			this->SetLocation(_coord);
			return (int)status;
		}

		const auto pCell = MapClass::Instance->GetCellAt(_coord);

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject) {
			auto _objCoord = pObject->GetCoords();
			const int nDist = Math::abs(_coord.Y - _objCoord.Y) + Math::abs(_coord.X - _objCoord.X);

			if (nDist <= this->Type->DamageRadius) {
				auto nDamage = (int)this->Type->Damage;
				pObject->ReceiveDamage(&nDamage, Game::AdjustHeight(nDist), this->Type->Warhead,
							  pTechnoInvoker, false, false, pTechnoInvoker ? pTechnoInvoker->Owner : this->Owner);
			}
		}
	}
	else if (status == BounceClass::Status::Impact) {
		this->UnInit();
	}

	this->SetLocation(_coord);
	return (int)status;
}

DEFINE_FUNCTION_JUMP(CALL, 0x424D5A, FakeAnimClass::_Middle);
DEFINE_FUNCTION_JUMP(CALL, 0x424687, FakeAnimClass::_Middle);
DEFINE_FUNCTION_JUMP(LJMP, 0x424F00, FakeAnimClass::_Middle);

DEFINE_FUNCTION_JUMP(CALL, 0x422702, FakeAnimClass::_Start);
DEFINE_FUNCTION_JUMP(CALL, 0x4243A1, FakeAnimClass::_Start);
DEFINE_FUNCTION_JUMP(CALL, 0x424925, FakeAnimClass::_Start);
DEFINE_FUNCTION_JUMP(LJMP, 0x424CE0, FakeAnimClass::_Start);

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E33B0, FakeAnimClass::_AI);
DEFINE_FUNCTION_JUMP(LJMP, 0x423AC0, FakeAnimClass::_AI);

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E353C, FakeAnimClass::_BounceAI);
DEFINE_FUNCTION_JUMP(LJMP, 0x423930, FakeAnimClass::_BounceAI);

ASMJIT_PATCH(0x42264D, AnimClass_Init, 0x5)
{
	GET(AnimClass*, pThis, ESI);
	GET_BASE(CoordStruct*, pCoord, 0xC);

	AnimExtData::OnInit(pThis, pCoord);

	return 0x0;
}

#ifdef ENABLE_NEWHOOKS

ASMJIT_PATCH(0x4242BA, AnimClass_AI_SetCoord, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (auto const pExt = AnimExtData::ExtMap.Find(pThis))
	{
		if (pExt->Something)
		{
			auto nCoord = pThis->GetCoords() + pExt->Something;
			pThis->SetLocation(nCoord);
		}
	}
	return 0x0;
}

ASMJIT_PATCH(0x422CC6, AnimClass_DrawIT_SpecialDraw, 0xA)
{
	GET(AnimClass* const, pThis, ESI);

	if (auto const pTypeExt = AnimTypeExtContainer::Instance.TryFind(pThis->Type))
	{
		R->AL(pTypeExt->SpecialDraw.Get());
		return 0x422CD0;
	}

	return 0x0;
}

#endif
//
//bool FakeInfantryClass::_Unlimbo(const CoordStruct& coords, DirType dir) {
//
//	if (!MapClass::Instance->GetCellAt(coords)->IsClearToMove(this->Type->SpeedType, this->Type->MovementZone))
//		return false;
//
//	return this->InfantryClass::Unlimbo(coords, dir);
//}
//
//DEFINE_FUNCTION _JUMP(CALL6, 0x4431C5, FakeInfantryClass::_Unlimbo);