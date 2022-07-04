#include "Body.h"
#include <Utilities/Macro.h>

#include <Ext/Tiberium/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/AnimHelpers.h>
#include <Utilities/Helpers.h>
#include <SmudgeClass.h>
#include <SmudgeTypeClass.h>

#include <Memory.h>

DEFINE_HOOK(0x685078, Generate_OreTwinkle_Anims, 0x7)
{
	GET(CellClass* const, location, ESI);

	if (location->GetContainedTiberiumValue() > 0)
	{
		if (auto const pTibExt = TiberiumExt::GetExtData(TiberiumClass::Array->GetItem(location->GetContainedTiberiumIndex())))
		{
			if (!ScenarioClass::Instance->Random(0, pTibExt->GetTwinkleChance() - 1))
			{
				if (auto pAnimtype = pTibExt->GetTwinkleAnim())
				{
					if (auto pAnim = GameCreate<AnimClass>(pAnimtype, location->GetCoords()))
					{
						//AnimExt::AnimCellUpdater::Marked.push_back(location);
						AnimExt::SetAnimOwnerHouseKind(pAnim, nullptr, nullptr, false);
					}
				}
			}
		}
	}

	return 0x6850E5;
}

/*
DEFINE_HOOK(0x423CD5, AnimClass_Expired_Extra_OnWater, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (!pThis->Type)
		return 0x423EFD;

	auto const nLocation = pThis->GetCenterCoord();
	auto pOwner = pThis->Owner;
	auto pType = pThis->Type;
	DWORD flags = 0x600u;
	int ForceZAdjust = 0;

	//Default

	const auto GetOriginalAnimResult = [pType]()
	{
		if (pType->IsMeteor)
		{
			auto const splash = RulesClass::Instance->SplashList;

			if (splash.Count > 0)
			{
				return splash[ScenarioClass::Instance->Random(0, splash.Count - 1)];
			}
		}

		return RulesClass::Instance->Wake;
	};

	AnimTypeClass* pSplashAnim = nullptr;
	TechnoClass* pInvoker = nullptr;

	if (auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type))
	{
		if (pAnimTypeExt->ExplodeOnWater.Get())
		{
			if (auto const pWarhead = pType->Warhead)
			{
				auto const nDamage = Game::F2I(pThis->Accum);
				pInvoker = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker);
				pOwner = pInvoker ? pInvoker->GetOwningHouse() : pOwner;

				if (pAnimTypeExt->Warhead_Detonate)
				{
					WarheadTypeExt::DetonateAt(pWarhead, nLocation, pInvoker, nDamage);
				}
				else
				{
					MapClass::DamageArea(nLocation, nDamage, pInvoker, pWarhead, pWarhead->Tiberium, pOwner);
					MapClass::FlashbangWarheadAt(nDamage, pWarhead, nLocation);
					auto nLand = pThis->GetCell() ? pThis->GetCell()->LandType : LandType::Clear;
					pSplashAnim = MapClass::SelectDamageAnimation(nDamage, pWarhead, nLand, nLocation);
					flags = 0x2600u;
					ForceZAdjust = -30;
				}
			}

			if (auto pExpireAnim = pType->ExpireAnim)
			{
				if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, nLocation, 0, 1, 0x2600u, -30, 0))
				{
					AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
					if (auto pExt = AnimExt::GetExtData(pAnim))
						pExt->Invoker = pInvoker;
				}
			}
		}
		else
		{
			if (pType->IsMeteor)
			{
				auto const splash = pAnimTypeExt->SplashList.GetElements(RulesClass::Instance->SplashList);

				if (splash.size() > 0)
				{
					auto nIndexR = (splash.size() - 1);
					auto nIndex = pAnimTypeExt->SplashIndexRandom.Get() ?
						ScenarioClass::Instance->Random(0, nIndexR) : pAnimTypeExt->SplashIndex.Get(nIndexR);

					pSplashAnim = splash.at(nIndex);
				}
			}
			else
			{
				pSplashAnim = pAnimTypeExt->WakeAnim.Get(RulesClass::Instance->Wake);
			}
		}

	}
	else
	{
		pSplashAnim = GetOriginalAnimResult();
	}

	if (pSplashAnim)
	{
		if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, nLocation, 0, 1, flags, ForceZAdjust))
		{
			AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, false);
			if (auto pExt = AnimExt::GetExtData(pSplashAnimCreated))
				pExt->Invoker = pInvoker;
		}
	}

	return 0x423EFD;
}

DEFINE_HOOK(0x423DE7, AnimClass_Expired_Extra_OnLand_DamageArea, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (auto const pType = pThis->Type)
	{
		auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
		TechnoClass* pTechOwner = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt && pAnimTypeExt->Damage_DealtByInvoker);
		auto const pOwner = pTechOwner ? pTechOwner->GetOwningHouse() : pThis->Owner;

		auto nCoords = pThis->Bounce.GetCoords();

		if (auto pExpireAnim = pType->ExpireAnim)
		{
			if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, nCoords, 0, 1, 0x2600u, -30, 0))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
				if (auto pExt = AnimExt::GetExtData(pAnim))
					pExt->Invoker = pTechOwner;
			}
		}

		if (auto const pWarhead = pType->Warhead)
		{
			auto const nDamage = Game::F2I(pType->Damage);
			MapClass::DamageArea(nCoords, nDamage, pTechOwner, pWarhead, pWarhead->Tiberium, pOwner);
			MapClass::FlashbangWarheadAt(nDamage, pWarhead, nCoords, false);
		}
	}

	return 0x423EFD;
}
*/
DEFINE_HOOK(0x423CC1, AnimClass_AI_HasExtras_Expired, 0x6)
{
	enum { SkipGameCode = 0x423EFD };

	GET(AnimClass* const, pThis, ESI);
	GET8(const bool, LandIsWater, BL);
	GET8(const bool, EligibleHeight, AL);

	if (!pThis || !pThis->Type)
		return SkipGameCode;

	auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	TechnoClass* pTechOwner = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt && pAnimTypeExt->Damage_DealtByInvoker);
	auto const pOwner = pThis->Owner ? pThis->Owner : pTechOwner ? pTechOwner->GetOwningHouse() : nullptr;

	//overriden instruction !
	R->Stack(0x13, EligibleHeight);

	if (!LandIsWater || EligibleHeight)
	{
		Helper::Otamaa::Detonate(Game::F2I(pThis->Type->Damage), pThis->Type->Warhead, pAnimTypeExt->Warhead_Detonate, pThis->Bounce.GetCoords(), pTechOwner, pOwner , pAnimTypeExt->Damage_ConsiderOwnerVeterancy.Get());

		if (auto const pExpireAnim = pThis->Type->ExpireAnim) {
			if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, pThis->Bounce.GetCoords(), 0, 1, 0x2600u, -30, 0)) {
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr,pTechOwner, false);
			}
		}
	}
	else
	{
		if (!pAnimTypeExt->ExplodeOnWater)
		{
			if (auto pSplashAnim = Helper::Otamaa::PickSplashAnim(pAnimTypeExt->SplashList, pAnimTypeExt->WakeAnim.Get(RulesGlobal->Wake), pAnimTypeExt->SplashIndexRandom.Get(), pThis->Type->IsMeteor))
			{
				if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, pThis->GetCenterCoord(), 0, 1, 0x600u, false)) {
					AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, pTechOwner, false);
				}
			}
		}
		else
		{
			auto const [bPlayWHAnim, nDamage] = Helper::Otamaa::Detonate(Game::F2I(pThis->Type->Damage), pThis->Type->Warhead, pAnimTypeExt->Warhead_Detonate, pThis->GetCenterCoord(), pTechOwner, pOwner, pAnimTypeExt->Damage_ConsiderOwnerVeterancy.Get());
			if(bPlayWHAnim)
			{
				if (auto pSplashAnim = MapClass::SelectDamageAnimation(nDamage, pThis->Type->Warhead, pThis->GetCell()->LandType, pThis->GetCenterCoord()))
				{
					if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, pThis->GetCenterCoord(), 0, 1, 0x2600u, -30)) {
						AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, pTechOwner, false);
					}
				}
			}
		}
	}

	return SkipGameCode;
}

//DEFINE_HOOK(0x423AC0, AnimClass_Update, 0x6)
//{
	//GET(AnimClass*, pThis, ECX);
	//auto pExt = AnimExt::ExtMap.Find(pThis);

	//if (!pExt && pThis->Type)
	//	Debug::Log("Failed ! To Find Ext for [%s] ! \n", pThis->get_ID());

	//return 0x0;
//}

//crash and corrup ESI pointer around
//DEFINE_HOOK(0x424F00, AnimClass_Middle_Broken, 0x4)
//{
	//GET(AnimClass*, pThis, ECX);
	//auto pExt = AnimExt::ExtMap.Find(pThis);
//	return 0x0;
//}

/* Original Ares
int __cdecl AnimClass_Expired_SpawnsParticle(REGISTERS *a1)
{
  signed int v1; // esi
  unsigned int v2; // ebx
  int v3; // eax
  int v4; // ecx
  int result; // eax
  ScenarioClass *v6; // edi
  int v7; // eax
  int v8; // ebx
  double v9; // st7
  double v10; // [esp+50h] [ebp-50h]
  int v11; // [esp+58h] [ebp-48h]
  int v12; // [esp+5Ch] [ebp-44h]
  int v13; // [esp+60h] [ebp-40h]
  double v14; // [esp+60h] [ebp-40h]
  ParticleTypeClass *pType; // [esp+6Ch] [ebp-34h]
  double v16; // [esp+70h] [ebp-30h]
  double v17; // [esp+78h] [ebp-28h]
  double v18; // [esp+80h] [ebp-20h]
  CoordStruct coord_4; // [esp+88h] [ebp-18h] BYREF
  CoordStruct v20; // [esp+94h] [ebp-Ch] BYREF

  v1 = a1->_ECX.data;
  v2 = a1->_ESI.data;
  pType = MEMORY[0xA83D9C][*(a1->_EAX.data + 716)];
  v3 = GetExt(AnimTypeExt, a1->_EAX.data);
  v4 = *(v3 + 24);
  result = *(v3 + 28);
  v12 = v4;
  v11 = result;
  if ( v4 || result )
  {
	v6 = MEMORY[0xA8B230];
	(*(*v2 + 72))(v2, &v20);
	v7 = CellClass::GetCellFloorHeight(0x87F7E8, &v20);
	v8 = v20.Z - v7;
	v17 = 6.283185307179586 / v1;
	v16 = 0.0;
	if ( v1 > 0 )
	{
	  do
	  {
		v13 = abs32(Randomizer::RandomRanged(&v6->Random, v12, v11));
		v10 = Randomizer::RandomRanged(&v6->Random, 1, 0x7FFFFFFF) * 4.656612873077393e-10 * v17 + v16;
		v18 = FastMath::Cos(v10);
		v9 = FastMath::Sin(v10);
		coord_4.Z = v20.Z;
		v14 = v13;
		coord_4.X = v20.X + (v14 * v18);
		coord_4.Y = v20.Y - (v9 * v14);
		coord_4.Z = v8 + CellClass::GetCellFloorHeight(8910824, &coord_4);
		ParticleSystemClass::SpawnParticle(MEMORY[0xA8ED78], pType, &coord_4);
		v16 = v16 + v17;
		--v1;
	  }
	  while ( v1 );
	}
	result = 0x42504D;
  }
  return result;
}
*/

//crash and corrup ESI pointer around
DEFINE_HOOK(0x424FE8, AnimClass_Middle_SpawnParticle, 0xC)
{
	GET(AnimClass*, pThis, ESI);

	{
		auto pType = pThis->Type;
		if (auto pTypeExt = AnimTypeExt::ExtMap.Find(pType))
		{
			auto pAnimTypeExt = pTypeExt;
			auto const pObject = AnimExt::GetTechnoInvoker(pThis, pTypeExt->Damage_DealtByInvoker.Get());
			auto const pHouse = pThis->Owner ? pThis->Owner : ((pObject) ? pObject->GetOwningHouse() : nullptr);

			Helper::Otamaa::SpawnMultiple(
				pAnimTypeExt->SpawnsMultiple,
				pAnimTypeExt->SpawnsMultiple_amouts,
				pThis->GetCenterCoord(), pObject, pHouse, pAnimTypeExt->SpawnsMultiple_Random.Get());

			if (pType->SpawnsParticle != -1)
			{
				auto const pParticleType = ParticleTypeClass::Array.get()->GetItem(pType->SpawnsParticle);
				auto const nCoord = pThis->GetCenterCoord();

				if (pType->NumParticles > 0 && pParticleType)
				{
					for (int i = 0; i < pType->NumParticles; ++i)
					{
						CoordStruct nDestCoord;
						if (pAnimTypeExt->ParticleChance.isset() ?
							(ScenarioGlobal->Random(0, 99) < abs(pAnimTypeExt->ParticleChance.Get())) : true)
						{
							nDestCoord = Helper::Otamaa::GetRandomCoordsInsideLoops(pAnimTypeExt->ParticleRangeMin.Get(), pAnimTypeExt->ParticleRangeMax.Get(), nCoord, i);
							ParticleSystemClass::Instance->SpawnParticle(pParticleType, &nDestCoord);
						}
					}
				}
			}

			if (!pTypeExt->Launchs.empty())
			{
				for (auto const& nLauch : pTypeExt->Launchs)
				{
					Helpers::Otamaa::LauchSW(
							nLauch.LaunchWhat, pHouse,
							pThis->GetCenterCoord(), nLauch.LaunchWaitcharge,
							nLauch.LaunchResetCharge,
							nLauch.LaunchGrant,
							nLauch.LaunchGrant_RepaintSidebar,
							nLauch.LaunchGrant_OneTime,
							nLauch.LaunchGrant_OnHold,
							nLauch.LaunchSW_Manual,
							nLauch.LaunchSW_IgnoreInhibitors,
							nLauch.LauchSW_IgnoreMoney
					);
				}
			}
		}
	}

	return 0x42504D;
}

DEFINE_HOOK(0x423991, AnimClass_BounceAI_BounceAnim, 0xA)
{
	GET(AnimTypeClass*, pBounceAnim, ECX);
	GET(AnimClass*, pThis, EBP);

	HouseClass* pHouse = nullptr;
	TechnoClass* pObject = nullptr;

	if (auto pTypeExt = AnimTypeExt::ExtMap.Find(pBounceAnim))
	{
		pObject = AnimExt::GetTechnoInvoker(pThis, pTypeExt->Damage_DealtByInvoker.Get());
		pHouse = pThis->Owner ? pThis->Owner : ((pObject) ? pObject->GetOwningHouse() : pHouse);
	}

	auto nCoord = pThis->GetCenterCoord();
	if (auto pAnim = GameCreate<AnimClass>(pBounceAnim, nCoord, 0, 1, 0x600, 0, 0)) {
		AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, nullptr, pObject, false);
	}

	return 0x4239D3;
}

// Draw Tiled !

DEFINE_HOOK(0x4236A7, AnimClass_Draw_Tiled_CustomPalette, 0xA)
{
	GET(AnimClass*, pThis, ESI);
	GET(int, nY_Loc, EDI);
	GET(int, nYadd_Loc, EBP);
	GET(BlitterFlags, nFlags, EBX);
	GET_STACK(int, nFrame, STACK_OFFS(0x110, 0xE4));
	LEA_STACK(Point2D*, nPoint, STACK_OFFS(0x110, 0xE0));
	GET_STACK(int, nSHPHeight, STACK_OFFS(0x110, 0xF0));
	GET_STACK(SHPStruct*, pShp, STACK_OFFS(0x110, 0xE8));
	GET_STACK(int, nX_Loc, STACK_OFFS(0x110, 0xD0));
	GET_STACK(int, nTintColor, STACK_OFFS(0x110, 0xF4));
	GET_STACK(int, nBrightness, STACK_OFFS(0x110, 0xD8));

	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (!pShp)
		return 0x42371B;

	auto pPal = FileSystem::ANIM_PAL();

	if (pTypeExt && pTypeExt->Palette.GetConvert()) {
		//if(!pTypeExt->Palette.Name.empty())
		//	Debug::Log("Anim[%s] with Custom Pal [%s] DrawTiled ! \n", pThis->get_ID(), pTypeExt->Palette.Name.c_str());
		pPal = pTypeExt->Palette.GetConvert();
	}

	auto Y_Doffs = pThis->Type->YDrawOffset;

	for (; nYadd_Loc >= 0;)
	{
		nPoint->X = nX_Loc;
		nPoint->Y = nYadd_Loc + Y_Doffs;

		DSurface::Temp->DrawSHP(
		pPal,
		pShp,
		nFrame,
		nPoint,
		&Drawing::SurfaceDimensions_Hidden(),
		nFlags,
		0,
		nY_Loc,
		ZGradient::Deg90,
		nBrightness,
		nTintColor,
		0,
		0,
		0,
		0);

		nYadd_Loc -= nSHPHeight;
		nY_Loc -= nSHPHeight + (nSHPHeight / 2);
		nPoint->Y = nYadd_Loc + Y_Doffs;
	}
	#ifdef Other_Impl_AnimPal
	do
	{
		DSurface::Temp->DrawSHP(
			pPal,
			pShp,
			nFrame,
			nPoint,
			&Drawing::SurfaceDimensions_Hidden(),
			nFlags,
			0,
			nY_Loc,
			ZGradient::Deg90,
			nBrightness,
			nTintColor,
			0,
			0,
			0,
			0);

		nYadd_Loc -= nSHPHeight;
		nY_Loc -= nSHPHeight + nSHPHeight / 2;
		nPoint->Y = nYadd_Loc + Y_Doffs;
	}
	while (nYadd_Loc >= 0);
	#endif
	return 0x42371B;
}

HouseClass* __fastcall AnimClassGetOwningHouse_Wrapper(AnimClass* pThis, void* _) {
	return pThis->Owner;
}

// Bruh ,..
DEFINE_JUMP(VTABLE,0x7E3390, GET_OFFSET(AnimClassGetOwningHouse_Wrapper));

static DWORD SpawnCreater(AnimClass* pThis, CellClass* pCell, Point2D& nVal)
{
	//if (auto pExt = AnimExt::ExtMap.Find(pThis))
	{
		auto pType = pThis->Type;
		if (!pType)
			return 0x0;

		if (auto pTypeExt = AnimTypeExt::ExtMap.Find(pType))
		{
			if (pTypeExt->SpawnCrater.Get(pThis->GetHeight() < 30))
			{
				auto nCoord = pThis->GetCenterCoord();
				if (!pType->Scorch || (pType->Crater && ScenarioGlobal->Random.RandomDouble() >= pTypeExt->CraterChance.Get()))
				{
					if (pType->Crater)
					{
						pCell->ReduceTiberium(6);
						if (pType->ForceBigCraters)
							SmudgeTypeClass::CreateRandomSmudgeFromTypeList(nCoord, 300, 300, true);
						else
							SmudgeTypeClass::CreateRandomSmudgeFromTypeList(nCoord, nVal.X, nVal.Y, false);
					}
				}
				else
				{
					bool bSpawn = (pTypeExt->ScorchChance.isset()) ? (ScenarioGlobal->Random.RandomDouble() >= pTypeExt->ScorchChance.Get()) : true;
					if (bSpawn)
						SmudgeTypeClass::CreateRandomSmudge(nCoord, nVal.X, nVal.Y, false);
				}
			}

			return 0x42513F;
		}
	}
	return 0x0;
}

DEFINE_HOOK(0x42504D, AnimClass_Middle_SpawnCreater, 0x4)
{
	GET(AnimClass*, pThis, ESI);
	GET_STACK(CellClass*, pCell, STACK_OFFS(0x30, 0x14));
	GET(int, nX, EBP);
	GET_STACK(int, nY, STACK_OFFS(0x30, 0x20));

	Point2D nVal { nX , nY };
	return SpawnCreater(pThis, pCell, nVal);
}

DEFINE_HOOK(0x4242F4, AnimClass_Trail_Override, 0x4)
{
	GET(AnimClass*, pMem, EDI);
	GET(AnimClass*, pThis, ESI);

	GameConstruct(pMem, pThis->Type->TrailerAnim, pThis->GetCoords(), 1, 1, 0x600, 0, false);

	if (auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type))
	{
		auto pTech = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker.Get());
		auto pOwner = pThis->Owner ? pThis->Owner : pTech ? pTech->GetOwningHouse() : nullptr;
		AnimExt::SetAnimOwnerHouseKind(pMem, pOwner, nullptr, pTech, false);
	}

	return 0x424322;
}

DEFINE_HOOK(0x423F9D, AnimClass_Spawns_Override , 0x8)
{
	GET(AnimClass*, pMem, EAX);
	GET(AnimClass*, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x8C, 0x4C));

	GameConstruct(pMem, pThis->Type->Spawns, pThis->GetCoords(), 0, 1, 0x600, 0, false);
	if (auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type)) {
		auto pTech = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker.Get());
		auto pOwner = pThis->Owner ? pThis->Owner : pTech ? pTech->GetOwningHouse() : nullptr;
		AnimExt::SetAnimOwnerHouseKind(pMem, pOwner, nullptr, pTech, false);
	}

	R->EAX(pMem);
	return 0x423FC3;
}

#ifdef OVERRIDE_SHP_DRAWING
struct DummyStructForDrawing
{
	static void Callme(
		Surface* dest_surface,
		ConvertClass* drawer,
		SHPReference* shape,
		int shapenum,
		Point2D* xy,
		RectangleStruct* rect1,
		BlitterFlags flags,
		int drawerval,
		int height_offset,
		int somearrayindex,
		int intensity,
		int useotherblitterset,
		SHPStruct* z_shape,
		int z_shape_frame,
		int shape_x_offset,
		int shape_y_offset)
	{
		SHPReference* v16 = nullptr;
		BSurface* v35 = nullptr;
		RectangleStruct z_shape_rect { 0,0,0,0 };

		if (shape)
		{
			if (shape->Type == -1)
			{
				if (!shape->Loaded)
				{
					shape->Load();
				}
				v16 = shape->AsReference();
			}
			if (v16)
			{
				RectangleStruct nDummy = dest_surface->Get_Rect();
				auto area_rect = *rect1;
				area_rect = Drawing::Intersect(area_rect, nDummy);

				if (area_rect.Width > 0 && area_rect.Height > 0)
				{
					drawer->CurrentZRemap = drawerval;
					int xpos = xy->X;
					int ypos = xy->Y;
					RectangleStruct shape_rect = v16->GetFrameBounds(shapenum);
					auto v21 = v16->GetPixels(shapenum);
					auto height = v16->Height;
					auto v23 = v21;
					auto width = v16->Width;
					MemoryBuffer Buffer { v23 ,shape_rect.Height * shape_rect.Width };
					BSurface shape_surface { shape_rect.Width, shape_rect.Height,1, &Buffer };

					if (z_shape)
					{
						z_shape_rect = z_shape->GetFrameBounds(z_shape_frame);
						MemoryBuffer nMemBuffer { z_shape_rect.Height * z_shape_rect.Width };
						v35 = GameCreate<BSurface>(z_shape_rect.Width, z_shape_rect.Height, 1, &nMemBuffer);
					}

					if ((BYTE(flags) & 2) != 0)
					{
						xpos += width / -2;
						ypos += height / -2;
					}

					if (z_shape)
					{
						shape_x_offset -= width / 2 - shape_rect.X;
						shape_y_offset -= height / 2 - shape_rect.Y;
					}

					xpos += shape_rect.X;
					ypos += shape_rect.Y;
					shape_rect.X = 0;
					shape_rect.Y = 0;
					int intersect_rect_height = 0;
					RectangleStruct reused_rect {};

					if (!z_shape)
						goto noZShapeFound;
					{
						shape_x_offset += z_shape_rect.X;
						shape_y_offset += z_shape_rect.Y;
						reused_rect.X = -shape_x_offset;
						reused_rect.Y = -shape_y_offset;
						reused_rect.Width = z_shape_rect.Width;
						reused_rect.Height = z_shape_rect.Height;
						auto intersect_rect = Drawing::Intersect(shape_rect, reused_rect);
						shape_rect.X = intersect_rect.X;
						shape_rect.Y = intersect_rect.Y;
						shape_rect.Width = intersect_rect.Width;
						intersect_rect_height = intersect_rect.Height;
						shape_rect.Height = intersect_rect_height;
					}

					if (shape_rect.Width > 0 && intersect_rect_height > 0)
					{
					noZShapeFound:
						if (drawerval)
						{
							flags |= (BlitterFlags)16;
						}

						if (v16->HasCompression(shapenum))
						{

							if (auto blitter = drawer->Select_RLE_Blitter(flags))
							{
								reused_rect.X = xpos;
								reused_rect.Y = ypos;
								reused_rect.Width = shape_rect.Width;
								reused_rect.Height = shape_rect.Height;
								Point2D ShapePoint = { shape_rect.X, shape_rect.Y };
								Point2D AreaPoint = { area_rect.X, area_rect.Y };

								Buffer_To_RLE_Surface_With_Z_Shape(
									dest_surface,
									&AreaPoint,
									&reused_rect,
									&shape_surface,
									&ShapePoint,
									&shape_rect,
									blitter,
									height_offset,
									somearrayindex,
									intensity,
									0,
									v35,
									shape_x_offset,
									shape_y_offset,
									useotherblitterset);
							}
						}
						else
						{
							if (auto blitter = drawer->Select_Blitter(flags))
							{
								reused_rect.Y = 0;
								reused_rect.X = 0;
								z_shape_rect.X = xpos;
								z_shape_rect.Y = ypos;
								z_shape_rect.Width = shape_rect.Width;
								z_shape_rect.Height = shape_rect.Height;
								reused_rect.Width = shape_surface.Width;
								reused_rect.Height = shape_surface.Height;

								Buffer_To_Surface_with_LastArg(
									dest_surface,
									&area_rect,
									&z_shape_rect,
									&shape_surface,
									&reused_rect,
									&shape_rect,
									blitter,
									height_offset,
									somearrayindex,
									intensity,
									useotherblitterset,
									0);
							}
						}
					}

					if (v35)
					{
						GameDelete(v35);
					}

					shape_surface.DestroyBuffer();
				}
			}
		}

	}
};

DEFINE_HOOK(0x4AED70, Game_DrawSHP, 0x0)
{
	GET(Surface*, dest_surface, ECX);
	GET(ConvertClass*, drawer, EDX);
	GET_STACK(SHPReference*, shape, 0x4);
	GET_STACK(int, shapenum, 0x8);
	GET_STACK(Point2D*, xy, 0xC);
	GET_STACK(RectangleStruct*, rect1, 0x10);
	GET_STACK(BlitterFlags, flags, 0x14);
	GET_STACK(int, drawerval, 0x18);
	GET_STACK(int, height_offset, 0x1C);
	GET_STACK(int, somearrayindex, 0x20);
	GET_STACK(int, intensity, 0x24);
	GET_STACK(int, useotherblitterset, 0x28);
	GET_STACK(SHPStruct*, z_shape, 0x2C);
	GET_STACK(int, z_shape_frame, 0x30);
	GET_STACK(int, shape_x_offset, 0x34);
	GET_STACK(int, shape_y_offset, 0x38);

	DummyStructForDrawing::Callme(dest_surface, drawer, shape, shapenum, xy, rect1, flags, drawerval, height_offset, somearrayindex, intensity, useotherblitterset, z_shape, z_shape_frame
		, shape_x_offset, shape_y_offset);

	return 0x4AF292;
}
#endif
