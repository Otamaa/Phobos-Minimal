#include "MapChoiceBoxClass.h"

#include <New/ChoiceBox/Types/ChoiceBoxTypeClass.h>
#include <New/ChoiceBox/Entities/Derived/WaypointChoiceBoxClass.h>
#include <New/ChoiceBox/Entities/Derived/ScreenChoiceBoxClass.h>

#include <Ext/Event/Body.h>

#include <StringTable.h>
#include <Surface.h>
#include <Drawing.h>
#include <TacticalClass.h>
#include <WWMouseClass.h>
#include <Unsorted.h>

#include <Utilities/Stream.h>
#include <Utilities/Debug.h>

#include <cstring>
#include <algorithm>

#include <Windows.h>
#include <cstdio>

// ========== 布局常量 ==========
constexpr static const int PADDINGX = 8;                // 背景框水平内边距
constexpr static const int PADDINGY = 8;                // 背景框垂直内边距
constexpr static const int LINE_HEIGHT = 18;            // 每行文字高度
constexpr static const int BUTTON_PADDINGX = 11;        // 按钮内文字水平内边距
constexpr static const int BUTTON_PADDINGY = 4;         // 按钮内文字垂直内边距
constexpr static const int BUTTON_SPACING = 10;         // 按钮之间的间距
constexpr static const int SECTION_SPACING = 4;         // 标题/描述/按钮区块之间的间距
constexpr static const int BOTTOM_SAFE_HEIGHT = 0;      // 底部安全区域（保留）

// DIFF: CLICK_EXPIRE_FRAMES moved to the header as a public constant so the
// ChoiceBoxClick event handler can reference it. Value unchanged (5).

// ========== 按钮布局元数据 ==========
struct MapChoiceBoxClass::BtnLayoutItem
{
	int Index;                               // 按钮索引
	std::wstring Text;                       // 解析后的宽字符串文本
	std::vector<std::wstring> TextLines;     // 按钮内换行后的各行文字
	int Width;                               // 按钮宽度（固定或自动）
	int Height;                              // 按钮高度（固定或自动撑高）
};

// ============================================================================
// 全局数组 - 单一所有者
//
// DIFF: was
//     std::vector<std::shared_ptr<MapChoiceBoxClass>> MapChoiceBoxClass::Array;
// plus a second shared_ptr array per derived class. shared_ptr was only ever
// there because each object sat in two arrays; nothing used weak_ptr, aliasing,
// custom deleters or cross-thread ownership.
// ============================================================================
PhobosOwnedArray<MapChoiceBoxClass> MapChoiceBoxClass::Array;

// ============================================================================
// 每帧鼠标边沿检测
//
// BUGFIX: the original kept `static bool s_prevLeftDown` inside
// CheckMouseClick(), which meant it was shared by every box instance. The first
// box in iteration order consumed the rising edge and set the flag, so any
// second box drawn in the same frame could never be clicked.
//
// Edge detection is a per-frame property of the mouse, not of a box, so it is
// computed once in DrawAll() and merely read by each box.
//
// VERIFY: GetAsyncKeyState is now desync-safe - it feeds only an event, never
// gamestate - but it still reads OS keyboard state directly, so clicks register
// while the game is paused or alt-tabbed. Routing this through the game's own
// input pipeline remains a separate, worthwhile fix.
// ============================================================================
static bool s_prevLeftDown = false;
static bool s_leftJustPressed = false;

static void UpdateMouseEdge()
{
	const bool currentLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	s_leftJustPressed = currentLeftDown && !s_prevLeftDown;
	s_prevLeftDown = currentLeftDown;
}

// ========== 构造 / 析构 ==========

MapChoiceBoxClass::~MapChoiceBoxClass() = default;

MapChoiceBoxClass::MapChoiceBoxClass(int id, const char* label, const ChoiceBoxTypeClass* pType)
	: ID(id)
	, Label(label ? label : "")
	, Type(pType)
{
	if (pType)
	{
		// 记录类型索引，用于序列化后重建指针
		for (size_t i = 0; i < ChoiceBoxTypeClass::Array.size(); ++i)
		{
			if (ChoiceBoxTypeClass::Array[i].get() == pType)
			{
				this->TypeIndex = static_cast<int>(i);
				break;
			}
		}

		if (pType->Duration >= 0)
		{
			this->RemainingFrames = pType->Duration;

			// ADD: absolute deadline for network answer arbitration.
			// Deterministic - creation runs from a lockstep TAction, so every
			// machine computes the same frame number.
			this->DeadlineFrame = Unsorted::CurrentFrame.get() + pType->Duration;
		}
	}
}

static std::wstring GetCSFText(const char* csfLabel)
{
	if (!csfLabel || csfLabel[0] == '\0')
		return L"";

	const wchar_t* textPtr = StringTable::TryFetchStringOrReturnDefault(csfLabel);

	if (textPtr && wcslen(textPtr) > 0)
		return textPtr;

	// Fallback: 直接输出 CSF 标签原文（如 "MSG:Attack"）
	std::wstring fallback;
	fallback.reserve(std::strlen(csfLabel));
	for (const char* p = csfLabel; *p; ++p)
		fallback += static_cast<wchar_t>(static_cast<unsigned char>(*p));
	return fallback;
}

// ========== 自动换行（与 TextBox 相同算法） ==========
static std::vector<std::wstring> WrapText(const wchar_t* text, int maxWidth)
{
	if (!text || wcslen(text) == 0 || maxWidth <= 0)
		return {};

	std::vector<std::wstring> lines;
	std::wstring wStr(text);
	std::wstring currentLine;

	for (size_t i = 0; i < wStr.length(); ++i)
	{
		wchar_t ch = wStr[i];

		if (ch == L'\n' || ch == L'\r')
		{
			if (!currentLine.empty())
			{
				lines.push_back(currentLine);
				currentLine.clear();
			}
			if (ch == L'\r' && i + 1 < wStr.length() && wStr[i + 1] == L'\n')
				++i;
			continue;
		}

		if (ch < 0x20 && ch != L'\t')
			continue;

		std::wstring testLine = currentLine + ch;
		RectangleStruct dims = Drawing::GetTextBox(testLine.c_str(), 0, 0 , 0);

		if (dims.Width > maxWidth)
		{
			if (currentLine.empty())
			{
				lines.push_back(testLine);
			}
			else
			{
				lines.push_back(currentLine);
				currentLine = ch;
			}
		}
		else
		{
			currentLine = testLine;
		}
	}

	if (!currentLine.empty())
		lines.push_back(currentLine);

	return lines;
}

// ========== 交互 ==========

void MapChoiceBoxClass::ResetChoice()
{
	this->ClickedIndex = -1;

	// ADD: arbitration state belongs to one answering round. Bounce mode rearms
	// the box, so both must clear or the second round is silently unanswerable.
	this->AnsweredBy = -1;
	this->IsAwaitingLocalResponse = false;
}

// Pure local hit-test. Writes nothing; safe to call on any machine.
bool MapChoiceBoxClass::HitTestButtons(int& outIndex) const
{
	const auto pMouse = WWMouseClass::Instance();

	if (!pMouse)
		return false;

	const Point2D& mousePos = pMouse->XY1;

	for (const auto& btn : this->m_buttonRects)
	{
		if (mousePos.X >= btn.Rect.X &&
			mousePos.X <= btn.Rect.X + btn.Rect.Width &&
			mousePos.Y >= btn.Rect.Y &&
			mousePos.Y <= btn.Rect.Y + btn.Rect.Height)
		{
			outIndex = btn.Index;
			return true;
		}
	}

	return false;
}

// ============================================================================
// DIFF: replaces CheckMouseClick().
//
// The original wrote ClickedIndex straight from local input, which made the
// answer local state that gamestate (TEvent 557/558) then branched on - every
// machine could reach a different conclusion. This raises a network event
// instead; ClickedIndex is written only by EventExt::ChoiceBoxClick::Respond,
// which executes on the same frame on every machine.
//
// Returns nothing: there is no longer an answer to report synchronously. The
// answer arrives later, via the event.
// ============================================================================
void MapChoiceBoxClass::PollLocalInput()
{
	// Already answered this round, or an answer is already in flight from this
	// machine. IsAwaitingLocalResponse is local debounce only - without it the
	// player would queue one event per frame during the command delay.
	if (this->AnsweredBy >= 0 || this->IsAwaitingLocalResponse)
		return;

	if (this->ClickedIndex >= 0)
		return;

	if (!s_leftJustPressed)
		return;

	int hitIndex = -1;

	if (!this->HitTestButtons(hitIndex))
		return;

	EventExt::ChoiceBoxClick::Raise(this->ID, hitIndex);
	this->IsAwaitingLocalResponse = true;
}

// ========== 绘制 ==========
void MapChoiceBoxClass::DrawAt(Point2D centerPos)
{
	if (!DSurface::Composite() || !TacticalClass::Instance())
		return;

	if (!this->Type)
		return;

	const auto& type = *this->Type;

	// 验证 Type 指针是否仍在 Array 中（防止悬空指针）
	{
		bool typeValid = false;
		for (auto& tp : ChoiceBoxTypeClass::Array)
		{
			if (tp.get() == this->Type)
			{
				typeValid = true;
				break;
			}
		}
		if (!typeValid)
		{
			this->Type = nullptr;
			return;
		}
	}

	// 计算各区块内容
	const auto& csfTitle = type.Title.Get();
	const auto& csfDesc = type.Description.Get();
	bool hasTitle = !csfTitle.empty();
	bool hasDesc = !csfDesc.empty();
	std::wstring titleText = hasTitle ? std::wstring(csfTitle) : std::wstring();
	std::wstring descText = hasDesc ? std::wstring(csfDesc) : std::wstring();
	int btnCount = type.Button_Count;

	if (btnCount <= 0)
		return;

	// 测量文本尺寸（支持自动换行）
	int maxW = type.MaxWidth;
	bool useWrapping = (maxW > 0);

	std::vector<std::wstring> titleLines, descLines;
	int titleWidth = 0, titleHeight = 0;
	if (hasTitle)
	{
		if (useWrapping)
		{
			titleLines = WrapText(titleText.c_str(), maxW);
			if (titleLines.empty()) { hasTitle = false; titleText.clear(); }
			else
			{
				titleWidth = maxW;
				titleHeight = static_cast<int>(titleLines.size()) * LINE_HEIGHT;
			}
		}
		else
		{
			titleLines = { titleText };
			RectangleStruct dims = Drawing::GetTextBox(titleText.c_str(),  0, 0 , 0);
			titleWidth = dims.Width;
			titleHeight = LINE_HEIGHT;
		}
	}

	int descWidth = 0, descHeight = 0;
	if (hasDesc)
	{
		if (useWrapping)
		{
			descLines = WrapText(descText.c_str(), maxW);
			if (descLines.empty()) { hasDesc = false; descText.clear(); }
			else
			{
				descWidth = maxW;
				descHeight = static_cast<int>(descLines.size()) * LINE_HEIGHT;
			}
		}
		else
		{
			descLines = { descText };
			RectangleStruct dims = Drawing::GetTextBox(descText.c_str(), 0, 0 , 0);
			descWidth = dims.Width;
			descHeight = LINE_HEIGHT;
		}
	}

	// ===== 测量按钮尺寸 =====
	int btnFixedW = type.Button_Width;   // >0 固定宽度（文字自动换行适配）
	int btnFixedH = type.Button_Height;  // >0 固定高度（文字超出截断）
	bool isVertical = (type.Button_Layout != 0);

	// 存储每个按钮的元数据
	std::vector<BtnLayoutItem> btnItems;
	btnItems.reserve(btnCount);

	// 刷新 maxW/useWrapping（用于后续背景框宽度计算）
	maxW = type.MaxWidth;
	useWrapping = (maxW > 0);

	for (int i = 0; i < btnCount; ++i)
	{
		BtnLayoutItem item;
		item.Index = i;

		// 获取按钮文字（CSF 解析，空时回退到 "BtnN"）
		std::wstring wtext = GetCSFText(
			(i < static_cast<int>(type.Buttons.size())) ? type.Buttons[i].Text.c_str() : "");
		if (wtext.empty())
		{
			char fallback[32];
			std::sprintf(fallback, "Btn%d", i);
			for (const char* p = fallback; *p; ++p)
				wtext += static_cast<wchar_t>(static_cast<unsigned char>(*p));
		}
		item.Text = wtext;

		// 按钮宽度：固定宽度或自动取文本宽度
		if (btnFixedW > 0)
		{
			item.Width = btnFixedW;
		}
		else
		{
			RectangleStruct dims = Drawing::GetTextBox(wtext.c_str(), 0, 0 , 0);
			item.Width = dims.Width + BUTTON_PADDINGX * 2;
		}

		// 按钮内文本自动换行
		int textMaxW = item.Width - BUTTON_PADDINGX * 2;
		if (textMaxW > 0)
		{
			item.TextLines = WrapText(wtext.c_str(), textMaxW);
		}
		else
		{
			item.TextLines = { wtext };
		}
		if (item.TextLines.empty())
			item.TextLines = { wtext };

		// 按钮高度：固定高度或按文本行数自动撑高
		if (btnFixedH > 0)
		{
			item.Height = btnFixedH;
		}
		else
		{
			item.Height = static_cast<int>(item.TextLines.size()) * LINE_HEIGHT
				+ BUTTON_PADDINGY * 2;
		}

		btnItems.push_back(std::move(item));
	}

	// ===== 执行按钮布局 =====
	struct BtnPos { int X, Y, W, H; };
	std::vector<BtnPos> btnPositions;
	btnPositions.reserve(btnCount);

	// 根据布局方向决定背景框宽度
	int btnContentMaxW = 0;
	int btnTotalH = 0;

	if (isVertical)
	{
		// 纵向：所有按钮居中排成一列
		int maxBtnW = 0;
		for (auto& item : btnItems)
			if (item.Width > maxBtnW) maxBtnW = item.Width;
		btnContentMaxW = maxBtnW;

		for (int i = 0; i < btnCount; ++i)
		{
			btnPositions.push_back({ 0, btnTotalH, btnItems[i].Width, btnItems[i].Height });
			btnTotalH += btnItems[i].Height;
			if (i < btnCount - 1)
				btnTotalH += BUTTON_SPACING;
		}
	}
	else
	{
		// 横向：先确定 bgWidth，再折行布局
		// 先计算单行总宽度（假设不折行），同时填充 btnPositions
		int curX = 0, curY = 0, rowH = 0;
		for (int i = 0; i < btnCount; ++i)
		{
			const auto& item = btnItems[i];
			btnPositions.push_back({ curX, curY, item.Width, item.Height });
			curX += item.Width + BUTTON_SPACING;
			if (item.Height > rowH) rowH = item.Height;
		}
		btnTotalH = rowH;
		if (curX > 0)
			btnContentMaxW = curX - BUTTON_SPACING;
	}

	// ===== 确定背景框尺寸 =====
	int maxTextWidth = 0;
	{
		int titleW = 0, descW = 0;
		// 取标题/描述/按钮的最大宽度（估算）
		if (hasTitle) titleW = titleWidth;
		if (hasDesc) descW = descWidth;
		maxTextWidth = std::max({ btnContentMaxW, titleW, descW });
	}

	int bgWidth;
	if (useWrapping)
	{
		bgWidth = maxW + PADDINGX * 2;
	}
	else
	{
		bgWidth = maxTextWidth + PADDINGX * 2;
	}

	// 横向模式下，如果按钮总宽度超过背景框，重新布局折行
	if (!isVertical && btnContentMaxW > bgWidth - PADDINGX * 2)
	{
		// 用背景框宽度重新折行
		btnPositions.clear();
		int availW = bgWidth - PADDINGX * 2;
		int curX = 0, curY = 0, rowH = 0;
		int newMaxW = 0;

		for (int i = 0; i < btnCount; ++i)
		{
			const auto& item = btnItems[i];
			int itemW = (item.Width > availW) ? availW : item.Width;

			if (curX + itemW > availW && curX > 0)
			{
				if (curX > newMaxW) newMaxW = curX;
				curX = 0;
				curY += rowH + BUTTON_SPACING;
				rowH = 0;
			}
			btnPositions.push_back({ curX, curY, itemW, item.Height });
			curX += itemW + BUTTON_SPACING;
			if (item.Height > rowH) rowH = item.Height;
		}
		if (curX > 0)
		{
			if (curX - BUTTON_SPACING > newMaxW)
				newMaxW = curX - BUTTON_SPACING;
			curY += rowH;
		}
		else
		{
			curY += rowH;
		}
		btnContentMaxW = newMaxW;
		btnTotalH = curY;
		// 若背景框是自动宽度则更新
		if (!useWrapping)
			bgWidth = std::max(btnContentMaxW, maxTextWidth) + PADDINGX * 2;
	}

	int bgHeight = PADDINGY * 2;
	if (hasTitle)
		bgHeight += titleHeight + SECTION_SPACING;
	if (hasDesc)
		bgHeight += descHeight + SECTION_SPACING;
	bgHeight += btnTotalH;

	int buttonsStartY = PADDINGY;
	if (hasTitle)
		buttonsStartY += titleHeight + SECTION_SPACING;
	if (hasDesc)
		buttonsStartY += descHeight + SECTION_SPACING;

	// 背景框左上角（居中定位）
	Point2D topLeft = {
		centerPos.X - (bgWidth / 2),
		centerPos.Y - (bgHeight / 2)
	};

	// ===== 屏幕钳制（仅 ScreenChoiceBoxClass） =====
	if (this->ClampToScreen())
	{
		int viewW = DSurface::ViewBounds->Width;
		int viewH = DSurface::ViewBounds->Height;
		constexpr int CLAMP_MARGIN = 4;

		if (topLeft.X < CLAMP_MARGIN)
			topLeft.X = CLAMP_MARGIN;
		if (topLeft.Y < CLAMP_MARGIN)
			topLeft.Y = CLAMP_MARGIN;
		if (topLeft.X + bgWidth > viewW - CLAMP_MARGIN)
			topLeft.X = viewW - CLAMP_MARGIN - bgWidth;
		if (topLeft.Y + bgHeight > viewH - CLAMP_MARGIN)
			topLeft.Y = viewH - CLAMP_MARGIN - bgHeight;
	}

	// ===== 底部裁剪 =====
	int viewHeight = DSurface::ViewBounds->Height;
	int clipBottomY = viewHeight - BOTTOM_SAFE_HEIGHT;

	if (topLeft.Y >= clipBottomY)
		return;

	int drawHeight = bgHeight;
	bool isClipped = false;
	if (topLeft.Y + bgHeight > clipBottomY)
	{
		drawHeight = clipBottomY - topLeft.Y;
		isClipped = true;
		if (drawHeight < LINE_HEIGHT)
			return;
	}

	if (topLeft.X + bgWidth < 0 || topLeft.Y + drawHeight < 0)
		return;

	RectangleStruct bgRect = { topLeft.X, topLeft.Y, bgWidth, drawHeight };

	// ===== 获取鼠标位置（用于按钮悬停高亮） =====
	Point2D mousePos = WWMouseClass::Instance->XY1;

	// ===== 绘制半透明黑色背景 =====
	ColorStruct bgColor = { 0, 0, 0 };
	DSurface::Composite->Fill_Rect_Trans(&bgRect, &bgColor, type.BackgroundOpacity);

	// 颜色整数值
	int colorInt = ColorStruct(
		static_cast<BYTE>(type.ColorR),
		static_cast<BYTE>(type.ColorG),
		static_cast<BYTE>(type.ColorB)).ToInit();

	Point2D p1, p2;

	// ===== 绘制边框 =====
	// 上边
	p1 = { topLeft.X, topLeft.Y };
	p2 = { topLeft.X + bgWidth - 1, topLeft.Y };
	DSurface::Composite->Draw_Line(p1, p2, colorInt);

	// 左边
	p1 = { topLeft.X, topLeft.Y };
	p2 = { topLeft.X, topLeft.Y + drawHeight - 1 };
	DSurface::Composite->Draw_Line(p1, p2, colorInt);

	// 右边
	p1 = { topLeft.X + bgWidth - 1, topLeft.Y };
	p2 = { topLeft.X + bgWidth - 1, topLeft.Y + drawHeight - 1 };
	DSurface::Composite->Draw_Line(p1, p2, colorInt);

	// 底边（裁剪时跳过）
	if (!isClipped)
	{
		p1 = { topLeft.X, topLeft.Y + bgHeight - 1 };
		p2 = { topLeft.X + bgWidth - 1, topLeft.Y + bgHeight - 1 };
		DSurface::Composite->Draw_Line(p1, p2, colorInt);
	}

	RectangleStruct bounds = DSurface::ViewBounds;

	// ===== 绘制标题（支持多行，可选居中） =====
	int currentY = topLeft.Y + PADDINGY;
	if (hasTitle)
	{
		for (const std::wstring& line : titleLines)
		{
			int textX = topLeft.X + PADDINGX;
			if (type.Title_Center)
			{
				RectangleStruct lineDims = Drawing::GetTextBox(line.c_str(), 0, 0, 0);
				int lineW = (lineDims.Width > 0) ? lineDims.Width : titleWidth;
				textX = topLeft.X + (bgWidth / 2) - (lineW / 2);
			}
			Point2D textPos = { textX, currentY };
			DSurface::Composite->DSurfaceDrawText(
				line.c_str(),
				&bounds,
				&textPos,
				colorInt,
				0,
				TextPrintType::Metal12 | TextPrintType::BrightColor);
			currentY += LINE_HEIGHT;
		}
		currentY += SECTION_SPACING;
	}

	// ===== 绘制描述（支持多行） =====
	if (hasDesc)
	{
		for (const auto& line : descLines)
		{
			Point2D textPos = { topLeft.X + PADDINGX, currentY };
			DSurface::Composite->DSurfaceDrawText(
				line.c_str(),
				&bounds,
				&textPos,
				colorInt,
				0,
				TextPrintType::Metal12 | TextPrintType::BrightColor);
			currentY += LINE_HEIGHT;
		}
		currentY += SECTION_SPACING;
	}

	// ===== 更新按钮区域缓存 =====
	const_cast<MapChoiceBoxClass*>(this)->UpdateButtonRects(
		topLeft, bgWidth, topLeft.Y + buttonsStartY, btnItems);

	// ===== 绘制按钮（多行布局） =====
	for (int i = 0; i < btnCount; ++i)
	{
		const auto& pos = btnPositions[i];

		// 计算按钮在屏幕上的实际位置（横向居中）
		int btnX, btnY;
		if (isVertical)
		{
			btnX = topLeft.X + (bgWidth - pos.W) / 2;
			btnY = topLeft.Y + buttonsStartY + pos.Y;
		}
		else
		{
			btnX = topLeft.X + (bgWidth - btnContentMaxW) / 2 + pos.X;
			btnY = topLeft.Y + buttonsStartY + pos.Y;
		}

		// 底部裁剪：完全在裁剪区外则跳过
		if (btnY >= topLeft.Y + drawHeight)
			continue;

		// 按钮可见高度（裁剪超出部分）
		int btnDrawH = pos.H;
		bool btnClipped = false;
		if (btnY + btnDrawH > topLeft.Y + drawHeight)
		{
			btnDrawH = topLeft.Y + drawHeight - btnY;
			btnClipped = true;
			if (btnDrawH <= 0)
				continue;
		}

		// 检测鼠标是否悬停（部分可见按钮仍可悬停高亮）
		bool isHover = (mousePos.X >= btnX && mousePos.X <= btnX + pos.W &&
			mousePos.Y >= btnY && mousePos.Y <= btnY + pos.H);

		// 检测是否已被选中
		bool isChosen = (this->ClickedIndex == i);

		// 按钮背景色（黑色半透明，悬停时偏白）
		ColorStruct btnColor = isHover ? ColorStruct { 255, 255, 255 } : ColorStruct { 0, 0, 0 };
		int btnOpacity = isHover
			? std::clamp(type.BackgroundOpacity * 1 / 4, 0, 100)
			: std::clamp(type.BackgroundOpacity / 2, 0, 100);

		RectangleStruct btnRect = { btnX, btnY, pos.W, btnDrawH };
		DSurface::Composite->Fill_Rect_Trans(&btnRect, &btnColor, btnOpacity);

		// 按钮边框 + 文字颜色（悬停/选中时加亮）
		int btnHighlightColor = colorInt;
		if (isChosen)
		{
			btnHighlightColor = ColorStruct(255, 0, 0).ToInit();
		}
		else if (isHover)
		{
			ColorStruct brightColor = {
				static_cast<BYTE>(std::min(255, type.ColorR + 100)),
				static_cast<BYTE>(std::min(255, type.ColorG + 100)),
				static_cast<BYTE>(std::min(255, type.ColorB + 100))
			};
			btnHighlightColor = brightColor.ToInit();
		}

		// 四条边框（底部边框在裁剪时跳过）
		p1 = { btnX, btnY };
		p2 = { btnX + pos.W - 1, btnY };
		DSurface::Composite->Draw_Line(p1, p2, btnHighlightColor);
		p1 = { btnX, btnY };
		p2 = { btnX, btnY + btnDrawH - 1 };
		DSurface::Composite->Draw_Line(p1, p2, btnHighlightColor);
		p1 = { btnX + pos.W - 1, btnY };
		p2 = { btnX + pos.W - 1, btnY + btnDrawH - 1 };
		DSurface::Composite->Draw_Line(p1, p2, btnHighlightColor);
		if (!btnClipped)
		{
			p1 = { btnX, btnY + pos.H - 1 };
			p2 = { btnX + pos.W - 1, btnY + pos.H - 1 };
			DSurface::Composite->Draw_Line(p1, p2, btnHighlightColor);
		}

		// 按钮文字：在按钮内部居中，支持多行/截断（悬停时颜色随之加亮）
		const auto& item = btnItems[i];
		int textAreaH = pos.H - BUTTON_PADDINGY * 2;
		int textAreaW = pos.W - BUTTON_PADDINGX * 2;

		if (textAreaH > 0 && textAreaW > 0)
		{
			int totalTextH = static_cast<int>(item.TextLines.size()) * LINE_HEIGHT;
			int lineOffsetY = (textAreaH - totalTextH) / 2;
			if (lineOffsetY < 0) lineOffsetY = 0;

			for (size_t li = 0; li < item.TextLines.size(); ++li)
			{
				int lineY = btnY + BUTTON_PADDINGY + lineOffsetY + static_cast<int>(li) * LINE_HEIGHT;

				if (btnFixedH > 0 && lineY + LINE_HEIGHT > btnY + pos.H)
					break;

				// 当前行超出裁剪区则停止
				if (lineY + LINE_HEIGHT > topLeft.Y + drawHeight)
					break;

				RectangleStruct txtDims = Drawing::GetTextBox(
					item.TextLines[li].c_str(), 0, 0, 0);

				Point2D textPos = {
					btnX + (pos.W - txtDims.Width) / 2,
					lineY
				};
				DSurface::Composite->DSurfaceDrawText(
					item.TextLines[li].c_str(),
					&bounds,
					&textPos,
					btnHighlightColor,
					0,
					TextPrintType::Metal12 | TextPrintType::BrightColor);
			}
		}
	}
}

void MapChoiceBoxClass::UpdateButtonRects(Point2D topLeft, int bgWidth, int buttonsStartY,
	const std::vector<BtnLayoutItem>& btnItems)
{
	this->m_buttonRects.clear();

	bool isVertical = (this->Type) ? (this->Type->Button_Layout != 0) : false;
	int btnCount = static_cast<int>(btnItems.size());

	if (isVertical)
	{
		for (int i = 0; i < btnCount; ++i)
		{
			const auto& item = btnItems[i];
			int btnX = topLeft.X + (bgWidth - item.Width) / 2;
			int btnY = buttonsStartY;
			for (int j = 0; j < i; ++j)
				btnY += btnItems[j].Height + BUTTON_SPACING;
			this->m_buttonRects.push_back({ i, { btnX, btnY, item.Width, item.Height } });
		}
	}
	else
	{
		// 横向多行：重新走一遍与 DrawAt 相同的 layout
		int availW = bgWidth - PADDINGX * 2;
		int curX = 0, curY = 0, rowH = 0;
		int maxRowW = 0;

		// 先计算 btnContentMaxW（与 DrawAt 保持一致）
		for (int i = 0; i < btnCount; ++i)
		{
			const auto& item = btnItems[i];
			int itemW = (item.Width > availW) ? availW : item.Width;

			if (curX + itemW > availW && curX > 0)
			{
				if (curX > maxRowW) maxRowW = curX;
				curX = 0;
				curY += rowH + BUTTON_SPACING;
				rowH = 0;
			}
			if (item.Height > rowH) rowH = item.Height;
			curX += itemW + BUTTON_SPACING;
		}
		if (curX > 0)
		{
			if (curX - BUTTON_SPACING > maxRowW)
				maxRowW = curX - BUTTON_SPACING;
		}
		int btnContentMaxW = maxRowW;

		// 再次遍历生成实际 Rects
		curX = 0; curY = 0; rowH = 0;
		for (int i = 0; i < btnCount; ++i)
		{
			const auto& item = btnItems[i];
			int itemW = (item.Width > availW) ? availW : item.Width;

			if (curX + itemW > availW && curX > 0)
			{
				curX = 0;
				curY += rowH + BUTTON_SPACING;
				rowH = 0;
			}
			int btnX = topLeft.X + (bgWidth - btnContentMaxW) / 2 + curX;
			int btnY = buttonsStartY + curY;
			this->m_buttonRects.push_back({ i, { btnX, btnY, itemW, item.Height } });
			curX += itemW + BUTTON_SPACING;
			if (item.Height > rowH) rowH = item.Height;
		}
	}
}

// ============================================================================
// 序列化
//
// DIFF: the base fields are now written ONCE.
//
// Previously WaypointChoiceBoxClass::Load called MapChoiceBoxClass::Load (which
// wrote ID, Label, TypeIndex, ClickedIndex, ClickedConsumed, RemainingFrames,
// ClickExpireCounter, IsExpired) and then ran its own Serialize, which wrote
// ID, Label, ClickedIndex, RemainingFrames and IsExpired a second time. It
// round-tripped because it was symmetric, but the second write silently
// overwrote the first and cost ~24 bytes per box.
//
// Derived Serialize() now handles derived fields only, and derived Load/Save
// chain to the base for the shared ones.
//
// SAVEGAME BREAK: on-disk layout changes. Accepted per dev-branch policy.
// ============================================================================

template <typename T>
bool MapChoiceBoxClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->ID)
		.Process(this->Label)
		.Process(this->TypeIndex)
		.Process(this->ClickedIndex)
		.Process(this->ClickedConsumed)
		.Process(this->RemainingFrames)
		.Process(this->ClickExpireCounter)
		.Process(this->IsExpired)
		.Process(this->DeadlineFrame)      // ADD:
		.Process(this->AnsweredBy)         // ADD:
		.Process(this->ExpiredAtFrame)     // ADD:
		// NOT serialized: IsAwaitingLocalResponse. It is local presentation
		// state; persisting it would leave a reloaded box permanently unable to
		// accept input on the machine that saved mid-flight.
		.Success();
}

bool MapChoiceBoxClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	if (!Serialize(Stm))
		return false;

	// 从 TypeIndex 重建 Type 指针
	//
	// VERIFY: TypeIndex is an index into ChoiceBoxTypeClass::Array, which is
	// rebuilt from INI. Stable within a session; if the mod's ChoiceBoxTypes
	// list is reordered between save and load, this silently resolves to a
	// different type.
	if (this->TypeIndex >= 0
		&& static_cast<size_t>(this->TypeIndex) < ChoiceBoxTypeClass::Array.size())
	{
		this->Type = ChoiceBoxTypeClass::Array[this->TypeIndex].get();
	}
	else
	{
		this->Type = nullptr;
	}

	// Local state never comes off the stream.
	this->IsAwaitingLocalResponse = false;

	return true;
}

bool MapChoiceBoxClass::Save(PhobosStreamWriter& Stm) const
{
	// const_cast is the established Phobos pattern: one Serialize template
	// serves both directions and cannot be const.
	return const_cast<MapChoiceBoxClass*>(this)->Serialize(Stm);
}

// ============================================================================
// 全局管理
// ============================================================================

void MapChoiceBoxClass::Clear()
{
	// DIFF: was three clear() calls that HAD to run in derived-then-base order.
	// With one owner there is no order to get wrong.
	Array.Clear();
}

void MapChoiceBoxClass::ClearAll()
{
	Clear();
}

void MapChoiceBoxClass::RemoveByID(int id)
{
	if (id < 0)
		return;

	// DIFF: replaces paired WaypointChoiceBoxClass::RemoveByID(id) +
	// ScreenChoiceBoxClass::RemoveByID(id) calls, each of which internally did
	// its own two-array erase.
	Array.RemoveIf([id](MapChoiceBoxClass& box) { return box.ID == id; });
}

MapChoiceBoxClass* MapChoiceBoxClass::FindByID(int id)
{
	if (id < 0)
		return nullptr;

	return Array.Find([id](MapChoiceBoxClass& box) { return box.ID == id; });
}

// ============================================================================
// 每帧阶段处理
//
// DIFF: the original ran all three phases once per derived array, via
// DrawChoiceBoxList<T>. Phases two and three only touch per-box counters and
// each box lived in exactly one array, so they are now run once over the whole
// array instead of once per kind. Drawing stays split by kind to preserve
// z-order (waypoint boxes under screen boxes).
// ============================================================================

// 阶段一：绘制并轮询本地点击
template <typename TDerived>
static void DrawChoiceBoxesOfKind()
{
	MapChoiceBoxClass::Array.ForEachOf<TDerived>([](TDerived& box)
	{
		if (box.IsExpired || !box.CanDraw())
			return;

		// 隐藏期：已点击且 Duration 刚好耗尽 → 不绘制，保留对象供 TEvent 检测
		if (box.ClickedIndex >= 0 && box.RemainingFrames == 0)
			return;

		Point2D drawPos;

		if (!box.GetDrawPosition(drawPos))
			return;

		box.DrawAt(drawPos);

		// DIFF: was `if (ptr->CheckMouseClick()) { ... }`, which set
		// ClickedIndex locally and then immediately cleared ClickedConsumed and
		// armed the bounce counter. Those two now happen in the event handler,
		// on the frame the answer resolves, on every machine.
		box.PollLocalInput();
	});
}

// 阶段二：处理隐藏期倒计时（含回弹模式）
static void TickHiddenPeriod()
{
	MapChoiceBoxClass::Array.ForEach([](MapChoiceBoxClass& box)
	{
		if (box.IsExpired || box.ClickExpireCounter < 0)
			return;

		if (--box.ClickExpireCounter > 0)
			return;

		// 检查被点击的按钮是否为回弹模式
		const bool isBounce = (box.ClickedIndex >= 0 && box.Type
			&& box.Type->Button_Mode == static_cast<int>(ChoiceBoxButtonMode::Bounce));

		// 未消费则继续等待（给 TEvent 更多时间）
		//
		// NOTE: this makes CLICK_EXPIRE_FRAMES a MINIMUM, not a timeout - the
		// box waits indefinitely until a trigger consumes the answer. A box
		// whose 557/558 trigger never fires therefore lives until Clear().
		if (!box.ClickedConsumed)
		{
			box.ClickExpireCounter = 0;
			return;
		}

		if (isBounce)
		{
			// 回弹：重置点击状态，不清除对象
			box.ResetChoice();          // DIFF: also clears AnsweredBy and
			box.ClickExpireCounter = -1; // IsAwaitingLocalResponse, so the next
			box.ClickedConsumed = false; // round can be answered again.
		}
		else
		{
			box.IsExpired = true;
			box.ExpiredAtFrame = Unsorted::CurrentFrame.get();
		}
	});
}

// 阶段三：处理 Duration 自动移除
static void TickDuration()
{
	const int currentFrame = Unsorted::CurrentFrame.get();
	const int grace = EventExt::ChoiceBoxClick::NetworkGrace();

	MapChoiceBoxClass::Array.ForEach([currentFrame, grace](MapChoiceBoxClass& box)
	{
		if (box.IsExpired || box.RemainingFrames < 0)
			return;

		--box.RemainingFrames;

		if (box.RemainingFrames > 0)
			return;

		// ====================================================================
		// DIFF: the timeout flip is delayed past the network grace window.
		//
		// TEvent 559 returns pBox->IsExpired and has NO consumption latch, so
		// it re-fires on every evaluation. If IsExpired flipped on the deadline
		// frame and a late-but-legal answer then cleared it, 559 may already
		// have fired. Delaying the flip removes the race instead of trying to
		// arbitrate it after the fact.
		//
		// NetworkGrace() is 0 in singleplayer, so campaign behaviour is
		// identical to before: the flip happens on exactly the same frame.
		// ====================================================================
		if (box.DeadlineFrame >= 0 && currentFrame <= box.DeadlineFrame + grace)
		{
			// Hold at 0 - visually finished, still accepting in-flight answers.
			box.RemainingFrames = 0;
			return;
		}

		// Duration 耗尽且已点击 → 不销毁，启动隐藏期供 TEvent 检测
		if (box.ClickedIndex >= 0)
		{
			box.RemainingFrames = 0;

			if (box.ClickExpireCounter < 0)
				box.ClickExpireCounter = MapChoiceBoxClass::CLICK_EXPIRE_FRAMES;
		}
		else
		{
			box.IsExpired = true;
			box.ClickedIndex = -2;
			box.ExpiredAtFrame = currentFrame;
		}
	});
}

// 阶段四：过期实例回收（可选）
//
// EXTENSION: disabled by default (POST_EXPIRY_LIFETIME == -1), which reproduces
// existing behaviour - expired boxes are never swept and live until Clear().
//
// Sweeping is NOT safe to enable blindly: TEvent 559 polls IsExpired on the box
// itself, so once FindByID returns null the timeout trigger can never fire.
// Any value here must exceed the worst-case gap between the expiry frame and
// trigger evaluation in your mission set.
static void SweepExpired()
{
	if COMPILETIMEEVAL(MapChoiceBoxClass::POST_EXPIRY_LIFETIME < 0)
		return;

	const int currentFrame = Unsorted::CurrentFrame.get();

	MapChoiceBoxClass::Array.RemoveIf([currentFrame](MapChoiceBoxClass& box)
	{
		if (!box.IsExpired || box.ExpiredAtFrame < 0)
			return false;

		return currentFrame > box.ExpiredAtFrame + MapChoiceBoxClass::POST_EXPIRY_LIFETIME;
	});
}

void MapChoiceBoxClass::DrawAll()
{
	// BUGFIX: mouse edge computed ONCE per frame, before any box polls it.
	// Previously each box ran its own edge detection against a shared static,
	// so only the first box drawn could ever register a click.
	UpdateMouseEdge();

	DrawWaypoint();
	DrawScreen();

	TickHiddenPeriod();
	TickDuration();
	SweepExpired();
}

void MapChoiceBoxClass::DrawWaypoint()
{
	DrawChoiceBoxesOfKind<WaypointChoiceBoxClass>();
}

void MapChoiceBoxClass::DrawScreen()
{
	DrawChoiceBoxesOfKind<ScreenChoiceBoxClass>();
}

bool MapChoiceBoxClass::CanDraw() const
{
	return true;
}

bool MapChoiceBoxClass::ClampToScreen() const
{
	return false;
}

// ============================================================================
// 全局存档/读档
// ============================================================================

bool MapChoiceBoxClass::SaveGlobals(PhobosStreamWriter& Stm)
{
	Stm.Save(Array.Size());

	for (auto const& pItem : Array)
	{
		// DIFF: the old-pointer placeholder is gone. It was written and then
		// loaded into a discarded `oldPtr` - half of Phobos' swizzle pattern,
		// with the RegisterChange half missing. Nothing referenced these
		// objects by pointer, so it was 4 dead bytes per entry.
		//
		// Cross-references use ID and TypeIndex, which is the better design and
		// is what the trigger system already relies on.
		std::string marker(pItem->GetTypeMarker());
		Stm.Save(marker);
		pItem->Save(Stm);
	}

	return true;
}

bool MapChoiceBoxClass::LoadGlobals(PhobosStreamReader& Stm)
{
	Clear();

	size_t Count = 0;

	if (!Stm.Load(Count))
		return false;

	// BUGFIX: was `Array.reserve(Count)` on an unvalidated stream value. A
	// corrupt or truncated savegame could request an enormous allocation.
	constexpr size_t MaxReasonableCount = 4096;

	if (Count > MaxReasonableCount)
	{
		Debug::Log("[MapChoiceBoxClass] Refusing implausible box count %zu from stream!\n", Count);
		return false;
	}

	Array.Reserve(Count);

	for (size_t i = 0; i < Count; ++i)
	{
		std::string typeMarker {};

		if (!Stm.Load(typeMarker))
			return false;

		// The marker string stays the on-disk discriminator so the save format
		// is unchanged; Kind is only used at runtime.
		std::unique_ptr<MapChoiceBoxClass> newObj;

		if (std::strcmp(typeMarker.data(), "WaypointChoiceBoxClass") == 0)
			newObj = std::make_unique<WaypointChoiceBoxClass>();
		else if (std::strcmp(typeMarker.data(), "ScreenChoiceBoxClass") == 0)
			newObj = std::make_unique<ScreenChoiceBoxClass>();
		else
		{
			Debug::Log("[MapChoiceBoxClass] Warning: unknown type marker \"%s\"!\n",
				typeMarker.data());
			return false;
		}

		if (!newObj->Load(Stm, false))
			return false;

		// DIFF: one insertion instead of two. If Load fails above, the
		// unique_ptr destroys the object on the way out - the old code could
		// leave a half-loaded object in the derived array.
		Array.Adopt(std::move(newObj));
	}

	return true;
}