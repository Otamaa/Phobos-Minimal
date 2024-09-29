#pragma once

#include <vector>
#include <ControlClass.h>

class TacticalButtonClass;
class SWColumnClass : public ControlClass
{
public:
	SWColumnClass() = default;
	SWColumnClass(unsigned int id, int x, int y, int width, int height);

	virtual ~SWColumnClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Clicked(DWORD* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier) override;

	bool AddButton(int superIdx);
	bool RemoveButton(int superIdx);
	void ClearButtons(bool remove = true);

	constexpr void SetHeight(int height) {
		this->Rect.Height = height;
	}

	std::vector<TacticalButtonClass*> Buttons {};
	int MaxButtons { 0 };
};