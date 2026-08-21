#include "UniqueTechnoColumnClass.h"
#include <MapClass.h>
#include <Unsorted.h>
#include <Ext/Rules/Body.h>

UniqueTechnoColumnClass UniqueTechnoColumnClass::Instance;

void UniqueTechnoColumnClass::InitClear()
{
	for (int i = 0; i < 8; ++i)
	{
		if (auto& pButton = this->Buttons[i])
		{
			GScreenClass::Instance->RemoveButton(pButton);
			GameDelete<true, false>(pButton);
			pButton = nullptr;
		}
	}

	this->Hovering = -1;
}

void UniqueTechnoColumnClass::InitIO()
{
	if (Unsorted::MAP_DEBUG_MODE())
		return;

	Point2D position { DSurface::Composite->Get_Width() - 65, 35 };

	for (int i = 0; i < 8; ++i)
	{
		const auto pButton = GameCreate<UniqueTechnoButtonClass>(i, position.X, position.Y);
		position.Y += 50;

		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->Buttons[i] = pButton;
	}
}

void UniqueTechnoColumnClass::SwitchVisible()
{
	this->Visible = !this->Visible;
	VocClass::PlayGlobal(FakeRulesClass::Instance->GUIMainButtonSound, Panning::Center, 1.0);

	for (int i = 0; i < 8; ++i)
	{
		if (const auto& pButton = this->Buttons[i])
			pButton->Disabled = !this->Visible;
	}
}

void UniqueTechnoColumnClass::Update()
{
	if (!this->Visible)
		return;

	for (int i = 0; i < 8; ++i)
	{
		if (const auto& pButton = this->Buttons[i])
			pButton->MarkRedraw();
	}
}
