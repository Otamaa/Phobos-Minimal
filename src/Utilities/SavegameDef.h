#pragma once

// include this file whenever something is to be saved.

#include "Savegame.h"

#include <Utilities/GameUniquePointers.h>
#include <Utilities/VectorHelper.h>

#include <unordered_map>
#include <map>
#include <set>
#include <bitset>
#include <memory>

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

#include "TranslucencyLevel.h"
#include "Swizzle.h"
#include "Debug.h"

namespace Savegame
{
	namespace detail
	{
		struct Selector
		{
			template <typename T>
			static bool ReadFromStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange)
			{
				return read_from_stream(Stm, Value, RegisterForChange, 0, 0);
			}

			template <typename T>
			static bool WriteToStream(PhobosStreamWriter& Stm, const T& Value)
			{
				return write_to_stream(Stm, Value, 0, 0);
			}

		private:
			// support for upper-case Load and lowercase load member functions.
			// this is more complex than needed, but allows for more consistency
			// in function naming.
			struct Dummy
			{
				Dummy(int a) { };
			};

			template <typename T>
			static auto read_from_stream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange, int, int)
				-> decltype(Value.Load(Stm, RegisterForChange))
			{
				return Value.Load(Stm, RegisterForChange);
			}

			template <typename T>
			static auto read_from_stream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange, Dummy, int)
				-> decltype(Value.load(Stm, RegisterForChange))
			{
				return Value.load(Stm, RegisterForChange);
			}

			template <typename T>
			static bool read_from_stream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange, Dummy, Dummy)
			{
				PhobosStreamObject<T> item;
				return item.ReadFromStream(Stm, Value, RegisterForChange);
			}

			template <typename T>
			static auto write_to_stream(PhobosStreamWriter& Stm, const T& Value, int, int)
				-> decltype(Value.Save(Stm))
			{
				return Value.Save(Stm);
			}

			template <typename T>
			static auto write_to_stream(PhobosStreamWriter& Stm, const T& Value, Dummy, int)
				-> decltype(Value.save(Stm))
			{
				return Value.save(Stm);
			}

			template <typename T>
			static bool write_to_stream(PhobosStreamWriter& Stm, const T& Value, Dummy, Dummy)
			{
				PhobosStreamObject<T> item;
				return item.WriteToStream(Stm, Value);
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
	T* RestoreObject(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		T* ptrOld = nullptr;
		if (!Stm.Load(ptrOld))
			return nullptr;

		if (ptrOld)
		{
			std::unique_ptr<T> ptrNew = ObjectFactory<T>()(Stm);

			if (Savegame::ReadPhobosStream(Stm, *ptrNew, RegisterForChange))
			{
				PhobosSwizzle::Instance.RegisterChange(ptrOld, ptrNew.get());
				return ptrNew.release();
			}
		}

		return nullptr;
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

		if (RegisterForChange)
			Swizzle swizzle(Value);

		return ret;
	}

	template <typename T>
	bool PhobosStreamObject<T>::WriteToStream(PhobosStreamWriter& Stm, const T& Value) const
	{
		Stm.Save(Value);
		return true;
	}

	// specializations
	//template <typename T , size_t count>
	//struct Savegame::PhobosStreamObject<T>
	//{
	//	bool ReadFromStream(PhobosStreamReader& Stm, T[count]& Value, bool RegisterForChange) const
	//	{
	//		std::memset(&value , 0, sizeof(T) * count);
	//
	//		for(int i = 0;  i < count; ++i){
	//			if (!Savegame::ReadPhobosStream(Stm, Value[i], RegisterForChange))
	//				return false;
	///		}
	//
	//		return true;
	//	}
	//
	//	bool WriteToStream(PhobosStreamWriter& Stm, const T[count]& Value) const
	//	{
	//		for(int i = 0;  i < count; ++i){
	//			if (!Savegame::WritePhobosStream(Stm, Value[i]))
	//				return false;
	//		}
	//
	//		return true;
	//	}
	//};

	template <typename T>
	struct Savegame::PhobosStreamObject<VectorClass<T , GameAllocator<T>>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, VectorClass<T, GameAllocator<T>>& Value, bool RegisterForChange) const
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

		bool WriteToStream(PhobosStreamWriter& Stm, const VectorClass<T, GameAllocator<T>>& Value) const
		{
			Stm.Save(Value.Capacity);

			for (auto ix = 0; ix < Value.Capacity; ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value.Items[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<VectorClass<T, DllAllocator<T>>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, VectorClass<T, DllAllocator<T>>& Value, bool RegisterForChange) const
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

		bool WriteToStream(PhobosStreamWriter& Stm, const VectorClass<T, DllAllocator<T>>& Value) const
		{
			Stm.Save(Value.Capacity);

			for (auto ix = 0; ix < Value.Capacity; ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value.Items[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<DynamicVectorClass<T ,GameAllocator<T>>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, DynamicVectorClass<T, GameAllocator<T>>& Value, bool RegisterForChange) const
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

		bool WriteToStream(PhobosStreamWriter& Stm, const DynamicVectorClass<T, GameAllocator<T>>& Value) const
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
	struct Savegame::PhobosStreamObject<DynamicVectorClass<T, DllAllocator<T>>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, DynamicVectorClass<T, DllAllocator<T>> & Value, bool RegisterForChange) const
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

		bool WriteToStream(PhobosStreamWriter& Stm, const DynamicVectorClass<T, DllAllocator<T>> & Value) const
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
	struct Savegame::PhobosStreamObject<TypeList<T, GameAllocator<T>>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, TypeList<T, GameAllocator<T>>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream<DynamicVectorClass<T, GameAllocator<T>>>(Stm, Value, RegisterForChange))
				return false;

			return Stm.Load(Value.unknown_18);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const TypeList<T, GameAllocator<T>>& Value) const
		{
			if (!Savegame::WritePhobosStream<DynamicVectorClass<T, GameAllocator<T>>>(Stm, Value))
				return false;

			Stm.Save(Value.unknown_18);
			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<TypeList<T, DllAllocator<T>>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, TypeList<T, DllAllocator<T>>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream<DynamicVectorClass<T, DllAllocator<T>>>(Stm, Value, RegisterForChange))
				return false;

			return Stm.Load(Value.unknown_18);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const TypeList<T, DllAllocator<T>>& Value) const
		{
			if (!Savegame::WritePhobosStream<DynamicVectorClass<T, DllAllocator<T>>>(Stm, Value))
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
			if (!(Savegame::ReadPhobosStream(Stm, Value.X, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.Y, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.Z, RegisterForChange)))
				return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Vector3D<T>& Value) const
		{
			if (!(Savegame::WritePhobosStream(Stm, Value.X)
				&& Savegame::WritePhobosStream(Stm, Value.Y)
				&& Savegame::WritePhobosStream(Stm, Value.Z)))
				return false;
			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<Vector2D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, Vector2D<T>& Value, bool RegisterForChange) const
		{
			if (!(Savegame::ReadPhobosStream(Stm, Value.X, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.Y, RegisterForChange)
				))
				return false;

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Vector2D<T>& Value) const
		{
			if (!(Savegame::WritePhobosStream(Stm, Value.X)
				&& Savegame::WritePhobosStream(Stm, Value.Y)
				))
				return false;

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<Vector4D<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, Vector4D<T>& Value, bool RegisterForChange) const
		{
			if (!(Savegame::ReadPhobosStream(Stm, Value.X, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.Y, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.Z, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.W, RegisterForChange)
				))
				return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const Vector4D<T>& Value) const
		{
			if (!(Savegame::WritePhobosStream(Stm, Value.X)
				&& Savegame::WritePhobosStream(Stm, Value.Y)
				&& Savegame::WritePhobosStream(Stm, Value.Z)
				&& Savegame::WritePhobosStream(Stm, Value.W)
				))
				return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<WeaponStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, WeaponStruct& Value, bool RegisterForChange) const
		{
			return Savegame::ReadPhobosStream(Stm, Value.WeaponType, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.FLH, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.BarrelLength, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.BarrelThickness, RegisterForChange)
				&& Savegame::ReadPhobosStream(Stm, Value.TurretLocked, RegisterForChange);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const WeaponStruct& Value) const
		{
			return Savegame::WritePhobosStream(Stm, Value.WeaponType)
				&& Savegame::WritePhobosStream(Stm, Value.FLH)
				&& Savegame::WritePhobosStream(Stm, Value.BarrelLength)
				&& Savegame::WritePhobosStream(Stm, Value.BarrelThickness)
				&& Savegame::WritePhobosStream(Stm, Value.TurretLocked);
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<TintStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, TintStruct& Value, bool RegisterForChange) const
		{
			if (!(Savegame::ReadPhobosStream<int>(Stm, Value.Red, RegisterForChange)
				&& Savegame::ReadPhobosStream<int>(Stm, Value.Green, RegisterForChange)
				&& Savegame::ReadPhobosStream<int>(Stm, Value.Blue, RegisterForChange)))
				return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const TintStruct& Value) const
		{
			if (!(Savegame::WritePhobosStream(Stm, Value.Red)
				&& Savegame::WritePhobosStream(Stm, Value.Green)
				&& Savegame::WritePhobosStream(Stm, Value.Blue)))
				return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<LightingStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, LightingStruct& Value, bool RegisterForChange) const
		{
			PhobosStreamObject<TintStruct> item;
			if (!(item.ReadFromStream(Stm, Value.Tint, RegisterForChange)
				&& Savegame::ReadPhobosStream<int>(Stm, Value.Ground, RegisterForChange)
				&& Savegame::ReadPhobosStream<int>(Stm, Value.Level, RegisterForChange)))
				return false;
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const LightingStruct& Value) const
		{
			PhobosStreamObject<TintStruct> item;

			if (!(item.WriteToStream(Stm, Value.Tint)
				&& Savegame::WritePhobosStream<int>(Stm, Value.Ground)
				&& Savegame::WritePhobosStream<int>(Stm, Value.Level)))
				return false;
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<CounterClass<GameAllocator<int>>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, CounterClass<GameAllocator<int>>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream<VectorClass<int, GameAllocator<int>>>(Stm, Value, RegisterForChange))
				return false;

			return Stm.Load(Value.Total);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const CounterClass<GameAllocator<int>>& Value) const
		{
			if (!Savegame::WritePhobosStream<VectorClass<int, GameAllocator<int>>>(Stm, Value))
				return false;

			Stm.Save(Value.Total);
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<CounterClass<DllAllocator<int>>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, CounterClass<DllAllocator<int>>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream<VectorClass<int, DllAllocator<int>>>(Stm, Value, RegisterForChange))
				return false;

			return Stm.Load(Value.Total);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const CounterClass<DllAllocator<int>>& Value) const
		{
			if (!Savegame::WritePhobosStream<VectorClass<int, DllAllocator<int>>>(Stm, Value))
				return false;

			Stm.Save(Value.Total);
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<ScriptActionNode>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, ScriptActionNode& Value, bool RegisterForChange) const {
			return Stm.Load(Value);
		};

		bool WriteToStream(PhobosStreamWriter& Stm, const ScriptActionNode& Value) const {
			Stm.Save(Value);
			return true;
		};
	};

	template <>
	struct Savegame::PhobosStreamObject<std::string>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::string& Value, bool RegisterForChange) const
		{
			size_t size = 0;

			if (Stm.Load(size)) {
				if (!size) {
					Value.clear();
					return true;
				}

				std::vector<char> buffer(size);
				if (Stm.Read(reinterpret_cast<BYTE*>(buffer.data()), size)) {
					Value.assign(buffer.begin(), buffer.end());
					return true;
				}
			}

			return false;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::string& Value) const
		{
			Stm.Save(Value.size());

			if (Value.empty())
				return true;

			Stm.Write(reinterpret_cast<const BYTE*>(Value.c_str()), Value.size());
			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::unique_ptr<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::unique_ptr<T>& Value, bool RegisterForChange) const
		{
			Value.reset(RestoreObject<T>(Stm, RegisterForChange));
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unique_ptr<T>& Value) const
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
			const auto ret = Stm.Load(hasvalue);

			if (ret && hasvalue) {
				auto ptrNew = GameCreate<BytePalette>();
				for (int i = 0; i < BytePalette::EntriesCount; ++i) {
					ColorStruct nDummy;
					Stm.Load(nDummy);
					ptrNew->Entries[i] = nDummy;
				}

				Value.reset(ptrNew);
				return true;
			}

			Value.reset(nullptr);
			return ret;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const UniqueGamePtr<BytePalette>& Value) const
		{
			const bool Exist = Value.get() != nullptr;
			Stm.Save(Exist);
			if(Exist){
				for (const auto& color : Value.get()->Entries) {
					Stm.Save(color);
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
			if (Value && !Value->IsReference())
				Debug::Log("Value contains SHP file data. Possible leak.\n");

			Value = nullptr;

			bool hasValue = true;
			if (Savegame::ReadPhobosStream(Stm, hasValue) && hasValue)
			{
				std::string name;
				if (Savegame::ReadPhobosStream(Stm, name))
				{
					if (auto pSHP = FileSystem::LoadSHPFile(name.c_str()))
					{
						Value = pSHP;
						return true;
					}
				}
			}

			return !hasValue;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, SHPStruct* const& Value) const
		{
			const char* filename = nullptr;
			if (Value)
			{
				if (auto pRef = Value->AsReference())
					filename = pRef->Filename;
				else
					Debug::Log("Cannot save SHPStruct, because it isn't a reference.\n");
			}

			if (Savegame::WritePhobosStream(Stm, filename != nullptr))
			{
				if (filename)
				{
					std::string file(filename);
					return Savegame::WritePhobosStream(Stm, file);
				}
			}

			return filename == nullptr;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<Theater_SHPStruct*>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, Theater_SHPStruct*& Value, bool RegisterForChange) const
		{
			if (Value && !Value->IsReference())
				Debug::Log("Value contains SHP file data. Possible leak.\n");

			Value = nullptr;

			bool hasValue = true;
			if (Savegame::ReadPhobosStream(Stm, hasValue) && hasValue)
			{
				std::string name;
				if (Savegame::ReadPhobosStream(Stm, name))
				{
					if (auto pSHP = FileSystem::LoadSHPFile(name.c_str()))
					{
						Value = static_cast<Theater_SHPStruct*>(pSHP);
						return true;
					}
				}
			}

			return !hasValue;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, Theater_SHPStruct* const& Value) const
		{
			const char* filename = nullptr;
			if (Value)
			{
				if (auto pRef = Value->AsReference())
					filename = pRef->Filename;
				else
					Debug::Log("Cannot save SHPStruct, because it isn't a reference.\n");
			}

			if (Savegame::WritePhobosStream(Stm, filename != nullptr))
			{
				if (filename)
				{
					std::string file(filename);
					return Savegame::WritePhobosStream(Stm, file);
				}
			}

			return filename == nullptr;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<RocketStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, RocketStruct& Value, bool RegisterForChange) const
		{
			if (!Stm.Load(Value))
				return false;

			if (RegisterForChange)
				Swizzle swizzle(Value.Type);

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const RocketStruct& Value) const
		{
			Stm.Save(Value);
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<BuildType>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, BuildType& Value, bool RegisterForChange) const
		{
			if (!Stm.Load(Value))
				return false;

			if (RegisterForChange)
				Swizzle swizzle(Value.CurrentFactory);

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const BuildType& Value) const
		{
			Stm.Save(Value);
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
			if (Stm.Load(HasValue))
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
			Stm.Save(Value.has_value());
			if (Value.has_value())
				return (Savegame::WritePhobosStream(Stm, Value.value()));

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::vector<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::vector<T>& Value, bool RegisterForChange) const
		{
			Value.clear();

			size_t Capacity = 0;
			if (!Stm.Load(Capacity))
				return false;

			Value.reserve(Capacity);

			size_t Count = 0;

			if (!Stm.Load(Count))
				return false;

			Value.resize(Count);

			for (auto ix = 0u; ix < Count; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::vector<T>& Value) const
		{
			Stm.Save(Value.capacity());
			Stm.Save(Value.size());

			for (auto ix = 0u; ix < Value.size(); ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<HelperedVector<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, HelperedVector<T>& Value, bool RegisterForChange) const
		{
			Value.clear();

			size_t Capacity = 0;
			if (!Stm.Load(Capacity))
				return false;

			Value.reserve(Capacity);

			size_t Count = 0;

			if (!Stm.Load(Count))
				return false;

			Value.resize(Count);

			for (auto ix = 0u; ix < Count; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const HelperedVector<T>& Value) const
		{
			Stm.Save(Value.capacity());
			Stm.Save(Value.size());

			for (auto ix = 0u; ix < Value.size(); ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value[ix]))
					return false;
			}

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<std::vector<bool>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::vector<bool>& Value, bool RegisterForChange) const
		{
			Value.clear();

			size_t Capacity = 0;
			if (!Stm.Load(Capacity))
				return false;

			Value.reserve(Capacity);

			size_t Count = 0;

			if (!Stm.Load(Count))
				return false;

			Value.resize(Count);

			for (auto ix = 0u; ix < Count; ++ix)
			{
				bool value = false;

				if (!Savegame::ReadPhobosStream(Stm, value, false))
					return false;

				Value.emplace_back(value);

			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::vector<bool>& Value) const
		{
			Stm.Save(Value.capacity());
			Stm.Save(Value.size());

			for (auto ix = 0u; ix < Value.size(); ++ix)
			{
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
			if (!Savegame::ReadPhobosStream(Stm, Value.first, RegisterForChange)
				|| !Savegame::ReadPhobosStream(Stm, Value.second, RegisterForChange))
			{
				return false;
			}
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::pair<_Ty1, _Ty2>& Value) const
		{
			if (!Savegame::WritePhobosStream(Stm, Value.first)
				|| !Savegame::WritePhobosStream(Stm, Value.second))
			{
				return false;
			}
			return true;
		}
	};

	template <typename TKey, typename TValue>
	struct Savegame::PhobosStreamObject<std::map<TKey, TValue>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::map<TKey, TValue>& Value, bool RegisterForChange) const
		{
			Value.clear();

			size_t Count = 0;
			if (!Stm.Load(Count))
				return false;

			if (Count <= 0)
				return true;

			for (auto ix = 0u; ix < Count; ++ix)
			{
				std::pair<TKey, TValue> buffer;
				if (!Savegame::ReadPhobosStream(Stm, buffer.first, RegisterForChange)
					|| !Savegame::ReadPhobosStream(Stm, buffer.second, RegisterForChange))
				{
					return false;
				}

				Value.insert(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::map<TKey, TValue>& Value) const
		{
			size_t const nSize = Value.size();
			Stm.Save(nSize);

			if (nSize <= 0)
				return true;

			for (const auto& [first, second] : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, first) || !
				Savegame::WritePhobosStream(Stm, second))
					return false;
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

			size_t Count = 0;
			if (!Stm.Load(Count)) {
				return false;
			}

			//Debug::Log("Loading std::set with(%s) size %d\n", typeid(T).name(), Count);

			if (!Count)
				return true;

			for (auto ix = 0u; ix < Count; ++ix) {

				T buffer = T();
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
				{
					return false;
				}
				Value.insert(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::set<T>& Value) const
		{
			Stm.Save(Value.size());
			//Debug::Log("Saving std::set with(%s) size %d\n", typeid(T).name(), Value.size());

			for (const auto& item : Value) {
				if (!Savegame::WritePhobosStream(Stm, item)) {
					return false;
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

			size_t Count = 0;
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
			Stm.Save(Value.size());

			for (const auto& item : Value) {
				if (!Savegame::WritePhobosStream(Stm, item)) {
					return false;
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

			size_t count = 0;

			if (!Stm.Load(count))
				return false;

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
			Stm.Save(Value.size());

			for (auto ix = 0u; ix < Value.size(); ++ix)
			{
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
			size_t nSize = 0;
			if (!Stm.Load(nSize))
				return false;

			if (nSize <= 0) //we loaded an queue but it is empty , just return true
				return true;

			for (size_t ix = 0u; ix < nSize; ++ix)
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
			Stm.Save(Value.size());

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
			size_t nSize = 0;
			if (!Stm.Load(nSize))
				return false;

			if (nSize <= 0)
				return true;

			for (size_t ix = 0u; ix < nSize; ++ix)
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
			Stm.Save(Value.size());

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
			std::memset(Value.data(), 0, sizeof(T) * size);

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

			size_t Count = 0;
			if (!Stm.Load(Count)) {
				return false;
			}

			if (Count <= 0)
				return true;

			for (auto ix = 0u; ix < Count; ++ix)
			{
				std::pair<TKey, TValue> buffer;
				if (!Savegame::ReadPhobosStream(Stm, buffer.first, RegisterForChange)
					|| !Savegame::ReadPhobosStream(Stm, buffer.second, RegisterForChange))
				{
					return false;
				}

				Value.insert(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unordered_map<TKey, TValue>& Value) const
		{
			size_t const nSize = Value.size();
			Stm.Save(nSize);

			if (nSize <= 0)
				return true;

			for (const auto& [first, second] : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, first) || !
				Savegame::WritePhobosStream(Stm, second))
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

			size_t Count = 0;
			if (!Stm.Load(Count))
			{
				return false;
			}

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
			Stm.Save(Value.size());

			for (const auto&[key , val] : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, key)
					|| !Savegame::WritePhobosStream(Stm, val))
				{
					return false;
				}
			}
			return true;
		}
	};

}
