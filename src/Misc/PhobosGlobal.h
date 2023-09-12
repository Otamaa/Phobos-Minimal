#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>

class AbstractClass;
class ObjectClass;
class AlphaShapeClass;
class PhobosGlobal
{
	static std::unique_ptr<PhobosGlobal> GlobalObject;
public:
	std::vector<unsigned char> ShpCompression1Buffer { };
	ColorStruct maxColor { };
	char BuildTimeDatas[0x720] { };
	bool DetonateDamageArea { true };
	PhobosMap<ObjectClass*, AlphaShapeClass*> ObjectLinkedAlphas { };
	int SpotHeight { 0 };

	std::vector<CellStruct> TempFoundationData1 { };
	std::vector<CellStruct> TempFoundationData2 { };
	std::vector<CellStruct> TempCoveredCellsData { };

public:
	static bool SaveGlobals(PhobosStreamWriter& stm) { return PhobosGlobal::Instance()->Serialize(stm); }
	static bool LoadGlobals(PhobosStreamReader& stm) { return PhobosGlobal::Instance()->Serialize(stm); }

	inline static PhobosGlobal* Instance() {
		return GlobalObject.get();
	}

	PhobosGlobal() = default;
	~PhobosGlobal() = default;

	static void Clear();
	static void PointerGotInvalid(AbstractClass* ptr, bool removed);

	static void Init() {
		GlobalObject = std::make_unique<PhobosGlobal>();
	}

	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(this->DetonateDamageArea)
			.Process(this->ObjectLinkedAlphas)
			.Process(this->SpotHeight)
			.Process(this->TempFoundationData1)
			.Process(this->TempFoundationData2)
			.Process(this->TempCoveredCellsData)
			.Success();
	}
};