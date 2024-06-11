#pragma once

#include <Utilities/Container.h>

template <typename T>
class Extension
{
	T* AttachedToObject;
	InitState Initialized;

public:
	static const DWORD Canary;

	Extension(T* const OwnerObject) : AttachedToObject{ OwnerObject }, Initialized{ InitState::Blank }
	{ }

	Extension(const Extension& other) = delete;

	void operator=(const Extension& RHS) = delete;

	virtual ~Extension() = default;

	// the object this Extension expands
	T* const& OwnerObject() const
	{
		return this->AttachedToObject;
	}

	void EnsureConstanted()
	{
		if (this->Initialized < InitState::Constanted)
		{
			this->InitializeConstants();
			this->Initialized = InitState::Constanted;
		}
	}

	void LoadFromINI(CCINIClass* pINI)
	{
		if (!pINI)
			return;

		switch (this->Initialized)
		{
		case InitState::Blank:
			this->EnsureConstanted();
		case InitState::Constanted:
			this->InitializeRuled();
			this->Initialized = InitState::Ruled;
		case InitState::Ruled:
			this->Initialize();
			this->Initialized = InitState::Inited;
		case InitState::Inited:
		case InitState::Completed:
			if (pINI == CCINIClass::INI_Rules)
				this->LoadFromRulesFile(pINI);

			this->LoadFromINIFile(pINI);
			this->Initialized = InitState::Completed;
		}
	}

	virtual void InvalidatePointer(void* ptr) {};
	virtual void Detach(void* ptr, bool all) {};

	virtual inline void SaveToStream(PhobosStreamWriter& Stm)
	{
		//Stm.Save(this->AttachedToObject);
		Stm.Save(this->Initialized);
	}

	virtual inline void LoadFromStream(PhobosStreamReader& Stm)
	{
		//Stm.Load(this->AttachedToObject);
		Stm.Load(this->Initialized);
	}

protected:
	// right after construction. only basic initialization tasks possible;
	// owner object is only partially constructed! do not use global state!
	virtual void InitializeConstants() { }

	virtual void InitializeRuled() { }

	// called before the first ini file is read
	virtual void Initialize() { }

	// for things that only logically work in rules - countries, sides, etc
	virtual void LoadFromRulesFile(CCINIClass* pINI) { }

	// load any ini file: rules, game mode, scenario or map
	virtual void LoadFromINIFile(CCINIClass* pINI) { }
		};

template <class T>
concept HasOffsetPointer = requires(T) { T::ExtPointerOffset; };

template <typename T>
class ExtMapCointainer
{
private:
	using base_type = typename T::base_type;
	using extension_type = typename T::ExtData;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using extension_type_ptr = extension_type*;
	using value_type = void*;
	using key_type = void*;
	using const_key_type = const void*;

	using map_type = std::vector<std::pair<base_type_ptr, extension_type_ptr>>;

	using const_iterator = map_type::const_iterator;
	using iterator = const_iterator;

	map_type Items;

	base_type* SavingObject;
	IStream* SavingStream;
	const char* Name;

public:
	explicit ExtMapCointainer(const char* pName) :
		Items(),
		SavingObject(nullptr),
		SavingStream(nullptr),
		Name(pName)
	{
	}

	virtual ~ExtMapCointainer() = default;

	void PointerGotInvalid(void* ptr)
	{
		if (!this->InvalidateExtDataIgnorable(ptr))
			this->InvalidateExtDataPointer(ptr);
	}

	void ObjectWantDetach(void* ptr, bool all)
	{
		//this->InvalidatePointer(ptr, bRemoved);

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
		if constexpr (HasOffsetPointer<T>)
			ResetExtensionPointer(key);

		if (auto const val = new extension_type(key))
		{
			val->EnsureConstanted();

			if constexpr (HasOffsetPointer<T>)
				SetExtensionPointer(key, val);

			this->Items.emplace_back(key, val);

			return val;
		}

		return nullptr;
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

	extension_type_ptr Find_Base(const_base_type_ptr key)
	{
		for (auto begin = this->Items.begin(); begin != this->Items.end(); ++begin) {
			if (begin->first == key) {
				return begin->second;
			}
		}

		return nullptr;
	}

	decltype(auto) GetIterator(const_base_type_ptr key) const
	{
		for (auto begin = this->Items.begin(); begin != this->Items.end(); ++begin) {
			return begin;
		}

		return this->Items.end();
	}

	extension_type_ptr Find(const_base_type_ptr key) const
	{
		if (!key)
			return nullptr;

		if constexpr (HasOffsetPointer<T>)
			return GetExtensionPointer(key);
		else
			return const_cast<ExtMapCointainer<T>*>(this)->Find_Base(key);
	}

	void Remove(base_type_ptr key)
	{
		auto Item = GetIterator(key);

		if (Item != this->Items.end())
		{
			auto delete_ = *Item;
			this->Items.erase(Item);
			delete delete_;

			if constexpr (HasOffsetPointer<T>)
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
	ExtMapCointainer(const ExtMapCointainer&) = delete;
	ExtMapCointainer& operator = (const ExtMapCointainer&) = delete;
	ExtMapCointainer& operator = (ExtMapCointainer&&) = delete;
};
