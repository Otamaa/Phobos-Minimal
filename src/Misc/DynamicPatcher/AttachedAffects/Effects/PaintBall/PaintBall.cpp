#include "PaintBall.h"

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>

void PaintballType::Read(INI_EX& parser, const char* pSection)
{
	Nullable<ColorStruct> nBuffColor { };
	nBuffColor.Read(parser, pSection, "PaintBall.Color"); //RGB 565

	if (!nBuffColor.isset() || nBuffColor.Get() == ColorStruct::Empty)
		return;

	Color = nBuffColor.Get();

	detail::read(BrightMultiplier, parser, pSection, "PaintBall.BrightMult", false);
	BrightMultiplier = std::clamp(BrightMultiplier, 0.0f, 2.0f);

	detail::read(Accumulate, parser, pSection, "PaintBall.Accumulate" , false);
	detail::read(IgnoreFog, parser, pSection, "PaintBall.IgnoreFog", false);
	detail::read(IgnoreShroud, parser, pSection, "PaintBall.IgnoreShroud", false);
	detail::read(Override, parser, pSection, "PaintBall.OverrideSameAffect", false);
}

void PaintBall::Init()
{
	if (!Color.isset()) {
		// readed as 3Bytes
		// but this is actually Color16
		// then need to make it ColorStruct
		// then convert it to DWORD
		Color16Struct nColor16 = {
			Data->Color.R,
			Data->Color.G,
			Data->Color.B
		};

		ColorStruct nColorAgain = ColorStruct { nColor16 };
		Color = Drawing::RGB2DWORD(nColorAgain);
	}
}

 bool PaintBall::AllowDraw(TechnoClass* pWho)
{
	if (auto const pBld = specific_cast<BuildingClass*>(pWho))
	{
		if (pBld->IsFogged && !Data->IgnoreFog)
			return false;
	}

	if (auto pCell = pWho->GetCell())
	{
		if (pCell->IsShrouded() && !Data->IgnoreShroud)
			return false;

		if (pCell->IsFogged() && !Data->IgnoreFog)
			return false;
	}

	return true;
}

 /*
void PaintBall::DrawSHP_Paintball(TechnoClass* pTech, REGISTERS* R)
{
	auto const& [rePaint, changeColor, changeBright] = NeedPaint();

	if (!rePaint || !PaintBall::AllowRedraw(pTech, !rePaint, Data->IgnoreShroud, Data->IgnoreFog))
		return;

	if (changeColor)
	{
		R->EAX(PaintBall::GetColor());
	}

	if (changeBright)
	{
		R->EBP(GetBright(R->EBP()));
	}
}

void PaintBall::DrawSHP_Paintball_BuildAnim(TechnoClass* pTech, REGISTERS* R)
{
	auto const& [rePaint, changeColor, changeBright] = NeedPaint();

	if (!rePaint || !PaintBall::AllowRedraw(pTech, !rePaint, Data->IgnoreShroud, Data->IgnoreFog))
		return;

	if (changeColor)
	{
		R->EBP(PaintBall::GetColor());
	}

	if (changeBright)
	{
		uintptr_t bright = R->Stack<uintptr_t>(0x38);
		R->Stack<uintptr_t>(0x38, PaintBall::GetBright(bright));
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
*/