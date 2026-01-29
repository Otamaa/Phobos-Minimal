
#include "Wrapper.h"

#include <Phobos.Lua.h>

#include <vector>
#include <string>

#include <TeamClass.h>

#include <Ext/Techno/Body.h>

#include <FootClass.h>
#include <UnitClass.h>

/*
* // the stack are push back
		- 3 is the first argument
		- 2 is the first argument
		- 1 is the third
*/

struct TeamClassWrapper
{
	static int SetStepCompleted(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		pTeam->StepCompleted = true;
		return 0;
	}

	static int GetCurrentScriptArg(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		lua_pushnumber(L, pTeam->CurrentScript->GetCurrentAction().Argument);
		return 1;
	}

	static int IsGuardAreaTimerTicking(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		const auto result = pTeam->GuardAreaTimer.IsTicking();
		lua_pushboolean(L, result);
		return 1;
	}

	static int GetAreaGuardTimerTimeLeft(lua_State* L)
	{

		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		const int result = pTeam->GuardAreaTimer.GetTimeLeft();
		lua_pushnumber(L, result);
		return 1;
	}

	static int StopGuardAreaTimer(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		pTeam->GuardAreaTimer.Stop();
		return 0;
	}

	static int StartGuardAreaTimer(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -2);
		pTeam->GuardAreaTimer.Start((int)lua_tointeger(L, -1));
		return 0;
	}

	static int GetFirstUnit(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		lua_pushnumber(L, (uintptr_t)pTeam->FirstUnit);
		return 1;
	}
};

struct _Unit
{
	static int GetNextTeamMember(lua_State* L)
	{
		FootClass* pFoot = (FootClass*)lua_tointeger(L, -1);
		lua_pushnumber(L, (uintptr_t)pFoot->NextTeamMember);
		return 1;
	}

};

struct _MissionClass
{
	static int QueueMission(lua_State* L)
	{
		MissionClass* pRadio = (MissionClass*)lua_tointeger(L, -3);
		pRadio->QueueMission((Mission)lua_tointeger(L, -2), lua_toboolean(L, -1));
		return 0;
	}
};

struct _TechnoExtData
{
	static int IsInWarfactory(lua_State* L)
	{
		TechnoClass* pRadio = (TechnoClass*)lua_tointeger(L, -1);
		lua_pushboolean(L, TechnoExtData::IsInWarfactory(pRadio));
		return 1;
	}
};

struct LuaScript
{
	int Number {};
	std::string Name {};
	std::string Lua_Name {};
	unique_luastate State {};

	/*void Initialize(int number, const char* name)
	{
		Number = number;
		Name = name;
		Lua_Name = Name + ".lua";

		InitState();
	}

	void InitState()
	{
		State.open_libraries(sol::lib::base, sol::lib::io);

		try {
			if (State.safe_script_file(Lua_Name).status() == sol::call_status::ok)
			{
				State["_TeamClass_GetCurrentScriptArg"] = TeamClassWrapper::GetCurrentScriptArg;
				State["_TeamClass_SetStepCompleted"] = TeamClassWrapper::SetStepCompleted;
				State["_TeamClass_IsGuardAreaTimerTicking"] = TeamClassWrapper::IsGuardAreaTimerTicking;
				State["_TeamClass_GetAreaGuardTimerTimeLeft"] = TeamClassWrapper::GetAreaGuardTimerTimeLeft;
				State["_TeamClass_GetFirstUnit"] = TeamClassWrapper::GetFirstUnit;
				State["_TeamClass_StartGuardAreaTimer"] = TeamClassWrapper::StartGuardAreaTimer;
				State["_TeamClass_StopGuardAreaTimer"] = TeamClassWrapper::StopGuardAreaTimer;
				State["_TechnoExtData_IsInWarfactory"] = _TechnoExtData::IsInWarfactory;
				State["_MissionClass_QueueMission"] = _MissionClass::QueueMission;
				State["_Unit_GetNextTeamMember"] = _Unit::GetNextTeamMember;
			}

		}catch(const sol::error& what) {

			Debug::LogInfo("Cannot Find [%s] File ! Reason (%s)", Lua_Name.c_str(), what.what());
			State = nullptr;
		}
	}*/
};

//static std::vector<LuaScript> LuaScripts {};
struct PairOfNumbers {
	int Alternate;
	int Original;
};
//static std::vector<PairOfNumbers> SriptNumbers {};
PairOfNumbers SriptNumbers[] = {

	        {150,14004},
			{71,10100},
			{72,10103},
			{73,10101},
			{76,10000},
			{80,10001},
			{86,10002},
			{90,10003},
			{104,10004},
			{77,10005},
			{81,10006},
			{87,10007},
			{91,10008},
			{105,10009},
			{74,10010},
			{78,10011},
			{84,10012},
			{88,10013},
			{75,10014},
			{79,10015},
			{85,10016},
			{89,10017},
			{95,10050},
			{99,10051},
			{106,10052},
			{97,10053},
			{101,10054},
			{108,10055},
			{96,10056},
			{100,10057},
			{107,10058},
			{98,10059},
			{102,10060},
			{109,10061},
			{112,10102},
			{92,12000},
			{103,12001},
			{110,12002},
			{93,14000},
			{83,14001},
			{82,14002},
			{111,14003},
			{126,16000},
			{125,16001},
			{124,16002},
			{113,16003},
			{94,16004},

			{500,18000},
			{501,18001},
			{502,18002},
			{503,18003},
			{504,18004},
			{505,18005},
			{506,18006},
			{507,18007},
			{508,18008},
			{509,18009},
			{510,18010},
			{511,18011},
			{512,18012},
			{513,18013},
			{514,18014},
			{515,18015},
			{516,18016},
			{517,18017},
			{518,18018},
			{519,18019},
			{520,18020},
			{521,18021},
			{522,18022},
			{523,18023},
			{524,18024},
			{525,18025},
			{526,18026},
			{527,18027},
			{528,18028},
			{529,18029},
			{530,18030},
			{531,18031},
			{532,18032},
			{533,18033},
			{534,18034},
			{535,18035},
			{536,18036},
			{537,18037},
			{538,18038},
			{539,18039},
			{540,18040},
			{541,18041},
			{542,18042},
			{543,18043},
			{544,18044},
			{545,18045},
			{546,18046},
			{547,18047},
			{548,18048},
			{549,18049},
			{550,18050},
			{551,18051},
			{552,18052},
			{553,18053},
			{554,18054},
			{555,18055},
			{556,18056},
			{557,18057},
			{558,18058},
			{559,18059},
			{560,18060},
			{561,18061},
			{562,18062},
			{563,18063},
			{564,18064},
			{565,18065},
			{566,18066},
			{567,18067},
			{568,18068},
			{569,18069},
			{570,18070},
			{571,18071}
};

bool LuaBridge::OnCalled(TeamClass* pTeam)
{
#ifdef EXPERIMENTAL
	auto const& [action, argument] = pTeam->CurrentScript->GetCurrentAction();

	for (auto& luascript : LuaScripts)
	{
		if (luascript.State && (TeamMissionType)luascript.Number == action)
		{
			lua_getglobal(luascript.State.get(), "OnExecute");

			if (lua_isfunction(luascript.State.get(), -1) != 1)
			{
				Debug::LogInfo("Lua [%s] Failed to call `OnExecute` Function ! Reason (function not found)", luascript.Lua_Name.c_str());
				luascript.State.reset(nullptr);
				return false;
			}
			lua_pushnumber(luascript.State.get(), (lua_Number)((uintptr_t)pTeam));

			if (lua_pcall(luascript.State.get(), 1, 0, 0) != LUA_OK)
			{
				luascript.State.reset(nullptr);
				return false;
			}

			return true;
		}
	}
#endif
	return false;
}

void LuaBridge::InitScriptLuaList(unique_luastate& sol_state)
{
	//const std::string filename = LuaData::LuaDir + "\\ScriptAlternativeNumbering.lua";

	//auto L = sol_state.get();

	//if (luaL_dofile(L, filename.c_str()) == LUA_OK)
	//{
	//	lua_getglobal(L, "Scripts");
	//	if (lua_istable(L, -1)) {
	//		const size_t scriptSize = (size_t)lua_rawlen(L, -1);
	//		SriptNumbers.resize(scriptSize);
	//		for (size_t i = 0; i < scriptSize; i++)
	//		{
	//			lua_pushinteger(L, lua_Integer(i + 1));
	//			lua_gettable(L, -2);
	//			if (lua_istable(L, -2))
	//			{
	//				lua_pushstring(L, "Original");
	//				lua_gettable(L, -2);
	//				const auto Originalnumber = (int)lua_tointeger(L, -1);
	//				lua_pop(L, 1);

	//				lua_pushstring(L, "Alternative");
	//				lua_gettable(L, -2);
	//				const auto AlternativeNumber = (int)lua_tointeger(L, -1);
	//				lua_pop(L, 1);

	//				SriptNumbers[i]. Originalnumber;
	//				SriptNumbers[i].Alternate = AlternativeNumber;
	//			}
	//			lua_pop(L, 1);
	//		}
	//	}
	//}

}

#include <Ext/Script/Body.h>

ASMJIT_PATCH(0x69192E, ScriptTypeClass_Read_INI_TeamMission, 0x7)
{
	GET(ScriptTypeClass*, pThis, ESI);
	GET(int, team, ECX);

	for (auto& cur : SriptNumbers) {
		if (cur.Alternate == team) {
			Debug::LogInfo("[{}]Replacing TMission[{} to {}]", pThis->ID, team, cur.Original);
			R->ECX(cur.Original);
			return 0x0;
		}
	}

	if (team >= (int)TeamMissionType::count
		&& team >= (int)AresScripts::count
		&& team >= (int)PhobosScripts::count)
		Debug::LogInfo("[{}]contains unknow TMission[{}]", pThis->ID, team);

	return 0x0;
}