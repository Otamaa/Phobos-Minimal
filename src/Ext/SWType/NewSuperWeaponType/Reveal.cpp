#include "Reveal.h"

#include <Misc/MapRevealer.h>
#include <Utilities/Helpers.h>

std::vector<const char*> SW_Reveal::GetTypeString() const
{
	return { "Reveal" };
}

bool SW_Reveal::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::PsychicReveal);
}

bool SW_Reveal::Activate(SuperClass* const pThis, const CellStruct& Coords, bool const IsPlayer)
{
	auto const pSW = pThis->Type;
	auto const pData = SWTypeExt::ExtMap.Find(pSW);

	if (pThis->IsCharged)
	{
		MapRevealer const revealer(Coords);

		if (revealer.AffectsHouse(pThis->Owner))
		{
			auto Apply = [=, &revealer](bool add)
			{
				auto const range = GetRange(pData);

				if (range.WidthOrRange < 0.0)
				{
					// reveal all cells without hundred thousands function calls
					//auto const Map = MapClass::Instance();
					MapClass::Instance->CellIteratorReset();
					while (auto const pCell = MapClass::Instance->CellIteratorNext())
					{
						if (revealer.IsCellAvailable(pCell->MapCoords) && revealer.IsCellAllowed(pCell->MapCoords))
						{
							revealer.Process1(pCell, false, add);
						}
					}
				}
				else
				{
					// default way to reveal, but reveal one cell at a time.
					auto const& base = revealer.Base();

					Helpers::Alex::for_each_in_rect_or_range<CellClass>(base, range.WidthOrRange, range.Height,
						[=, &revealer](CellClass* pCell) -> bool
					{
						auto const& cell = pCell->MapCoords;
						if (revealer.IsCellAvailable(cell) && revealer.IsCellAllowed(cell))
						{
							if (range.height() > 0 || cell.DistanceFrom(base) < range.range())
							{
								revealer.Process1(pCell, false, add);
							}
						}
						return true;
					});
				}
			};
			Apply(false);
			Apply(true);

			MapClass::Instance->MarkNeedsRedraw(1);
		}
	}

	return true;
}

void SW_Reveal::Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) 
{ }

void SW_Reveal::Initialize(SWTypeExt::ExtData* pData)
{
	pData->SW_RadarEvent = false;

	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_PsychicRevealReady);

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::ParaDrop;
	pData->CursorType = int(MouseCursorType::PsychicReveal);
}

void SW_Reveal::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{ 
	pData->Get()->Action = (this->GetRange(pData).WidthOrRange < 0.0) ? Action::None : (Action)AresNewActionType::SuperWeaponAllowed;
}

int SW_Reveal::GetSound(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Sound.Get(RulesClass::Instance->PsychicRevealActivateSound);
}

SWRange SW_Reveal::GetRange(const SWTypeExt::ExtData* pData) const
{
	if (pData->SW_Range->empty())
	{
		// real default values, that is, force max cellspread range of 10
		auto const radius = std::min(RulesClass::Instance->PsychicRevealRadius, 10);
		return { radius };
	}
	return pData->SW_Range;
}