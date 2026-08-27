#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

// Forward-declared only - this header stays free of the actual stream
// framework's include, keeping it cheap for any ext-data header to pull
// in just to declare a LuaSlots member. Load/Save's bodies (which need
// the complete types) live in Phobos.LuaSlots.cpp.
class PhobosStreamReader;
class PhobosStreamWriter;

// =================================================================
// Phobos.LuaSlots.h
//
// A small, C++-interpretable, typed data buffer that any ext-data class
// can embed as a member to expose per-object state to Lua that:
//   - survives save/load automatically (once wired into that class's own
//     Serialize<T> chain, e.g. `.Process(this->LuaSlots)`)
//   - is freed automatically when the object is destroyed (it lives
//     inside the ext-data instance itself, not a separate registry)
//   - can be READ AND ACTED ON by other C++ systems, unlike an opaque
//     byte blob or string - a debug overlay, a sync-check tool, or
//     another gameplay system can inspect a slot's Type and value
//     directly, without knowing anything about Lua's encoding.
//
// Deliberately has NO dependency on <lua.hpp> or any engine headers -
// ext-data headers (Team/Body.h, and eventually Techno/House's own
// ext-data) should be able to include this and declare a LuaSlots
// member without dragging Lua's headers into every translation unit
// that includes them. The actual Lua-facing Get/Set bindings live in
// Phobos.Lua.cpp, which includes both this header and the specific
// ext-data header it's exposing slots for.
//
// CAVEAT: slots are per-object GLOBAL storage, not scoped to whichever
// Lua system/mission is currently using them. Two unrelated systems
// both using slot 0 for different purposes WILL clobber each other if
// the object can transition between them without clearing. Clear a
// slot (ClearSlot) when a system is done with it - see
// team_missions_example.lua's ScatterAndRegroup for the pattern.
// =================================================================

enum class PhobosLuaSlotType : uint8_t
{
	Empty = 0,
	Int,      // int32_t
	Float,    // double
	Bool,
	Cell,     // a pair of int16_t (X, Y). Layout-compatible with a typical
	// CellStruct{X,Y}, but intentionally NOT that type itself -
	// this header stays engine-independent. VERIFY CellStruct's
	// actual member order/size in your tree before relying on a
	// reinterpret_cast between the two.
	ObjectId, // a TechnoClass::UniqueID-style stable id (int32_t), meant
	// to be resolved back to a live object later via
	// World.GetById() - NOT a raw pointer. Raw pointers need
	// save/load pointer-swizzling (see how TeamExtData::TeamLeader
	// is processed via a special two-arg Process(ptr, true)
	// overload) that a generic tagged union can't replicate.
};

struct PhobosLuaSlot
{
	PhobosLuaSlotType Type;

	union
	{
		int32_t AsInt;
		double AsFloat;
		bool AsBool;
		struct { int16_t X; int16_t Y; } AsCell;
		int32_t AsObjectId;
	};

	PhobosLuaSlot() : Type(PhobosLuaSlotType::Empty), AsInt(0) {}
	~PhobosLuaSlot() = default;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;
};

static_assert(std::is_trivially_copyable<PhobosLuaSlot>::value,
	"PhobosLuaSlot should stay trivially copyable even though it is no "
	"longer serialized as raw bytes - other code (Lua bindings, debug "
	"tooling) still copies/compares it by value.");

// tune freely per ext-data type if 8 is too few/many for a
// given use case - this is just a compile-time constant.
constexpr int kPhobosLuaSlotCount = 8;

// a real class, not a plain std::array alias, specifically so
// it can own its OWN Load/Save. A raw union has no reliably-serializable
// "whole object" representation for a generic reflection-style stream
// Process<T> to walk - it doesn't know which member is active without
// the Type tag being read first, and padding/alignment inside the union
// is implementation-defined. Load/Save below process each slot as
// [Type tag][only the payload that Type actually implies], which is
// exactly the deliberate, type-aware serialization a union needs and a
// generic Process<T> can't safely infer on its own.
//
// The Load(Reader&, bool)/Save(Writer&) const signature matches this
// stream framework's own customization hook (see PhobosStreamReader::
// Process<T>'s dispatch) - implementing it here is what lets
// TeamExtData::Serialize's existing `.Process(this->LuaSlots)` call
// dispatch to these instead of attempting (and failing) a raw copy.
class PhobosLuaSlots
{
public:
	PhobosLuaSlot& operator[](int index) { return this->Slots[index]; }
	const PhobosLuaSlot& operator[](int index) const { return this->Slots[index]; }

	static constexpr int Count = kPhobosLuaSlotCount;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	std::array<PhobosLuaSlot, kPhobosLuaSlotCount> Slots {};
};
