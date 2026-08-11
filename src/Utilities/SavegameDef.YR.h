#pragma once

// =============================================================================
// SavegameDef.YR.h
//
// Tier 3 of the SavegameDef split: every specialization over a YRpp or game
// type. This is the ONLY tier that pulls the expensive headers -- RulesClass.h,
// SidebarClass.h, ScenarioClass.h, ScriptTypeClass.h, FileSystem.h, SHP.h.
//
// Include this only from TUs that actually serialize game objects. Utility TUs
// should include SavegameDef.Std.h (or .Core.h) instead and will then stop
// rebuilding when any of the headers below change.
//
// SPLIT NOTE: bodies moved verbatim from the original SavegameDef.h.
// =============================================================================

#include "SavegameDef.Std.h"

#include <ArrayClasses.h>
#include <CellStruct.h>
#include <CoordStruct.h>
#include <FileFormats/SHP.h>
#include <FileSystem.h>
#include <IndexBitfield.h>
#include <Leptons.h>
#include <Point2D.h>
#include <Point2DByte.h>
#include <Point3D.h>
#include <RocketStruct.h>
#include <RulesClass.h>
#include <ScenarioClass.h>
#include <ScriptTypeClass.h>
#include <SidebarClass.h>
#include <Timers.h>

#include <Helpers/String.h>
#include <Utilities/GameUniquePointers.h>
#include <Utilities/TranslucencyLevel.h>
#include <Utilities/VectorHelper.h>
#include <Utilities/Debug.h>

namespace Savegame
{
#pragma region YRSpecializations

	template <typename T>
	struct Savegame::PhobosStreamObject<IndexBitfield<T>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, IndexBitfield<T>& value, bool register_for_change) const
		{
			if (!Savegame::ReadPhobosStream(stm, value.data, register_for_change))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const IndexBitfield<T>& value) const
		{
			if (!Savegame::WritePhobosStream(stm, value.data))
				return false;

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<MinMaxValue<T>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, MinMaxValue<T>& value, bool register_for_change) const
		{
			if (!Savegame::ReadPhobosStream(stm, value.Min, register_for_change))
				return false;
			if (!Savegame::ReadPhobosStream(stm, value.Max, register_for_change))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const MinMaxValue<T>& value) const
		{
			if (!Savegame::WritePhobosStream(stm, value.Min))
				return false;
			if (!Savegame::WritePhobosStream(stm, value.Max))
				return false;

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<PartialVector4D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, PartialVector4D<T>& value, bool register_for_change) const
		{
			if (!Savegame::ReadPhobosStream(stm, value.ValueCount, register_for_change))
				return false;
			for (size_t idx = 0; idx < value.ValueCount; ++idx)
			{
				if (!Savegame::ReadPhobosStream(stm, value[idx], register_for_change))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const PartialVector4D<T>& value) const
		{
			if (!Savegame::WritePhobosStream(stm, value.ValueCount))
				return false;
			for (size_t idx = 0; idx < value.ValueCount; ++idx)
			{
				if (!Savegame::WritePhobosStream(stm, value[idx]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<PartialVector3D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, PartialVector3D<T>& value, bool register_for_change) const
		{
			if (!Savegame::ReadPhobosStream(stm, value.ValueCount, register_for_change))
				return false;
			for (size_t idx = 0; idx < value.ValueCount; ++idx)
			{
				if (!Savegame::ReadPhobosStream(stm, value[idx], register_for_change))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const PartialVector3D<T>& value) const
		{
			if (!Savegame::WritePhobosStream(stm, value.ValueCount))
				return false;
			for (size_t idx = 0; idx < value.ValueCount; ++idx)
			{
				if (!Savegame::WritePhobosStream(stm, value[idx]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<PartialVector2D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, PartialVector2D<T>& value, bool register_for_change) const
		{
			if (!Savegame::ReadPhobosStream(stm, value.ValueCount, register_for_change))
				return false;
			for (size_t idx = 0; idx < value.ValueCount; ++idx)
			{
				if (!Savegame::ReadPhobosStream(stm, value[idx], register_for_change))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const PartialVector2D<T>& value) const
		{
			if (!Savegame::WritePhobosStream(stm, value.ValueCount))
				return false;
			for (size_t idx = 0; idx < value.ValueCount; ++idx)
			{
				if (!Savegame::WritePhobosStream(stm, value[idx]))
					return false;
			}

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<CellStruct>
	{
		bool ReadFromStream(PhobosStreamReader& stm, CellStruct& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const CellStruct& value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<Leptons>
	{
		bool ReadFromStream(PhobosStreamReader& stm, Leptons& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const Leptons& value) const;
	};

	// VERIFY: FixedString comes from Helpers/String.h in the original include
	// block. If it actually lives in a dependency-free Phobos utility header,
	// move this specialization down to SavegameDef.Core.h -- it has no game
	// dependency of its own.
	template <size_t Capacity, typename CharT>
	struct Savegame::PhobosStreamObject<FixedString<Capacity, CharT>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, FixedString<Capacity, CharT>& value, bool register_for_change) const
		{
			// Read the fixed-size buffer directly
			CharT buffer[Capacity];
			if (!stm.Read(reinterpret_cast<PhobosByteStream::data_t*>(buffer), sizeof(buffer)))
				return false;

			// Ensure null termination (safety against corrupted saves)
			buffer[Capacity - 1] = CharT {};

			// Use the fast assign with known max length
			// This skips strlen since we know the max size
			value.assign(buffer, Capacity - 1);

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const FixedString<Capacity, CharT>& value) const
		{
			// Write the fixed-size buffer directly
			CharT buffer[Capacity] = {};  // Zero-initialize entire buffer

			// Get current string length efficiently
			const size_t current_len = value.size();

			// Copy only the actual string content (not the whole capacity)
			// but still write the full Capacity to maintain save file format
			if (current_len > 0)
			{
				std::char_traits<CharT>::copy(buffer, value.data(),
					std::min(current_len, Capacity - 1));
			}
			// buffer[current_len] is already '\0' from zero-initialization

			// Write the entire fixed buffer to maintain consistent save format
			return stm.Write(reinterpret_cast<const PhobosByteStream::data_t*>(buffer), sizeof(buffer));
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<DirStruct>
	{
		bool ReadFromStream(PhobosStreamReader& stm, DirStruct& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const DirStruct& value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<CoordStruct>
	{
		bool ReadFromStream(PhobosStreamReader& stm, CoordStruct& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const CoordStruct& value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<Point2D>
	{
		bool ReadFromStream(PhobosStreamReader& stm, Point2D& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const Point2D& value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<Point2DBYTE>
	{
		bool ReadFromStream(PhobosStreamReader& stm, Point2DBYTE& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const Point2DBYTE& value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<Point3D>
	{
		bool ReadFromStream(PhobosStreamReader& stm, Point3D& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const Point3D& value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<ColorStruct>
	{
		bool ReadFromStream(PhobosStreamReader& stm, ColorStruct& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const ColorStruct& value) const;
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<VectorClass<T>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, VectorClass<T>& value, bool register_for_change) const
		{
			value.clear();
			int capacity = 0;

			if (!Savegame::ReadPhobosStream(stm, capacity))
				return false;

			value.reserve(capacity);

			for (auto ix = 0; ix < capacity; ++ix)
			{
				if (!Savegame::ReadPhobosStream(stm, value.Items[ix], register_for_change))
					return false;
			}

			return stm.RegisterChange(&value);
		}

		bool WriteToStream(PhobosStreamWriter& stm, const VectorClass<T>& value) const
		{
			if (!Savegame::WritePhobosStream(stm, value.Capacity))
				return false;

			for (auto ix = 0; ix < value.Capacity; ++ix)
			{
				if (!Savegame::WritePhobosStream(stm, value.Items[ix]))
					return false;
			}

			return stm.RegisterChange(&value);
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<DynamicVectorClass<T>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, DynamicVectorClass<T>& value, bool register_for_change) const
		{
			value.reset();
			int capacity = 0;

			if (!Savegame::ReadPhobosStream(stm, capacity, register_for_change))
				return false;

			value.reserve(capacity);

			if (!Savegame::ReadPhobosStream(stm, value.Count, register_for_change) || !Savegame::ReadPhobosStream(stm, value.CapacityIncrement, register_for_change))
				return false;

			for (auto ix = 0; ix < value.Count; ++ix)
			{
				if (!Savegame::ReadPhobosStream(stm, value.Items[ix], register_for_change))
					return false;
			}

			return stm.RegisterChange(&value);
		}

		bool WriteToStream(PhobosStreamWriter& stm, const DynamicVectorClass<T>& value) const
		{
			if (!Savegame::WritePhobosStream(stm, value.Capacity))
				return false;

			if (!Savegame::WritePhobosStream(stm, value.Count))
				return false;

			if (!Savegame::WritePhobosStream(stm, value.CapacityIncrement))
				return false;

			for (auto ix = 0; ix < value.Count; ++ix)
			{
				if (!Savegame::WritePhobosStream(stm, value.Items[ix]))
					return false;
			}

			return stm.RegisterChange(&value);
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<TypeList<T>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, TypeList<T>& value, bool register_for_change) const
		{
			if (!Savegame::ReadPhobosStream<DynamicVectorClass<T>>(stm, value, register_for_change))
				return false;

			return Savegame::ReadPhobosStream(stm, value.unknown_18, register_for_change) && stm.RegisterChange(&value);
		}

		bool WriteToStream(PhobosStreamWriter& stm, const TypeList<T>& value) const
		{
			if (!Savegame::WritePhobosStream<DynamicVectorClass<T>>(stm, value))
				return false;

			return Savegame::WritePhobosStream(stm, value.unknown_18) && stm.RegisterChange(&value);
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<Vector3D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, Vector3D<T>& value, bool register_for_change) const
		{
			if (!Savegame::ReadPhobosStream(stm, value.X, register_for_change))
				return false;

			if (!Savegame::ReadPhobosStream(stm, value.Y, register_for_change))
				return false;

			if (!Savegame::ReadPhobosStream(stm, value.Z, register_for_change))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const Vector3D<T>& value) const
		{
			if (!Savegame::WritePhobosStream(stm, value.X))
				return false;

			if (!Savegame::WritePhobosStream(stm, value.Y))
				return false;

			if (!Savegame::WritePhobosStream(stm, value.Z))
				return false;

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<Vector2D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, Vector2D<T>& value, bool register_for_change) const
		{
			if (!Savegame::ReadPhobosStream(stm, value.X, register_for_change))
				return false;

			if (!Savegame::ReadPhobosStream(stm, value.Y, register_for_change))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const Vector2D<T>& value) const
		{
			if (!Savegame::WritePhobosStream(stm, value.X))
				return false;

			if (!Savegame::WritePhobosStream(stm, value.Y))
				return false;

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<Vector4D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& stm, Vector4D<T>& value, bool register_for_change) const
		{
			if (!Savegame::ReadPhobosStream(stm, value.X, register_for_change))
				return false;

			if (!Savegame::ReadPhobosStream(stm, value.Y, register_for_change))
				return false;

			if (!Savegame::ReadPhobosStream(stm, value.Z, register_for_change))
				return false;

			if (!Savegame::ReadPhobosStream(stm, value.W, register_for_change))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const Vector4D<T>& value) const
		{
			if (!Savegame::WritePhobosStream(stm, value.X))
				return false;

			if (!Savegame::WritePhobosStream(stm, value.Y))
				return false;

			if (!Savegame::WritePhobosStream(stm, value.Z))
				return false;

			if (!Savegame::WritePhobosStream(stm, value.W))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<WeaponStruct>
	{
		bool ReadFromStream(PhobosStreamReader& stm, WeaponStruct& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const WeaponStruct& value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<TintStruct>
	{
		bool ReadFromStream(PhobosStreamReader& stm, TintStruct& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const TintStruct& value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<LightingStruct>
	{
		bool ReadFromStream(PhobosStreamReader& stm, LightingStruct& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const LightingStruct& value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<CounterClass>
	{
		bool ReadFromStream(PhobosStreamReader& stm, CounterClass& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const CounterClass& value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<ScriptActionNode>
	{
		bool ReadFromStream(PhobosStreamReader& stm, ScriptActionNode& value, bool register_for_change) const;
		bool WriteToStream(PhobosStreamWriter& stm, const ScriptActionNode& value) const;
	};

	// BUGFIX: was `static_assert(true, ...)` -- never fired, so a UniqueGamePtr
	// member silently serialized nothing. Now a hard error at point of use.
	// The BytePalette specialization below shows the shape a real one takes.
	template <typename T>
	struct Savegame::PhobosStreamObject<UniqueGamePtr<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, UniqueGamePtr<T>& Value, bool RegisterForChange) const
		{
			static_assert(detail::AlwaysFalse<T>,
				"PhobosStreamObject<UniqueGamePtr<T>>: not implemented for this T. "
				"Add an explicit specialization (see UniqueGamePtr<BytePalette>).");
			return false;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const UniqueGamePtr<T>& Value) const
		{
			static_assert(detail::AlwaysFalse<T>,
				"PhobosStreamObject<UniqueGamePtr<T>>: not implemented for this T. "
				"Add an explicit specialization (see UniqueGamePtr<BytePalette>).");
			return false;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<BytePalette>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, BytePalette& Value, bool RegisterForChange) const;
		bool WriteToStream(PhobosStreamWriter& Stm, const BytePalette& Value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<UniqueGamePtr<BytePalette>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, UniqueGamePtr<BytePalette>& Value, bool RegisterForChange) const;
		bool WriteToStream(PhobosStreamWriter& Stm, const UniqueGamePtr<BytePalette>& Value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<SHPCaches*>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, SHPCaches*& Value, bool RegisterForChange) const;
		bool WriteToStream(PhobosStreamWriter& Stm, SHPCaches* const& Value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<RocketStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, RocketStruct& Value, bool RegisterForChange) const;
		bool WriteToStream(PhobosStreamWriter& Stm, const RocketStruct& Value) const;
	};

	template <>
	struct Savegame::PhobosStreamObject<BuildType>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, BuildType& Value, bool RegisterForChange) const;
		bool WriteToStream(PhobosStreamWriter& Stm, const BuildType& Value) const;
	};

	template <typename T, typename Alloc>
	struct Savegame::PhobosStreamObject<HelperedVector<T, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, HelperedVector<T, Alloc>& Value, bool RegisterForChange) const
		{
			return Savegame::ReadPhobosStream<std::vector<T, Alloc>>(Stm, Value, RegisterForChange);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const HelperedVector<T, Alloc>& Value) const
		{
			return Savegame::WritePhobosStream<std::vector<T, Alloc>>(Stm, Value);
		}
	};

#pragma endregion

}