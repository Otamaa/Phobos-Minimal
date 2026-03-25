#include <exception>
#include <Windows.h>

#include <GeneralDefinitions.h>
#include <SpecificStructures.h>
#include <AnimClass.h>
#include <AnimTypeClass.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/AnimExt.h>
#include <Misc/Kratos/Extension/BulletExt.h>

#include <Misc/Kratos/Ext/Helper/Scripts.h>

#include <Misc/Kratos/Ext/BulletType/BulletStatus.h>
#include <Misc/Kratos/Ext/Common/CommonStatus.h>
#include <Misc/Kratos/Ext/Common/ExpandAnimsManager.h>

#include <Ext/Bullet/Body.h>

// ----------------
// Extension
// ----------------
#ifndef _ENABLE_HOOKS

ASMJIT_PATCH(0x4664BA, BulletClass_CTOR, 0x5)
{
	// skip this Allocate just left BulletClass_Load_Suffix => LoadKey to Allocate
	// when is loading a save game.
	if (!Phobos::Otamaa::DoingLoadGame)
	{
		GET(BulletClass*, pItem, ESI);
		BulletExtContainer::Instance.Allocate(pItem);
		BulletExt::ExtMap.TryAllocate(pItem);
	}
	return 0;
}

ASMJIT_PATCH(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ECX);

	BulletExtContainer::Instance.Remove(pItem);

	if (BulletExt::ExtData* ext = BulletExt::ExtMap.Find(pItem))
	{
		ext->SetExtStatus(nullptr);
		ext->_GameObject->Foreach([](Component* c)
			{if (auto cc = dynamic_cast<IBulletScript*>(c)) { cc->OnUnInit(); } });
	}
	BulletExt::ExtMap.Remove(pItem);

	return 0;
}

// ----------------
// Component
// ----------------

ASMJIT_PATCH(0x466556, BulletClass_Init, 0x6)
{
	GET(FakeBulletClass*, pItem, ECX);

	pItem->_GetExtData()->Name = pItem->Type->ID;

	if (auto pExt = BulletExt::ExtMap.Find(pItem))
	{
		pExt->_GameObject->Foreach([](Component* c)
			{if (auto cc = dynamic_cast<IBulletScript*>(c)) { cc->OnInit(); } });
	}

	return 0;
}

ASMJIT_PATCH(0x468B5D, BulletClass_Put, 0x6)
{
	GET(BulletClass*, pThis, EBX);
	GET_STACK(CoordStruct*, pCoord, -0x20);
	DirType faceDir = DirType::North;

	if (auto pExt = BulletExt::ExtMap.Find(pThis))
	{
		pExt->_GameObject->Foreach([pCoord, faceDir](Component* c)
			{ if (auto cc = dynamic_cast<IBulletScript*>(c)) { cc->OnPut(pCoord, faceDir); } });
	}

	return 0;
}

ASMJIT_PATCH(0x4666F7, BulletClass_Update, 0x6)
{
	GET(BulletClass*, pThis, EBP);

	if (auto pExt = BulletExt::ExtMap.Find(pThis))
	{
		pExt->_GameObject->Foreach([](Component* c)
			{ c->OnUpdate(); });
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x467FEE, BulletClass_UpdateEnd, 0x6)
ASMJIT_PATCH(0x466781, BulletClass_UpdateEnd, 0x6)
{
	GET(BulletClass*, pThis, EBP);

	if (auto pExt = BulletExt::ExtMap.Find(pThis))
	{
		pExt->_GameObject->Foreach([](Component* c)
			{ c->OnUpdateEnd(); });
	}

	return 0;
}

ASMJIT_PATCH(0x4690C1, BulletClass_Detonate_Early, 0x8)
{
	GET(BulletClass*, pThis, ECX);
	GET_BASE(CoordStruct*, pPos, 0x8);

	if (auto pExt = BulletExt::ExtMap.Find(pThis))
	{
		bool skip = false;
		pExt->_GameObject->Foreach([&](Component* c)
			{ if (auto cc = dynamic_cast<IBulletScript*>(c)) { cc->OnDetonate(pPos, skip); } });

		if (skip)
		{
			return 0x46A2FB;
		}
	}
	return 0;
}

// ----------------
// Feature
// ----------------

#pragma region Remap

// when shooter dead, project's house will be 0
ASMJIT_PATCH(0x469A75, BulletClass_Detonate_GetHouse, 0x7)
{
	GET(FakeBulletClass*, pBullet, ESI);
	GET(HouseClass*, pHouse, ECX);

	pHouse = pBullet->Owner ? pBullet->Owner->Owner : pBullet->_GetExtData()->Owner;

	if (!pHouse)
	{
		// GetStatus save the House
		if (HouseClass* pSourceHouse = GetSourceHouse(pBullet))
		{
			R->ECX(pSourceHouse);
		}
	}
	return 0;
}

namespace BulletColour
{
	BulletClass* pBullet = nullptr;
	PaintData myData{};
}

ASMJIT_PATCH(0x4683E2, BulletClass_DrawSHP_Colour, 0x5)
{
	GET(BulletClass*, pBullet, ESI);
	BulletColour::pBullet = nullptr;
	if (BulletStatus* status = GetStatus<BulletExt, BulletStatus>(pBullet))
	{
		PaintData data = status->MyPaintData;
		if (data.ChangeColor || data.ChangeBright)
		{
			BulletColour::pBullet = pBullet;
			BulletColour::myData = data;
			if (data.ChangeColor)
			{
				R->Stack(0, data.Data.Color2);
			}
		}
	}
	return 0;
}

ASMJIT_PATCH(0x4683E7, BulletClass_DrawSHP_Bright, 0x9)
{
	GET(BulletClass*, pBullet, ESI);
	if (pBullet == BulletColour::pBullet && BulletColour::myData.ChangeBright)
	{
		R->Stack(0, GetBright(1000, BulletColour::myData.Data.BrightMultiplier));
	}
	BulletColour::pBullet = nullptr;
	return 0;
}

ASMJIT_PATCH(0x46B201, BulletClass_DrawVXL_Colour, 0x7)
{
	R->EDI(BlitterFlags::None);
	R->Stack(0, Add2RGB565(ToColorAdd(ColorStruct::Red)));
	GET_STACK(BulletClass*, pBullet, 0x10 - 0x4);
	if (BulletStatus* status = GetStatus<BulletExt, BulletStatus>(pBullet))
	{
		status->DrawVXL_Paintball(R);
	}
	return 0;
}
#pragma endregion

#pragma region Trajectory
// 除 ROT>0 和 Vertical 之外的抛射体会在此根据重力对储存的向量变量进行运算
// 对Arcing抛射体的重力进行削减
ASMJIT_PATCH(0x46745C, BulletClass_Update_ChangeVelocity, 0x7)
{
	GET(BulletClass*, pBullet, EBP);
	if (BulletStatus* status = GetStatus<BulletExt, BulletStatus>(pBullet))
	{
		if (status->IsArcing() && status->SpeedChanged)
		{
			LEA_STACK(VelocityClass*, pVelocity, 0x90);
			VelocityClass velocity = pBullet->Velocity;
			pVelocity->X = velocity.X;
			pVelocity->Y = velocity.Y;
			pVelocity->Z = status->LocationLocked ? 0 : velocity.Z;
		}
	}
	return 0;
}

// 除 ROT>0 和 Vertical 之外的抛射体会在Label_158根据速度向量获取坐标
// Arcing抛射体即使向量非常小也会试图移动1点
ASMJIT_PATCH(0x4677C7, BulletClass_Update_ChangeVelocity_Locked, 0x8)
{
	GET(BulletClass*, pBullet, EBP);
	if (BulletStatus* status = GetStatus<BulletExt, BulletStatus>(pBullet))
	{
		if (status->IsArcing() && status->SpeedChanged && status->LocationLocked)
		{
			CoordStruct location = pBullet->Location;
			R->ESI(location.X);
			R->EDI(location.Y);
			R->EAX(location.Z);
		}
	}
	return 0;
}

// 导弹类抛射体当高度低于地面高度时强制引爆
// 让直线导弹可以潜地
ASMJIT_PATCH(0x466E18, BulletClass_CheckHight_UnderGround, 0x6)
{
	GET(BulletClass*, pBullet, ECX);
	if (pBullet->GetHeight() <= 0)
	{
		if (BulletStatus* status = GetStatus<BulletExt, BulletStatus>(pBullet))
		{
			if (!status->SubjectToGround)
			{
				R->Stack<bool>(0x18, false);
				R->Stack<unsigned int>(0x20, 0);
			}
		}
	}
	return 0;
}
#pragma endregion
#endif