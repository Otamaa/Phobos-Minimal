#include "MapTextBoxClass.h"

#include <New/TextBox/Entities/Derived/TechnoTextBoxClass.h>
#include "../Derived/WaypointTextBoxClass.h"

#include <StringTable.h>
#include <Surface.h>
#include <Drawing.h>
#include <TacticalClass.h>
#include <WWMouseClass.h>

#include <Utilities/Stream.h>
#include <Utilities/Debug.h>

#include <cstring>
#include <algorithm>
#include <memory>

// ========== 文本框布局常量 ==========
constexpr static const int PADDINGX = 6;
constexpr static const int PADDINGY = 4;
constexpr static const int LINE_HEIGHT = 18;
constexpr static const int BOTTOM_SAFE_HEIGHT = 0;

// ============================================================================
// 全局数组 - 单一所有者
//
// DIFF: was vector<shared_ptr<MapTextBoxClass>> plus one shared_ptr array per
// derived class. shared_ptr existed only because each object sat in two
// arrays; nothing used weak_ptr, aliasing, custom deleters or threads.
// ============================================================================
PhobosOwnedArray<MapTextBoxClass> MapTextBoxClass::Array;

static std::vector<std::wstring> WrapText(const wchar_t* text, int maxWidth)
{
	if (!text || wcslen(text) == 0 || maxWidth <= 0)
		return {};

	std::vector<std::wstring> lines;      // 最终换行结果
	std::wstring wStr(text);              // 源文本副本
	std::wstring currentLine;             // 当前正在累积的行

	for (size_t i = 0; i < wStr.length(); ++i)
	{
		wchar_t ch = wStr[i];

		// 处理显式换行符 \n 或 \r\n
		if (ch == L'\n' || ch == L'\r')
		{
			if (!currentLine.empty())
			{
				lines.push_back(currentLine);
				currentLine.clear();
			}
			if (ch == L'\r' && i + 1 < wStr.length() && wStr[i + 1] == L'\n')
				++i; // 跳过 \r\n 中的 \n
			continue;
		}

		// 过滤掉控制字符（保留制表符 \t）
		if (ch < 0x20 && ch != L'\t')
			continue;

		// 试探性：将当前字符拼接到行尾，测量总宽度
		std::wstring testLine = currentLine + ch;
		RectangleStruct dims = Drawing::GetTextBox(testLine.c_str(), 0, 0 , 0);

		if (dims.Width > maxWidth)
		{
			// 超过最大宽度，触发换行
			if (currentLine.empty())
			{
				// 单个字符就已经超宽，强制放入该行
				lines.push_back(testLine);
			}
			else
			{
				// 将当前行存入结果，用当前字符另起新行
				lines.push_back(currentLine);
				currentLine = ch;
			}
		}
		else
		{
			// 未超宽，继续累加
			currentLine = testLine;
		}
	}

	// 处理最后剩余的一行
	if (!currentLine.empty())
		lines.push_back(currentLine);

	return lines;
}

// ========== 构造 / 析构 ==========

MapTextBoxClass::~MapTextBoxClass() = default;

MapTextBoxClass::MapTextBoxClass(const char* csfLabel,
	int maxWidth, int opacityPercent,
	int colorR, int colorG, int colorB)
	: CurrentLabel(csfLabel ? csfLabel : "")
	, MaxLineWidth(maxWidth > 0 ? maxWidth : 250)
	, BackgroundOpacity(std::clamp(opacityPercent, 0, 100))
	, ColorR(colorR)
	, ColorG(colorG)
	, ColorB(colorB)
{
	// DIFF: no `m_cache(std::make_unique<Cache>())` - the cache is a value
	// member now and its NSDMIs already leave it dirty and empty.
}

// ========== 布局更新 ==========

void MapTextBoxClass::UpdateLayout()
{
	// DIFF: the `if (!m_cache) m_cache = make_unique<Cache>()` guard is gone.

	// 优先从 CSF 文件中获取本地化文本
	const wchar_t* textPtr = StringTable::TryFetchStringOrReturnDefault(this->CurrentLabel.c_str());

	// Fallback: CSF 标签不存在时，直接将 ANSI 标签名逐字节转成宽字符
	std::wstring fallbackText;

	if (!textPtr || wcslen(textPtr) == 0)
	{
		if (this->CurrentLabel.empty())
		{
			m_cache.CachedLines.clear();
			m_cache.CachedBgWidth = 0;
			m_cache.CachedBgHeight = 0;
			m_cache.IsLayoutDirty = false;
			return;
		}

		// 逐字节转换（标签名通常是 ASCII）
		fallbackText.reserve(this->CurrentLabel.length());

		for (char ch : this->CurrentLabel)
			fallbackText += static_cast<wchar_t>(static_cast<unsigned char>(ch));

		textPtr = fallbackText.c_str();
	}

	// 再次检查文本是否有效
	if (!textPtr || wcslen(textPtr) == 0)
	{
		m_cache.CachedLines.clear();
		m_cache.CachedBgWidth = 0;
		m_cache.CachedBgHeight = 0;
		m_cache.IsLayoutDirty = false;
		return;
	}

	// 执行自动换行
	m_cache.CachedLines = WrapText(textPtr, this->MaxLineWidth);

	if (m_cache.CachedLines.empty())
	{
		m_cache.CachedBgWidth = 0;
		m_cache.CachedBgHeight = 0;
		m_cache.IsLayoutDirty = false;
		return;
	}

	// 计算最长行的像素宽度
	int maxLineW = 0;

	for (const std::wstring& line : m_cache.CachedLines)
	{
		RectangleStruct dims = Drawing::GetTextBox(line.c_str(), 0, 0, 0);

		if (dims.Width > maxLineW)
			maxLineW = dims.Width;
	}

	// 背景框尺寸 = 文字区域 + 内边距
	m_cache.CachedBgWidth = maxLineW + (PADDINGX * 2);
	m_cache.CachedBgHeight = (static_cast<int>(m_cache.CachedLines.size()) * LINE_HEIGHT) + (PADDINGY * 2);
	m_cache.IsLayoutDirty = false;
}

// ========== 缓存管理 ==========

void MapTextBoxClass::ResetCache()
{
	// DIFF: was two make_unique calls. Value assignment restores every NSDMI,
	// including IsLayoutDirty = true.
	m_cache = {};
}

// ============================================================================
// 生存判定 / 绘制判定
//
// See the header for why these must stay separate. Base defaults: always alive,
// always drawable.
// ============================================================================

bool MapTextBoxClass::IsAlive() const
{
	return true;
}

bool MapTextBoxClass::CanDraw() const
{
	return true;
}
// ========== 绘制 ==========

void MapTextBoxClass::DrawAt(Point2D centerPos)
{
	// 检查渲染表面和战术视图是否可用
	if (!DSurface::Composite() || !TacticalClass::Instance())
		return;

	// 初始化缓存并进行延迟布局更新
	if (m_cache.IsLayoutDirty)
		UpdateLayout();

	// 无有效内容则不绘制
	if (m_cache.CachedLines.empty() || m_cache.CachedBgWidth <= 0 || m_cache.CachedBgHeight <= 0)
		return;

	int bgWidth = m_cache.CachedBgWidth;
	int bgHeight = m_cache.CachedBgHeight;

	// 背景框左上角（居中定位）
	Point2D topLeft = {
		centerPos.X - (bgWidth / 2),
		centerPos.Y - (bgHeight / 2)
	};

	// ===== 底部裁剪 =====
	// 文本框不应超出屏幕底部（避免被界面栏遮挡）
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
			return; // 裁剪后连一行都放不下，直接跳过
	}

	// 完全在屏幕左侧或上方，跳过
	if (topLeft.X + bgWidth < 0 || topLeft.Y + drawHeight < 0)
		return;

	RectangleStruct bgRect = { topLeft.X, topLeft.Y, bgWidth, drawHeight };

	// ===== 鼠标悬停检测 =====
	// 鼠标移到文本框上时暂停绘制，让玩家能点选被遮挡的单位
	RectangleStruct fullBgRect = { topLeft.X, topLeft.Y, bgWidth, bgHeight };
	Point2D mousePos = WWMouseClass::Instance->XY1;
	if (mousePos.X >= fullBgRect.X &&
		mousePos.X <= fullBgRect.X + fullBgRect.Width &&
		mousePos.Y >= fullBgRect.Y &&
		mousePos.Y <= fullBgRect.Y + fullBgRect.Height)
	{
		return;
	}

	// ===== 绘制半透明黑色背景 =====
	ColorStruct bgColor = { 0, 0, 0 };
	DSurface::Composite->Fill_Rect_Trans(&bgRect, &bgColor, this->BackgroundOpacity);

	// 将 RGB 分量转为游戏引擎所需的颜色整数值
	int colorInt = ColorStruct(
		static_cast<unsigned char>(this->ColorR),
		static_cast<unsigned char>(this->ColorG),
		static_cast<unsigned char>(this->ColorB)).ToInit();
	Point2D p1, p2;

	// ===== 绘制边框（上、左、右、下） =====
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

	// 底边（被裁剪时跳过，避免画出界）
	if (!isClipped)
	{
		p1 = { topLeft.X, topLeft.Y + bgHeight - 1 };
		p2 = { topLeft.X + bgWidth - 1, topLeft.Y + bgHeight - 1 };
		DSurface::Composite->Draw_Line(p1, p2, colorInt);
	}

	// ===== 逐行绘制文字 =====
	RectangleStruct bounds = DSurface::ViewBounds;
	int currentY = topLeft.Y + PADDINGY;
	for (const std::wstring& line : m_cache.CachedLines)
	{
		// 当前行超出裁剪区域则停止
		if (currentY + LINE_HEIGHT > topLeft.Y + drawHeight + PADDINGY)
			break;

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
}

// ============================================================================
// 序列化
//
// DIFF: base fields are written once, by the base. The derived classes used to
// re-write CurrentLabel, MaxLineWidth, BackgroundOpacity, the three colour
// components and RemainingFrames themselves without chaining here, so the base
// Serialize was effectively dead for derived instances. They now chain and
// write only their own fields.
//
// SAVEGAME BREAK: layout changes (TypeIndex added, field order normalised).
// Accepted per dev-branch policy.
// ============================================================================

template <typename T>
bool MapTextBoxClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->CurrentLabel)
		.Process(this->MaxLineWidth)
		.Process(this->BackgroundOpacity)
		.Process(this->ColorR)
		.Process(this->ColorG)
		.Process(this->ColorB)
		.Process(this->RemainingFrames)
		.Process(this->TypeIndex)       // ADD:
		.Success();
}

bool MapTextBoxClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	if (!Serialize(Stm))
		return false;

	// Layout is derived state - recompute rather than persist it.
	this->ResetCache();

	return true;
}

bool MapTextBoxClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<MapTextBoxClass*>(this)->Serialize(Stm);
}

// ============================================================================
// 全局管理
// ============================================================================

void MapTextBoxClass::Clear()
{
	// DIFF: was three clear() calls that HAD to run derived-then-base.
	Array.Clear();
}

void MapTextBoxClass::ClearAll()
{
	// BUGFIX: ClearAll() used to be `Array.clear()` on the base array ONLY,
	// while Clear() cleared all three. The two were documented as synonyms.
	// Every object survived in its derived array - invisible, because nothing
	// drew it, but still matched by Find()/FindOrCreate().
	Clear();
}

// ============================================================================
// 全局绘制入口
//
// DIFF: the original made three passes - decrement-and-collect, then a removal
// loop doing a strcmp dispatch plus a nested find_if into the derived array and
// another find into the base array (O(n*m) to delete a handful of labels), then
// a separate full pass in CleanupDeadLabels().
//
// It is now one pass. RemoveIf visits each element exactly once, in order, so
// the counter decrement and the draw call are safe inside the predicate.
//
// Re-entrancy is the real constraint, not the mutation: DrawAt() must never
// create or destroy a label, or the container would be mutated mid-compaction.
// Today it only renders.
// ============================================================================

void MapTextBoxClass::DrawAll()
{
	Array.RemoveIf([](MapTextBoxClass& label)
	{
		// Sole owner - a null entry would mean someone bypassed Create().
		// Permanent invalidity first: this is the CleanupDeadLabels() pass,
		// folded in. Reads gamestate only, so every machine agrees.
		if (!label.IsAlive())
			return true;

		// 倒计时递减
		if (label.RemainingFrames >= 0)
		{
			--label.RemainingFrames;

			if (label.RemainingFrames <= 0)
			{
				// DIFF: the original pushed to `expired` and then FELL THROUGH
				// to draw one final frame. This drops that frame. Invisible in
				// practice, but it is a deviation.
				return true;
			}
		}

		// Transient visibility - may read local state; machines may disagree.
		if (!label.CanDraw())
			return false;

		Point2D pos {};

		if (!label.GetDrawPosition(pos))
			return false;

		label.DrawAt(pos);

		return false;
	});

	// DIFF: TechnoTextBoxClass::CleanupDeadLabels() is gone - its work is the
	// IsAlive() check above, and it no longer costs a second full pass.
}

// ============================================================================
// 全局存档/读档
// ============================================================================

bool MapTextBoxClass::SaveGlobals(PhobosStreamWriter& Stm)
{
	Stm.Save(Array.Size());

	for (auto const& pItem : Array)
	{
		// DIFF: the old-pointer placeholder is gone. It was written and then
		// read back into a discarded `oldPtr` - half of Phobos' swizzle
		// pattern, with the RegisterChange half missing. Nothing referenced
		// these objects by pointer, so it was 4 dead bytes per entry.
		std::string marker(pItem->GetTypeMarker());
		Stm.Save(marker);
		pItem->Save(Stm);
	}

	return true;
}

bool MapTextBoxClass::LoadGlobals(PhobosStreamReader& Stm)
{
	Clear();

	size_t Count = 0;

	if (!Stm.Load(Count))
		return false;

	// BUGFIX: was `Array.reserve(Count)` on an unvalidated stream value - a
	// corrupt or truncated savegame could request an enormous allocation.
	constexpr size_t MaxReasonableCount = 4096;

	if (Count > MaxReasonableCount)
	{
		Debug::Log("[MapTextBoxClass] Refusing implausible label count %zu from stream!\n", Count);
		return false;
	}

	Array.Reserve(Count);

	for (size_t i = 0; i < Count; ++i)
	{
		std::string typeMarker {};

		if (!Stm.Load(typeMarker))
			return false;

		// The marker string stays the on-disk discriminator; Kind is runtime.
		std::unique_ptr<MapTextBoxClass> newObj;

		if (std::strcmp(typeMarker.data(), "WaypointTextBoxClass") == 0)
			newObj = std::make_unique<WaypointTextBoxClass>();
		else if (std::strcmp(typeMarker.data(), "TechnoTextBoxClass") == 0)
			newObj = std::make_unique<TechnoTextBoxClass>();
		else
		{
			Debug::Log("[MapTextBoxClass] Warning: unknown type marker \"%s\"!\n",
				typeMarker.data());
			return false;
		}

		if (!newObj->Load(Stm, false))
			return false;

		// DIFF: one insertion instead of two. If Load fails, the unique_ptr
		// destroys the object on the way out - the old code could leave a
		// half-loaded object stranded in the derived array.
		Array.Adopt(std::move(newObj));
	}

	return true;
}