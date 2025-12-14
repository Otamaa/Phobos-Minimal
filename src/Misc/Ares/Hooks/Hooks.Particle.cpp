#include <Ext/Particle/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/ParticleSystem/Body.h>
#include <Ext/ParticleSystemType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Helpers.h>

#include "Header.h"

#include <Notifications.h>
#include <Ext/Rules/Body.h>
#include <SpotlightClass.h>

//void ParicleSystem_Web_AI(ParticleSystemClass* pThis)
//{
//	for (auto& particle : pThis->Particles)
//		particle->BehaviourUpdate();
//
//	for (int i = pThis->Particles.Count - 1; i > 0; --i) {
//		auto particle = pThis->Particles[i];
//
//		if (pThis->Particles[i]->TimeToDelete)
//		{
//			if (particle->Type->NextParticle != -1)
//			{
//				const auto pNextType = ParticleTypeClass::Array->Items[particle->Type->NextParticle];
//				const auto nCoord = pNextType->NextParticleOffset + particle->Location;
//
//				{
//					auto particle_ = GameCreate<ParticleClass>(pNextType, nCoord, CoordStruct::Empty, nullptr);
//					particle = std::exchange(pThis->Particles[i], particle_);
//					particle_->Velocity = particle->Velocity;
//					particle_->GasVelocity = particle->GasVelocity;
//				}
//			}
//
//			particle->UnInit();
//		}
//		else {
//			particle->BehaviourCoordUpdate();
//		}
//	}
//}
//
//void Particle_Web_AI(ParticleClass* pThis)
//{
//	auto pCell = MapClass::Instance->GetCellAt(pThis->Location);
//
//	if (auto pWarhead = pThis->Type->Warhead) {
//		for(auto pCur = pCell->FirstObject; pCur; pCur = pCur->NextObject) {
//			if (pCur && pCur->IsAlive && pCur->Health > 0) {
//				int damage = pThis->Type->Damage;
//				pCur->ReceiveDamage(&damage, 0, pWarhead, nullptr, false, false, nullptr);
//			}
//		}
//	}
//
//	const int Id = pThis->Fetch_ID();
//	const int Ecs = LOWORD(pThis->Type->MaxEC) - pThis->RemainingEC + Id;
//	const int Ecs_ = LOBYTE(pThis->Type->StateAIAdvance) + (pThis->Fetch_ID() & 1);
//
//	if (!(Ecs % Ecs_))
//		++pThis->StartStateAI;
//
//	if (pThis->StartStateAI == pThis->Type->EndStateAI) {
//		if (pThis->Type->DeleteOnStateLimit)
//			pThis->TimeToDelete = false;
//		else
//			pThis->StartStateAI = 0;
//	}
//}

//ASMJIT_PATCH(0x6453D7, ParticleTypeClass_ReadINI_BehavesLike_A, 0x5)
//{
//	LEA_STACK(const char*, pResult, 0x14);
//	R->EBX(ParticleTypeClass::BehavesFromString(pResult));
//	return 0x6453FF;
//}
//
//ASMJIT_PATCH(0x644423, ParticleSystemTypeClass_ReadINI_BehavesLike_A, 0x8)
//{
//	LEA_STACK(const char*, pResult, 0x20);
//	R->EAX(ParticleSystemTypeClass::BehavesFromString(pResult));
//	return 0x644461;
//}

ASMJIT_PATCH(0x6458D7, ParticleTypeClass_ReadINI_BehavesLike_B, 0x6)
{
	GET(const char*, pResult, EBX);

	for (size_t i = 0; i < ParticleTypeClass::BehavesString.c_size(); ++i)
	{
		if (IS_SAME_STR_(pResult, ParticleTypeClass::BehavesString[i]))
		{
			switch (i)
			{
			case 0:
				R->EDI(ParticleTypeBehavesLike::Gas);
				return 0x6458FF;
			case 1:
				R->EDI(ParticleTypeBehavesLike::Smoke);
				return 0x6458FF;
			case 2:
				R->EDI(ParticleTypeBehavesLike::Fire);
				return 0x6458FF;
			case 3:
				R->EDI(ParticleTypeBehavesLike::Spark);
				return 0x6458FF;
			case 4:
				R->EDI(ParticleTypeBehavesLike::Railgun);
				return 0x6458FF;
			default:
				break;
			}
		}
	}

	if (IS_SAME_STR_(pResult, "Web"))
	{
		R->EDI(ParticleTypeBehavesLike(5)); //result;
		return 0x6453FF;
	}

	R->EDI(ParticleTypeBehavesLike::None); //result;
	return 0x6458FF;
}

ASMJIT_PATCH(0x644857, ParticleSystemTypeClass_ReadINI_BehavesLike_B, 0x6)
{
	GET(const char*, pResult, EBX);

	for (size_t i = 0; i < ParticleSystemTypeClass::BehavesString.c_size(); ++i)
	{
		if (IS_SAME_STR_(pResult, ParticleSystemTypeClass::BehavesString[i]))
		{
			switch (i)
			{
			case 0:
				R->EDI(ParticleSystemTypeBehavesLike::Smoke);
				return 0x64487F;
			case 1:
				R->EDI(ParticleSystemTypeBehavesLike::Gas);
				return 0x64487F;
			case 2:
				R->EDI(ParticleSystemTypeBehavesLike::Fire);
				return 0x64487F;
			case 3:
				R->EDI(ParticleSystemTypeBehavesLike::Spark);
				return 0x64487F;
			case 4:
				R->EDI(ParticleSystemTypeBehavesLike::Railgun);
				return 0x64487F;
			default:
				break;
			}
		}
	}

	if (IS_SAME_STR_(pResult, "Web")) {
		R->EDI(ParticleSystemTypeBehavesLike(5)); //result;
		return 0x64487F;
	}

	R->EDI(ParticleSystemTypeBehavesLike::None); //result;
	return 0x64487F;
}

//ASMJIT_PATCH(0x62FCF0, ParticleSytemClass_FireDirectioon_AI_DirMult, 0x7)
//{
//	GET(int, facing, EAX);
//	GET(ParticleSystemClass*, pThis, ESI);
//	const auto& mult = ParticleSystemTypeExtContainer::Instance.Find(pThis->Type)->FacingMult[facing];
//	R->ECX(mult.X);
//	R->EAX(mult.Y);
//	return 0x62FCFE;
//}

#ifndef PARTICLESTUFFSOVERRIDE
ASMJIT_PATCH(0x72590E, AnnounceInvalidPointer_Particle, 0x9)
{
	GET(AbstractType, nWhat, EBX);

	if (nWhat == AbstractType::Particle)
	{
		GET(ParticleClass*, pThis, ESI);

		if (auto pSys = pThis->ParticleSystem) {
			pSys->Particles.erase(pThis);
		}

		return 0x725C08;
	}

	return nWhat == AbstractType::ParticleSystem ?
		0x725917 : 0x7259DA;
}

//ASMJIT_PATCH(0x62EE3F, ParticleClass_SmokeAI_ZeroRadius, 0x6)
//{
//	GET(ParticleTypeClass*, pType, EDX);
//	const auto radius = pType->Radius >> 3;
//
//	if (radius == 0)
//		Debug::FatalError("[%s] Particle with 0 raius , please fix !", pType->ID);
//
//	R->EAX(radius);
//	return 0x62EE48;
//}

//ASMJIT_PATCH(0x62C2C2, ParticleClass_Update_Gas_Damage, 6)
//{
// 	GET(ParticleClass*, pParticle, EBP);
// 	GET(ObjectClass*, pTarget, ESI);
// 	GET(int, nDistance, ECX);
//
// 	if (pTarget->InLimbo)
// 		return 0x62C309;
//
// 	if (auto pTechno = generic_cast<TechnoClass*>(pTarget))
// 	{
// 		if (pTechno->IsSinking || pTechno->IsCrashing || pTechno->TemporalTargetingMe)
// 			return 0x62C309;
//
// 		if (pTechno->WhatAmI() != BuildingClass::AbsID && TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
// 			return 0x62C309;
// 	}
//
// 	auto const& [pAttacker, pOwner] = ParticleExt::GetOwnership(pParticle);
// 	int nDamage = pParticle->Type->Damage;
// 	pTarget->ReceiveDamage(&nDamage, nDistance, pParticle->Type->Warhead, pAttacker, false, false, pOwner);
//
//	return 0x62C309;
//}

// static void ParticleClass_Gas_Transmography(ObjectClass* pItem, TechnoClass* pAttacker , HouseClass* pOwner , int distance, const CoordStruct& loc, ParticleTypeExtData* pTypeExt, HouseClass* transmoOwner)
// {
// 	int damage = pTypeExt->This()->Damage;
// 	if (pItem->ReceiveDamage(&damage, distance, pTypeExt->This()->Warhead, pAttacker, false, false, pOwner) == DamageState::NowDead) {
// 		if (pTypeExt->TransmogrifyChance >= 0) {

// 			if (pTypeExt->TransmogrifyOwner != OwnerHouseKind::Neutral)
// 				transmoOwner = HouseExtData::GetHouseKind(pTypeExt->TransmogrifyOwner, true, nullptr, pOwner, pItem->GetOwningHouse());

// 			CoordStruct loc_ = loc;
// 			TechnoExt_ExtData::SpawnVisceroid(loc_, pTypeExt->TransmogrifyType, pTypeExt->TransmogrifyChance, pTypeExt->Transmogrify, transmoOwner);
// 		}
// 	}
// }

// ASMJIT_PATCH(0x62C23D, ParticleClass_Update_Gas_DamageRange, 6)
// {
// 	GET(ParticleClass*, pThis, EBP);
// 	auto pTypeExt = ParticleTypeExtContainer::Instance.Find(pThis->Type);

// 	const auto& [pAttacker, pOwner] = ParticleExtData::GetOwnership(pThis);
// 	HouseClass* transmoOwner = HouseExtData::FindNeutral();

// 	if (pTypeExt->DamageRange.Get() <= 0.0)
// 	{
// 		if(auto pCell = MapClass::Instance->TryGetCellAt(pThis->Location)) {
// 			for (auto pOccupy = pCell->FirstObject; pOccupy; pOccupy = pOccupy->NextObject)
// 			{
// 				if (pOccupy->IsAlive && pOccupy->Health > 0)
// 				{
// 					 if (auto pTechno = flag_cast_to<TechnoClass*, false>(pOccupy))
// 					 {
// 				 		if (pTechno->IsSinking || pTechno->IsCrashing || pTechno->TemporalTargetingMe)
// 				 			continue;

// 				 		if (pTechno->WhatAmI() != BuildingClass::AbsID && TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
// 							continue;
// 					 }

// 					 auto nX = Math::abs(pThis->Location.X - pOccupy->Location.X);
// 					 auto nY = Math::abs(pThis->Location.Y - pOccupy->Location.Y);
// 					 ParticleClass_Gas_Transmography(pOccupy, pAttacker, pOwner, Game::AdjustHeight(nX + nY), pOccupy->Location, pTypeExt, transmoOwner);
// 				}
// 			}
// 		}

// 	} else {

// 		const auto pVec = Helpers::Alex::getCellSpreadItems(pThis->Location, std::ceil(pTypeExt->DamageRange.Get()));

// 		for (const auto pItem : pVec)
// 		{
// 			if (!pItem->IsAlive || pItem->Health <= 0)
// 				continue;

// 			if (pItem->IsSinking || pItem->IsCrashing || pItem->TemporalTargetingMe)
// 				continue;

// 			if (pItem->WhatAmI() != BuildingClass::AbsID && TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pItem)))
// 				continue;

// 			auto nX = Math::abs(pThis->Location.X - pItem->Location.X);
// 			auto nY = Math::abs(pThis->Location.Y - pItem->Location.Y);
// 			ParticleClass_Gas_Transmography(pItem, pAttacker, pOwner, Game::AdjustHeight(nX + nY), pItem->Location, pTypeExt, transmoOwner);
// 		}
// 	}

// 	return 0x62C313;
// }

ASMJIT_PATCH(0x62D015, ParticleClass_Draw_Palette, 6)
{
	GET(ParticleClass*, pThis, EDI);

	ConvertClass* pConvert = FileSystem::ANIM_PAL();
	const auto pTypeExt = ParticleTypeExtContainer::Instance.Find(pThis->Type);
	if (const auto pConvertData = pTypeExt->Palette.GetConvert()) {
		pConvert = pConvertData;
	}

	R->EDX(pConvert);
	return 0x62D01B;
}

// ASMJIT_PATCH(0x62CCB8, ParticleClass_Update_Fire, 7)
// {
// 	GET(ParticleClass*, pThis, ESI);

// 	pThis->RemainingDC = LOWORD(pThis->Type->MaxDC);
// 	auto const& [pAttacker, pOwner] = ParticleExtData::GetOwnership(pThis);
// 	const auto pCell = MapClass::Instance->GetCellAt(pThis->Location);
// 	const auto pTypeExt = ParticleTypeExtContainer::Instance.Find(pThis->Type);

// 	for (auto pOccupy = pCell->GetContent(pThis->Location.Z); pOccupy; pOccupy = pOccupy->NextObject) {

// 		if (pOccupy && pOccupy->IsAlive && pOccupy->Health > 0 && !pOccupy->InLimbo)
// 		{
// 			if (pThis->ParticleSystem && pOccupy == pThis->ParticleSystem->Owner)
// 				continue;

// 			if (auto pTechno = flag_cast_to<TechnoClass*, false>(pOccupy))
// 			{
// 				if (pTechno->IsSinking || pTechno->IsCrashing || pTechno->TemporalTargetingMe)
// 					continue;

// 				if (pTechno->WhatAmI() != BuildingClass::AbsID && TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
// 					continue;
// 			}

// 			int damage = pThis->Type->Damage;
// 			int length = (int)(pThis->Location.DistanceFrom(pOccupy->GetCoords()) / 10.0);

// 			pOccupy->ReceiveDamage(&damage, length, pThis->Type->Warhead, pAttacker, false, false, pOwner);
// 			if (pTypeExt->Fire_DamagingAnim) {
// 				if (auto pAnimType = MapClass::SelectDamageAnimation(pThis->Type->Damage, pThis->Type->Warhead, pCell->LandType, pThis->Location)) {
// 					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pThis->Location),
// 						pOwner, pOccupy->GetOwningHouse(), pAttacker, false, false);
// 				}
// 			}
// 		}
// 	}

// 	return 0x62CE14;
// }

ASMJIT_PATCH(0x6D9427, TacticalClass_DrawUnits_ParticleSystems, 9)
{
	GET(Layer, layer, EAX);

	if (layer == Layer::Air)
		ParticleSystemExtData::UpdateInAir();

	//return layer == Layer::Ground ? 0x6D9430 : 0x6D95A1;
	// Fixed position and layer of info tip and reveal production cameo on selected building
	// Part1
	// Author: Belonit
	return 0x6D95A1;
}

// Fixed position and layer of info tip and reveal production cameo on selected building
// Author: Belonit
// Call DrawInfoTipAndSpiedSelection in new location
// ASMJIT_PATCH(0x6D9781, Tactical_RenderLayers_DrawInfoTipAndSpiedSelection, 0x5)
// {
// 	GET(TechnoClass*, pThis, EBX);
// 	GET(Point2D*, pLocation, EAX);
//
// 	const auto pBuilding = cast_to<BuildingClass*, false>(pThis);
//
// 	if (pBuilding && pBuilding->IsSelected && pBuilding->IsOnMap && BuildingExtContainer::Instance.Find(pBuilding)->LimboID <= -1)
// 	{
// 		const int foundationHeight = pBuilding->Type->GetFoundationHeight(0);
// 		const int typeHeight = pBuilding->Type->Height;
// 		const int yOffest = (Unsorted::CellHeightInPixels * (foundationHeight + typeHeight)) >> 2;
//
// 		Point2D centeredPoint = { pLocation->X, pLocation->Y - yOffest };
// 		pBuilding->DrawInfoTipAndSpiedSelection(&centeredPoint, &DSurface::ViewBounds);
// 	}
//
// 	return 0;
// }

ASMJIT_PATCH(0x62E380, ParticleSystemClass_SpawnParticle, 0xA)
{
	GET(ParticleSystemClass*, pThis, ECX);

	return ParticleSystemExtContainer::Instance.Find(pThis)->What != ParticleSystemExtData::Behave::None
		? 0x62E428 : 0;
}

ASMJIT_PATCH(0x62E2AD, ParticleSystemClass_Draw, 6)
{
	GET(ParticleSystemClass*, pThis, EDI);
	GET(ParticleSystemTypeClass*, pThisType, EAX);

	if (pThisType->ParticleCap > 0)
	{
		R->ECX(pThis->Particles.Count +
			ParticleSystemExtContainer::Instance.Find(pThis)->OtherParticleData.size());
	}
	else
	{
		R->ECX(0);
	}

	return 0x62E2B3;
}

//ASMJIT_PATCH(0x62CE40, ParticleClass_Update_Add, 0x9)
//{
//	GET(ParticleClass*, pThis, ECX);
//
//	if (pThis->Type->BehavesLike == ParticleTypeBehavesLike(5)) {
//		Particle_Web_AI(pThis);
//	}
//
//	return 0x0;
//}

//ASMJIT_PATCH(0x62FD60, ParticleSystemClass_Update, 0x9)
//{
//	GET(ParticleSystemClass*, pThis, ECX);
//
//	//if (pThis->Type->BehavesLike == ParticleSystemTypeBehavesLike(5)) {
//	//	ParicleSystem_Web_AI(pThis);
//	//	return 0x0;
//	//}
//
//	const bool Handled = ParticleSystemExtContainer::Instance.Find(pThis)->UpdateHandled();
//
//	return Handled ? 0x62FE43 : 0;
//}

//ASMJIT_PATCH(0x62E15D, ParticleSystemClass_DTOR_NullType, 0x6)
//{
//	GET(ParticleSystemClass*, pThis, EDI);
//	Debug::LogInfo("Detaching ParticleSystemClass [%x] Type of [%s]", pThis, pThis->Type->ID);
//	return 0x0;
//}

//donot detach the type so we can identify bug
DEFINE_JUMP(LJMP, 0x62E15D, 0x62E163);
#endif