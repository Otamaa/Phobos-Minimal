#pragma once

#include <GadgetClass.h>

#include <string>

class MessageScrollClass;
class MessageButtonClass;
class MessageToggleClass;
class MessageLabelClass;
class MessageColumnClass
{
public:
	static MessageColumnClass Instance;
	static constexpr int TextReservedSpace = 8;
	static constexpr int HighOpacity = 90;
	static constexpr int MediumOpacity = 60;
	static constexpr int LowOpacity = 30;

public:
	MessageColumnClass() = default;
	~MessageColumnClass() { this->Initialize(); }

	void InitClear();
	void InitIO();

private:
	void Initialize(int x = 0, int y = 0, int maxCount = 0, int maxRecord = 0, int maxChars = 0, int width = 640);

public:
	MessageLabelClass* AddMessage(const wchar_t* name, const wchar_t* message, int timeout, bool silent, int delay = 0);

	void MouseEnter(bool block = false);
	void MouseLeave(bool block = false);
	bool CanScrollUp();
	bool CanScrollDown();
	void ScrollUp();
	void ScrollDown();
	void SetScroll(int index = 0);
	void Expand();
	void PackUp(bool clear = false);

private:
	void CleanUp();
	void Refresh();
	void Update();

public:
	void Toggle();
	void Manage();
	void DrawAll();

	int GetWidth() const { return this->Width; }
	size_t GetTextColor() const { return (this->Color.B << 16) | (this->Color.G << 8) | this->Color.R; }
	ColorStruct GetColor() const { return this->Color; }
	int GetScrollIndex() const { return this->ScrollIndex; }
	bool IsHovering() const { return this->Hovering; }
	bool IsExpanded() const { return this->Expanded; }
	bool IsDrawing() const { return this->Drawing; }
	bool IsBlocked() const { return (this->Expanded || this->Blocked) && this->Hovering; }

	static int GetSystemTime();

	static inline bool IsStickyButton(const GadgetClass* pButton)
	{

		return pButton == GadgetClass::StickyButton();;
	}

	static inline void IncreaseBrightness(ColorStruct& color, int level = 1)
	{
		color.R = static_cast<BYTE>(255 - ((255 - color.R) >> level));
		color.G = static_cast<BYTE>(255 - ((255 - color.G) >> level));
		color.B = static_cast<BYTE>(255 - ((255 - color.B) >> level));
	}

	static inline void DecreaseBrightness(ColorStruct& color, int level = 1)
	{
		color.R = static_cast<BYTE>(color.R >> level);
		color.G = static_cast<BYTE>(color.G >> level);
		color.B = static_cast<BYTE>(color.B >> level);
	}

	bool GetThumbDimension(int* pMax, int* pHeight, int* pPosY = nullptr) const;

private:
	static bool AddRecordString(const std::wstring& message, size_t copySize = std::wstring::npos);

	void RemoveTextLabel(MessageLabelClass* pLabel);
	int GetLabelCount() const;
	MessageLabelClass* GetLastLabel() const;
	int GetMaxScroll() const;

private:

	MessageLabelClass* LabelList { nullptr };
	Point2D LabelsPos { Point2D::Empty };

	int MaxCount { 0 };
	int MaxRecord { 0 };
	int MaxChars { 0 };
	int Height { 0 };
	int Width { 0 };
	ColorStruct Color { ColorStruct { 0, 0, 0 } };

	MessageToggleClass* Button_Toggle { nullptr };
	MessageButtonClass* Button_Up { nullptr };
	MessageButtonClass* Button_Down { nullptr };
	MessageScrollClass* Scroll_Bar { nullptr };
	MessageScrollClass* Scroll_Board { nullptr };

	int ScrollIndex { 0 };
	bool Hovering { false };
	bool Expanded { false };
	bool Drawing { false };
	bool Blocked { false };
};