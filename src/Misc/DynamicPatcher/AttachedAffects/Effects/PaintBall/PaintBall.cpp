#include "PaintBall.h"

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Anim/Body.h>

void PaintballType::Read(INI_EX& parser, const char* pSection)
{
	Nullable<ColorStruct> nBuffColor { };
	nBuffColor.Read(parser, pSection, "PaintBall.Color"); //RGB 565

	if (!nBuffColor.isset() || nBuffColor.Get() == ColorStruct::Empty)
		return;

	Color = nBuffColor.Get();

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

	nBuffBool = Override;
	nBuffBool.Read(parser, pSection, "PaintBall.OverrideSameAffect");
	Override = nBuffBool.Get();
}

void PaintBall::Enable(int duration, WarheadTypeClass* pAffector, const PaintballType& data)
{

	if (!Token)
	{
		if (duration <= -1)
			return;

		Token = pAffector;
		Data = data;
		timer.Start(duration);
		return;
	}

	if (Token == pAffector)
	{
		if (!data.Accumulate && IsActive())
		{
			if (!data.Override)
				return;
			else
			{
				timer.Stop();
				timer.Start(duration);
				return;
			}
		}
		else
		{
			auto nTimeLeft = timer.GetTimeLeft() + duration;
			if (nTimeLeft <= 0)
			{
				timer.Stop();
				Token = nullptr;
				Data.clear();

			}
			else
			{
				timer.Add(nTimeLeft);
			}
		}

		return;
	}

	if (Token != pAffector)
	{
		if (duration <= -1)
			return;

		if (IsActive())
		{
			timer.Stop();
			Token = nullptr;
			Data.clear();
		}

		Token = pAffector;
		Data = (data);
		timer.Start(duration);
		return;
	}
}

uintptr_t PaintBall::GetColor()
{
	// readed as 3Bytes
	// but this is actually Color16
	// then need to make it ColorStruct
	// then convert it to DWORD
	Color16Struct nColor16 = { Data.get().Color.R,Data.get().Color.G,Data.get().Color.B };
	ColorStruct nColorAgain = ColorStruct { nColor16 };
	return Drawing::RGB2DWORD(nColorAgain);

	//return GeneralUtils::GetColorFromColorAdd(Data.get().Color);
}

static inline bool AllowRedraw(TechnoClass* pWho, bool bForce, bool bIgnoreShroud, bool bIgnoreFog)
{
	if (auto const pBld = specific_cast<BuildingClass*>(pWho))
	{
		if (pBld->IsFogged && !bIgnoreFog)
			return false;
	}

	if (auto pCell = pWho->GetCell())
	{
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

	if (!rePaint || !AllowRedraw(pTech, !rePaint, Data.get().IgnoreShroud, Data.get().IgnoreFog))
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

	if (!rePaint || !AllowRedraw(pTech, !rePaint, Data.get().IgnoreShroud, Data.get().IgnoreFog))
		return;

	if (changeColor)
	{
		R->EBP(GetColor());
	}

	if (changeBright)
	{
		uintptr_t bright = R->Stack<uintptr_t>(0x38);
		R->Stack<uintptr_t>(0x38, GetBright(bright));
	}
}

void PaintBall::Update(TechnoClass* pThis)
{
	if (this->IsActive())
	{
		if (pThis->WhatAmI() == BuildingClass::AbsID)
		{
				BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pThis))->LighningNeedUpdate = true;
		}

	}
	else
	{
		this->Disable(true);
	}

}

void PaintBall::DrawVXL_Paintball(TechnoClass* pTech, REGISTERS* R, bool isBuilding)
{
	auto const& [rePaint, changeColor, changeBright] = NeedPaint();

	if (!rePaint || !AllowRedraw(pTech, !rePaint, Data.get().IgnoreShroud, Data.get().IgnoreFog))
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