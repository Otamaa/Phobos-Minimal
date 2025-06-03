#pragma once

#include <vector>
#include <ControlClass.h>

class SWButtonClass;
class SWColumnClass : public ControlClass
{
public:
	SWColumnClass() = default;
	SWColumnClass(unsigned int id, int maxButtons, int x, int y, int width, int height);

	virtual ~SWColumnClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Clicked(DWORD* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier) override;

	bool AddButton(int superIdx);
	bool RemoveButton(int superIdx);
	void ClearButtons(bool remove = true);

	void SetHeight(int height);

	std::vector<SWButtonClass*> Buttons {};
	int MaxButtons { 0 };
};