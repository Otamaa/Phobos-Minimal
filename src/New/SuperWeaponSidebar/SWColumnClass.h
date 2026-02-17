#pragma once

#include <vector>
#include "SWButtonClass.h"

class SWColumnClass : public GadgetClass
{
public:
	SWColumnClass() = default;
	SWColumnClass(int maxButtons, int x, int y, int width, int height);

	virtual ~SWColumnClass(){
		// The vanilla game did not consider adding/deleting buttons midway through the game,
		// so this behavior needs to be made known to the global variable and then remove it
		if (SWButtonClass::LastFocused == this)
		{
			this->OnMouseLeave();
		}
	}

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Clicked(WWKey* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier) override;

	bool AddButton(int superIdx);
	bool RemoveButton(int superIdx);
	void ClearButtons(bool remove = true);

	void SetHeight(int height);

	static constexpr int StartID = 2101;

public:
	std::vector<SWButtonClass*> Buttons {};
	int MaxButtons { 0 };
};