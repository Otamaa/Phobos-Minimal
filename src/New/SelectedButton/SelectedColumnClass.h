#pragma once
#include <GadgetClass.h>
#include <string>
#include <vector>

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
	mutable std::vector<int> NameScroll_CumulativeWidths {};
	mutable int NameScroll_MaxOffset { 0 };
	mutable int NameScroll_StartTime { 0 }; // SystemTimer::GetTime() at selection - real wall-clock anchor, not a logic-tick count (CurrentFrame() ticks faster than real-time when uncapped FPS speeds up the sim, see FPS/timer code further down this file)
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