#pragma once

class InfantryTypeClass;
class UnitTypeClass;
class AircraftTypeClass;
struct TechnoImageReplacer
{
	static void Replace(InfantryTypeClass* pType);
	static void Replace(UnitTypeClass* pType);
	static void Replace(AircraftTypeClass* pType);
};
