#pragma once

#include <Utilities/SavegameDef.h>

class PhobosGlobal
{
	static std::unique_ptr<PhobosGlobal> GlobalObject;
public:
	std::vector<unsigned char> ShpCompression1Buffer;
	ColorStruct maxColor;
	char BuildTimeDatas[0x720];
	bool DetonateDamageArea;

public:
	static bool SaveGlobals(PhobosStreamWriter& stm) { return PhobosGlobal::Instance()->Serialize(stm); }
	static bool LoadGlobals(PhobosStreamReader& stm) { return PhobosGlobal::Instance()->Serialize(stm); }

	inline static PhobosGlobal* Instance() {
		return GlobalObject.get();
	}

	PhobosGlobal() :
		ShpCompression1Buffer { }
		, maxColor { }
		, BuildTimeDatas { }
		, DetonateDamageArea { true }
	{ }

	~PhobosGlobal() = default;

	static void Clear();
	static void PointerGotInvalid(void* ptr, bool removed);

	static void Init() {
		GlobalObject = std::make_unique<PhobosGlobal>(); 
	}


	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(this->DetonateDamageArea)
			.Success();
	}
};