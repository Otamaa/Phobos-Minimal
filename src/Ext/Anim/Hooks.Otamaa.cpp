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
#include <Ext/WeaponType/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/Cell/Body.h>
#include <Utilities/AnimHelpers.h>
#include <Utilities/Helpers.h>

#include <Misc/Ares/Hooks/Header.h>

#include <SmudgeClass.h>
#include <SmudgeTypeClass.h>

#include <Memory.h>

void ApplyVeinsDamage(AnimClass* pThis ,int VeinDamage , WarheadTypeClass* VeinWarhead)
{
	//causing game to lock up atm
	if (pThis->Type->IsVeins && VeinWarhead  &&  RulesExtData::Instance()->VeinsAttack_interval)
	{
		auto coord = pThis->GetCoords();
		auto pCoorCell = MapClass::Instance->GetCellAt(coord);
		auto pFirst = pCoorCell->FirstObject;

		if (!pFirst
			|| pFirst->GetHeight() > 0
			|| pCoorCell->OverlayTypeIndex != 126
			|| pCoorCell->OverlayData < 0x30u
			|| pCoorCell->SlopeIndex
			)  {
			pThis->__ToDelete_197 = true; // wut
		}

		if (Unsorted::CurrentFrame % RulesExtData::Instance()->VeinsAttack_interval == 0)  {
			while (pFirst != nullptr)
			{
				ObjectClass* pNext = pFirst->NextObject;

				if (auto pTechno = flag_cast_to<TechnoClass* , false>(pFirst)) {
					const auto pType = pTechno->GetTechnoType();
					if (!TechnoTypeExtContainer::Instance.Find(pType)->IsDummy &&pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo){
						if (pTechno->WhatAmI() != UnitClass::AbsID || ((UnitClass*)pTechno)->DeathFrameCounter <= 0)   {
							if ( (!RulesExtData::Instance()->VeinsDamagingWeightTreshold.isset() || pType->Weight >= RulesExtData::Instance()->VeinsDamagingWeightTreshold)
								&& !pType->ImmuneToVeins
								&& !pTechno->HasAbility(AbilityType::VeinProof)
								&& pTechno->GetHeight() <= 5
								)  {
								int dmg = VeinDamage;
								pFirst->ReceiveDamage(&dmg, 0, VeinWarhead, nullptr, false, false, nullptr);
							}
						}
					}
				}

				pFirst = pNext;
			}
		}
	}
}

// ASMJIT_PATCH(0x424cfb, AnimClass_Init_Additionals, 6)
// {
// 	GET(FakeAnimClass*, pThis, ESI);
//
// 	auto const pTypeExt = pThis->_GetTypeExtData();
//
// /	if (pTypeExt->AltReport.isset()) {
// 		VocClass::PlayIndexAtPos(pTypeExt->AltReport, pThis->GetCoords(), nullptr);
// 	}
//
// 	if (pTypeExt->AdditionalHeight > 0)
// 		pThis->Location.Z += pTypeExt->AdditionalHeight;
//
// 	return 0;
// }

 ASMJIT_PATCH(0x4243BC, AnimClass_AI_Veins, 0x6)
 {
 	enum {
 		ContinueDrawTiberium = 0x4243CC,
 		ContinueNotTiberium = 0x42442E
 	};

 	GET(AnimClass*, pThis, ESI);
 	ApplyVeinsDamage(pThis, RulesClass::Instance->VeinDamage, RulesExtData::Instance()->Veinhole_Warhead);
 	return pThis->Type->IsAnimatedTiberium  ?
 		ContinueDrawTiberium : ContinueNotTiberium;
 }

#include <Ext/Cell/Body.h>

ASMJIT_PATCH(0x685078, Generate_OreTwinkle_Anims, 0x7)
{
	GET(FakeCellClass* const, location, ESI);

	const int tib_idx = location->_GetTiberiumType();
	const int value = tib_idx == -1 ? 0 : TiberiumClass::Array->Items[tib_idx]->Value;

	if (value > 0)
	{
		const auto pTibExt = ((FakeTiberiumClass*)TiberiumClass::Array->Items[tib_idx])->_GetExtData();

		if (!ScenarioClass::Instance->Random.RandomFromMax(pTibExt->GetTwinkleChance() - 1)) {
			if (auto pAnimtype = pTibExt->GetTwinkleAnim()) {
				GameCreate<AnimClass>(pAnimtype, location->GetCoords(), 1);
			}
		}
	}

	return 0x6850E5;
}

ASMJIT_PATCH(0x423CC1, AnimClass_AI_HasExtras_Expired, 0x6)
{
	enum { SkipGameCode = 0x423EFD };

	GET(AnimClass* const, pThis, ESI);
	GET8(bool, LandIsWater, BL);
	GET8(bool, EligibleHeight, AL);

	//overriden instruction !
	R->Stack(STACK_OFFS(0x8C, 0x78), R->AL());

	return AnimExtData::OnExpired(pThis, LandIsWater, EligibleHeight) ?
		SkipGameCode : 0x0 ;
}

/// replae this entirely since the function using lea for getting int and seems broke everyone else stacks
void FakeAnimClass::_Middle()
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

		if (this->Type->MiddleFrameWidth == -1) {
			this->Type->MiddleFrameWidth = bounds.Width;
		}

		if (this->Type->MiddleFrameHeight == -1) {
			this->Type->MiddleFrameHeight = bounds.Height;
		}

		width = this->Type->MiddleFrameWidth;
		height = this->Type->MiddleFrameHeight;
	}

	AnimExtData::OnMiddle(this);
	AnimExtData::OnMiddle_SpawnSmudge(this, centercell, { width ,height });
	this->_GetExtData()->OnMiddle();
}

void FakeAnimClass::_Start() {
	this->Mark(MarkType::Change);

	auto pTypeExt = this->_GetTypeExtData();

	if (pTypeExt->AdditionalHeight > 0)
		this->Location.Z += pTypeExt->AdditionalHeight;

	auto _gCoords = this->GetCoords();

	if (pTypeExt->DetachedReport.isset()) {
		VocClass::PlayAt(pTypeExt->DetachedReport.Get(), _gCoords);
	}

	if (pTypeExt->AltReport.isset()) {
		VocClass::PlayIndexAtPos(pTypeExt->AltReport, _gCoords, nullptr);
	}

	if (!this->IsPlaying && this->Type->Report != -1) {
		VocClass::PlayIndexAtPos(this->Type->Report, _gCoords, &this->Audio3);
	} else {
		this->Audio3.AudioEventHandleStop();
	}

	this->Audio4.AudioEventHandleStop();

	if (!this->Type->MiddleFrameIndex) {
		this->_Middle();
	}

	if (!this->IsPlaying && this->Type->TiberiumChainReaction) {
		CellClass* cptr = this->GetCell();
		int tib = cptr->GetContainedTiberiumIndex();

		if (tib != -1)	{
			TiberiumClass* tiberium = TiberiumClass::Array->Items[tib];
			auto pExt = TiberiumExtContainer::Instance.Find(tiberium);

			cptr->ReduceTiberium(cptr->OverlayData + 1);

			if (tiberium->Debris.size() > 0) {
				int chance = pExt->GetDebrisChance();
				if (ScenarioClass::Instance->Random.RandomFromMax(99) < chance) {
					auto SpawnLoc = this->GetCoords();
					SpawnLoc.Z += 10;

					auto pSpawn = GameCreate<AnimClass>(tiberium->Debris[ScenarioClass::Instance->Random.RandomFromMax(tiberium->Debris.size() - 1)], SpawnLoc);
					pSpawn->LightConvert = ColorScheme::Array->Items[tiberium->Color]->LightConvert;
					pSpawn->TintColor = cptr->Intensity_Normal;
				}
			}


			int damage = pExt->GetExplosionDamage();
			auto pWarhead = pExt->GetExplosionWarhead();

			DamageArea::Apply(&this->Location, damage, nullptr, pWarhead, false, nullptr);

			cptr->RecalcAttributes(-1);
			MapClass::Instance->ResetZones(cptr->MapCoords);
			MapClass::Instance->RecalculateSubZones(cptr->MapCoords);
		}
	}

	this->_GetExtData()->OnStart();
}

ASMJIT_PATCH(0x423B95, AnimClass_AI_HideIfNoOre_Threshold, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET(FakeAnimTypeClass* const, pType, EDX);

	if (pType && pType->HideIfNoOre)
	{
		auto const pCell = pThis->GetCell();

		pThis->Invisible = !pCell || pCell->GetContainedTiberiumValue()
			<= Math::abs(pType->_GetExtData()->HideIfNoOre_Threshold.Get());

		return 0x423BBF;
	}

	return 0x0;

} //was 8

ASMJIT_PATCH(0x424807, AnimClass_AI_Next, 0x6) //was 8
{
	GET(FakeAnimClass*, pThis, ESI);

	if (pThis->Type)
	{
		const auto pExt = pThis->_GetExtData();
		const auto pTypeExt = pThis->_GetTypeExtData();

		if (pExt->AttachedSystem && pExt->AttachedSystem->Type != pTypeExt->AttachedSystem.Get())
			pExt->AttachedSystem = nullptr;

		if (!pExt->AttachedSystem && pTypeExt->AttachedSystem)
			pExt->CreateAttachedSystem();
	}

	return 0x0;
}

ASMJIT_PATCH(0x423BC8, AnimClass_Update_CreateUnit_MarkOccupationBits, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	AnimTypeExtData::CreateUnit_MarkCell(pThis);
	return 0;
}

ASMJIT_PATCH(0x424932, AnimClass_Update_CreateUnit_ActualAffects, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	AnimTypeExtData::CreateUnit_Spawn(pThis);
	return 0;
}

ASMJIT_PATCH(0x423B0B, AnimClass_Update_AlphaLight, 6)
{
	GET(AnimClass*, pThis, ESI);
	// flaming guys do the update via base class
	if (!pThis->Type->IsFlamingGuy)
	{
		TechnoExt_ExtData::UpdateAlphaShape(pThis);
	}

	return 0;
}

ASMJIT_PATCH(0x4242CA, AnimClass_Update_FixIE_TrailerSeperation, 0x6)
{
	enum
	{
		PlayTrail = 0x4242D5,
		StopTrail = 0x424322,
	};

	GET(AnimTypeClass*, AT, EAX);
	int trailSep = AT->TrailerSeperation;

	R->ECX(trailSep);

	return trailSep >= 1
		? PlayTrail : StopTrail;
}

// MakeInfantry that fails to place will just end the source animation and cleanup instead of memleaking to game end
ASMJIT_PATCH(0x424B23, AnimClass_Update_FailedToUnlimboInfantry, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(InfantryClass*, pInf, EDI);

	pInf->UnInit();
	pThis->TimeToDie = 1;
	pThis->UnInit();

	return 0x424B29;
}

ASMJIT_PATCH(0x423F31, AnimClass_Spawns_Override, 0x6)
{
	GET(FakeAnimClass*, pThis, ESI);
	GET_STACK(int, X, 0x88 - 0x4C);
	GET_STACK(int, Y, 0x88 - 0x48);
	GET_STACK(int, Z, 0x88 - 0x44);

	if (!pThis->Type->Spawns || pThis->Type->SpawnCount <= 0)
		return 0x423FC6;

	CoordStruct nCoord { X , Y , Z };

	const auto nMax = pThis->Type->SpawnCount == 1 ?
		1 : ScenarioClass::Instance->Random.RandomFromMax((pThis->Type->SpawnCount * 2));

	const auto pAnimTypeExt = pThis->_GetTypeExtData();
	TechnoClass* pTech = AnimExtData::GetTechnoInvoker(pThis);
	HouseClass* pOwner = pThis->Owner ? pThis->Owner : pTech ? pTech->GetOwningHouse() : nullptr;
	auto nDelay = pAnimTypeExt->Spawns_Delay.Get();

	for (int i = nMax; i > 0; --i)
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pThis->Type->Spawns, nCoord, nDelay, 1, AnimFlag(0x600), 0, false),
		pOwner,
		nullptr,
		pTech,
		false
		);
	}

	R->Stack(0x88 - 0x4C, nCoord.X);
	R->Stack(0x88 - 0x48, nCoord.Y);
	R->Stack(0x88 - 0x44, nCoord.Z);
	return 0x423FC6;
}

ASMJIT_PATCH(0x424538, AnimClass_AI_DamageDelay, 0x6)
{
	enum { SkipDamageDelay = 0x42465D, CheckIsAlive = 0x42464C };

	GET(AnimClass*, pThis, ESI);

	if (pThis->InLimbo)
		return CheckIsAlive;

	return AnimExtData::DealDamageDelay(pThis);
}

ASMJIT_PATCH(0x424AEC, AnimClass_AI_SetMission, 0x6)
{
	GET(FakeAnimClass*, pThis, ESI);
	GET(InfantryClass*, pInf, EDI);

	const auto pTypeExt = pThis->_GetTypeExtData();
	const auto Is_AI = !pInf->Owner->IsControlledByHuman();

	if (!pTypeExt->ScatterAnimToInfantry(Is_AI))
		pInf->QueueMission(pTypeExt->GetAnimToInfantryMission(Is_AI), false);
	else
		pInf->Scatter(CoordStruct::Empty, true, false);

	return 0x0;
}

//the stack is change , so i need to replace everything if i want just use normal hook
//this make it unnessesary
//replace the vtable call

DEFINE_FUNCTION_JUMP(CALL6, 0x424B04, FakeInfantryClass::_Dummy);

bool __fastcall Is_Visible_To_Psychic(HouseClass* house, CellClass* cell) {
	JMP_STD(0x43B4C0);
}

#include <GameOptionsClass.h>
#include <OverlayClass.h>
#include <OverlayTypeClass.h>
#include <TacticalClass.h>
void FakeAnimClass::_ApplyVeinsDamage() {

}

void FakeAnimClass::_ApplyDeformTerrrain() {

}

void FakeAnimClass::_AI() {
	if (!this->IsPlaying && this->Type->Report != -1) {
		VocClass::PlayIndexAtPos(this->Type->Report, this->GetCoords(), &this->Audio3);
	}

	if (this->Type->IsFlamingGuy) {
		this->FlamingGuy_AI();
		this->ObjectClass::Update();
	}

	if (this->Type->PsiWarning) {
		this->Invisible = !Is_Visible_To_Psychic(this->Owner, this->GetCell());
	}

	if (this->Type == RulesClass::Instance->Behind) {
		this->Invisible = !GameOptionsClass::Instance->ShowHidden;
	}

	if (this->Type->HideIfNoOre) {
		auto pCell = this->GetCell();
		this->Invisible = pCell->GetContainedTiberiumIndex() == -1 || !pCell->GetContainedTiberiumValue();
	}

	if (this->Type->MakeInfantry != -1) {
		this->MarkAllOccupationBits(this->Location);
	}

	if (this->Unpaused && this->PausedAnimFrame == this->Animation.Value) {
		this->Unpaused = false;
	}

	if (this->HasExtras) {
		BounceClass::Status land = this->BounceAI();

		if (land == BounceClass::Status::Impact || land == BounceClass::Status::Bounce) {
			auto pCell = this->GetCell();
			const bool water = pCell->LandType == LandType::Water;
			const bool isBridge = this->Location.Z >= CellClass::BridgeHeight  + MapClass::Instance->GetCellFloorHeight(this->Location);

			if (!water || isBridge) {
				if (this->Type->ExpireAnim) {
					GameCreate<AnimClass>(this->Type->ExpireAnim,
						this->Location,
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
					this->Location,
					0,
					1,
					AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200,
					0
				);
			}
			else
			{
				GameCreate<AnimClass>(RulesClass::Instance->Wake,
					this->Location,
					0,
					1,
					AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200,
					0
				);

				CoordStruct _splashCoord = this->Location;
				_splashCoord.Z += 3;
				GameCreate<AnimClass>(RulesClass::Instance->SplashList[0],
					_splashCoord,
					0,
					1,
					AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200,
					0
				);
			}

			if (!water || isBridge) {
				auto _bounceCoords = this->Bounce.GetCoords();
				if (this->Type->Spawns && this->Type->SpawnCount > 0) {
					const int count = ScenarioClass::Instance->Random.RandomFromMax(this->Type->SpawnCount)
								+ ScenarioClass::Instance->Random.RandomFromMax(this->Type->SpawnCount);

					for (int i = 0; i < count; i++) {
						GameCreate<AnimClass>(this->Type->Spawns,
							_bounceCoords,
							0,
							1,
							AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200,
							0
						);
					}
				}

				this->_ApplyDeformTerrrain();

				RectangleStruct updaterect(0, 0, 0, 0);
				if (this->Type->IsTiberium && !isBridge) {
					const int _radius = this->Type->TiberiumSpreadRadius;

					for (int x = -_radius; x <= _radius; ++x) {
						for (int y = -_radius; y <= _radius; y++) {
							if ((int)Math::sqrt((double)x * (double)x + (double)y * (double)y) <= _radius) {
								CellClass* cellptr = MapClass::Instance->GetCellAt(CellClass::Coord2Cell(_bounceCoords) + CellSpread::AdjacentCell[(int)x & 7]);

								if (cellptr->CanTiberiumGerminate(nullptr) && this->Type->TiberiumSpawnType != nullptr) {
									GameCreate<OverlayClass>(
										OverlayTypeClass::Array->Items[this->Type->TiberiumSpawnType->ArrayIndex + ScenarioClass::Instance->Random.RandomFromMax(3)],
										pCell->MapCoords,
										-1);
									cellptr->OverlayData = ScenarioClass::Instance->Random.RandomFromMax(2);
									RectangleStruct overlayrect = cellptr->GetOverlayShapeRect();
									overlayrect.Y -= TacticalClass::view_bound->Y;
									updaterect = RectangleStruct::Union(updaterect, overlayrect);
								}
							}
						}
					}

					TacticalClass::Instance->RegisterDirtyArea(updaterect, false);
				}
			}

			this->UnInit();
			return;
		}
	}

	if (this->IsAlive) {
		if (!this->__ToDelete_197) {
			if (this->Type->TrailerAnim) {
				int _separation = this->Type->TrailerSeperation;
				if (_separation == 1 || !(Unsorted::CurrentFrame() % _separation)) {
					CoordStruct _coord = this->GetCoords();
					GameCreate<AnimClass>(this->Type->TrailerAnim, _coord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
				}
			}
		}
	}

	if (this->Type == RulesClass::Instance->DropZoneAnim) {
		this->__ToDelete_197 = this->GetCell()->GetBuilding() != nullptr;
	}

	if (this->__ToDelete_197){
		this->UnInit();
		return;
	}

	if (this->SkipProcessOnce){
		this->SkipProcessOnce = false;
		return;
	}

	if (this->LoopDelay) {
		this->LoopDelay--;
		if (!this->LoopDelay)
			this->_Start();

		return;
	}

	if (this->IsAlive) {

		if (this->Type->IsVeins) {
			this->_ApplyVeinsDamage();
		}

		if (this->Type->IsAnimatedTiberium) {
			auto _animated_coords = this->GetCoords() - CoordStruct(384 , 384 , 0);
			auto pCell = MapClass::Instance->GetCellAt(_animated_coords);

			if (pCell->OverlayTypeIndex == -1 || OverlayTypeClass::Array->Items[pCell->OverlayTypeIndex]->CellAnim != this->Type) {
				this->__ToDelete_197 = true;
			}
		}

		if (this->Type->End == -1) {
			this->Type->End = this->Type->GetImage()->Frames;
			if (this->Type->Shadow)
				this->Type->End /= 2;
		}

		if (this->Type->LoopEnd == -1) {
			this->Type->LoopEnd = this->Type->End;
		}

		this->Mark(MarkType::Change);

		if (!this->PowerOff && !this->Paused) {
			if (this->Animation.Timer.GetValue() && this->Animation.Value == 0) {
				this->Animation.HasChanged = true;
			}

			//do damaging

			if (this->IsAlive) {
				//mid point

				if (this->Type->PingPong) {

				}

				//loops
				//shadow
				//invert ??
				//next


				//MakeInfantry

				this->UnInit();
			}
		}
	}
}

DEFINE_FUNCTION_JUMP(CALL, 0x424D5A, FakeAnimClass::_Middle);
DEFINE_FUNCTION_JUMP(CALL, 0x424687, FakeAnimClass::_Middle);
DEFINE_FUNCTION_JUMP(LJMP, 0x424F00, FakeAnimClass::_Middle);

DEFINE_FUNCTION_JUMP(CALL, 0x422702, FakeAnimClass::_Start);
DEFINE_FUNCTION_JUMP(CALL, 0x4243A1, FakeAnimClass::_Start);
DEFINE_FUNCTION_JUMP(CALL, 0x424925, FakeAnimClass::_Start);
DEFINE_FUNCTION_JUMP(LJMP, 0x424CE0, FakeAnimClass::_Start);

//TODOO :  AI , Bounce_AI


ASMJIT_PATCH(0x423991, AnimClass_UpdateBounce_BounceAnim, 0x5)
{
	GET(FakeAnimTypeClass*, pBounceAnim, ECX);
	GET(AnimClass*, pThis, EBP);

	//const auto pTypeExt = pBounceAnim->_GetExtData();
	TechnoClass* pObject = AnimExtData::GetTechnoInvoker(pThis);
	HouseClass* pHouse = pThis->Owner ? pThis->Owner : ((pObject) ? pObject->GetOwningHouse() : nullptr);

	auto nCoord = pThis->GetCoords();
	AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pBounceAnim, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0),
		pHouse,
		nullptr,
		pObject,
		false
	);

	return 0x4239D3;
}

// Deferred creation of attached particle systems for debris anims.
ASMJIT_PATCH(0x423939, AnimClass_UpdateBounce_AttachedSystem, 0x6)
{
	GET(FakeAnimClass*, pThis, EBP);
	pThis->_GetExtData()->CreateAttachedSystem();
	return 0;
}

ASMJIT_PATCH(0x4239F0, AnimClass_UpdateBounce_Damage, 0x8)
{
	enum
	{
		DoNotDealDamage = 0x423A92,
		DealDamage = 0x4239F8,
		GoToNext = 0x423A83,
	};

	GET(ObjectClass*, pObj, EDI);
	GET(AnimClass*, pThis, EBP);

	const auto pType = pThis->Type;
	const auto nRadius = pType->DamageRadius;

	if (!pObj || nRadius < 0 || CLOSE_ENOUGH(pType->Damage, 0.0) || !pType->Warhead)
		return DoNotDealDamage;

	const auto nCoord = pThis->Bounce.GetCoords();
	//const auto pAnimTypeExt = AnimTypeExtContainer::Instance.Find(pType);
	TechnoClass* const pInvoker = AnimExtData::GetTechnoInvoker(pThis);
	const auto nLoc = pObj->Location;
	const auto nDist = Math::abs(nLoc.Y - nCoord.Y) + abs(nLoc.X - nCoord.X);

	if (nDist < nRadius) {
		auto nDamage = (int)pType->Damage;
		pObj->ReceiveDamage(&nDamage, Game::AdjustHeight(nDist), pType->Warhead,
					  pInvoker, false, false, pInvoker ? pInvoker->Owner : pThis->Owner);
	}

	//return !pObj || !pType->Warhead ||
	//	pType->DamageRadius < 0 || pType->Damage == 0.0 ?
	//	DoNotDealDamage : DealDamage;
	return GoToNext;
}

ASMJIT_PATCH(0x42264D, AnimClass_Init, 0x5)
{
	GET(AnimClass*, pThis, ESI);
	GET_BASE(CoordStruct*, pCoord, 0xC);

	AnimExtData::OnInit(pThis, pCoord);

	return 0x0;
}

#ifdef PerformanceHoggers
//ASMJIT_PATCH_AGAIN(0x42429E, AnimClass_UpdateEnd, 0x6)
//ASMJIT_PATCH_AGAIN(0x42437E, AnimClass_UpdateEnd, 0x6)
//ASMJIT_PATCH_AGAIN(0x4243A6, AnimClass_UpdateEnd, 0x6)
//ASMJIT_PATCH_AGAIN(0x424567, AnimClass_UpdateEnd, 0x6)
//ASMJIT_PATCH_AGAIN(0x4246DC, AnimClass_UpdateEnd, 0x6)
//ASMJIT_PATCH_AGAIN(0x424B42, AnimClass_UpdateEnd, 0x6)
//ASMJIT_PATCH_AGAIN(0x4247EB, AnimClass_UpdateEnd, 0x6)
//ASMJIT_PATCH_AGAIN(0x42492A, AnimClass_UpdateEnd, 0x6)
//ASMJIT_PATCH_AGAIN(0x424B29, AnimClass_UpdateEnd, 0x6)
//ASMJIT_PATCH(0x424B1B, AnimClass_UpdateEnd, 0x6)
//{
//	GET(AnimClass*, pThis, ESI);
//	AnimExtContainer::Instance.Find(pThis)->SpawnsStatusData.OnUpdate(pThis);
//	return 0;
//}
//
//ASMJIT_PATCH(0x424785, AnimClass_Loop, 0x6)
//{
//	GET(AnimClass*, pThis, ESI);
//	AnimExtContainer::Instance.Find(pThis)->SpawnsStatusData.OnLoop(pThis);
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x4247F3, AnimClass_Done, 0x6)
//ASMJIT_PATCH(0x424298, AnimClass_Done, 0x6)
//{
//	GET(AnimClass*, pThis, ESI);
//	AnimExtContainer::Instance.Find(pThis)->SpawnsStatusData.OnDone(pThis);
//	return 0;
//}
//
//ASMJIT_PATCH(0x424801, AnimClass_Next, 0x6)
//{
//	GET(AnimClass*, pThis, ESI);
//	GET(AnimTypeClass*, pNextAnimType, ECX);
//	AnimExtContainer::Instance.Find(pThis)->SpawnsStatusData.OnNext(pThis , pNextAnimType);
//	return 0x0;
//}
#endif

#ifdef ENABLE_NEWHOOKS
TODO : retest for desync


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