#include "Phobos.h"

#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Cast.h>

#include <TacticalClass.h>
#include <HouseClass.h>
#include <Unsorted.h>

class ExtSelection final : public TacticalClass
{
public:
	using callback_type = bool(__fastcall*)(ObjectClass*);
	static inline struct TacticalSelectablesHelper
	{
		inline size_t size()
		{
			return TacticalClass::Instance->SelectableCount;
		}

		inline TacticalSelectableStruct* begin()
		{
			return &Unsorted::TacticalSelectables[0];
		}

		inline TacticalSelectableStruct* end()
		{
			return &Unsorted::TacticalSelectables[size()];
		}
	} Array {};

	// Reversed from Is_Selectable, w/o Select call
	static bool ObjectClass_IsSelectable(ObjectClass* pThis)
	{
		const auto pOwner = pThis->GetOwningHouse();
		return pOwner
			&& pOwner->ControlledByCurrentPlayer()
			&& pThis->CanBeSelected()
			&& pThis->CanBeSelectedNow();
	}

	// Reversed from Tactical::Select
	bool IsInSelectionRect(LTRBStruct* pRect, const TacticalSelectableStruct& selectable)
	{
		if (selectable.Techno
			&& selectable.Techno->IsAlive
			&& !selectable.Techno->InLimbo
			&& selectable.Techno->AbstractFlags & AbstractFlags::Techno
			)
		{
			int nLocalX = selectable.Point.X - this->TacticalPos.X;
			int nLocalY = selectable.Point.Y - this->TacticalPos.Y;

			if ((nLocalX >= pRect->Left && nLocalX < pRect->Right + pRect->Left) &&
				(nLocalY >= pRect->Top && nLocalY < pRect->Bottom + pRect->Top)) {
				return true;
			}
		}
		return false;
	}

	bool IsHighPriorityInRect(LTRBStruct* rect)
	{
		for (const auto& selected : Array){
			if (this->IsInSelectionRect(rect, selected) && ObjectClass_IsSelectable(selected.Techno)){
				return !TechnoTypeExtContainer::Instance.Find(selected.Techno->GetTechnoType())->LowSelectionPriority;
			}
		}

		return false;
	}

	// Reversed from Tactical::Select
	void SelectFiltered(LTRBStruct* pRect, callback_type fpCheckCallback, bool bPriorityFiltering)
	{
		Unsorted::MoveFeedback = true;

		if (pRect->Right <= 0 || pRect->Bottom <= 0 || this->SelectableCount <= 0)
			return;

		for (const auto& selected : Array){
			if (this->IsInSelectionRect(pRect, selected))
			{
				const auto pTechno = selected.Techno;
				const auto pTechnoType = pTechno->GetTechnoType();
				const auto TypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

				if (bPriorityFiltering && TypeExt->LowSelectionPriority)
					continue;

				if (TypeExt && Game::IsTypeSelecting())
					Game::UICommands_TypeSelect_7327D0(TypeExt->GetSelectionGroupID());
				else if (fpCheckCallback)
					(*fpCheckCallback)(pTechno);
				else
				{
					const auto pBldType = type_cast<BuildingTypeClass*>(pTechnoType);
					const auto pOwner = pTechno->GetOwningHouse();

					if (pOwner
						&& pOwner->ControlledByCurrentPlayer()
						&& pTechno->CanBeSelected()
						&& (!pBldType || pBldType->IsUndeployable())
						)
					{
						Unsorted::MoveFeedback = !pTechno->Select();
					}
				}
			}
		}

		Unsorted::MoveFeedback = true;
	}

	static // Reversed from Tactical::MakeSelection
	void __fastcall Tactical_MakeFilteredSelection(ExtSelection* pThis, void*_, callback_type fpCheckCallback)
	{
		if (pThis->Band.Left || pThis->Band.Top) {
			int nLeft = pThis->Band.Left;
			int nRight = pThis->Band.Right;
			int nTop = pThis->Band.Top;
			int nBottom = pThis->Band.Bottom;

			if (nLeft > nRight)
				std::swap(nLeft, nRight);
			if (nTop > nBottom)
				std::swap(nTop, nBottom);

			LTRBStruct rect { nLeft , nTop, nRight - nLeft + 1, nBottom - nTop + 1 };

			const bool bPriorityFiltering = Phobos::Config::PrioritySelectionFiltering
			&& pThis->IsHighPriorityInRect(&rect);

			pThis->SelectFiltered(&rect, fpCheckCallback, bPriorityFiltering);

			pThis->Band.Left = 0;
			pThis->Band.Top = 0;
		}
	}
};
static_assert(sizeof(ExtSelection) == sizeof(TacticalClass), "MustBe Same!");


// Replace single call
DEFINE_JUMP(CALL,0x4ABCEB, MiscTools::to_DWORD(&ExtSelection::Tactical_MakeFilteredSelection));

// Replace vanilla function. For in case another module tries to call the vanilla function at offset
DEFINE_JUMP(LJMP, 0x6D9FF0, MiscTools::to_DWORD(&ExtSelection::Tactical_MakeFilteredSelection));

