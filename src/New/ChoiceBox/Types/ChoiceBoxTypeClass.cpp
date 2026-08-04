#include "ChoiceBoxTypeClass.h"

#include <Phobos.h>
#include <CCINIClass.h>

#include <Utilities/INIParser.h>
#include <Utilities/Stream.h>
#include <Utilities/TemplateDef.h>

#include <cstdio>

Enumerable<ChoiceBoxTypeClass>::container_t Enumerable<ChoiceBoxTypeClass>::Array;

// ========== ChoiceBoxButton 序列化 ==========
bool ChoiceBoxButton::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm.Process(this->Text, RegisterForChange).Success();
}

bool ChoiceBoxButton::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(const_cast<std::string&>(this->Text)).Success();
}

// ========== INI 加载 ==========
void ChoiceBoxTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	this->Title.Read(exINI, section, "Title");
	this->Title_Center.Read(exINI, section, "Title.Center");
	this->Description.Read(exINI, section, "Description");
	this->MaxWidth.Read(exINI, section, "MaxWidth");
	if (this->MaxWidth <= 0)
		this->MaxWidth = 250;
	this->BackgroundOpacity.Read(exINI, section, "BackgroundOpacity");
	this->Duration.Read(exINI, section, "Duration");

	// Button.Count
	this->Button_Count.Read(exINI, section, "Button.Count");

	// Button.Layout - 枚举字符串 Horizontal/Vertical
	if (pINI->ReadString(section, "Button.Layout", "", Phobos::readBuffer))
	{
		const char* layoutStr = Phobos::readBuffer;
		if (_stricmp(layoutStr, "Vertical") == 0)
		{
			this->Button_Layout = ChoiceBoxButtonLayout::Vertical;
		}
		else if (_stricmp(layoutStr, "Horizontal") == 0)
		{
			this->Button_Layout = ChoiceBoxButtonLayout::Horizontal;
		}
		else
			this->Button_Layout.Read(exINI, section, "Button.Layout");
	}

	// Button.Mode - 枚举字符串 Normal/Bounce
	if (pINI->ReadString(section, "Button.Mode", "", Phobos::readBuffer))
	{
		const char* modeStr = Phobos::readBuffer;
		if (_stricmp(modeStr, "Bounce") == 0)
		{
			this->Button_Mode = ChoiceBoxButtonMode::Bounce;
		}
		else if (_stricmp(modeStr, "Normal") == 0)
		{
			this->Button_Mode = ChoiceBoxButtonMode::Normal;
		}
		else
			this->Button_Mode.Read(exINI, section, "Button.Mode");
	}

	// Button.Width - 固定宽度（0=自动）
	this->Button_Width.Read(exINI, section, "Button.Width");

	// Button.Height - 固定高度（0=自动撑高）
	this->Button_Height.Read(exINI, section, "Button.Height");

	// 逐个读取 Button.Text1~N
	this->Buttons.clear();
	for (int i = 1; i <= this->Button_Count; ++i)
	{
		char key[32];
		std::sprintf(key, "Button.Text%d", i);

		ChoiceBoxButton btn;

		if (pINI->ReadString(section, key, "", Phobos::readBuffer))
		{
			btn.Text = Phobos::readBuffer;
		}

		this->Buttons.push_back(btn);
	}

	// Color 格式：Color=255,215,0  （RGB 逗号分隔）
	if (pINI->ReadString(section, "Color", "", Phobos::readBuffer))
	{
		const char* pColor = Phobos::readBuffer;
		int r = 255, g = 215, b = 0;
		if (std::sscanf(pColor, "%d,%d,%d", &r, &g, &b) >= 3)
		{
			this->ColorR = std::clamp(r, 0, 255);
			this->ColorG = std::clamp(g, 0, 255);
			this->ColorB = std::clamp(b, 0, 255);
		}
	}
}

// ========== 序列化模板 ==========
template <typename T>
void ChoiceBoxTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Title)
		.Process(this->Title_Center)
		.Process(this->Description)
		.Process(this->Button_Count)
		.Process(this->Button_Layout)
		.Process(this->Button_Mode)
		.Process(this->Button_Width)
		.Process(this->Button_Height)
		.Process(this->Buttons)
		.Process(this->MaxWidth)
		.Process(this->BackgroundOpacity)
		.Process(this->ColorR)
		.Process(this->ColorG)
		.Process(this->ColorB)
		.Process(this->Duration)
		;
}

void ChoiceBoxTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void ChoiceBoxTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
