#include "TiberiumEaterTypeClass.h"

#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

#include <TiberiumClass.h>

void TiberiumEaterTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);
	char tempBuffer[32];

	this->TransDelay.Read(exINI, pSection, "TiberiumEater.TransDelay");
	this->CashMultiplier.Read(exINI, pSection, "TiberiumEater.CashMultiplier");
	this->AmountPerCell.Read(exINI, pSection, "TiberiumEater.AmountPerCell");

	for (size_t idx = 0; ; ++idx)
	{
		Nullable<Point2D> cell {};
		IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "TiberiumEater.Cell%d", idx);
		cell.Read(exINI, pSection, tempBuffer);

		if (idx >= this->Cells.size())
		{
			if (!cell.isset())
				break;

			this->Cells.emplace_back(cell.Get().X * Unsorted::LeptonsPerCell, cell.Get().Y * Unsorted::LeptonsPerCell);
		}
		else
		{
			if (!cell.isset())
				continue;

			this->Cells[idx] = Point2D { cell.Get().X * Unsorted::LeptonsPerCell, cell.Get().Y * Unsorted::LeptonsPerCell };
		}
	}

	this->Display.Read(exINI, pSection, "TiberiumEater.Display");
	this->DisplayToHouse.Read(exINI, pSection, "TiberiumEater.DisplayToHouse");
	this->DisplayOffset.Read(exINI, pSection, "TiberiumEater.DisplayOffset");
	this->Anims.Read(exINI, pSection, "TiberiumEater.Anims");

	this->Anims_Tiberiums.resize(TiberiumClass::Array->size());
	for (size_t idx = 0; idx < TiberiumClass::Array->size(); ++idx) {
		IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "TiberiumEater.Anims.Tiberium%d", idx);
		this->Anims_Tiberiums[idx].Read(exINI, pSection, tempBuffer);
	}

	this->AnimMove.Read(exINI, pSection, "TiberiumEater.AnimMove");
}

template <class T>
bool TiberiumEaterTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->TransDelay)
		.Process(this->CashMultiplier)
		.Process(this->AmountPerCell)
		.Process(this->Cells)
		.Process(this->Display)
		.Process(this->DisplayToHouse)
		.Process(this->DisplayOffset)
		.Process(this->Anims)
		.Process(this->Anims_Tiberiums)
		.Process(this->AnimMove)
		.Success();
}

#pragma region(save/load)

bool TiberiumEaterTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool TiberiumEaterTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<TiberiumEaterTypeClass*>(this)->Serialize(stm);
}

#pragma endregion