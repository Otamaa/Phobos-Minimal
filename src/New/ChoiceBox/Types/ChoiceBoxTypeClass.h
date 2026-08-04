#pragma once

#include <Utilities/Template.h>
#include <Utilities/Enumerable.h>

#include <Utilities/CSFText.h>

class PhobosStreamWriter;
class PhobosStreamReader;

enum ChoiceBoxButtonMode : int
{
	Normal = 0,	    /// 普通模式
	Bounce = 1, 	/// 回弹模式
};

enum ChoiceBoxButtonLayout : int
{
	Horizontal = 0, // 横向
	Vertical = 1, 	// 纵向
};

struct ChoiceBoxButton
{
	std::string Text;             ///< CSF 标签（INI: Button.TextN）

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;
};

class ChoiceBoxTypeClass final : public Enumerable<ChoiceBoxTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "ChoiceBoxTypes";
	static COMPILETIMEEVAL const char* ClassName = "ChoiceBoxTypeClass";

public:
	// ===== 文本内容 =====
	Valueable<CSFText> Title {};                    // 标题 CSF 标签
	Valueable<bool> Title_Center { false };                // 标题文本是否居中
	Valueable<CSFText> Description {};              // 描述 CSF 标签

	// ===== 按钮配置 =====
	Valueable<int> Button_Count { 0 };                 // 按钮数量（INI: Button.Count）
	Valueable<int> Button_Layout { 0 };                // 布局方向 0=横向 1=纵向（INI: Button.Layout=Horizontal/Vertical）
	Valueable<int> Button_Mode { 0 };                  // 按钮模式 0=普通 1=回弹（INI: Button.Mode）
	Valueable<int> Button_Width { 0 };                 // 按钮固定宽度，0=自动（INI: Button.Width）
	Valueable<int> Button_Height { 0 };                // 按钮固定高度，0=自动撑高（INI: Button.Height）
	ValueableVector<ChoiceBoxButton> Buttons {};    // 按钮文字列表（INI: Button.TextN）

	// ===== 外观参数 =====
	Valueable<int> MaxWidth { 250 };                     // 文本最大像素宽度，≤0 时默认 250（INI: MaxWidth）
	Valueable<int> BackgroundOpacity { 75 };            // 背景不透明度 0-100（INI: BackgroundOpacity）
	Valueable<int> ColorR { 255 };                       // 文字/边框颜色 R 分量（INI: Color=R,G,B）
	Valueable<int> ColorG { 215 };                       // 文字/边框颜色 G 分量
	Valueable<int> ColorB { 0 };                       // 文字/边框颜色 B 分量
	Valueable<int> Duration { -1 };                     // 自动移除帧数，-1=无限显示（INI: Duration）

	ChoiceBoxTypeClass(const char* const pTitle) : Enumerable(pTitle)
	{ }

	~ChoiceBoxTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& stm);
	void SaveToStream(PhobosStreamWriter& stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
