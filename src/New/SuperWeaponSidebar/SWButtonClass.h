#pragma once

#include <GadgetClass.h>

class SWButtonClass : public GadgetClass
{
public:
	static COMPILETIMEEVAL reference<GadgetClass*, 0x8B3E94> LastFocused {};

	SWButtonClass() = default;
	SWButtonClass(int superIdx, int x, int y, int width, int height);

	~SWButtonClass() {
		// The vanilla game did not consider adding/deleting buttons midway through the game,
		// so this behavior needs to be made known to the global variable and then remove it
		if (LastFocused == this) {
			this->OnMouseLeave();
		}
	}

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag fags, WWKey* pKey, KeyModifier modifier) override;

	COMPILETIMEEVAL void SetColumn(int column)
	{
		this->ColumnIndex = column;
	}

	bool LaunchSuper() const;

public:
	static COMPILETIMEEVAL int StartID = 2200;

	bool IsHovering { false };
	int ColumnIndex { -1 };
	int SuperIndex { -1 };
};