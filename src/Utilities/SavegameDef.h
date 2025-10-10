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
#include <Utilities/VectorHelper.h>

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

		// General array type detection
		template <typename T>
		concept IsFixedArray = std::is_array_v<T> && std::extent_v<T> > 0;

		// More specific array type concepts (if you need special handling)
		template <typename T>
		concept IsCharArray = IsFixedArray<T> && std::is_same_v<std::remove_extent_t<T>, char>;

		template <typename T>
		concept IsWCharArray = IsFixedArray<T> && std::is_same_v<std::remove_extent_t<T>, wchar_t>;

		// SFINAE helper to detect PhobosStreamObject specializations
		template<typename T>
		struct has_phobos_stream_object_specialization
		{
		private:
			// Test if we can instantiate PhobosStreamObject<T> and call its methods
			template<typename U>
			static auto test_read(int) -> decltype(
				std::declval<PhobosStreamObject<U>>().ReadFromStream(
					std::declval<PhobosStreamReader&>(),
					std::declval<U&>(),
					std::declval<bool>()
				), std::true_type {}
			);

			template<typename U>
			static std::false_type test_read(...);

			template<typename U>
			static auto test_write(int) -> decltype(
				std::declval<PhobosStreamObject<U>>().WriteToStream(
					std::declval<PhobosStreamWriter&>(),
					std::declval<const U&>()
				), std::true_type {}
			);

			template<typename U>
			static std::false_type test_write(...);

		public:
			static constexpr bool read_value = decltype(test_read<T>(0))::value;
			static constexpr bool write_value = decltype(test_write<T>(0))::value;
			static constexpr bool value = read_value && write_value;
		};

		template<typename T>
		constexpr bool has_phobos_stream_object_v = has_phobos_stream_object_specialization<T>::value;

		struct Selector
		{
			template <typename T>
			static bool ReadFromStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange)
			{
				if constexpr (HasLoad<T>)
					return Value.Load(Stm, RegisterForChange);
				else if constexpr (Hasload<T>)
					return Value.load(Stm, RegisterForChange);
				else if constexpr (IsFixedArray<T>)
				{
					static_assert(std::is_same_v<T, T>, "Detected as fixed array");

					if constexpr (has_phobos_stream_object_v<T>)
					{
						static_assert(std::is_same_v<T, T>, "Has PhobosStreamObject specialization - using it");
						PhobosStreamObject<T> item;
						return item.ReadFromStream(Stm, Value, RegisterForChange);
					}
					else
					{
						static_assert(has_phobos_stream_object_v<T>,
									  "ARRAY TYPE DETECTED but no PhobosStreamObject specialization found. "
									  "Create PhobosStreamObject<T[N]> specialization for your array type.");
						return false;
					}
				}
				else if constexpr (has_phobos_stream_object_v<T>)
				{
					static_assert(std::is_same_v<T, T>, "Using generic PhobosStreamObject");
					// Handle other types with PhobosStreamObject specialization
					PhobosStreamObject<T> item;
					return item.ReadFromStream(Stm, Value, RegisterForChange);
				}
				else
				{
					static_assert(HasLoad<T> || Hasload<T> || has_phobos_stream_object_v<T> || IsFixedArray<T>,
								  "ReadFromStream: Type must implement Load/load returning bool, "
								  "or specialize PhobosStreamObject<T>.");
					return false;
				}
			}

			template <typename T>
			static bool WriteToStream(PhobosStreamWriter& Stm, const T& Value)
			{
				if constexpr (HasSave<T>)
					return Value.Save(Stm);
				else if constexpr (Hassave<T>)
					return Value.save(Stm);
				else if constexpr (IsFixedArray<T>)
				{
					if constexpr (has_phobos_stream_object_v<T>)
					{
						PhobosStreamObject<T> item;
						return item.WriteToStream(Stm, Value);
					}
					else
					{
						static_assert(has_phobos_stream_object_v<T>,
									  "ARRAY TYPE DETECTED but no PhobosStreamObject specialization found. "
									  "Create PhobosStreamObject<T[N]> specialization for your array type.");
						return false;
					}
				}
				else if constexpr (has_phobos_stream_object_v<T>)
				{
					// Handle other types with PhobosStreamObject specialization
					PhobosStreamObject<T> item;
					return item.WriteToStream(Stm, Value);
				}
				else
				{
					static_assert(HasSave<T> || Hassave<T> || has_phobos_stream_object_v<T> || IsFixedArray<T>,
								  "WriteToStream: Type must implement Save/save returning bool, "
								  "or specialize PhobosStreamObject<T>.");
					return false;
				}
			}
		};
	}

	template<typename T>
	concept IsDataTheTypeCorrect = !Savegame::detail::IsFixedArray<T>;

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
	bool PhobosStreamObject<T>::ReadFromStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange) const
	{
		if constexpr (IsDataTheTypeCorrect<T>)
		{
			bool ret = Stm.Load(Value);
			if COMPILETIMEEVAL(std::is_pointer<T>::value) {
				if (RegisterForChange) {
					PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP(Value, PhobosCRT::GetTypeIDName<T>().c_str());
				}
			}

			return ret;
		} else {
			static_assert(IsDataTheTypeCorrect<T>, "Use specialization for this type");
			return false;
		}
	}

	template <typename T>
	bool PhobosStreamObject<T>::WriteToStream(PhobosStreamWriter& Stm, const T& Value) const
	{
		if constexpr (IsDataTheTypeCorrect<T>)
		{ 	return Stm.Save(Value); } else {
			static_assert(IsDataTheTypeCorrect<T>, "Use specialization for this type");
			return false;
		}
	}


#pragma region Spe

	template <size_t Size>
	struct Savegame::PhobosStreamObject<char[Size]>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, char(&Value)[Size], bool RegisterForChange) const
		{
			// Use existing std::string template
			std::string temp {};
			if (!Savegame::ReadPhobosStream(Stm, temp, RegisterForChange))
				return false;

			// Copy to fixed array with bounds checking
			size_t copyLen = std::min(temp.length(), Size - 1);
			std::memcpy(Value, temp.c_str(), copyLen);
			Value[copyLen] = '\0';  // Ensure null termination

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const char(&Value)[Size]) const
		{
			// Convert to std::string and use existing template
			std::string temp(Value, strnlen(Value, Size));
			return Savegame::WritePhobosStream(Stm, temp);
		}
	};

	template <size_t Size>
	struct Savegame::PhobosStreamObject<wchar_t[Size]>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, wchar_t(&Value)[Size], bool RegisterForChange) const
		{
			// Use existing std::wstring template
			std::wstring temp {};
			if (!Savegame::ReadPhobosStream(Stm, temp, RegisterForChange))
				return false;

			// Copy to fixed array with bounds checking
			size_t copyLen = std::min(temp.length(), Size - 1);
			std::memcpy(Value, temp.c_str(), copyLen * sizeof(wchar_t));
			Value[copyLen] = L'\0';  // Ensure null termination

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const wchar_t(&Value)[Size]) const
		{
			// Convert to std::wstring and use existing template
			std::wstring temp(Value, wcsnlen(Value, Size));
			return Savegame::WritePhobosStream(Stm, temp);
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<const char*>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, const char*& Value, bool RegisterForChange) const
		{
			static_assert(true, "Not Implemented !");
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const char*& Value) const
		{
			static_assert(true, "Not Implemented !");
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<const wchar_t*>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, const wchar_t*& Value, bool RegisterForChange) const
		{
			static_assert(true, "Not Implemented !");
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const wchar_t*& Value) const
		{
			static_assert(true, "Not Implemented !");
			return true;
		}
	};

	template <typename T, size_t N>
	struct Savegame::PhobosStreamObject<T[N]>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, T(&Value)[N], bool RegisterForChange) const
		{
			// Read each element of the array
			for (size_t i = 0; i < N; ++i) {
				Value[i] = T {};
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
			if (!Savegame::ReadPhobosStream(Stm, Value.ValueCount, RegisterForChange))
				return false;

			for (size_t i = 0; i < Value.ValueCount; ++i)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value[i], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const PartialVector4D<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.ValueCount))
				return false;

			for (size_t i = 0; i < Value.ValueCount; ++i)
			{
				if (!Savegame::WritePhobosStream(Stm, Value[i]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<PartialVector3D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, PartialVector3D<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.ValueCount, RegisterForChange))
				return false;

			for (size_t i = 0; i < Value.ValueCount; ++i)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value[i], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const PartialVector3D<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.ValueCount))
				return false;

			for (size_t i = 0; i < Value.ValueCount; ++i)
			{
				if (!Savegame::WritePhobosStream(Stm, Value[i]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<PartialVector2D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, PartialVector2D<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream(Stm, Value.ValueCount, RegisterForChange))
				return false;

			for (size_t i = 0; i < Value.ValueCount; ++i)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value[i], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const PartialVector2D<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.ValueCount))
				return false;

			for (size_t i = 0; i < Value.ValueCount; ++i)
			{
				if (!Savegame::WritePhobosStream(Stm, Value[i]))
					return false;
			}

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<bool>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, bool& Value, bool RegisterForChange) const
		{
			int rad = 0;
			if (!Savegame::ReadPhobosStream(Stm, rad, RegisterForChange))
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
	struct Savegame::PhobosStreamObject<BYTE>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, BYTE& Value, bool RegisterForChange) const
		{
			int rad = 0;
			if (!Savegame::ReadPhobosStream(Stm, rad, RegisterForChange))
				return false;
			Value = BYTE(rad);
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const BYTE& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, (int)Value))
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
			return Savegame::ReadPhobosStream(Stm, Value.value);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Leptons& Value) const
		{
			return Savegame::WritePhobosStream(Stm, Value.value);
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

			Value = (size_t)rad;
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

			Value = (unsigned short)rad;
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

			Value = (short)rad;
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

			Value.Raw = (unsigned short)rad;
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

			if (!Savegame::ReadPhobosStream(Stm, Value.Z))
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

			if (!Savegame::ReadPhobosStream(Stm, Capacity))
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
			if (!Stm.Save(Value.Capacity))
				return false;

			if (!Stm.Save(Value.Count))
				return false;

			if (!Stm.Save(Value.CapacityIncrement))
				return false;

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

			return Stm.Save(Value.unknown_18);
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
			if (!Stm.Load(Value.Red)) return false;
			if (!Stm.Load(Value.Green)) return false;
			if (!Stm.Load(Value.Blue)) return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const TintStruct& Value) const
		{
			if (!Stm.Save(Value.Red)) return false;
			if (!Stm.Save(Value.Green)) return false;
			if (!Stm.Save(Value.Blue)) return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<LightingStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, LightingStruct& Value, bool RegisterForChange) const
		{
			if (!Stm.Load(Value.Tint)) return false;
			if (!Stm.Load(Value.Ground)) return false;
			if (!Stm.Load(Value.Level)) return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const LightingStruct& Value) const
		{
			if (!Stm.Save(Value.Tint)) return false;
			if (!Stm.Save(Value.Ground)) return false;
			if (!Stm.Save(Value.Level)) return false;
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

			return Stm.Save(Value.Total);
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
			if (!Stm.Save(Value.Action))
				return false;

			if (!Stm.Save(Value.Argument))
				return false;

			return true;
		};
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<MemoryPoolUniquePointer<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, MemoryPoolUniquePointer<T>& Value, bool RegisterForChange) const
		{
			bool hasValue = false;
			if (!Savegame::ReadPhobosStream(Stm, hasValue))
				return false;

			if (hasValue)
			{

				long ptrOld = 0l;
				if (!Stm.Load(ptrOld))
					return false;

				MemoryPoolUniquePointer<T> ptrNew = MemoryPoolUniquePointer<T>()(Stm);
				static_assert(detail::HasLoad<T> || detail::Hasload<T>,
					"Savegame::PhobosStreamObject<MemoryPoolUniquePointer<T>>: Type must implement Load/load returning bool");

				if (Savegame::ReadPhobosStream(Stm, *ptrNew, RegisterForChange)) {
					PHOBOS_SWIZZLE_REGISTER_POINTER(ptrOld, ptrNew.get(), PhobosCRT::GetTypeIDName<T>().c_str())
					Value.reset(ptrNew.release());
					return true;
				}

				return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const MemoryPoolUniquePointer<T>& Value) const
		{
			const bool hasValue = Value.get() != nullptr;

			if (!Savegame::WritePhobosStream(Stm, hasValue))
				return false;

			if (hasValue) {

				if (!Savegame::WritePhobosStream(Stm, (long)Value.get()))
					return false;

				static_assert(detail::HasSave<T> || detail::Hassave<T>,
					"Savegame::PhobosStreamObject<MemoryPoolUniquePointer<T>>: Type must implement Load/load returning bool");

				return Savegame::WritePhobosStream(Stm, *Value.get());
			}
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<UniqueGamePtr<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, UniqueGamePtr<T>& Value, bool RegisterForChange) const
		{
			static_assert(true, "Not Implemented !");
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const UniqueGamePtr<T>& Value) const
		{
			static_assert(true, "Not Implemented !");
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<BytePalette>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, BytePalette& Value, bool RegisterForChange) const
		{
			for (int i = 0; i < BytePalette::EntriesCount; ++i) {
				if (!Savegame::ReadPhobosStream(Stm, Value.Entries[i], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const BytePalette& Value) const
		{
			for (int i = 0; i < BytePalette::EntriesCount; ++i) {
				if (!Savegame::WritePhobosStream(Stm, Value.Entries[i]))
					return false;
			}

			return true;
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
				if (!Savegame::ReadPhobosStream(Stm, *ptrNew, RegisterForChange))
					return false;

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

				if (!Savegame::WritePhobosStream(Stm, *Value.get()))
					return false;
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
			if (!Savegame::ReadPhobosStream(Stm, Value.ItemIndex)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.ItemType)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.Cat)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.CurrentFactory, RegisterForChange)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.Status)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.Progress)) return false;
			if (!Savegame::ReadPhobosStream(Stm, Value.FlashEndFrame)) return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const BuildType& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.ItemIndex)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.ItemType)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.Cat)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.CurrentFactory)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.Status)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.Progress)) return false;
			if (!Savegame::WritePhobosStream(Stm, Value.FlashEndFrame)) return false;

			return true;
		}
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

	template <typename... Types>
	struct Savegame::PhobosStreamObject<std::tuple<Types...>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::tuple<Types...>& Value, bool RegisterForChange) const
		{
			// Read and validate tuple size
			size_t storedCount = 0;
			if (!Savegame::ReadPhobosStream(Stm, storedCount, false))
				return false;

			constexpr size_t expectedCount = sizeof...(Types);
			if (storedCount != expectedCount)
			{
				// Size mismatch - corrupted data or version incompatibility
				Debug::Log("Tuple size mismatch: expected %zu, got %zu\n", expectedCount, storedCount);
				return false;
			}

			return ReadTupleHelper(Stm, Value, RegisterForChange, std::index_sequence_for<Types...>{});
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::tuple<Types...>& Value) const
		{
			// Write tuple size first
			constexpr size_t tupleCount = sizeof...(Types);
			if (!Savegame::WritePhobosStream(Stm, tupleCount))
				return false;

			return WriteTupleHelper(Stm, Value, std::index_sequence_for<Types...>{});
		}

	private:
		// Helper for reading tuple elements recursively
		template <size_t... Indices>
		bool ReadTupleHelper(PhobosStreamReader& Stm, std::tuple<Types...>& Value, bool RegisterForChange, std::index_sequence<Indices...>) const
		{
			// Fold expression to read each element (C++17)
			return (... && ReadElement<Indices>(Stm, Value, RegisterForChange));
		}

		template <size_t Index>
		bool ReadElement(PhobosStreamReader& Stm, std::tuple<Types...>& Value, bool RegisterForChange) const
		{
			return Savegame::ReadPhobosStream(Stm, std::get<Index>(Value), RegisterForChange);
		}

		// Helper for writing tuple elements recursively
		template <size_t... Indices>
		bool WriteTupleHelper(PhobosStreamWriter& Stm, const std::tuple<Types...>& Value, std::index_sequence<Indices...>) const
		{
			// Fold expression to write each element (C++17)
			return (... && WriteElement<Indices>(Stm, Value));
		}

		template <size_t Index>
		bool WriteElement(PhobosStreamWriter& Stm, const std::tuple<Types...>& Value) const
		{
			return Savegame::WritePhobosStream(Stm, std::get<Index>(Value));
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::basic_string_view<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::basic_string_view<T>& Value, bool RegisterForChange) const
		{
			static_assert(true, "Not Implemented !");
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::basic_string_view<T>& Value) const
		{
			static_assert(true, "Not Implemented !");
			return true;
		}
	};

	template <typename CharT, typename Traits, typename Alloc>
	struct Savegame::PhobosStreamObject<std::basic_string<CharT, Traits, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::basic_string<CharT, Traits, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int size = 0;
			if (!Savegame::ReadPhobosStream(Stm, size, RegisterForChange))
				return false;
			if (size > 0) {
				Value.resize(size);

				if (!Stm.Read(reinterpret_cast<PhobosByteStream::data_t*>(Value.data()), size * sizeof(CharT)))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::basic_string<CharT, Traits, Alloc>& Value) const
		{
			const auto stringSize = Value.size();

			if (!Savegame::WritePhobosStream(Stm, stringSize))
				return false;

			if (stringSize == 0)
				return true;

			Stm.Write(reinterpret_cast<const PhobosByteStream::data_t*>(Value.data()), stringSize * sizeof(CharT));

			return true;
		}
	};

	template <typename T, typename dx>
	struct Savegame::PhobosStreamObject<std::unique_ptr<T, dx>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::unique_ptr<T, dx>& Value, bool RegisterForChange) const
		{
			static_assert(std::is_same_v<dx, std::default_delete<T>>,
			"Savegame::PhobosStreamObject<std::unique_ptr<T, Deleter>>: Custom deleters are not supported for serialization!");

			bool hasValue = false;
			if (!Savegame::ReadPhobosStream(Stm, hasValue))
				return false;

			if (hasValue)
			{
				long ptrOld = 0l;
				if (!Savegame::ReadPhobosStream(Stm, ptrOld))
					return false;

				std::unique_ptr<T, dx> ptrNew = ObjectFactory<T>()(Stm);
				static_assert(detail::HasLoad<T> || detail::Hasload<T>,
					"Savegame::PhobosStreamObject<std::unique_ptr<T>>: Type must implement Load/load returning bool");

				if (Savegame::ReadPhobosStream(Stm, *ptrNew, RegisterForChange))
				{

					PHOBOS_SWIZZLE_REGISTER_POINTER(ptrOld, ptrNew.get(), PhobosCRT::GetTypeIDName<T>().c_str())
						Value.reset(ptrNew.release());
					return true;
				}

				return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unique_ptr<T, dx>& Value) const
		{
			static_assert(std::is_same_v<dx, std::default_delete<T>>,
				"Savegame::PhobosStreamObject<std::unique_ptr<T, Deleter>>: Custom deleters are not supported for serialization!");

			const bool hasValue = Value.get() != nullptr;

			if (!Savegame::WritePhobosStream(Stm, hasValue))
				return false;

			if (hasValue)
			{

				if (!Savegame::WritePhobosStream(Stm, (long)Value.get()))
					return false;

				static_assert(detail::HasSave<T> || detail::Hassave<T>,
					"Savegame::PhobosStreamObject<std::unique_ptr<T>>: Type must implement Load/load returning bool");

				return Savegame::WritePhobosStream(Stm, *Value.get());
			}

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
					if (!Stm.Save(value))
						return false;

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
			if (!Savegame::WritePhobosStream(Stm, Value.has_value()))
				return false;

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

			if (!Savegame::ReadPhobosStream(Stm, Count)) {
				Debug::Log("Vector %s load failed to read count\n", name.c_str());
				return false;
			}

			Debug::Log("Vector %s loading %d elements \n", name.c_str(), Count);

			if (Count <= -1)
				Count = 0;

			Value.resize(Count);
			if ((int)Value.size() != Count) {
				Debug::Log("Vector %s resize failed! Expected %d, got %zu\n", name.c_str(), Count, Value.size());
				__debugbreak();
			}

			for (auto ix = 0; ix < Count; ++ix) {
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
			if (!Savegame::WritePhobosStream(Stm, Value.size()))
				return false;

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

	template <typename TKey, typename TValue, typename cmp, typename Alloc>
	struct Savegame::PhobosStreamObject<std::map<TKey, TValue, cmp , Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::map<TKey, TValue, cmp, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Savegame::ReadPhobosStream(Stm, Count))
				return false;

			if (Count <= -1)
				Count = 0;

			for (auto ix = 0; ix < Count; ++ix) {
				std::pair<TKey, TValue> buffer {};
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
					return false;

				Value.insert(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::map<TKey, TValue, cmp, Alloc>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.size()))
				return false;

			for (const std::pair<const TKey, TValue>& internal : Value) {
				if (!Savegame::WritePhobosStream(Stm, internal))
					return false;
			}

			return true;
		}
	};

	template <typename T, typename cmp , typename Alloc>
	struct Savegame::PhobosStreamObject<std::set<T, cmp , Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::set<T, cmp, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Savegame::ReadPhobosStream(Stm, Count)) {
				return false;
			}

			if (Count > 0)
			{
				if COMPILETIMEEVAL(std::is_pointer<T>::value) {
					std::vector<T> buffer(Count, nullptr);
					for (auto ix = 0; ix < Count; ++ix) {
						if (!Savegame::ReadPhobosStream(Stm, buffer[ix], RegisterForChange)) {
							return false;
						}
					}

					Value.insert(buffer.begin(), buffer.end());

				} else {

					for (auto ix = 0; ix < Count; ++ix) {
						T buffer = T {};
						if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange)) {
							return false;
						}

						Value.insert(buffer);
					}
				}
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::set<T, cmp, Alloc>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.size()))
				return false;

			if (Value.size() > 0) {
				for (const auto& item : Value) {
					if (!Savegame::WritePhobosStream(Stm, item)) {
						return false;
					}
				}
			}

			return true;
		}
	};

	template <typename T, typename Alloc>
	struct Savegame::PhobosStreamObject<std::list<T, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::list<T, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Savegame::ReadPhobosStream(Stm,Count)) {
				return false;
			}

			if (Count <= -1)
				Count = 0;

			for (auto ix = 0; ix < Count; ++ix) {
				T buffer = T {};
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange)) {
					return false;
				}
				Value.push_back(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::list<T, Alloc>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.size()))
				return false;

			if (Value.size() > 0) {
				for (const auto& item : Value) {
					if (!Savegame::WritePhobosStream(Stm, item)) {
						return false;
					}
				}
			}

			return true;
		}
	};

	template <typename T, typename Alloc>
	struct Savegame::PhobosStreamObject<std::deque<T, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::deque<T, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int count = 0;

			if (!Savegame::ReadPhobosStream(Stm, count))
				return false;

			if (count > 0) {

				Value.resize(count);

				for (auto ix = 0; ix < count; ++ix)
				{
					if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange))
						return false;
				}
			}
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::deque<T, Alloc>& Value) const
		{
			if(!Savegame::WritePhobosStream(Stm, Value.size())){
				return false;
			}

			if(Value.size() > 0){
				for (auto ix = 0u; ix < Value.size(); ++ix) {
					if (!Savegame::WritePhobosStream(Stm, Value[ix]))
						return false;
				}
			}

			return true;
		}
	};

	template <typename T, typename Container>
	struct Savegame::PhobosStreamObject<std::queue<T , Container>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::queue<T, Container>& Value, bool RegisterForChange) const
		{
			Value = std::queue<T, Container>();

			int nSize = 0;
			if (!Savegame::ReadPhobosStream(Stm, nSize))
				return false;

			if (nSize <= -1)
				nSize = 0;

			for (int ix = 0; ix < nSize; ++ix)
			{
				T buffer { };
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
					return false;

				Value.push(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::queue<T, Container>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.size()))
				return false;

			if (Value.size() > 0)
			{
				//make an copy
				std::queue<T, Container> Quee = Value;

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

	template <typename T ,typename Container, typename Compare>
	struct Savegame::PhobosStreamObject<std::priority_queue<T, Container, Compare>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::priority_queue<T, Container, Compare>& Value, bool RegisterForChange) const
		{
			Value = std::priority_queue<T, Container, Compare>();

			int nSize = 0;
			if (!Savegame::ReadPhobosStream(Stm, nSize))
				return false;

			if (nSize > 0)
			{
				for (int ix = 0; ix < nSize; ++ix) {
					T buffer {};
					if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
						return false;

					Value.push(buffer);
				}
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::priority_queue<T, Container, Compare>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.size()))
				return false;

			if (Value.size() > 0)
			{
				// Copy the underlying container instead of popping
				// We need to access the protected container, so we create a temporary copy
				std::priority_queue<T, Container, Compare> tempQueue = Value;

				while (!tempQueue.empty())
				{
					if (!Savegame::WritePhobosStream(Stm, tempQueue.top()))
						return false;

					tempQueue.pop();
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
			for (auto ix = 0; ix < size; ++ix) {
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

	template <typename TKey, typename TValue , typename hasher , typename cmp , typename Alloc>
	struct Savegame::PhobosStreamObject<std::unordered_map<TKey, TValue, hasher,cmp , Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::unordered_map<TKey, TValue, hasher, cmp, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Savegame::ReadPhobosStream(Stm, Count)) {
				return false;
			}

			if (Count > 0){
				for (auto ix = 0; ix < Count; ++ix)
				{
					std::pair<TKey, TValue> buffer {};
					if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange)) {
						return false;
					}

					Value.insert(buffer);
				}
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unordered_map<TKey, TValue, hasher, cmp, Alloc>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.size()))
				return false;

			if (Value.size() > 0) {
				for (const std::pair<const TKey, TValue>& internal : Value) {
					if (!Savegame::WritePhobosStream(Stm, internal))
						return false;
				}
			}

			return true;
		}
	};

	template <typename TKey, typename TValue, typename Cmp, typename Alloc>
	struct Savegame::PhobosStreamObject<std::multimap<TKey, TValue, Cmp , Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::multimap<TKey, TValue, Cmp, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Stm.Load(Count)) {
				return false;
			}

			if (Count > 0) {
				for (auto ix = 0; ix < Count; ++ix)
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
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::multimap<TKey, TValue, Cmp, Alloc>& Value) const
		{
			if (!Stm.Save((int)Value.size()))
				return false;

			if (Value.size() > 0) {
				for (const std::pair<const TKey, TValue>& internal : Value) {
					if (!Savegame::WritePhobosStream(Stm, internal))
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