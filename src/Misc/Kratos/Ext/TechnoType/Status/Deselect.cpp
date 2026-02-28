#include "../TechnoStatus.h"

#include <Misc/Kratos/Extension/WarheadTypeExt.h>

#include <Misc/Kratos/Ext/Helper/Scripts.h>

void TechnoStatus::OnUpdate_Deselect()
{
	_disableSelectable = Deselect->IsAlive();
	if (_disableSelectable)
	{
		Disappear = Deselect->Data.Disappear;
	}
	else
	{
		Disappear = false;
	}
	if (pTechno->IsSelected && _disableSelectable)
	{
		pTechno->Deselect();
	}
}

bool TechnoStatus::OnSelect_Deselect()
{
	return !_disableSelectable;
}
