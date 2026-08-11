#pragma once

// =============================================================================
// SavegameDef.Std.h
//
// Tier 2 of the SavegameDef split: every specialization over a standard-library
// type. Depends on Tier 1 and on <...> headers only -- no YRpp, no game classes.
//
// A utility TU that serializes std::vector / std::map / std::optional needs
// this and nothing else, and will not rebuild when a game header changes.
//
// SPLIT NOTE: bodies moved verbatim from the original SavegameDef.h.
//
// BUGFIX (includes): the original relied on transitive includes for <list>,
// <deque>, <unordered_set>, <tuple> and <vector> -- it specialized those types
// without including their headers. They are listed explicitly below.
// =============================================================================

#include <array>
#include <bitset>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Savegame
{
#pragma region StdSpecializations

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

	// BUGFIX: was `static_assert(true, ...)` -- a no-op that let string_view
	// members silently serialize nothing. Now a hard error.
	// NOTE: a string_view does not own its buffer, so it cannot be restored
	// meaningfully; serialize the owning std::string instead.
	template <typename T>
	struct Savegame::PhobosStreamObject<std::basic_string_view<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::basic_string_view<T>& Value, bool RegisterForChange) const
		{
			static_assert(detail::AlwaysFalse<T>,
				"PhobosStreamObject<std::basic_string_view>: a string_view does not own "
				"its buffer and cannot be restored. Serialize the owning std::basic_string.");
			return false;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::basic_string_view<T>& Value) const
		{
			static_assert(detail::AlwaysFalse<T>,
				"PhobosStreamObject<std::basic_string_view>: a string_view does not own "
				"its buffer and cannot be restored. Serialize the owning std::basic_string.");
			return false;
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

			if (size > 0)
			{
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

					PHOBOS_SWIZZLE_REGISTER_POINTER(ptrOld, ptrNew.get(), PhobosCRT::GetTypeIDName<T>().c_str());
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
					"Savegame::PhobosStreamObject<std::unique_ptr<T>>: Type must implement Save/save returning bool");

				return Savegame::WritePhobosStream(Stm, *Value.get());
			}

			return true;
		}
	};

	template <typename T, typename A, typename dx>
	struct Savegame::PhobosStreamObject<std::unique_ptr<std::vector<T, A>, dx>>
	{
		using payload_type = std::vector<T, A>;

		bool ReadFromStream(PhobosStreamReader& Stm, std::unique_ptr<payload_type, dx>& Value, bool RegisterForChange) const
		{
			static_assert(std::is_same_v<dx, std::default_delete<payload_type>>,
				"Savegame::PhobosStreamObject<std::unique_ptr<std::vector<T>, Deleter>>: Custom deleters are not supported for serialization!");

			bool hasValue = false;
			if (!Savegame::ReadPhobosStream(Stm, hasValue))
				return false;

			if (hasValue)
			{
				long ptrOld = 0l;
				if (!Savegame::ReadPhobosStream(Stm, ptrOld))
					return false;

				std::unique_ptr<payload_type, dx> ptrNew = ObjectFactory<payload_type>()(Stm);

				// NOTE: no HasLoad/Hasload gate here - the payload is serialized through
				// PhobosStreamObject<std::vector<T, A>>, which already exists.
				if (Savegame::ReadPhobosStream(Stm, *ptrNew, RegisterForChange))
				{
					PHOBOS_SWIZZLE_REGISTER_POINTER(ptrOld, ptrNew.get(), PhobosCRT::GetTypeIDName<payload_type>().c_str());
					Value.reset(ptrNew.release());
					return true;
				}

				return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unique_ptr<payload_type, dx>& Value) const
		{
			static_assert(std::is_same_v<dx, std::default_delete<payload_type>>,
				"Savegame::PhobosStreamObject<std::unique_ptr<std::vector<T>, Deleter>>: Custom deleters are not supported for serialization!");

			const bool hasValue = Value.get() != nullptr;

			if (!Savegame::WritePhobosStream(Stm, hasValue))
				return false;

			if (hasValue)
			{
				if (!Savegame::WritePhobosStream(Stm, (long)Value.get()))
					return false;

				// NOTE: no HasSave/Hassave gate here - see ReadFromStream.
				return Savegame::WritePhobosStream(Stm, *Value.get());
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::shared_ptr<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::shared_ptr<T>& Value, bool RegisterForChange) const
		{
			static_assert(detail::HasLoad<T> || detail::Hasload<T>,
			"Savegame::PhobosStreamObject<std::shared_ptr<T>>: Type must implement Load/load returning bool");

			// BUGFIX: the original ended with an unconditional `Value.reset();`
			// AFTER the assignment above, so every shared_ptr loaded back as
			// null and the registry lookup was dead work. The reset now only
			// happens on the null-pointer path.
			T* ptrOld = nullptr;
			if (!Stm.Load(ptrOld))
				return false;

			if (!ptrOld)
			{
				Value.reset();
				return true;
			}

			std::shared_ptr<void> existing;

			auto it = SavegameGlobal::GlobalSharedRegistry.find(ptrOld);
			if (it != SavegameGlobal::GlobalSharedRegistry.end())
				existing = it->second.lock();

			if (existing)
			{
				Value = std::static_pointer_cast<T>(existing);
			}
			else
			{
				// DIFF: make_shared moved inside the else branch. The original
				// allocated it unconditionally and threw it away on the
				// registry-hit path.
				std::shared_ptr<T> ptrNew = std::make_shared<T>();
				Value = ptrNew;
				SavegameGlobal::GlobalSharedRegistry[ptrOld] = ptrNew;
				PHOBOS_SWIZZLE_REGISTER_POINTER(ptrOld, ptrNew.get(), PhobosCRT::GetTypeIDName<T>().c_str());
			}

			// SUSPECT: the payload itself is never read back here -- only the
			// pointer identity is restored. If T carries state, a
			// ReadPhobosStream(Stm, *Value, RegisterForChange) is missing on
			// the freshly-created branch. Left as-is; verify against the
			// matching WriteToStream/PersistObject before changing the format.
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::shared_ptr<T>& Value) const
		{
			static_assert(detail::HasSave<T> || detail::Hassave<T>,
			"Savegame::PhobosStreamObject<std::shared_ptr<T>>: Type must implement Save/save returning bool");
			return PersistObject(Stm, Value.get());
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
				if (!Stm.RegisterChange(&Value))
					return false;

				if (!HasValue)
				{
					return true;
				}

				Value.emplace();
				if (Savegame::ReadPhobosStream(Stm, *Value, RegisterForChange))
				{
					return true;
				}
			}

			return false;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::optional<T>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.has_value()))
				return false;

			if (!Stm.RegisterChange(&Value))
				return false;

			if (Value.has_value())
				return Savegame::WritePhobosStream(Stm, Value.value());

			return true;
		}
	};

	template <typename T, typename Alloc>
	struct Savegame::PhobosStreamObject<std::vector<T, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::vector<T, Alloc>& Value, bool RegisterForChange) const
		{
			int Count = 0;

			if (!Savegame::ReadPhobosStream(Stm, Count))
			{
				Debug::Log("Vector %s load failed to read count\n",
					PhobosCRT::GetTypeIDName<T>().c_str());
				return false;
			}

			if (Count <= -1)
				Count = 0;

			if (Count > 0)
			{
				Value.resize(Count);

				if ((int)Value.size() != Count)
				{
					Debug::Log("Vector %s resize failed! Expected %d, got %zu\n",
						PhobosCRT::GetTypeIDName<T>().c_str(), Count, Value.size());
					__debugbreak();
				}

				for (auto ix = 0; ix < Count; ++ix)
				{
					// DIFF: removed the per-element Debug::Log and the
					// unconditional GetTypeIDName() call at function entry.
					// GetTypeIDName allocates a std::string on EVERY vector
					// load, and the per-element log made a 10k-element vector
					// emit 10k lines. Logging now only happens on failure,
					// where the type name is actually wanted.
					if COMPILETIMEEVAL(std::is_same_v<T, bool>)
					{
						bool temp {};

						if (!Savegame::ReadPhobosStream(Stm, temp, RegisterForChange))
						{
							Debug::Log("Failed to load vector %s element %u\n",
								PhobosCRT::GetTypeIDName<T>().c_str(), ix);
							return false;
						}

						Value[ix] = temp;
					}
					else
					{
						if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange))
						{
							Debug::Log("Failed to load vector %s element %u\n",
								PhobosCRT::GetTypeIDName<T>().c_str(), ix);
							return false;
						}
					}
				}
			}

			return Stm.RegisterChange(&Value);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::vector<T, Alloc>& Value) const
		{
			int Count = (int)Value.size();
			if (!Savegame::WritePhobosStream(Stm, Count))
				return false;

			for (auto ix = 0; ix < Count; ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value[ix]))
					return false;
			}

			return Stm.RegisterChange(&Value);
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
	struct Savegame::PhobosStreamObject<std::map<TKey, TValue, cmp, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::map<TKey, TValue, cmp, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Savegame::ReadPhobosStream(Stm, Count))
				return false;

			if (Count <= -1)
				Count = 0;

			for (auto ix = 0; ix < Count; ++ix)
			{
				std::pair<TKey, TValue> buffer {};
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
					return false;

				Value.insert(buffer);
			}

			return 	Stm.RegisterChange(&Value);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::map<TKey, TValue, cmp, Alloc>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.size()))
				return false;

			for (const std::pair<const TKey, TValue>& internal : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, internal))
					return false;
			}

			return 	Stm.RegisterChange(&Value);;
		}
	};

	template <typename T, typename Hash, typename KeyEqual, typename Alloc>
	struct Savegame::PhobosStreamObject<std::unordered_set<T, Hash, KeyEqual, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::unordered_set<T, Hash, KeyEqual, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();
			int Count = 0;
			if (!Savegame::ReadPhobosStream(Stm, Count))
			{
				return false;
			}

			if (Count > 0)
			{
				if COMPILETIMEEVAL(std::is_pointer<T>::value)
				{
					std::vector<T> buffer(Count, nullptr);
					for (auto ix = 0; ix < Count; ++ix)
					{
						if (!Savegame::ReadPhobosStream(Stm, buffer[ix], RegisterForChange))
						{
							return false;
						}
					}
					Value.insert(buffer.begin(), buffer.end());
				}
				else
				{
					for (auto ix = 0; ix < Count; ++ix)
					{
						T buffer = T {};
						if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
						{
							return false;
						}
						Value.insert(buffer);
					}
				}
			}
			return 	Stm.RegisterChange(&Value);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unordered_set<T, Hash, KeyEqual, Alloc>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.size()))
				return false;

			if (Value.size() > 0)
			{
				for (const auto& item : Value)
				{
					if (!Savegame::WritePhobosStream(Stm, item))
					{
						return false;
					}
				}
			}
			return 	Stm.RegisterChange(&Value);
		}
	};

	template <typename T, typename cmp, typename Alloc>
	struct Savegame::PhobosStreamObject<std::set<T, cmp, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::set<T, cmp, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Savegame::ReadPhobosStream(Stm, Count))
			{
				return false;
			}

			if (Count > 0)
			{
				if COMPILETIMEEVAL(std::is_pointer<T>::value)
				{
					std::vector<T> buffer(Count, nullptr);
					for (auto ix = 0; ix < Count; ++ix)
					{
						if (!Savegame::ReadPhobosStream(Stm, buffer[ix], RegisterForChange))
						{
							return false;
						}
					}

					Value.insert(buffer.begin(), buffer.end());

				}
				else
				{

					for (auto ix = 0; ix < Count; ++ix)
					{
						T buffer = T {};
						if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
						{
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

			if (Value.size() > 0)
			{
				for (const auto& item : Value)
				{
					if (!Savegame::WritePhobosStream(Stm, item))
					{
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
			if (!Savegame::ReadPhobosStream(Stm, Count))
			{
				return false;
			}

			if (Count <= -1)
				Count = 0;

			for (auto ix = 0; ix < Count; ++ix)
			{
				T buffer = T {};
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
				{
					return false;
				}
				Value.push_back(buffer);
			}

			return 	Stm.RegisterChange(&Value);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::list<T, Alloc>& Value) const
		{
			const int Count = (int)Value.size();
			if (!Savegame::WritePhobosStream(Stm, Count))
				return false;

			if (Count > 0)
			{
				for (const auto& item : Value)
				{
					if (!Savegame::WritePhobosStream(Stm, item))
					{
						return false;
					}
				}
			}

			return 	Stm.RegisterChange(&Value);
		}
	};

	template <typename T, typename Alloc>
	struct Savegame::PhobosStreamObject<std::deque<T, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::deque<T, Alloc>& Value, bool RegisterForChange) const
		{
			int count = 0;

			if (!Savegame::ReadPhobosStream(Stm, count))
				return false;

			if (count > 0)
			{
				Value.resize(count);

				for (auto ix = 0; ix < count; ++ix)
				{
					if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange))
						return false;
				}
			}

			return 	Stm.RegisterChange(&Value);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::deque<T, Alloc>& Value) const
		{
			const int Count = (int)Value.size();
			if (!Savegame::WritePhobosStream(Stm, Count))
			{
				return false;
			}

			for (int ix = 0; ix < Count; ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value[ix]))
					return false;
			}

			return 	Stm.RegisterChange(&Value);
		}
	};

	template <typename T, typename Container>
	struct Savegame::PhobosStreamObject<std::queue<T, Container>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::queue<T, Container>& Value, bool RegisterForChange) const
		{
			Value = std::queue<T, Container>();

			int nSize = 0;
			if (!Savegame::ReadPhobosStream(Stm, nSize))
				return false;

			for (int ix = 0; ix < nSize; ++ix)
			{
				T buffer { };
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
					return false;

				Value.push(buffer);
			}

			return 	Stm.RegisterChange(&Value);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::queue<T, Container>& Value) const
		{
			const int Count = (int)Value.size();
			if (!Savegame::WritePhobosStream(Stm, Count))
				return false;

			if (Count > 0)
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

			return 	Stm.RegisterChange(&Value);
		}
	};

	template <typename T, typename Container, typename Compare>
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
				for (int ix = 0; ix < nSize; ++ix)
				{
					T buffer {};
					if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
						return false;

					Value.push(buffer);
				}
			}

			return 	Stm.RegisterChange(&Value);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::priority_queue<T, Container, Compare>& Value) const
		{
			const int Count = (int)Value.size();

			if (!Savegame::WritePhobosStream(Stm, Count))
				return false;

			if (Count > 0)
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

			return 	Stm.RegisterChange(&Value);
		}
	};

	template <typename T, size_t size>
	struct Savegame::PhobosStreamObject<std::array<T, size>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::array<T, size>& Value, bool RegisterForChange) const
		{
			int Count = 0;
			if (!Savegame::ReadPhobosStream(Stm, Count, RegisterForChange))
				return false;

			const int acceptedSize = (int)size;

			//not valid
			if (Count != acceptedSize)
				return false;

			for (auto ix = 0; ix < Count; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange))
				{
					return false;
				}
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::array<T, size>& Value) const
		{
			//safeguard
			int Count = (int)size;
			if (!Savegame::WritePhobosStream(Stm, Count))
				return false;

			for (auto ix = 0; ix < Count; ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value[ix]))
				{
					return false;
				}
			}

			return true;
		}
	};

	template <typename TKey, typename TValue, typename hasher, typename cmp, typename Alloc>
	struct Savegame::PhobosStreamObject<std::unordered_map<TKey, TValue, hasher, cmp, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::unordered_map<TKey, TValue, hasher, cmp, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Savegame::ReadPhobosStream(Stm, Count))
			{
				return false;
			}

			if (Count > 0)
			{
				for (auto ix = 0; ix < Count; ++ix)
				{
					std::pair<TKey, TValue> buffer {};
					if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
					{
						return false;
					}

					Value.insert(buffer);
				}
			}

			return 	Stm.RegisterChange(&Value);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unordered_map<TKey, TValue, hasher, cmp, Alloc>& Value) const
		{
			const int Count = (int)Value.size();
			if (!Savegame::WritePhobosStream(Stm, Count))
				return false;

			if (Count > 0)
			{
				for (const std::pair<const TKey, TValue>& internal : Value)
				{
					if (!Savegame::WritePhobosStream(Stm, internal))
						return false;
				}
			}

			return 	Stm.RegisterChange(&Value);
		}
	};

	template <typename TKey, typename TValue, typename Cmp, typename Alloc>
	struct Savegame::PhobosStreamObject<std::multimap<TKey, TValue, Cmp, Alloc>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::multimap<TKey, TValue, Cmp, Alloc>& Value, bool RegisterForChange) const
		{
			Value.clear();

			int Count = 0;
			if (!Stm.Load(Count))
			{
				return false;
			}

			if (Count > 0)
			{

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

			return 	Stm.RegisterChange(&Value);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::multimap<TKey, TValue, Cmp, Alloc>& Value) const
		{
			const int Count = (int)Value.size();

			if (!Stm.Save(Count))
				return false;

			if (Count > 0)
			{
				for (const std::pair<const TKey, TValue>& internal : Value)
				{
					if (!Savegame::WritePhobosStream(Stm, internal))
						return false;
				}
			}

			return 	Stm.RegisterChange(&Value);
		}
	};

#pragma endregion

}