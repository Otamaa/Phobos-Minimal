#pragma once

#include <HouseClass.h>

#include "GOExtension.h"

class HouseExt : public GOExtension<HouseClass, HouseExt>
{
public:
	static constexpr DWORD Canary = 0x11111111;
	// static constexpr size_t ExtPointerOffset = 0x16098;

	/// @brief 创建Component实例，并加入到GameObject中.
	/// 创建时需要使用new，不能使用GameCreate.
	/// @param globalScripts 待附加的脚本列表
	/// @param ext ExtData
	static void AddGlobalScripts(std::list<std::string>& globalScripts, ExtData* ext);

	static HouseExt::ExtContainer ExtMap;
};
