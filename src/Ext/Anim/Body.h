#pragma once
#include <AnimClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/AnimType/Body.h>
#include "AlternateExt/Body.h"

class AnimExt
{
public:
	using base_type = AnimClass;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true);
	static TechnoClass* GetTechnoInvoker(AnimClass* pThis,bool DealthByOwner);
	//ToDo :
	// utilize this ,..
	// here as dummy atm
	struct AnimCellUpdater
	{
		static std::vector<CellClass*> Marked;

		static void Update() { }

		static void Invalidate() { }

		static void Print() {
			for (auto const& pair : Marked) {
				auto nCoord = pair->GetCoords();
				Debug::Log("Cell[%x] at [%d,%d,%d] Has Twinkle Anim ! \n", pair , nCoord.X , nCoord.Y , nCoord.Z);
			}
		}

		static void Clear() {
			Marked.clear();
		}
	};
};
