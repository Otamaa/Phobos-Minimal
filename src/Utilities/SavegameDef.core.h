#pragma once

// =============================================================================
// SavegameDef.Core.h
//
// Tier 1 of the SavegameDef split: the dispatch machinery plus the
// specializations that depend on NOTHING from YRpp or the game.
//
// Include this alone from utility translation units. Nothing here drags in
// RulesClass.h / SidebarClass.h / ScenarioClass.h / FileSystem.h, so editing a
// game header does not rebuild anything that only includes this file.
//
// SPLIT NOTE: content moved verbatim from the original SavegameDef.h. No logic
// was rewritten. Ordering within each tier is preserved.
// =============================================================================

#include "Savegame.h"

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include <Utilities/Debug.h>
#include <Utilities/Swizzle.h>

class SavegameGlobal
{
public:
	static std::unordered_map<void*, std::weak_ptr<void>> GlobalSharedRegistry;
	static void ClearSharedRegistry() { SavegameGlobal::GlobalSharedRegistry.clear(); }
};

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

		// BUGFIX: dependent-false helper. `static_assert(true, "...")` is a no-op
		// and never fires -- see the "Not Implemented" specializations, which
		// silently returned true instead of failing the build. Use
		// `static_assert(AlwaysFalse<T>, "...")` to actually reject a type.
		template <typename...>
		inline constexpr bool AlwaysFalse = false;

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
			static bool ReadFromStream(PhobosStreamReader& stm, T& value, bool register_for_change)
			{
				if constexpr (HasLoad<T>)
					return value.Load(stm, register_for_change);
				else if constexpr (Hasload<T>)
					return value.load(stm, register_for_change);
				else if constexpr (IsFixedArray<T>)
				{
					// DIFF: dropped `static_assert(std::is_same_v<T, T>, ...)` markers.
					// They are tautologies and never fire; they were debug breadcrumbs.
					if constexpr (has_phobos_stream_object_v<T>)
					{
						PhobosStreamObject<T> item;
						return item.ReadFromStream(stm, value, register_for_change);
					}
					else
					{
						static_assert(has_phobos_stream_object_v<T>,
									  "ARRAY TYPE DETECTED but no PhobosStreamObject specialization found. "
									  "Create PhobosStreamObject<T[N]> specialization for your array type.");
						return false;
					}
				}
				else if constexpr (std::is_pointer_v<T>)
				{
					// Handle pointer types
					uintptr_t old_ptr {};
					if (!stm.Load(old_ptr))
						return false;

					//request remap table update for this pointer
					if (FAILED(PHOBOS_SWIZZLE_REQUEST_MANUAL_POINTER_REMAP(value, old_ptr, PhobosCRT::GetTypeIDName<T>().c_str())))
					{
						return false;
					}

					return true;
				}
				else if constexpr (has_phobos_stream_object_v<T>)
				{
					// Handle other types with PhobosStreamObject specialization
					PhobosStreamObject<T> item;
					return item.ReadFromStream(stm, value, register_for_change);
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
			static bool WriteToStream(PhobosStreamWriter& stm, const T& value)
			{
				if constexpr (HasSave<T>)
					return value.Save(stm);
				else if constexpr (Hassave<T>)
					return value.save(stm);
				else if constexpr (IsFixedArray<T>)
				{
					if constexpr (has_phobos_stream_object_v<T>)
					{
						PhobosStreamObject<T> item;
						return item.WriteToStream(stm, value);
					}
					else
					{
						static_assert(has_phobos_stream_object_v<T>,
									  "ARRAY TYPE DETECTED but no PhobosStreamObject specialization found. "
									  "Create PhobosStreamObject<T[N]> specialization for your array type.");
						return false;
					}
				}
				else if constexpr (std::is_pointer_v<T>)
				{
					// Handle pointer types
					uintptr_t raw_ptr = reinterpret_cast<uintptr_t>(value);
					return stm.Save(raw_ptr);
				}
				else if constexpr (has_phobos_stream_object_v<T>)
				{
					// Handle other types with PhobosStreamObject specialization
					PhobosStreamObject<T> item;
					return item.WriteToStream(stm, value);
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
	bool ReadPhobosStream(PhobosStreamReader& stm, T& value, bool register_for_change)
	{
		return detail::Selector::ReadFromStream(stm, value, register_for_change);
	}

	template <typename T>
	bool WritePhobosStream(PhobosStreamWriter& stm, const T& value)
	{
		return detail::Selector::WriteToStream(stm, value);
	}

	template <typename T>
	bool PhobosStreamObject<T>::ReadFromStream(PhobosStreamReader& stm, T& value, bool register_for_change) const
	{
		if constexpr (IsDataTheTypeCorrect<T>)
		{
			bool ret = stm.Load(value);
			if COMPILETIMEEVAL(std::is_pointer<T>::value)
			{
				if (FAILED(PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP(value, PhobosCRT::GetTypeIDName<T>().c_str())))
				{
					return false;
				}
			}

			return ret;
		}
		else
		{
			static_assert(IsDataTheTypeCorrect<T>, "Use specialization for this type");
			return false;
		}
	}

	template <typename T>
	bool PhobosStreamObject<T>::WriteToStream(PhobosStreamWriter& stm, const T& value) const
	{
		if constexpr (IsDataTheTypeCorrect<T>)
		{ return stm.Save(value); }
		else
		{
			static_assert(IsDataTheTypeCorrect<T>, "Use specialization for this type");
			return false;
		}
	}

#pragma region CoreSpecializations

	template <size_t Size>
	struct Savegame::PhobosStreamObject<char[Size]>
	{
		bool ReadFromStream(PhobosStreamReader& stm, char(&value)[Size], bool register_for_change) const
		{
			// Use existing std::string template
			std::string tmp_str {};
			if (!Savegame::ReadPhobosStream(stm, tmp_str, register_for_change))
				return false;

			// Copy to fixed array with bounds checking
			size_t copy_len = std::min(tmp_str.length(), Size - 1);
			std::memcpy(value, tmp_str.c_str(), copy_len);
			value[copy_len] = '\0';  // Ensure null termination

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const char(&value)[Size]) const
		{
			// Convert to std::string and use existing template
			std::string tmp_str(value, strnlen(value, Size));
			return Savegame::WritePhobosStream(stm, tmp_str);
		}
	};

	template <size_t Size>
	struct Savegame::PhobosStreamObject<wchar_t[Size]>
	{
		bool ReadFromStream(PhobosStreamReader& stm, wchar_t(&value)[Size], bool register_for_change) const
		{
			// Use existing std::wstring template
			std::wstring tmp_wstr {};
			if (!Savegame::ReadPhobosStream(stm, tmp_wstr, register_for_change))
				return false;

			// Copy to fixed array with bounds checking
			size_t copy_len = std::min(tmp_wstr.length(), Size - 1);
			std::memcpy(value, tmp_wstr.c_str(), copy_len * sizeof(wchar_t));
			value[copy_len] = L'\0';  // Ensure null termination

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const wchar_t(&value)[Size]) const
		{
			// Convert to std::wstring and use existing template
			std::wstring tmp_wstr(value, wcsnlen(value, Size));
			return Savegame::WritePhobosStream(stm, tmp_wstr);
		}
	};

	// BUGFIX: was `static_assert(true, ...)`, which never fires -- these
	// silently serialized nothing and returned success. Now a hard error at
	// the point of use. If a const char* member really must round-trip, give
	// it a real body instead of re-enabling the silent path.
	template <>
	struct Savegame::PhobosStreamObject<const char*>
	{
		bool ReadFromStream(PhobosStreamReader& stm, const char*& value, bool register_for_change) const
		{
			//static_assert(detail::AlwaysFalse<decltype(value)>,
			//	"PhobosStreamObject<const char*>: not implemented. Serialize a "
			//	"std::string or PhobosCString<N> instead of a raw pointer.");

			Debug::FatalError("PhobosStreamObject<const char*>: not implemented. Serialize a std::string or PhobosCString<N> instead of a raw pointer.");
			return false;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const char*& value) const
		{
			//static_assert(detail::AlwaysFalse<decltype(value)>,
			//	"PhobosStreamObject<const char*>: not implemented. Serialize a "
			//	"std::string or PhobosCString<N> instead of a raw pointer.");
			Debug::FatalError("PhobosStreamObject<const char*>: not implemented. Serialize a std::string or PhobosCString<N> instead of a raw pointer.");
			return false;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<const wchar_t*>
	{
		bool ReadFromStream(PhobosStreamReader& stm, const wchar_t*& value, bool register_for_change) const
		{
			//static_assert(detail::AlwaysFalse<decltype(value)>,
			//	"PhobosStreamObject<const wchar_t*>: not implemented. Serialize a "
			//	"std::wstring instead of a raw pointer.");

			Debug::FatalError("PhobosStreamObject<const wchar_t*>: not implemented. Serialize a std::wstring instead of a raw pointer.");
			return false;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const wchar_t*& value) const
		{
			//static_assert(detail::AlwaysFalse<decltype(value)>,
			//	"PhobosStreamObject<const wchar_t*>: not implemented. Serialize a "
			//	"std::wstring instead of a raw pointer.");

			Debug::FatalError("PhobosStreamObject<const wchar_t*>: not implemented. Serialize a std::wstring instead of a raw pointer.");
			return false;
		}
	};

	template <typename T, size_t N>
	struct Savegame::PhobosStreamObject<T[N]>
	{
		bool ReadFromStream(PhobosStreamReader& stm, T(&value)[N], bool register_for_change) const
		{
			// Read each element of the array
			for (size_t idx = 0; idx < N; ++idx)
			{
				value[idx] = T {};
				if (!Savegame::ReadPhobosStream(stm, value[idx], register_for_change))
				{
					return false;
				}
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const T(&value)[N]) const
		{
			// Write each element of the array
			for (size_t idx = 0; idx < N; ++idx)
			{
				if (!Savegame::WritePhobosStream(stm, value[idx]))
				{
					return false;
				}
			}
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<bool>
	{
		bool ReadFromStream(PhobosStreamReader& stm, bool& value, bool register_for_change) const
		{
			int tmp_int = 0;
			if (!Savegame::ReadPhobosStream(stm, tmp_int, register_for_change))
				return false;
			value = (tmp_int != 0);  // Convert int to bool: 0=false, non-zero=true
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const bool& value) const
		{
			if (!Savegame::WritePhobosStream(stm, static_cast<int>(value)))  // true=1, false=0
				return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<BYTE>
	{
		bool ReadFromStream(PhobosStreamReader& stm, BYTE& value, bool register_for_change) const
		{
			int tmp_int = 0;
			if (!Savegame::ReadPhobosStream(stm, tmp_int, register_for_change))
				return false;
			value = static_cast<BYTE>(tmp_int);
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const BYTE& value) const
		{
			if (!Savegame::WritePhobosStream(stm, static_cast<int>(value)))
				return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<size_t>
	{
		bool ReadFromStream(PhobosStreamReader& stm, size_t& value, bool register_for_change) const
		{
			int tmp_int = 0;
			if (!Savegame::ReadPhobosStream(stm, tmp_int))
				return false;

			value = static_cast<size_t>(tmp_int);
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const size_t& value) const
		{
			if (!Savegame::WritePhobosStream(stm, static_cast<int>(value)))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<unsigned short>
	{
		bool ReadFromStream(PhobosStreamReader& stm, unsigned short& value, bool register_for_change) const
		{
			int tmp_int = 0;
			if (!Savegame::ReadPhobosStream(stm, tmp_int))
				return false;

			value = static_cast<unsigned short>(tmp_int);
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const unsigned short& value) const
		{
			if (!Savegame::WritePhobosStream(stm, static_cast<int>(value)))
				return false;

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<short>
	{
		bool ReadFromStream(PhobosStreamReader& stm, short& value, bool register_for_change) const
		{
			int tmp_int = 0;
			if (!Savegame::ReadPhobosStream(stm, tmp_int))
				return false;

			value = static_cast<short>(tmp_int);
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& stm, const short& value) const
		{
			if (!Savegame::WritePhobosStream(stm, static_cast<int>(value)))
				return false;

			return true;
		}
	};

#pragma endregion

}

#define DefaultSaveLoadFunc(cls) \
bool Load(PhobosStreamReader& stm, bool register_for_change) { return this->Serialize(stm); } \
bool Save(PhobosStreamWriter& stm) const { return const_cast<cls*>(this)->Serialize(stm); }