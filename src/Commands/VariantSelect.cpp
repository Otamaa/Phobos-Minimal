#include "VariantSelect.h"

#include <Utilities/GeneralUtils.h>
#include <WeaponTypeClass.h>
#include <TechnoTypeClass.h>
#include <TechnoClass.h>
#include <ObjectClass.h>

inline constexpr DWORD kDoubleClickMs = 500; // Ctrl+T double-press window: matches the native T key's 0x1F4 = 500 ms
inline constexpr int kMaxRefs = 64;
inline constexpr int kMaxGroups = 128;
inline constexpr int kSelectionScanLimit = 512;

struct Group
{
	TechnoTypeClass* type;
	WeaponTypeClass* weapon;
	int count;
};

struct VariantKey
{
	TechnoTypeClass* type;       // 0 = not a techno
	WeaponTypeClass* weapon;     // 0 for Gunner=no (plain type selection)
	int currentWeapon;  // -1 for Gunner=no
	int weaponCount;
	int passengers;
	bool gunner;
};

const char* VariantSelectCommandClass::GetName() const
{
	return "SelectSameVariant";
}

const wchar_t* VariantSelectCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_SELECT_VARIANT", L"Select Same Variant");
}

const wchar_t* VariantSelectCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* VariantSelectCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_SELECT_VARIANT_DESC",
		L"Select units with the same type and the same real weapon. Press twice quickly for the whole map.");
}

struct Scan
{
	std::vector<VariantKey> refs;
	int scanned;
	int selectable;
	int added;

	bool MatchesRef(TechnoTypeClass* type, WeaponTypeClass* weapon) const
	{
		for (const VariantKey& ref : refs)
		{
			if (ref.type == type && ref.weapon == weapon)
				return true;
		}
		return false;
	}

};

bool __fastcall Techno_Player_Controllable(TechnoClass* pTech)
{ JMP_FAST(0x732580); }


VariantKey KeyOf(TechnoClass* pTechno)
{
	VariantKey key = { 0, 0, -1, 0, 0, false };

	const auto type = pTechno->GetTechnoType();
	if (!type)
		return key;

	key.type = type;
	key.gunner = type->Gunner != 0;
	key.passengers = pTechno->Passengers.NumPassengers;
	if (!key.gunner)
		return key; // non-IFV logic: plain type selection

	key.weaponCount = type->WeaponCount;

	int current = pTechno->CurrentWeaponNumber;
	if (current < 0 || (key.weaponCount && current >= key.weaponCount))
		current = 0; // engine fallback semantics (0x70DC70): illegal value uses Weapon1

	key.currentWeapon = current;
	key.weapon = type->GetWeapon(current)->WeaponType;
	return key;
}

bool ConsiderCandidate(TechnoClass* obj)
{
	if (!obj)
		return false;

	if (!obj->IsAlive || obj->Health <= 0 || obj->InLimbo || obj->IsCrashing || obj->IsSinking)
		return false;

	return true;
}

void CollectRefs(Scan& scan, const DynamicVectorClass<ObjectClass*>* pSelection)
{
	for (int i = 0; i < pSelection->Count && i < kSelectionScanLimit; ++i)
	{
		auto pSelectedTechno = flag_cast_to<TechnoClass*>(pSelection->Items[i]);

		if (!ConsiderCandidate(pSelectedTechno))
			continue;

		const VariantKey key = KeyOf(pSelectedTechno);
		if (!key.type)
			continue;
		if (scan.MatchesRef(key.type, key.weapon))
			continue;

		scan.refs.push_back(key);
	}
}

bool ScanWholeMap(Scan& scan)
{
	//void** items = ReadAt<void**>(Engine::Addr::TechnoItems);
	//const int count = ReadAt<int>(Engine::Addr::TechnoCount);
	//if (!items || count <= 0)
	//	return false;
	//for (int i = 0; i < count; ++i)
	//	ConsiderCandidate(scan, items[i], false);
	return true;
}

bool ScanScreen(Scan& scan)
{
	//const Address tactical = ReadAt<Address>(Engine::Addr::TacticalInstance);
	//if (!tactical)
	//	return false;
	//const int count = ReadAt<int>(tactical + Engine::Off::TacticalCount);
	//for (int i = 0; i < count; ++i)
	//{
	//	void* obj = ReadAt<void*>(Engine::Addr::TacticalArray +
	//							  static_cast<Address>(i) * Engine::kTacticalEntrySize);
	//	ConsiderCandidate(scan, obj, true);
	//}
	return true;
}

void Run(bool wholeMap)
{

	if (!ObjectClass::CurrentObjects->Items || ObjectClass::CurrentObjects->Count <= 0)
	{
		Debug::Log("variant-select: selection empty, nothing to do\n");
		return;
	}

	Scan scan { };
	CollectRefs(scan, ObjectClass::CurrentObjects.operator->());

	if (scan.refs.size() <= 0) {
		Debug::Log("variant-select: no techno in selection\n");
		return;
	}

	const bool scanned = wholeMap ? ScanWholeMap(scan) : ScanScreen(scan);

	if (!scanned)
		return;

}

void VariantSelectCommandClass::Execute(WWKey dwUnk) const
{
	static DWORD lastPressTick = 0;

	const DWORD now = Unsorted::CurrentFrame();
	const bool wholeMap = lastPressTick != 0 && now - lastPressTick <= kDoubleClickMs;
	lastPressTick = now;

	Debug::Log(wholeMap ? "--- variant-select pass (MAP, double-press) ---\n"
							: "--- variant-select pass (SCREEN, single-press) ---\n");
	Run(wholeMap);
}