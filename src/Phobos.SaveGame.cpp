#include "Phobos.SaveGame.h"

#include <iomanip>

#include <Utilities/Debug.h>
#include <Misc/Ares/Hooks/Header.h>

#include <Ext/Unit/Body.h>
#include <Ext/UnitType/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/OverlayType/Body.h>
#include <Ext/Parasite/Body.h>
#include <Ext/Bomb/Body.h>
#include <Ext/Temporal/Body.h>
#include <Ext/Particle/Body.h>
#include <Ext/ParticleSystem/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/ParticleSystemType/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/SmudgeType/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/Wave/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/TEvent/Body.h>

#define JSON_SERIALIZE_FIELD(json, ext, field) \
    json[#field] = ext->field

#define JSON_SERIALIZE_FIELD_IF(json, ext, field, condition) \
    if (condition) json[#field] = ext->field

#define JSON_SERIALIZE_PTR_ID(json, ext, field) \
    json[#field] = ext->field ? ext->field->ID : ""

#define JSON_SERIALIZE_COORD(json, name, coord) \
    json[name] = {coord.X, coord.Y, coord.Z}

#define JSON_DESERIALIZE_FIELD(json, ext, field, default_val) \
    ext->field = json.value(#field, default_val)

#define JSON_DESERIALIZE_PTR_FIND(json, ext, field, TypeClass) \
    do { \
        std::string id = json.value(#field, ""); \
        ext->field = id.empty() ? nullptr : TypeClass::Find(id.c_str()); \
    } while(0)

#define JSON_DESERIALIZE_COORD(json, name, coord) \
    do { \
        if (json.contains(name) && json[name].is_array() && json[name].size() >= 3) { \
            coord.X = json[name][0]; \
            coord.Y = json[name][1]; \
            coord.Z = json[name][2]; \
        } \
    } while(0)

bool ExtensionSaveJson::Save(const wchar_t* baseSave)
{
	try
	{
		std::filesystem::path extPath = GetExtPath(baseSave);

		Debug::Log("[ExtSave] Base: %ls\n", baseSave);
		Debug::Log("[ExtSave] Ext:  %ls\n", extPath.wstring().c_str());

		if (extPath.has_parent_path() && !extPath.parent_path().empty()) {
			std::error_code ec;
			std::filesystem::create_directories(extPath.parent_path(), ec);
			if (ec) {
				Debug::Log("[ExtSave] Failed to create directory: %s\n", ec.message().c_str());
				return false;
			}
		}

		json root;
		const SaveHeader _header {
			.Magic = Game::Savegame_Magic(),
			.Version = AresGlobalData::version,
			.VersionIdentifier = AresGlobalData::ModIdentifier,
			.Timestamp = std::time(nullptr),
			.ModName = AresGlobalData::ModName,
			.ModVersion = AresGlobalData::ModVersion
		};

		SaveHeader::WriteSaveHeader(root, _header);
		if (!AnimExtContainer::Instance.SaveAll(root)) {
			Debug::FatalErrorAndExit("[ExtSave] Failed to create file\n");
		}

		std::ofstream file(extPath, std::ios::out | std::ios::trunc);
		if (!file.is_open()) {
			Debug::Log("[ExtSave] Failed to create file\n");
			return false;
		}

		// Pretty print with 2-space indent for easy debugging
		file << std::setw(2) << root;
		file.close();

		Debug::Log("[ExtSave] Done! (%llu bytes)\n", std::filesystem::file_size(extPath));
		return true;
	}
	catch (const std::exception& e)
	{
		Debug::Log("[ExtSave] Exception during save: %s\n", e.what());
		return false;
	}
}

bool ExtensionSaveJson::Load(const wchar_t* baseSave)
{
	try
	{
		std::filesystem::path extPath = GetExtPath(baseSave);

		Debug::Log("[ExtLoad] Base: %ls\n", baseSave);
		Debug::Log("[ExtLoad] Ext:  %ls\n", extPath.wstring().c_str());

		if (!std::filesystem::exists(extPath)) {
			Debug::Log("[ExtLoad] Extension save not found - using defaults\n");
			return true; // Not an error
		}

		Debug::Log("[ExtLoad] File size: %llu bytes\n", std::filesystem::file_size(extPath));

		std::ifstream file(extPath);
		if (!file.is_open()) {
			Debug::Log("[ExtLoad] Failed to open file for reading\n");
			return false;
		}

		json root = json::parse(file);
		file.close();

		SaveHeader _header;

		// Validate header
		if (!SaveHeader::ReadSaveHeader(root, _header)) {
			Debug::Log("[ExtLoad] Missing header\n");
			return false;
		}

		if (_header.Magic != Game::Savegame_Magic()) {
			Debug::Log("[ExtLoad] Invalid magic: %d\n", _header.Magic);
			return false;
		}

		if (_header.Version != AresGlobalData::version) {
			Debug::Log("[ExtLoad] Invalid version: %d\n", _header.Version);
			return false;
		}

		if (_header.VersionIdentifier != AresGlobalData::ModIdentifier) {
			Debug::Log("[ExtLoad] Invalid version identifier: %d\n", _header.VersionIdentifier);
			return false;
		}

		if (_header.ModName != AresGlobalData::ModName) {
			Debug::Log("[ExtLoad] Invalid Mod : %s\n", _header.ModName);
			return false;
		}

		if (_header.ModVersion != AresGlobalData::ModVersion) {
			Debug::Log("[ExtLoad] Invalid Mod version: %d\n", _header.ModVersion);
			return false;
		}

		Debug::Log("[ExtLoad] Header OK\n");

		if (!AnimExtContainer::Instance.LoadAll(root)) {
			Debug::Log("[ExtLoad] Deserialization failed\n");
			return false;
		}

		Debug::Log("[ExtLoad] Load complete\n");
		return true;
	}
	catch (const json::exception& e)
	{
		Debug::Log("[ExtLoad] JSON parse error: %s\n", e.what());
		return false;
	}
	catch (const std::exception& e)
	{
		Debug::Log("[ExtLoad] Exception during load: %s\n", e.what());
		return false;
	}
}

