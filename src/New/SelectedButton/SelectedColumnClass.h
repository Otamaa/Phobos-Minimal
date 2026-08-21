#pragma once
#include <GadgetClass.h>
#include <string>

class SelectedColumnClass : public GadgetClass
{
public:
	SelectedColumnClass() = default;
	SelectedColumnClass(int x, int y, int width, int height);

	~SelectedColumnClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;

	void DrawInfo() const;

private:

	mutable std::wstring NameScroll_Cache {};
	mutable std::wstring NameScroll_VisibleText {};
	mutable int NameScroll_CharOffset { 0 };
	mutable int NameScroll_MaxOffset { 0 };
	mutable int NameScroll_PauseFrames { 0 };
	mutable int NameScroll_LastGameFrame { -1 };
	mutable bool NameScroll_Reverse { false };
};

class SelectedBottomClass : public GadgetClass
{
public:
	SelectedBottomClass() = default;
	SelectedBottomClass(int x, int y, int width, int height);

	~SelectedBottomClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;

	void DrawInfo() const;
};