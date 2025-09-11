#pragma once

// include this file whenever something is to be saved.

#include "Savegame.h"

#include <unordered_map>
#include <map>
#include <set>
#include <bitset>
#include <memory>
#include <string>
#include <array>

#include <ArrayClasses.h>
#include <FileSystem.h>
#include <FileFormats/SHP.h>
#include <RulesClass.h>
#include <SidebarClass.h>
#include <ScriptTypeClass.h>
#include <queue>
#include <optional>
#include <RocketStruct.h>
#include <ScenarioClass.h>
#include <Timers.h>
#include <Helpers/String.h>
#include <Leptons.h>
#include <CellStruct.h>
#include <CoordStruct.h>
#include <Point2D.h>
#include <Point2DByte.h>
#include <Point3D.h>
#include <IndexBitfield.h>

#include "TranslucencyLevel.h"
#include "Swizzle.h"
#include "Debug.h"
#include "MemoryPoolUniquePointer.h"
#include "GameUniquePointers.h"

namespace Savegame
{
	namespace detail
	{
		// concepts for detecting supported functions
		template <typename T>
		concept HasLoad = requires(T v, PhobosStreamReader & stm, bool reg)
		{
			{ v.Load(stm, reg) } -> std::same_as<bool>;
		};

		template <typename T>
		concept Hasload = requires(T v, PhobosStreamReader & stm, bool reg)
		{
			{ v.load(stm, reg) } -> std::same_as<bool>;
		};

		template <typename T>
		concept HasSave = requires(const T v, PhobosStreamWriter & stm)
		{
			{ v.Save(stm) } -> std::same_as<bool>;
		};

		template <typename T>
		concept Hassave = requires(const T v, PhobosStreamWriter & stm)
		{
			{ v.save(stm) } -> std::same_as<bool>;
		};

		struct Selector
		{
			template <typename T>
			static bool ReadFromStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange)
			{
				if constexpr (HasLoad<T>)
					return Value.Load(Stm, RegisterForChange);

				else if constexpr (Hasload<T>)
					return Value.load(Stm, RegisterForChange);

				else if constexpr (requires { typename PhobosStreamObject<T>; }) // fallback only if exists
				{
					PhobosStreamObject<T> item;
					return item.ReadFromStream(Stm, Value, RegisterForChange);
				}
				else
				{
					static_assert(HasLoad<T> || Hasload<T>,
								  "ReadFromStream: Type must implement Load/load returning bool, "
								  "or specialize PhobosStreamObject<T>.");
				}
			}

			template <typename T>
			static bool WriteToStream(PhobosStreamWriter& Stm, const T& Value)
			{
				if constexpr (HasSave<T>)
					return Value.Save(Stm);

				else if constexpr (Hassave<T>)
					return Value.save(Stm);

				else if constexpr (requires { typename PhobosStreamObject<T>; }) // fallback only if exists
				{
					PhobosStreamObject<T> item;
					return item.WriteToStream(Stm, Value);
				}
				else
				{
					static_assert(HasSave<T> || Hassave<T>,
								  "WriteToStream: Type must implement Save/save returning bool, "
								  "or specialize PhobosStreamObject<T>.");
				}
			}
		};
	}

	template <typename T>
	bool ReadPhobosStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange)
	{
		return detail::Selector::ReadFromStream(Stm, Value, RegisterForChange);
	}

	template <typename T>
	bool WritePhobosStream(PhobosStreamWriter& Stm, const T& Value)
	{
		return detail::Selector::WriteToStream(Stm, Value);
	}

	template <typename T>
	bool PersistObject(PhobosStreamWriter& Stm, const T* pValue)
	{
		if (!Savegame::WritePhobosStream(Stm, pValue))
			return false;

		if (pValue)
			return Savegame::WritePhobosStream(Stm, *pValue);

		return true;
	}

	template <typename T>
	bool PhobosStreamObject<T>::ReadFromStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange) const
	{
		bool ret = Stm.Load(Value);

		if COMPILETIMEEVAL(std::is_pointer<T>::value) {
			if (RegisterForChange) {
				PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP(Value, PhobosCRT::GetTypeIDName<T>().c_str());
			}
		}

		return ret;
	}

	template <typename T>
	bool PhobosStreamObject<T>::WriteToStream(PhobosStreamWriter& Stm, const T& Value) const
	{
		Stm.Save(Value);
		return true;
	}


#pragma region Spe

	template <typename T, size_t N>
	struct Savegame::PhobosStreamObject<T[N]>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, T(&Value)[N], bool RegisterForChange) const
		{
			// Read each element of the array
			for (size_t i = 0; i < N; ++i) {
				if (!Savegame::ReadPhobosStream(Stm, Value[i], RegisterForChange)) {
					return false;
				}
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const T(&Value)[N]) const
		{
			// Write each element of the array
			for (size_t i = 0; i < N; ++i) {
				if (!Savegame::WritePhobosStream(Stm, Value[i])) {
					return false;
				}
			}
			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<IndexBitfield<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, IndexBitfield<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.data, RegisterForChange))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const IndexBitfield<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.data))
				return false;

			return true;
		}
	};


	template <typename T>
	struct Savegame::PhobosStreamObject<MinMaxValue<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, MinMaxValue<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.Min, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Max, RegisterForChange))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const MinMaxValue<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.Min))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Max))
				return false;


			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<PartialVector4D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, PartialVector4D<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.X, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Y, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Z, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.W, RegisterForChange))
				return false;

			if (!Stm.Load(Value.ValueCount))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const PartialVector4D<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Y))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Z))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.W))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.ValueCount))
				return false;

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<PartialVector3D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, PartialVector3D<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.X, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Y, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Z, RegisterForChange))
				return false;

			if (!Stm.Load(Value.ValueCount))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const PartialVector3D<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Y))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Z))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.ValueCount))
				return false;

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<PartialVector2D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, PartialVector2D<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm , Value.X , RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Y, RegisterForChange))
				return false;

			if (!Stm.Load(Value.ValueCount))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const PartialVector2D<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Y))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.ValueCount))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<bool>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, bool& Value, bool RegisterForChange) const
		{
			int rad = 0;
			if (!Stm.Load(rad))
				return false;
			Value = (rad != 0);  // Convert int to bool: 0=false, non-zero=true
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const bool& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, (int)Value))  // true=1, false=0
				return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<CellStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, CellStruct& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.X, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Y, RegisterForChange))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const CellStruct& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Y))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<Leptons>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, Leptons& Value, bool RegisterForChange) const
		{
			return Stm.Load(Value.value);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Leptons& Value) const
		{
			Stm.Save(Value.value);
			return true;
		}
	};

	template <size_t Size , typename T>
	struct Savegame::PhobosStreamObject<FixedString<Size, T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, FixedString<Size, T>& Value, bool RegisterForChange) const
		{
			// Read the fixed-size buffer directly
			T buffer[Size] = {};
			if (!Stm.Read(reinterpret_cast<PhobosByteStream::data_t*>(buffer), Size))
				return false;

			// Ensure null termination and assign
			buffer[Size - 1] = '\0';
			Value.assign(buffer);
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const FixedString<Size, T>& Value) const
		{
			// Write the fixed-size buffer directly
			T buffer[Size] = {};

			// Copy string data, ensuring it fits
			const T* str = Value.operator const T*();
			size_t len = std::min(strlen(str), Size - 1);
			std::memcpy(buffer, str, len);
			// buffer is already zero-initialized, so null termination is guaranteed

			Stm.Write(reinterpret_cast<const PhobosByteStream::data_t*>(buffer), Size);
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<size_t>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, size_t& Value, bool RegisterForChange) const
		{
			int rad = 0;
			if (!Savegame::ReadPhobosStream(Stm, rad))
				return false;

			Value = rad;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const size_t& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, (int)Value))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<unsigned short>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, unsigned short& Value, bool RegisterForChange) const
		{
			int rad = 0;
			if (!Savegame::ReadPhobosStream(Stm, rad))
				return false;

			Value = rad;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const unsigned short& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, (int)Value))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<short>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, short& Value, bool RegisterForChange) const
		{
			int rad = 0;
			if (!Savegame::ReadPhobosStream(Stm, rad))
				return false;

			Value = rad;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const short& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, (int)Value))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<DirStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, DirStruct& Value, bool RegisterForChange) const
		{
			int rad = 0;
			if (!Savegame::ReadPhobosStream(Stm, rad))
				return false;

			Value.Raw = rad;
			// Note: Pad is not serialized as it appears to be just alignment padding
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const DirStruct& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, (int)Value.Raw))
				return false;

			// Note: Pad is not serialized as it appears to be just alignment padding
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<CoordStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, CoordStruct& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Y))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Z))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const CoordStruct& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Y))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Z))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<Point2D>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, Point2D& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Y))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Point2D& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Y))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<Point2DBYTE>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, Point2DBYTE& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Y))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Point2DBYTE& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Y))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<Point3D>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, Point3D& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Y))
				return false;

			if (Savegame::ReadPhobosStream(Stm, Value.Z))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Point3D& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Y))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Z))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<ColorStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, ColorStruct& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.R))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.G))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.B))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const ColorStruct& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.R))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.G))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.B))
				return false;

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<VectorClass<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, VectorClass<T>& Value, bool RegisterForChange) const
		{
			Value.Clear();
			int Capacity = 0;

			if (!Stm.Load(Capacity))
				return false;

			Value.Reserve(Capacity);

			for (auto ix = 0; ix < Capacity; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value.Items[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const VectorClass<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.Capacity))
				return false;

			for (auto ix = 0; ix < Value.Capacity; ++ix) {
				if (!Savegame::WritePhobosStream(Stm, Value.Items[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<DynamicVectorClass<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, DynamicVectorClass<T>& Value, bool RegisterForChange) const
		{
			Value.Reset();
			int Capacity = 0;

			if (!Stm.Load(Capacity))
				return false;

			Value.Reserve(Capacity);

			if (!Stm.Load(Value.Count) || !Stm.Load(Value.CapacityIncrement))
				return false;

			for (auto ix = 0; ix < Value.Count; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value.Items[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const DynamicVectorClass<T>& Value) const
		{
			Stm.Save(Value.Capacity);
			Stm.Save(Value.Count);
			Stm.Save(Value.CapacityIncrement);

			for (auto ix = 0; ix < Value.Count; ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value.Items[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<TypeList<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, TypeList<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream<DynamicVectorClass<T>>(Stm, Value, RegisterForChange))
				return false;

			return Stm.Load(Value.unknown_18);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const TypeList<T>& Value) const
		{
			if (!Savegame::WritePhobosStream<DynamicVectorClass<T>>(Stm, Value))
				return false;

			Stm.Save(Value.unknown_18);
			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<Vector3D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, Vector3D<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.X, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Y, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Z, RegisterForChange))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Vector3D<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Y))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Z))
				return false;

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<Vector2D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, Vector2D<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.X, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Y, RegisterForChange))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Vector2D<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Y))
				return false;

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<Vector4D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, Vector4D<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.X, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Y, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.Z, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.W, RegisterForChange))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Vector4D<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.X))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Y))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Z))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.W))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<WeaponStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, WeaponStruct& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.WeaponType, RegisterForChange)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.FLH, RegisterForChange)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.BarrelLength, RegisterForChange)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.BarrelThickness, RegisterForChange)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.TurretLocked, RegisterForChange)) return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const WeaponStruct& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.WeaponType)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.FLH)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.BarrelLength)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.BarrelThickness)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.TurretLocked)) return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<TintStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, TintStruct& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.Red, RegisterForChange)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.Green, RegisterForChange)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.Blue, RegisterForChange)) return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const TintStruct& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.Red)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.Green)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.Blue)) return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<LightingStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, LightingStruct& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.Tint, RegisterForChange)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.Ground, RegisterForChange)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.Level, RegisterForChange)) return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const LightingStruct& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.Tint)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.Ground)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.Level)) return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<CounterClass>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, CounterClass& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream<VectorClass<int>>(Stm, Value, RegisterForChange))
				return false;

			return Stm.Load(Value.Total);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const CounterClass& Value) const
		{
			if (!Savegame::WritePhobosStream<VectorClass<int>>(Stm, Value))
				return false;

			Stm.Save(Value.Total);
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<ScriptActionNode>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, ScriptActionNode& Value, bool RegisterForChange) const {

			if (!Stm.Load(Value.Action))
				return false;

			if (!Stm.Load(Value.Argument))
				return false;

			return true;
		};

		bool WriteToStream(PhobosStreamWriter& Stm, const ScriptActionNode& Value) const {
			if (!Savegame::WritePhobosStream(Stm, Value.Action))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.Argument))
				return false;

			return true;
		};
	};

	template <>
	struct Savegame::PhobosStreamObject<std::string>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::string& Value, bool RegisterForChange) const
		{
			Value.clear();

			// Get underlying stream and check integrity
			//auto* stream = Stm.Getstream();
			//size_t streamOffset = stream->Offset();
			//GameDebugLog::Log("[std::string] READ start at offset %zu\n", streamOffset);

			int size = 0;
			if (!Stm.Load(size))
			{
				//GameDebugLog::Log("[std::string] CRITICAL: Failed to load size at offset %zu\n", streamOffset);
				return false;
			}

			//size_t afterSizeOffset = stream->Offset();
			//GameDebugLog::Log("[std::string] Loaded size=%zu, offset %zu->%zu (+%zu bytes)\n",
			//	size, streamOffset, afterSizeOffset, afterSizeOffset - streamOffset);

			// Sanity checks for corruption
			if (size > 10000)
			{
				GameDebugLog::Log("[std::string] CRITICAL: size %zu seems too large! Possible corruption\n", size);
				__debugbreak();
				return false;
			}

			if (size == -1)
			{
				GameDebugLog::Log("[std::string] CRITICAL: size is -1! Stream corruption detected\n");
				__debugbreak();
				return false;
			}

			if (!size)
			{
				//GameDebugLog::Log("[std::string] Empty string loaded successfully\n");
				return true;
			}

			std::vector<char> buffer(size);
			if (!Stm.Read(reinterpret_cast<PhobosByteStream::data_t*>(buffer.data()), (size_t)size))
			{
				GameDebugLog::Log("[std::string] CRITICAL: Failed to read %zu bytes of string data\n", size);
				return false;
			}

			Value.assign(buffer.begin(), buffer.end());
			//GameDebugLog::Log("[std::string] Successfully loaded: '%s' (length %zu)\n",
			//	Value.c_str(), Value.length());

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::string& Value) const
		{
			// Get underlying stream
			//auto* stream = Stm.Getstream();

			//size_t streamSizeBefore = stream->Size();
			//GameDebugLog::Log("[std::string] WRITE start at size %zu, string='%s' (length %zu)\n",
			//	streamSizeBefore, Value.c_str(), Value.size());

			// Store size in local variable to prevent optimization issues
			const int stringSize = Value.size();

			//GameDebugLog::Log("[std::string] About to save size %zu\n", stringSize);

			// Write size with validation
			if (!Stm.Save(stringSize))
			{
				//GameDebugLog::Log("[std::string] CRITICAL: Failed to save size %zu\n", stringSize);
				return false;
			}

			//size_t afterSizeWrite = stream->Size();
			//size_t sizeBytes = afterSizeWrite - streamSizeBefore;
			//GameDebugLog::Log("[std::string] Wrote size=%zu, stream %zu->%zu (+%zu bytes)\n",
			//	stringSize, streamSizeBefore, afterSizeWrite, sizeBytes);

			// Validate size was written correctly
			//if (sizeBytes != sizeof(size_t))
			//{
			//	GameDebugLog::Log("[std::string] ERROR: Expected %zu bytes for size_t, but wrote %zu bytes\n",
			//		sizeof(size_t), sizeBytes);
			//	__debugbreak();
			//	return false;
			//}

			if (Value.empty())
			{
				//GameDebugLog::Log("[std::string] Empty string, not writing data\n");
				//size_t totalBytes = afterSizeWrite - streamSizeBefore;
				//GameDebugLog::Log("[std::string] Total bytes written: %zu\n", totalBytes);
				return true;
			}

			//GameDebugLog::Log("[std::string] About to write %zu bytes of string data\n", stringSize);

			// Write string data with validation
			const char* dataPtr = Value.c_str();

			if (!Stm.Write(reinterpret_cast<const PhobosByteStream::data_t*>(dataPtr), (size_t)stringSize))
			{
				//GameDebugLog::Log("[std::string] CRITICAL: Failed to write %zu bytes of string data\n", stringSize);
				return false;
			}

			//size_t finalSize = stream->Size();
			//size_t totalBytesWritten = finalSize - streamSizeBefore;
			//size_t expectedTotal = sizeof(size_t) + stringSize;

			//GameDebugLog::Log("[std::string] Wrote data, final size %zu (+%zu total bytes)\n",
			//	finalSize, totalBytesWritten);

			// Critical validation - this is where we catch the corruption
			//if (totalBytesWritten != expectedTotal)
			//{
			//	GameDebugLog::Log("[std::string] CRITICAL CORRUPTION: Expected %zu total bytes, but wrote %zu bytes\n",
			//		expectedTotal, totalBytesWritten);
			//	GameDebugLog::Log("[std::string] Size bytes: %zu, String bytes: %zu, Actual total: %zu\n",
			//		sizeof(size_t), stringSize, totalBytesWritten);
			//	__debugbreak();
			//	return false;
			//}

			//GameDebugLog::Log("[std::string] Successfully wrote string '%s' (%zu + %zu = %zu bytes)\n",
			//	Value.c_str(), sizeof(size_t), stringSize, totalBytesWritten);

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<std::string_view>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::string_view& Value, bool RegisterForChange) const
		{
			static_assert(true, "Not Implemented !");
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::string_view & Value) const
		{
			static_assert(true, "Not Implemented !");
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<std::wstring>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::wstring& Value, bool RegisterForChange) const
		{
			Value.clear();
			int size = 0;

			if (Stm.Load(size)) {
				if (!size) {
					return true;
				}

				std::vector<wchar_t> buffer(size);
				if (Stm.Read(reinterpret_cast<PhobosByteStream::data_t*>(buffer.data()), size * sizeof(wchar_t)))
				{
					Value.assign(buffer.begin(), buffer.end());
					return true;
				}
			}

			return false;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::wstring& Value) const
		{
			Stm.Save((int)Value.size());

			if (Value.empty())
				return true;

			Stm.Write(reinterpret_cast<const PhobosByteStream::data_t*>(Value.c_str()), Value.size() * sizeof(wchar_t));
			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::unique_ptr<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::unique_ptr<T>& Value, bool RegisterForChange) const
		{
			T* ptrOld = nullptr;
			if (!Stm.Load(ptrOld))
				return false;

			if (ptrOld) {

				std::unique_ptr<T> ptrNew = ObjectFactory<T>()(Stm);

				if (Savegame::ReadPhobosStream(Stm, *ptrNew, RegisterForChange)) {
					PHOBOS_SWIZZLE_REGISTER_POINTER((long)ptrOld, ptrNew.get(), PhobosCRT::GetTypeIDName<T>().c_str())
					ptrOld = ptrNew.release();
				}
			}

			Value.reset(ptrOld);
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unique_ptr<T>& Value) const
		{
			return PersistObject(Stm, Value.get());
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<MemoryPoolUniquePointer<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, MemoryPoolUniquePointer<T>& Value, bool RegisterForChange) const
		{
			T* ptrOld = nullptr;
			if (!Stm.Load<T*>(ptrOld))
				return false;

			if (ptrOld) {

				MemoryPoolUniquePointer<T> ptrNew = ObjectFactory<T>()(Stm);

				if (Savegame::ReadPhobosStream(Stm, *ptrNew, RegisterForChange)) {
					PHOBOS_SWIZZLE_REGISTER_POINTER((long)ptrOld, ptrNew.get(), PhobosCRT::GetTypeIDName<T>().c_str())
					ptrOld = ptrNew.release();
				}
			}

			Value.reset(ptrOld);
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const MemoryPoolUniquePointer<T>& Value) const
		{
			return PersistObject(Stm, Value.get());
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<UniqueGamePtr<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, UniqueGamePtr<T>& Value, bool RegisterForChange) const
		{
			T* ptrOld = nullptr;
			if (!Stm.Load<T*>(ptrOld))
				return false;

			if (ptrOld) {

				UniqueGamePtr<T> ptrNew = ObjectFactory<T>()(Stm);

				if (Savegame::ReadPhobosStream(Stm, *ptrNew, RegisterForChange)) {
					PHOBOS_SWIZZLE_REGISTER_POINTER((long)ptrOld, ptrNew.get(), PhobosCRT::GetTypeIDName<T>().c_str())
					ptrOld = ptrNew.release();
				}
			}

			Value.reset(ptrOld);
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const UniqueGamePtr<T>& Value) const
		{
			return PersistObject(Stm, Value.get());
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<UniqueGamePtr<BytePalette>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, UniqueGamePtr<BytePalette>& Value, bool RegisterForChange) const
		{
			bool hasvalue = false;
			if (!Savegame::ReadPhobosStream(Stm, hasvalue))
				return false;

			if (hasvalue) {
				BytePalette* ptrOld = nullptr;
				if (!Stm.Load(ptrOld))
					return false;

				auto ptrNew = GameCreate<BytePalette>();
				for (int i = 0; i < BytePalette::EntriesCount; ++i) {
					if (!Savegame::ReadPhobosStream(Stm, ptrNew->Entries[i], RegisterForChange))
						return false;
				}

				Value.reset(ptrNew);

			} else {
				Value.reset(nullptr);
			}
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const UniqueGamePtr<BytePalette>& Value) const
		{
			const bool Exist = Value.get() != nullptr;
			if (!Savegame::WritePhobosStream(Stm, Exist))
				return false;

			if (Exist) {
				if (!Savegame::WritePhobosStream(Stm, Value.get()))
					return false;

				for (int i = 0; i < BytePalette::EntriesCount; ++i) {

					if (!Savegame::WritePhobosStream(Stm, Value.get()->Entries[i]))
						return false;
				}
			}
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<SHPStruct*>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, SHPStruct*& Value, bool RegisterForChange) const
		{
			bool HasAny = false;
			//GameDebugLog::Log("[SHPStruct] Loading HasAny...\n");

			if (!Savegame::ReadPhobosStream(Stm, HasAny)) {
				//GameDebugLog::Log("[SHPStruct] Failed to load HasAny!\n");
				return false;
			}

			//GameDebugLog::Log("[SHPStruct] HasAny = %s\n", HasAny ? "true" : "false");

			if (!HasAny) {
				Value = nullptr;
				return true;
			}

			Value = nullptr;
			std::string name {};
			//GameDebugLog::Log("[SHPStruct] Loading filename...\n");

			if (!Savegame::ReadPhobosStream(Stm, name, RegisterForChange)) {
				//GameDebugLog::Log("[SHPStruct] Failed to load filename!\n");
				return false;
			}

			//GameDebugLog::Log("[SHPStruct] Loaded filename: '%s' (length: %zu)\n",
			//	name.c_str(), name.length());

			if (auto pSHP = FileSystem::LoadSHPFile(name.c_str())) {
				Value = pSHP;
				//GameDebugLog::Log("[SHPStruct] Successfully loaded SHP\n");
				return true;
			} else {
				//GameDebugLog::Log("[SHPStruct] Warning: Could not load SHP '%s', setting to nullptr\n", name.c_str());
				Value = nullptr;
				return true;  // This is likely where it's failing
			}
		}

		bool WriteToStream(PhobosStreamWriter& Stm, SHPStruct* const& Value) const
		{
			const bool HasAny = Value != nullptr;
			if (!Savegame::WritePhobosStream(Stm, HasAny))
				return false;

			if(!HasAny)
				return true;

			//GameDebugLog::Log("[SHPStruct] Value = %p\n", Value);

			const char* filename = nullptr;
			if (auto pRef = Value->AsReference()) {
				filename = pRef->Filename;
				//GameDebugLog::Log("[SHPStruct] AsReference() = %p\n", pRef);
				//GameDebugLog::Log("[SHPStruct] Filename = %s\n", pRef->Filename ? pRef->Filename : "NULL");
			}

			if (!filename)
				Debug::FatalErrorAndExit("Invalid SHP !");

			std::string file(filename);
			return Savegame::WritePhobosStream(Stm, file);
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<RocketStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, RocketStruct& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.PauseFrames)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.TiltFrames)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.PitchInitial)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.PitchFinal)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.TurnRate)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.RaiseRate)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.Acceleration)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.Altitude)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.Damage)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.EliteDamage))	return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.BodyLength)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.LazyCurve)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.Type, RegisterForChange)) return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const RocketStruct& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.PauseFrames)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.TiltFrames)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.PitchInitial)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.PitchFinal)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.TurnRate)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.RaiseRate)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.Acceleration)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.Altitude)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.Damage)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.EliteDamage))	return false;
			if (!Savegame::WritePhobosStream(Stm, Value.BodyLength)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.LazyCurve)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.Type)) return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<BuildType>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, BuildType& Value, bool RegisterForChange) const
		{
			if (!Stm.Read(reinterpret_cast<PhobosByteStream::data_t*>(&Value), sizeof(BuildType)))
				return false;

			if (RegisterForChange)
				PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP(Value.CurrentFactory, "BuildType::Factory")

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const BuildType& Value) const
		{
			Stm.Write(reinterpret_cast<const PhobosByteStream::data_t*>(&Value), sizeof(BuildType));
			return true;
		}
	};

	template <size_t Size>
	struct Savegame::PhobosStreamObject<std::bitset<Size>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::bitset<Size>& Value, bool RegisterForChange) const
		{
			unsigned char value = 0;
			for (auto i = 0u; i < Size; ++i)
			{
				auto pos = i % 8;

				if (pos == 0 && !Stm.Load(value))
					return false;

				Value.set(i, ((value >> pos) & 1) != 0);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::bitset<Size>& Value) const
		{
			unsigned char value = 0;
			for (auto i = 0u; i < Size; ++i)
			{
				auto pos = i % 8;

				if (Value[i])
					value |= 1 << pos;

				if (pos == 7 || i == Size - 1)
				{
					Stm.Save(value);
					value = 0;
				}
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::optional<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::optional<T>& Value, bool RegisterForChange) const
		{
			Value.reset();
			bool HasValue = false;
			if (Savegame::ReadPhobosStream(Stm, HasValue))
			{
				if (!HasValue)
				{
					return true;
				}

				T nOld {};
				if (Savegame::ReadPhobosStream(Stm, nOld, RegisterForChange))
				{
					Value = nOld;
					return true;
				}
			}

			return false;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::optional<T>& Value) const
		{
			Savegame::WritePhobosStream(Stm, Value.has_value());
			if (Value.has_value())
				return (Savegame::WritePhobosStream(Stm, Value.value()));

			return true;
		}
	};

	template <typename T, typename Alloc>
	struct Savegame::PhobosStreamObject<std::vector<T, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::vector<T, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			auto name = PhobosCRT::GetTypeIDName<T>();

			if (!Stm.Load(Count)) {
				Debug::Log("Vector %s load failed to read count\n", name.c_str());
				return false;
			}

			Debug::Log("Vector %s loading %d elements \n", name.c_str(), Count);

			Value.resize(Count);

			for (auto ix = 0u; ix < Count; ++ix) {
				Debug::Log("Loading vector %s element %u/%d\n", name.c_str(), ix, Count);

				if COMPILETIMEEVAL(std::is_same_v<T, bool>) {
					bool temp {};

					if (!Savegame::ReadPhobosStream(Stm, temp, RegisterForChange)) {
						Debug::Log("Failed to load vector %s element %u\n", name.c_str(), ix);
						return false;
					}

					Value[ix] = temp;
				} else {
					if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange)) {
						Debug::Log("Failed to load vector %s element %u\n", name.c_str(), ix);
						return false;
					}
				}
			}

			Debug::Log("Successfully loaded vector %s with %d elements\n", name.c_str(), Count);
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::vector<T, Alloc>& Value) const
		{
			Stm.Save((int)Value.size());

			for (auto ix = 0u; ix < Value.size(); ++ix) {
				if (!Savegame::WritePhobosStream(Stm, Value[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename _Ty1, typename _Ty2>
	struct Savegame::PhobosStreamObject<std::pair<_Ty1, _Ty2>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::pair<_Ty1, _Ty2>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.first, RegisterForChange))
				return false;

			if (!Savegame::ReadPhobosStream(Stm, Value.second, RegisterForChange))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::pair<_Ty1, _Ty2>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.first))
				return false;

			if (!Savegame::WritePhobosStream(Stm, Value.second))
				return false;

			return true;
		}
	};

	template <typename TKey, typename TValue>
	struct Savegame::PhobosStreamObject<std::map<TKey, TValue>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::map<TKey, TValue>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Stm.Load(Count))
				return false;

			for (auto ix = 0u; ix < Count; ++ix)
			{
				std::pair<TKey, TValue> buffer;
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
					return false;

				Value.insert(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::map<TKey, TValue>& Value) const
		{
			int const nSize = Value.size();
			Stm.Save(nSize);

			if (nSize) {
				for (const auto& [first, second] : Value) {
					if (!Savegame::WritePhobosStream(Stm, first))
						return false;

					if (!Savegame::WritePhobosStream(Stm, second))
						return false;
				}
			}
			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::set<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::set<T>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Stm.Load(Count)) {
				return false;
			}

			if (!Count)
				return true;
			//Debug::LogInfo("Loading std::set with(%s) size %d", typeid(T).name(), Count);

			if COMPILETIMEEVAL(std::is_pointer<T>::value) {
				std::vector<T> buffer(Count, nullptr);
				for (auto ix = 0u; ix < Count; ++ix) {
					if (!Savegame::ReadPhobosStream(Stm, buffer[ix], RegisterForChange)) {
						return false;
					}
				}

				Value.insert(buffer.begin(), buffer.end());
			} else{

				for (auto ix = 0u; ix < Count; ++ix) {

					T buffer = T();
					if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
					{
						return false;
					}
					Value.insert(buffer);
				}
			}
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::set<T>& Value) const
		{
			Stm.Save((int)Value.size());
			//Debug::LogInfo("Saving std::set with(%s) size %d", typeid(T).name(), Value.size());

			if(Value.empty()){
				for (const auto& item : Value) {
					if (!Savegame::WritePhobosStream(Stm, item)) {
						return false;
					}
				}
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::list<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::list<T>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Stm.Load(Count)) {
				return false;
			}

			for (auto ix = 0u; ix < Count; ++ix) {
				T buffer = T();
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange)) {
					return false;
				}
				Value.push_back(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::list<T>& Value) const
		{
			Stm.Save((int)Value.size());

			if(!Value.empty()){
				for (const auto& item : Value) {
					if (!Savegame::WritePhobosStream(Stm, item)) {
						return false;
					}
				}
			}
			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::deque<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::deque<T>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int count = 0;

			if (!Stm.Load(count))
				return false;

			if (!count)
				return true;

			Value.resize(count);

			for (auto ix = 0u; ix < count; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::deque<T>& Value) const
		{
			Stm.Save((int)Value.size());

			for (auto ix = 0u; ix < Value.size(); ++ix) {
				if (!Savegame::WritePhobosStream(Stm, Value[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::queue<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::queue<T>& Value, bool RegisterForChange) const
		{
			int nSize = 0;
			if (!Stm.Load(nSize))
				return false;

			if (!nSize)
				return true;

			for (int ix = 0u; ix < nSize; ++ix)
			{
				T buffer { };
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
					return false;

				Value.push(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::queue<T>& Value) const
		{
			Stm.Save((int)Value.size());

			if (Value.size() > 0)
			{
				//make an copy
				std::queue<T> Quee = Value;

				while (!Quee.empty())
				{
					if (!Savegame::WritePhobosStream(Stm, Quee.front()))
						return false;

					Quee.pop();
				}
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::priority_queue<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::priority_queue<T>& Value, bool RegisterForChange) const
		{
			int nSize = 0;
			if (!Stm.Load(nSize))
				return false;

			if (!nSize)
				return true;


			for (int ix = 0u; ix < nSize; ++ix)
			{
				T buffer { };
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
					return false;

				Value.push(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::priority_queue<T>& Value) const
		{
			Stm.Save((int)Value.size());

			if (Value.size() > 0)
			{
				std::priority_queue<T> Quee = Value;

				while (!Quee.empty())
				{
					if (!Savegame::WritePhobosStream(Stm, Quee.top()))
						return false;

					Quee.pop();
				}
			}

			return true;
		}
	};

	template <typename T , size_t size>
	struct Savegame::PhobosStreamObject<std::array<T, size>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::array<T, size>& Value, bool RegisterForChange) const
		{
			__stosb(reinterpret_cast<PhobosByteStream::data_t*>(Value.data()), 0, sizeof(T) * size);

			for (auto ix = 0u; ix < size; ++ix) {
				if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange)) {
					return false;
				}
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::array<T, size>& Value) const
		{
			for (const auto& item : Value) {
				if (!Savegame::WritePhobosStream(Stm, item)) {
					return false;
				}
			}

			return true;
		}
	};

	template <typename TKey, typename TValue>
	struct Savegame::PhobosStreamObject<std::unordered_map<TKey, TValue>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::unordered_map<TKey, TValue>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Stm.Load(Count)) {
				return false;
			}

			if (!Count)
				return true;

			for (auto ix = 0u; ix < Count; ++ix)
			{
				std::pair<TKey, TValue> buffer;
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange)) {
					return false;
				}

				Value.insert(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unordered_map<TKey, TValue>& Value) const
		{
			int const nSize = Value.size();
			Stm.Save(nSize);

			if (!nSize)
				return true;

			for (const auto&[first , second] : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, first))
					return false;

				if(!Savegame::WritePhobosStream(Stm, second))
					return false;
			}

			return true;
		}
	};

	template <typename TKey, typename TValue, typename Cmp>
	struct Savegame::PhobosStreamObject<std::multimap<TKey, TValue, Cmp>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::multimap<TKey, TValue, Cmp>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Stm.Load(Count)) {
				return false;
			}

			if (!Count)
				return true;

			for (auto ix = 0u; ix < Count; ++ix)
			{
				TKey key = TKey();

				if (!Savegame::ReadPhobosStream(Stm, key, false))
					return false;

				Value.emplace(key, TValue());
				auto it = Value.end();
				--it;

				if (!Savegame::ReadPhobosStream(Stm, it->second, RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::multimap<TKey, TValue, Cmp>& Value) const
		{
			Stm.Save((int)Value.size());

			if(!Value.empty()){
				for (const auto&[key , val] : Value)
				{
					if (!Savegame::WritePhobosStream(Stm, key))
						return false;

					if (!Savegame::WritePhobosStream(Stm, val))
						return false;

				}
			}
			return true;
		}
	};

#pragma endregion

}

#define DefaultSaveLoadFunc(cls) \
bool Load(PhobosStreamReader& Stm, bool RegisterForChange) { return this->Serialize(Stm); } \
bool Save(PhobosStreamWriter& Stm) const { return const_cast<cls*>(this)->Serialize(Stm); }