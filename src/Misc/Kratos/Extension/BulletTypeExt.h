#pragma once

#include <BulletTypeClass.h>

#include "TypeExtension.h"

class BulletTypeExt : public TypeExtension<BulletTypeClass, BulletTypeExt>
{
public:
	/// @brief 储存一些通用设置或者其他平台的设置
	class TypeData : public INIConfig
	{
	public:
		// Ares

		// Phobos
		bool AAOnly = false; // 只允许攻击空军

		virtual void Read(INIBufferReader* ini) override
		{
			AAOnly = ini->Get("AAOnly", AAOnly);
		}

	};

	static constexpr DWORD Canary = 0xF00DF00D;
	// static constexpr size_t ExtPointerOffset = 0x18;

	static BulletTypeExt::ExtContainer ExtMap;
};
