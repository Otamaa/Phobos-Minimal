#include "AutoLoad.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/GameConfig.h>

// resemble the original code 
// but more centralized 


bool AutoLoadCommandClass::g_enabled;
int AutoLoadCommandClass::g_stagger;
volatile LONG AutoLoadCommandClass::g_wantLoad;
volatile LONG AutoLoadCommandClass::g_wantVariant;

int AutoLoadCommandClass::s_armed;
int AutoLoadCommandClass::s_countdown;
DWORD AutoLoadCommandClass::s_lastVariantTick;

const char* AutoLoadCommandClass::GetName() const
{
	return "AutoLoad";
}

const wchar_t* AutoLoadCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_AUTO_LOAD", L"Auto Load	");
}

const wchar_t* AutoLoadCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* AutoLoadCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_AUTO_LOAD_DESC", L"Automatically load passengers.");
}


#ifdef _Autoload

void AutoLoadCommandClass::TickAll()
{
	if (!g_state.configRead)
		return; // the command was never used: nothing to do

	if (g_state.stagger)
	{
		StartDueSessions();
		Loader::TickAllSessions(); // sessions run on the presser's machine only
		return;
	}
	TickLegacy();
}

void AutoLoadCommandClass::ReadConfig()
{
	GameConfig ares_ini { "AutoLoad.ini" };
}

void AutoLoadCommandClass::Reset()
{
	g_state.armed = false;
	g_state.countdown = 0;
	g_state.continuePass = false;

	Loader::g_pass.End();
	Loader::g_plan.Clear();
	Loader::g_released.units.Clear();
	for (Loader::PendingSnapshot& pending : Loader::g_pending)
		pending.used = false;
	for (Loader::Session& session : Loader::g_sessions)
	{
		session.active = false;
		session.frame = 0;
		session.released = 0;
		session.lanes.Clear();
	}
}


#include <Ext/Tactical/Body.h>

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

struct Scan
{
	std::vector<VariantKey> refs;
	int scanned;
	int selectable;
	int added;

	bool MatchesRef(TechnoTypeClass* type, WeaponStruct* weapon) const
	{
		for (const VariantKey& ref : refs) {
			if (ref.type == type && ref.weapon == weapon)
				return true;
		}
		return false;
	}

};

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
	key.weapon = type->GetWeapon(current);
	return key;
}

bool __fastcall Techno_Player_Controllable(TechnoClass* pTech)
{ JMP_FAST(0x732580); }

void ConsiderCandidate(Scan& scan, TechnoClass* obj)
{
	if (!obj)
		return;

	++scan.scanned;

	if (!obj->IsAlive || obj->Health <= 0 || obj->InLimbo ||obj->IsCrashing || obj->IsSinking)
		return;

	if (!Techno_Player_Controllable(obj))
		return;

	++scan.selectable;

	const VariantKey key = KeyOf(obj);
	if (!key.type)
		return;

	if (!scan.MatchesRef(key.type, key.weapon))
		return;
	
	if (obj->IsSelectable())
		++scan.added;
}

/// Everything the command needs between frames (game thread only).
struct FrameState
{
	bool configRead;
	bool stagger;       // AutoLoad.ini [AutoLoad] Stagger=1
	bool armed;         // legacy: delayed trigger
	int countdown;      // legacy: frames left before the pass starts
	bool continuePass;  // legacy: an unfinished pass wants one more frame
};

FrameState g_state;

// ------------------------------------------------------------------------
// Config (AutoLoad.ini next to gamemd.exe; hotkeys now come from Phobos' key menu)
// ------------------------------------------------------------------------

void EnsureConfig()
{
	if (g_state.configRead)
		return;
	g_state.configRead = true;

	// "Stagger=1": plan once, release waves gated on boarding (see AL_Session.h).
	// "Stagger=0" (default): legacy single-frame dispatch.
	g_state.stagger = GetPrivateProfileIntA("AutoLoad", "Stagger", 0, ".\\AutoLoad.ini") == 1;
	Log::Write("config: stagger = ", g_state.stagger ? "1" : "0 (legacy dispatch)");
}

// ------------------------------------------------------------------------
// Stagger=1
// ------------------------------------------------------------------------

/// Starts the sessions of snapshots taken on an earlier frame.
void StartDueSessions()
{
	for (Loader::PendingSnapshot& pending : Loader::g_pending)
	{
		if (!pending.used || --pending.delay > 0)
			continue;
		const bool isReplace = Loader::AnySessionActive();
		pending.used = false;
		Loader::StartSessionFromSnapshot(isReplace, { pending.objects, pending.count });
	}
}

/// Snapshots the live selection at the press moment; every snapshot starts its own session
/// and never knocks out an older one.
void QueueSelectionSnapshot()
{
	const Engine::SelectionView live = Engine::LiveSelection();
	if (!live.items || live.count <= 0)
	{
		Log::Write("hotkey pressed but selection empty, nothing queued");
		return;
	}

	const int slot = Loader::EnqueuePending(live.items, live.count);
	if (slot < 0)
	{
		Log::Write("pending queue full, new press dropped (old sessions kept)");
		return;
	}
	// Execute() runs before this frame's TickAll(): wait one extra frame so the snapshot is
	// planned on the NEXT frame (same timing as the original DLL).
	Loader::g_pending[slot].delay = 2;
	Log::Write("hotkey snapshot queued: slot=", slot);
}

// ------------------------------------------------------------------------
// Stagger=0 (legacy: single-frame dispatch, may continue over several frames)
// ------------------------------------------------------------------------

/// A pass that is not finished asks for one more frame (the OutList is emptied each frame).
void RequestNextFrameIfPassActive()
{
	if (Loader::g_pass.active)
		g_state.continuePass = true;
}

void StartLegacyPass()
{
	Loader::g_pass.active = true;
	Loader::g_pass.done.Clear();
	Loader::g_pass.budget.Clear();
	Loader::g_pass.ClearSameType(); // same-type counts live as long as this pass's budget
	Log::Write("--- auto-load pass started ---");
	Loader::RunLegacyStep();
	RequestNextFrameIfPassActive();
}

void TickLegacy()
{
	if (g_state.continuePass)
	{
		g_state.continuePass = false;
		Loader::RunLegacyStep();
		RequestNextFrameIfPassActive();
		return;
	}

	if (!g_state.armed || --g_state.countdown > 0)
		return;
	g_state.armed = false;
	StartLegacyPass();
}

void AutoLoadCommandClass::Execute(WWKey dwUnk) const
{
	EnsureConfig();

	if (g_state.stagger)
	{
		QueueSelectionSnapshot();
		return;
	}

	if (Loader::g_pass.active)
	{
		g_state.continuePass = true; // continuation of an unfinished pass: no extra delay
		return;
	}

	// New trigger: delay the dispatch to the next frame (Execute runs before this frame's
	// TickAll), clear of other input of the frame in which the hotkey went down.
	g_state.armed = true;
	g_state.countdown = 2;
}
#else 
void AutoLoadCommandClass::Execute(WWKey dwUnk) const
{

}
#endif