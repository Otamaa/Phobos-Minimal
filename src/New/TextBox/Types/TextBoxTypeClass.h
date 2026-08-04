#pragma once

#include <Utilities/Template.h>
#include <Utilities/Enumerable.h>

class PhobosStreamWriter;
class PhobosStreamReader;

class TextBoxTypeClass final : public Enumerable<TextBoxTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "TextBoxTypes";
	static COMPILETIMEEVAL const char* ClassName = "TextBoxTypeClass";

public:
	// ===== 样式参数 =====
	Valueable<int> MaxWidth { 250 };            // 单行最大像素宽度 (1-1000)，文字超出后自动换行
	Valueable<int> BackgroundOpacity { 75 };   // 背景不透明度 (0-100)，0=全透明，100=纯黑
	Valueable<int> ColorR { 250 };      // 文字/边框颜色 — R 分量
	Valueable<int> ColorG { 250 };      // 文字/边框颜色 — G 分量
	Valueable<int> ColorB { 0 };        // 文字/边框颜色 — B 分量
	Valueable<int> Duration { -1 };     // 自动移除帧数，-1=无限显示，需手动清除

	TextBoxTypeClass(const char* const pTitle) : Enumerable(pTitle)	{ }
	~TextBoxTypeClass() = default;

	// ===== 加载/保存 =====
	void LoadFromINI(CCINIClass* pINI);                         // 从 INI 读取
	void LoadFromStream(PhobosStreamReader& stm);               // 从存档流加载
	void SaveToStream(PhobosStreamWriter& stm);                 // 保存到存档流

private:
	template <typename T>
	void Serialize(T& Stm);
};
