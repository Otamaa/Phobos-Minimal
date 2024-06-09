#pragma once

#include <Utilities/Savegame.h>
#include <Utilities/Debug.h>

#include <unordered_map>

namespace Container
{
	class MapBase final
	{
	public:
		using key_type = void*;
		using const_key_type = const void*;
		using value_type = void*;
		using map_type = std::unordered_map<const_key_type, value_type>;
		using const_iterator = map_type::const_iterator;
		using iterator = const_iterator;

		MapBase() = default;
		MapBase(MapBase const&) = delete;
		~MapBase() = default;

		MapBase& operator=(MapBase const&) = delete;
		MapBase& operator=(MapBase&&) = delete;

		value_type find(const_key_type key) const
		{
			auto const it = this->Items.find(key);
			if (it != this->Items.end())
				return it->second;

			return nullptr;
		}

		void insert(const_key_type key, value_type value)
		{
			this->Items.emplace(key, value);
		}

		value_type remove(const_key_type key)
		{
			auto const it = this->Items.find(key);
			if (it != this->Items.cend())
			{
				auto const value = it->second;
				this->Items.erase(it);

				return value;
			}

			return nullptr;
		}

		void clear()
		{
			// this leaks all objects inside. this case is logged.
			this->Items.clear();
		}

		size_t size() const
		{
			return this->Items.size();
		}

		const_iterator begin() const
		{
			return this->Items.cbegin();
		}

		const_iterator end() const
		{
			return this->Items.cend();
		}

	private:
		map_type Items;
	};

	template <typename Key, typename Value>
	class Map final
	{
	public:
		using key_type = Key*;
		using const_key_type = const Key*;
		using value_type = Value*;
		using iterator = typename std::unordered_map<key_type, value_type>::const_iterator;

		Map() = default;
		Map(Map const&) = delete;

		Map& operator=(Map const&) = delete;
		Map& operator=(Map&&) = delete;

		value_type find(const_key_type key) const
		{
			return static_cast<value_type>(this->Items.find(key));
		}

		value_type insert(const_key_type key, value_type value)
		{
			this->Items.insert(key, value);
			return value;
		}

		value_type remove(const_key_type key)
		{
			return static_cast<value_type>(this->Items.remove(key));
		}

		void clear()
		{
			this->Items.clear();
		}

		size_t size() const
		{
			return this->Items.size();
		}

		iterator begin() const
		{
			auto ret = this->Items.begin();
			return reinterpret_cast<iterator&>(ret);
		}

		iterator end() const
		{
			auto ret = this->Items.end();
			return reinterpret_cast<iterator&>(ret);
		}

	private:
		MapBase Items;
	};

	template <class T>
	concept HasOffset = requires(T) { T::ExtPointerOffset; };

	template <typename T>
	class Main
	{
	private:
		using base_type = typename T::base_type;
		using extension_type = typename T::ExtData;
		using base_type_ptr = base_type*;
		using const_base_type_ptr = const base_type*;
		using extension_type_ptr = extension_type*;
		using map_type = Map<base_type, extension_type>;

		map_type Items;

		base_type* SavingObject;
		IStream* SavingStream;
		std::string Name;

	public:
		constexpr explicit Main(const char* pName) :
			Items(),
			SavingObject(nullptr),
			SavingStream(nullptr),
			Name(pName)
		{
		}

		virtual ~Container() = default;

		void PointerGotInvalid(void* ptr)
		{
			if (!this->InvalidateExtDataIgnorable(ptr))
				this->InvalidateExtDataPointer(ptr);
		}

		void ObjectWantDetach(void* ptr, bool all)
		{
			if (!this->DetachExtDataIgnorable(ptr))
				this->DetachExtDataPointer(ptr, all);
		}

	protected:

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const
		{
			return true;
		}

		void InvalidateExtDataPointer(void* const ptr) const
		{
			for (const auto& i : this->Items)
				i.second->InvalidatePointer(ptr);
		}

		virtual bool DetachExtDataIgnorable(void* const ptr) const
		{
			return true;
		}

		void DetachExtDataPointer(void* const ptr, bool all) const
		{
			for (const auto& i : this->Items)
				i.second->Detach(ptr, all);
		}

	private:

		extension_type_ptr GetExtensionPointer(const_base_type_ptr key) const
		{
			return (extension_type_ptr)(*(uintptr_t*)((char*)key + T::ExtPointerOffset));
		}

		void SetExtensionPointer(base_type_ptr key, extension_type_ptr value)
		{
			(*(uintptr_t*)((char*)key + T::ExtPointerOffset)) = (uintptr_t)value;
		}

		void ResetExtensionPointer(base_type_ptr key)
		{
			(*(uintptr_t*)((char*)key + T::ExtPointerOffset)) = 0;
		}

	public:

		extension_type_ptr Allocate(base_type_ptr key)
		{
			if constexpr (HasOffset<T>)
				ResetExtensionPointer(key);

			if (auto const val = new extension_type(key))
			{
				val->EnsureConstanted();

				if constexpr (HasOffset<T>)
					SetExtensionPointer(key, val);

				this->Items.insert(key, val);

				return val;
			}

			return nullptr;
		}

		extension_type_ptr TryAllocate(base_type_ptr key, bool bCond, const std::string_view& nMessage)
		{
			if (!key || (!bCond && !nMessage.empty()))
			{
				Debug::Log("%s \n", nMessage.data());
				return nullptr;
			}

			return Allocate(key);
		}

		extension_type_ptr TryAllocate(base_type_ptr key)
		{
			if (!key)
			{
				Debug::Log("Attempted to allocate %s from nullptr!\n", typeid(extension_type).name());
				return nullptr;
			}

			return Allocate(key);
		}

		extension_type_ptr FindOrAllocate(base_type_ptr key)
		{
			// Find Always check for nullptr here
			if (auto const ptr = Find(key))
				return ptr;

			return Allocate(key);
		}

		extension_type_ptr Find(const_base_type_ptr key) const
		{
			if (!key)
				return nullptr;

			if constexpr (HasOffset<T>)
				return GetExtensionPointer(key);
			else
				return this->Items.find(key);
		}

		void Remove(base_type_ptr key)
		{
			if (auto Item = Find(key))
			{
				this->Items.remove(key);
				delete Item;

				if constexpr (HasOffset<T>)
					ResetExtensionPointer(key);
			}
		}

		void Clear()
		{
			if (this->Items.size())
			{
				Debug::Log("Cleared %u items from %s.\n", this->Items.size(), this->Name);
				this->Items.clear();
			}
		}

		void LoadFromINI(const_base_type_ptr key, CCINIClass* pINI)
		{
			if (auto ptr = this->Find(key))
				ptr->LoadFromINI(pINI);
		}

		void PrepareStream(base_type_ptr key, IStream* pStm)
		{
			//Debug::Log("[PrepareStream] Next is %p of type '%s'\n", key, this->Name);

			this->SavingObject = key;
			this->SavingStream = pStm;
		}

		void SaveStatic()
		{
			if (this->SavingObject && this->SavingStream)
			{
				//Debug::Log("[SaveStatic] Saving object %p as '%s'\n", this->SavingObject, this->Name);
				if (!this->Save(this->SavingObject, this->SavingStream))
					Debug::FatalErrorAndExit("SaveStatic - Saving object %p as '%s' failed!\n", this->SavingObject, this->Name);
			}
			else
			{
				Debug::Log("SaveStatic - Object or Stream not set for '%s': %p, %p\n",
					this->Name, this->SavingObject, this->SavingStream);
			}

			this->SavingObject = nullptr;
			this->SavingStream = nullptr;
		}

		void LoadStatic()
		{
			if (this->SavingObject && this->SavingStream)
			{
				//Debug::Log("[LoadStatic] Loading object %p as '%s'\n", this->SavingObject, this->Name);
				if (!this->Load(this->SavingObject, this->SavingStream))
					Debug::FatalErrorAndExit("LoadStatic - Loading object %p as '%s' failed!\n", this->SavingObject, this->Name);
			}
			else
			{
				Debug::Log("LoadStatic - Object or Stream not set for '%s': %p, %p\n",
					this->Name, this->SavingObject, this->SavingStream);
			}

			this->SavingObject = nullptr;
			this->SavingStream = nullptr;
		}

		decltype(auto) begin() const
		{
			return this->Items.begin();
		}

		decltype(auto) end() const
		{
			return this->Items.end();
		}

		size_t size() const
		{
			return this->Items.size();
		}

	protected:
		// override this method to do type-specific stuff
		virtual bool Save(base_type_ptr key, IStream* pStm)
		{
			return this->SaveKey(key, pStm) != nullptr;
		}

		// override this method to do type-specific stuff
		virtual bool Load(base_type_ptr key, IStream* pStm)
		{
			return this->LoadKey(key, pStm) != nullptr;
		}

		extension_type_ptr SaveKey(base_type_ptr key, IStream* pStm)
		{
			// this really shouldn't happen
			if (!key)
			{
				Debug::Log("SaveKey - Attempted for a null pointer! WTF!\n");
				return nullptr;
			}

			// get the value data
			auto buffer = this->Find(key);
			if (!buffer)
			{
				Debug::Log("SaveKey - Could not find value.\n");
				return nullptr;
			}

			// write the current pointer, the size of the block, and the canary
			PhobosByteStream saver(sizeof(*buffer));
			PhobosStreamWriter writer(saver);

			writer.Save(T::Canary);
			writer.Save(buffer);

			// save the data
			buffer->SaveToStream(writer);

			// save the block
			if (!saver.WriteBlockToStream(pStm))
			{
				Debug::Log("SaveKey - Failed to save data.\n");
				return nullptr;
			}

			//Debug::Log("[SaveKey] Save used up 0x%X bytes\n", saver.Size());

			return buffer;
		}

		extension_type_ptr LoadKey(base_type_ptr key, IStream* pStm)
		{
			// this really shouldn't happen
			if (!key)
			{
				Debug::Log("LoadKey - Attempted for a null pointer! WTF!\n");
				return nullptr;
			}

			extension_type_ptr buffer = this->Allocate(key);

			if (!buffer)
			{
				Debug::Log("LoadKey - Could not find or allocate value.\n");
				return nullptr;
			}

			PhobosByteStream loader(0);
			if (!loader.ReadBlockFromStream(pStm))
			{
				Debug::Log("LoadKey - Failed to read data from save stream?!\n");
				return nullptr;
			}

			PhobosStreamReader reader(loader);

			if (reader.Expect(T::Canary) && reader.RegisterChange(buffer))
			{
				buffer->LoadFromStream(reader);
				if (reader.ExpectEndOfBlock())
					return buffer;
			}

			return nullptr;
		}

	private:
		Main(const Main&) = delete;
		Main& operator = (const Main&) = delete;
		Main& operator = (Main&&) = delete;
	};

}