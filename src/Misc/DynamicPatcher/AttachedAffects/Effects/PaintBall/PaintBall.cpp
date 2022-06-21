#ifdef COMPILE_PORTED_DP_FEATURES
#include "PaintBall.h"

#include <Ext/Techno/Body.h>

void PaintballType::Read(INI_EX& parser, const char* pSection)
{
	Nullable<ColorStruct> nBuffColor { };
	nBuffColor.Read(parser, pSection, "PaintBall.Color");

	if (!nBuffColor.isset() || nBuffColor.Get() == ColorStruct::Empty)
		return;

	Color = nBuffColor.Get();
	Debug::Log("Found PaintBall.Color = %d %d %d for [%s] ! \n", Color.R, Color.G, Color.B, pSection);

	Valueable<float> nBuffFloat { BrightMultiplier };
	nBuffFloat = std::clamp(nBuffFloat.Get(), 0.0f, 2.0f);
	nBuffFloat.Read(parser, pSection, "PaintBall.BrightMult");
	BrightMultiplier = nBuffFloat.Get();

	Valueable<bool> nBuffBool { Accumulate };
	nBuffBool.Read(parser, pSection, "PaintBall.Accumulate");
	Accumulate = nBuffBool.Get();

	nBuffBool = IgnoreFog;
	nBuffBool.Read(parser, pSection, "PaintBall.IgnoreFog");
	IgnoreFog = nBuffBool.Get();

	nBuffBool = IgnoreShroud;
	nBuffBool.Read(parser, pSection, "PaintBall.IgnoreShroud");
	IgnoreShroud = nBuffBool.Get();
}

void PaintBall::Enable(int duration, WarheadTypeClass* pAffector, const PaintballType& data)
{

	if (!Token) {
		if (duration <= -1)
			return;

		Token = pAffector;
		Data = std::make_unique<PaintballType>(data);
		timer.Start(duration);
		return;
	}

	if (Token == pAffector) {
		if (!data.Accumulate && IsActive()) {
			return;
		} else {
			auto nTimeLeft = timer.GetTimeLeft() + duration;
			if (nTimeLeft <= 0) {
				timer.Stop();
				Token = nullptr;
				Data.reset();

			} else {
				timer.Add(nTimeLeft);
			}
		}

		return;
	}

	if(Token != pAffector) {
		if (duration <= -1)
			return;

		if(IsActive()) {
			timer.Stop();
			Token = nullptr;
			Data.reset();
		}

		Token = pAffector;
		Data = std::make_unique<PaintballType>(data);
		timer.Start(duration);
		return;
	}
}

uintptr_t PaintBall::GetColor() {
	auto nColor = Data->Color;
	unsigned short B = static_cast<unsigned short>(nColor.B >> 3);
	unsigned short G = static_cast<unsigned short>(nColor.G >> 2);
	unsigned short R = static_cast<unsigned short>(nColor.R >> 3);
	Color16Struct nRet = { R,G,B };
	return nRet.ToInit();
}

static inline bool AllowRedraw(TechnoClass* pWho, bool bForce ,bool bIgnoreShroud , bool bIgnoreFog)
{
	if(auto pCell = pWho->GetCell()) {

		if (pCell->IsShrouded() && !bIgnoreShroud)
			return false;

		if (pCell->IsFogged() && !bIgnoreFog)
			return false;
	}

	if (pWho->Berzerk)
		return false;

	if ((pWho->Airstrike && pWho->Airstrike->Target == pWho))
		return false;

	if (pWho->IsIronCurtained())
		return false;

	return true;
}

void PaintBall::DrawSHP_Paintball(TechnoClass* pTech, REGISTERS* R)
{
	auto const& [rePaint, changeColor, changeBright] = NeedPaint();

	if (!rePaint || !AllowRedraw(pTech ,!rePaint,Data->IgnoreShroud,Data->IgnoreFog))
		return;

	if (changeColor)
	{
		R->EAX(GetColor());
	}

	if (changeBright)
	{
		R->EBP(GetBright(R->EBP()));
	}
}

void PaintBall::DrawSHP_Paintball_BuildAnim(TechnoClass* pTech, REGISTERS* R)
{
	auto const& [rePaint, changeColor, changeBright] = NeedPaint();

	if (!rePaint || !AllowRedraw(pTech, !rePaint, Data->IgnoreShroud, Data->IgnoreFog))
		return;

	if (changeColor) {
		R->EBP(GetColor());
	}

	if (changeBright) {
		uintptr_t bright = R->Stack<uintptr_t>(0x38);
		R->Stack<uintptr_t>(0x38, GetBright(bright));
	}
}

void PaintBall::DrawVXL_Paintball(TechnoClass* pTech, REGISTERS* R, bool isBuilding)
{
	auto const& [rePaint, changeColor, changeBright] = NeedPaint();

	if (!rePaint || !AllowRedraw(pTech, !rePaint, Data->IgnoreShroud, Data->IgnoreFog))
		return;

	if (changeColor)
	{
		uintptr_t color = GetColor();

		if (isBuilding)
		{
			R->Stack<uintptr_t>(0x24, color);
		}
		else
		{
			R->ESI(color);
		}
	}

	if (changeBright)
	{
		if (isBuilding)
		{
			// Vxl turret
			uintptr_t bright = R->Stack<uintptr_t>(0x20);
			R->Stack<uintptr_t>(0x20, GetBright(bright));
		}
		else
		{
			uintptr_t bright = R->Stack<uintptr_t>(0x1E0);
			R->Stack<uintptr_t>(0x1E0, GetBright(bright));
		}
	}
}

DEFINE_HOOK(0x73C15F, UnitClass_DrawVXL_Colour, 0x7)
{
	GET(UnitClass* const, pOwnerObject, EBP);

	if(auto pExt = TechnoExt::GetExtData(pOwnerObject))
		pExt->PaintBallState.DrawVXL_Paintball(pOwnerObject, R, false);

	return 0;
}

DEFINE_HOOK(0x423630, AnimClass_Draw_It, 0x6)
{
	GET(AnimClass*, pAnim, ESI);

	if (pAnim && pAnim->IsBuildingAnim) {
		CoordStruct location = pAnim->GetCoords();
		if (auto pCell = MapClass::Instance->TryGetCellAt(location)) {
			if (auto pBuilding = pCell->GetBuilding()) {
				if (auto pExt = TechnoExt::GetExtData(pBuilding)) {
					pExt->PaintBallState.DrawSHP_Paintball_BuildAnim(pBuilding, R);
				}
			}
		}
	}

	return 0;
}

// case VISUAL_NORMAL
DEFINE_HOOK(0x7063FF, TechnoClass_DrawSHP_Colour, 0x7)
{
	GET(TechnoClass* const, pOwnerObject, ESI);

	if (auto pExt = TechnoExt::GetExtData(pOwnerObject)) {
		pExt->PaintBallState.DrawSHP_Paintball(pOwnerObject, R);
	}

	return 0;
}

DEFINE_HOOK(0x706640, TechnoClass_DrawVXL_Colour, 0x5)
{
	GET(TechnoClass* const, pOwnerObject, ECX);

	if (pOwnerObject->WhatAmI() == AbstractType::Building) {
		if (auto pExt = TechnoExt::GetExtData(pOwnerObject)) {
			pExt->PaintBallState.DrawVXL_Paintball(pOwnerObject, R, true);
		}
	}

	return 0;
}
#endif