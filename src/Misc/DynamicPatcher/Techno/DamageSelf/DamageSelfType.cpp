#include "DamageSelfType.h"

#include <WarheadTypeClass.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>

void DamageSelfType::Read(INI_EX& parser, const char* pSection)
{
	Valueable<bool> Enabled { false };
	Enabled.Read(parser, pSection, "SelfDamaging");
	Enable = Enabled.Get();

	if (Enable)
	{
		Valueable<bool> DeactiveWhenCivilian_ { true };
		DeactiveWhenCivilian_.Read(parser, pSection, "SelfDamaging.DeactiveWhenCivilian");
		DeactiveWhenCivilian = DeactiveWhenCivilian_.Get();

		Valueable<WarheadTypeClass*> nWHDumMy { RulesClass::Instance->C4Warhead };
		nWHDumMy.Read(parser, pSection, "SelfDamaging.Warhead");
		Warhead = nWHDumMy.Get();

		Valueable<int> nIntDummy { Damage };
		nIntDummy.Read(parser, pSection, "SelfDamaging.Damage");
		Damage = nIntDummy.Get();

		nIntDummy = ROF;
		nIntDummy.Read(parser, pSection, "SelfDamaging.ROF");
		ROF = nIntDummy.Get();

		Valueable<bool> nBoolDummy { PlayWarheadAnim };
		nBoolDummy.Read(parser, pSection, "SelfDamaging.WarheadAnim");
		PlayWarheadAnim = nIntDummy.Get();

		nBoolDummy = IgnoreArmor;
		nBoolDummy.Read(parser, pSection, "SelfDamaging.IgnoreArmor");
		IgnoreArmor = nIntDummy.Get();

		nBoolDummy = Decloak;
		nBoolDummy.Read(parser, pSection, "SelfDamaging.Decloak");
		Decloak = nIntDummy.Get();

		Valueable<KillMethod> nKillType { Type };
		nKillType.Read(parser, pSection, "SelfDamaging.KillType");
		Type = nKillType.Get();
	}
}


void DamageSelfState::OnPut(std::unique_ptr<DamageSelfState>& pState, const DamageSelfType& DData)
{
	if (DData.Enable) {
		pState = std::make_unique<DamageSelfState>(DData.ROF,DData);
	}
}

int DamageSelfState::GetRealDamage(ObjectClass* pObj, int damage, bool ignoreArmor, WarheadTypeClass* pWH)
{
	int realDamage = damage;
	if (!ignoreArmor)
	{
		// 计算实际伤害
		if (realDamage > 0)
		{
			realDamage = MapClass::GetTotalDamage(damage, pWH, pObj->GetType()->Armor, 0);
		}
		else
		{
			realDamage = -MapClass::GetTotalDamage(-damage, pWH, pObj->GetType()->Armor, 0);
		}
	}
	return realDamage;
}

void DamageSelfState::PlayWHAnim(ObjectClass* pObj, int realDamage, WarheadTypeClass* pWH)
{
	CoordStruct location = pObj->GetCoords();
	LandType landType = LandType::Clear;

	if (auto pCell = MapClass::Instance->GetCellAt(location)) {
		landType = pCell->LandType;
	}

	if (auto pWHAnimType = MapClass::SelectDamageAnimation(realDamage, pWH, landType, location))
	{
		AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pWHAnimType, location),
			pObj->GetOwningHouse(),
			nullptr,
			false
		);
	}
}

void DamageSelfState::TechnoClass_Update_DamageSelf(TechnoClass* pTechno)
{
	if (CanHitSelf() && Data)
	{
		auto pHouse = pTechno->GetOwningHouse();

		// 检查平民
		if (!Data->DeactiveWhenCivilian || (pHouse && !pHouse->Type->MultiplayPassive))
		{
			int realDamage = Data->Damage;

			if (Data->Type == KillMethod::Vanish)
			{
				// 静默击杀，需要计算实际伤害

				// 计算实际伤害
				realDamage = GetRealDamage(pTechno, realDamage, Data->IgnoreArmor, Data->Warhead);

				if (realDamage >= pTechno->Health)
				{
					// Logger.Log($"{Game.CurrentFrame} {pTechno}[{pTechno.Ref.Type.Ref.Base.Base.ID}] 收到自伤 {realDamage} 而死，设置了平静的移除");
					// 本次伤害足够打死目标，移除单位
					Debug::Log(__FUNCTION__" Called \n");
					TechnoExt::HandleRemove(pTechno , nullptr, false , false);
					return;
				}
			}

			if (realDamage < 0 || pTechno->CloakState == CloakState::Uncloaked || Data->Decloak)
			{
				// 维修或者显形直接炸
				int nDamage = Data->Damage;
				if(pTechno->Health > 0 && pTechno->IsAlive && !pTechno->IsSinking && !pTechno->IsCrashing)
					pTechno->ReceiveDamage(&nDamage, 0, Data->Warhead, nullptr, Data->IgnoreArmor, pTechno->GetTechnoType()->Crewed, pHouse);
			}
			else
			{
				// 不显形不能使用ReceiveDamage，改成直接扣血
				if (Data->Type != KillMethod::Vanish)
				{
					// 非静默击杀，实际伤害未计算过
					realDamage = GetRealDamage(pTechno, realDamage, Data->IgnoreArmor, Data->Warhead);
				}

				// 扣血
				if (realDamage >= pTechno->Health)
				{
					// 本次伤害足够打死目标
					if(pTechno->Health > 0 &&pTechno->IsAlive && !pTechno->IsSinking && !pTechno->IsCrashing)
						pTechno->ReceiveDamage(&realDamage, 0, Data->Warhead, nullptr, true, pTechno->GetTechnoType()->Crewed, pHouse);
				}
				else
				{
					// 血量可以减到负数不死
					pTechno->Health -= realDamage;
				}
			}

			// 播放弹头动画
			if (Data->PlayWarheadAnim)
			{
				PlayWHAnim(pTechno, realDamage, Data->Warhead);
			}

			Reset();
		}
	}
}
