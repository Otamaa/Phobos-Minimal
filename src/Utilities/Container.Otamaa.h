#pragma once
#include "Container.h"


template<typename T>
class DetailedExtension : public Extension<T>
{
	bool _awaked;
	bool _started;

public:
	DetailedExtension(T* const OwnerObject) : Extension<T> { OwnerObject }
	{ }

	DetailedExtension(const DetailedExtension& other) = delete;

	void operator=(const DetailedExtension& RHS) = delete;

	virtual ~DetailedExtension() = default;

	virtual void OnUpdate() { }
	virtual void OnLimbo() { }
	virtual void OnUnlimbo() { }
	virtual void OnInit() { }

};

template <typename T>
class OriginalContainer
{
private:
	using base_type = typename T::base_type;
	using extension_type = typename T::ExtData;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using extension_type_ptr = extension_type*;

	ContainerMap<base_type, extension_type> Items;

	base_type_ptr SavingObject;
	IStream* SavingStream;
	FixedString<0x100> Name;

public:
	explicit OriginalContainer(const char* pName) :
		Items {},
		SavingObject { nullptr },
		SavingStream { nullptr },
		Name {}
	{  Name = pName; }

	virtual ~OriginalContainer() = default;

	void PointerGotInvalid(void* ptr, bool bRemoved)
	{
		this->InvalidatePointer(ptr, bRemoved);
		if (!this->InvalidateExtDataIgnorable(ptr))
			this->InvalidateExtDataPointer(ptr, bRemoved);
	}

	auto begin() const
	{
		return this->Items.begin();
	}

	auto end() const
	{
		return this->Items.end();
	}

	auto size() const
	{
		return this->Items.size();
	}

	auto GetName() const
	{
		return this->Name.data();
	}

protected:
	virtual void InvalidatePointer(void* ptr, bool bRemoved) { }

	virtual bool InvalidateExtDataIgnorable(void* const ptr) const
	{
		return true;
	}

	void InvalidateExtDataPointer(void* ptr, bool bRemoved)
	{
		for (const auto& i : this->Items)
			i.second->InvalidatePointer(ptr, bRemoved);
	}

public:

#define Alloc_EXT_ORIGINAL(key)\
if (auto val = new extension_type(key)) {\
	val->EnsureConstanted();\
	return this->Items.insert(key, val); }\
    Debug::Log("CTOR of %s failed to allocate extension ! WTF!\n", this->Name.data());\
return nullptr;

	extension_type_ptr JustAllocate(base_type_ptr key)
	{
		Alloc_EXT_ORIGINAL(key);
	}

	extension_type_ptr FindOrAllocate(base_type_ptr key)
	{
		if (key == nullptr)
		{
			Debug::Log("CTOR of %s attempted for a NULL pointer! WTF!\n", this->Name.data());
			return nullptr;
		}

		if (auto const ptr = this->Items.find(key))
			return ptr;

		Alloc_EXT_ORIGINAL(key);
	}
#undef Alloc_EXT_ORIGINAL

	extension_type_ptr Find(const_base_type_ptr key) const
	{
		return this->Items.find(key);
	}

	extension_type_ptr operator[] (const_base_type_ptr key) const
	{
		return this->Items.find(key);
	}

	void Remove(const_base_type_ptr key)
	{
		if (auto Item = this->Items.remove(key))
		{
			Item->Uninitialize();
			delete Item;
		}
	}

	void Clear()
	{
		if (this->Items.size())
		{
			Debug::Log("Cleared %u items from %s.\n", this->Items.size(), this->Name.data());
			this->Items.clear();
		}
	}

	void LoadAllFromINI(CCINIClass* pINI)
	{
		for (const auto& i : this->Items)
			i.second->LoadFromINI(pINI);
	}

	void LoadFromINI(const_base_type_ptr key, CCINIClass* pINI)
	{
		if (auto const ptr = this->Items.find(key))
			ptr->LoadFromINI(pINI);
	}

	void PrepareStream(base_type_ptr key, IStream* pStm)
	{
		Debug::Log("[PrepareStream] Next is %p of type '%s'\n", key, this->Name.data());
		this->SavingObject = key;
		this->SavingStream = pStm;
	}

	IStream* GetStream() const
	{
		return this->SavingStream;
	}

	void SaveStatic()
	{
		if (this->SavingObject && this->SavingStream)
		{
			Debug::Log("[SaveStatic] Saving object %p as '%s'\n", this->SavingObject, this->Name.data());
			if (!this->Save(this->SavingObject, this->SavingStream))
				Debug::FatalErrorAndExit("[SaveStatic] Saving failed!\n");
		}
		else
		{
			Debug::Log("[SaveStatic] Object or Stream not set for '%s': %p, %p\n",
				this->Name.data(), this->SavingObject, this->SavingStream);
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
	}

	void LoadStatic()
	{
		if (this->SavingObject && this->SavingStream)
		{
			Debug::Log("[LoadStatic] Loading object %p as '%s'\n", this->SavingObject, this->Name.data());
			if (!this->Load(this->SavingObject, this->SavingStream))
				Debug::FatalErrorAndExit("[LoadStatic] Loading object %p as '%s failed!\n", this->SavingObject, this->Name.data());
		}
		else
		{
			Debug::Log("[LoadStatic] Object or Stream not set for '%s': %p, %p\n",
				this->Name.data(), this->SavingObject, this->SavingStream);
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
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
			Debug::Log("[SaveKey] Attempted for a null pointer! WTF!\n");
			return nullptr;
		}

		// get the value data
		auto buffer = this->Find(key);
		if (!buffer)
		{
			Debug::Log("[SaveKey] Could not find value.\n");
			return nullptr;
		}

		// write the current pointer, the size of the block, and the canary
		PhobosByteStream saver(sizeof(*buffer));
		PhobosStreamWriter writer(saver);

		writer.Save(extension_type::Canary);
		writer.Save(buffer);

		// save the data
		buffer->SaveToStream(writer);

		// save the block
		if (!saver.WriteBlockToStream(pStm))
		{
			Debug::Log("[SaveKey] Failed to save data.\n");
			return nullptr;
		}

		Debug::Log("[SaveKey] Save used up 0x%X bytes\n", saver.Size());

		return buffer;
	}

	extension_type_ptr LoadKey(base_type_ptr key, IStream* pStm)
	{
		// this really shouldn't happen
		if (!key)
		{
			Debug::Log("[LoadKey] Attempted for a null pointer! WTF!\n");
			return nullptr;
		}

		// get the value data
		auto buffer = this->FindOrAllocate(key);
		if (!buffer)
		{
			Debug::Log("[LoadKey] Could not find or allocate value.\n");
			return nullptr;
		}

		PhobosByteStream loader(0);
		if (!loader.ReadBlockFromStream(pStm))
		{
			Debug::Log("[LoadKey] Failed to read data from save stream?!\n");
			return nullptr;
		}

		PhobosStreamReader reader(loader);
		if (reader.Expect(extension_type::Canary) && reader.RegisterChange(buffer))
		{
			buffer->LoadFromStream(reader);
			if (reader.ExpectEndOfBlock())
				return buffer;
		}

		return nullptr;
	}

private:
	OriginalContainer(const OriginalContainer&) = delete;
	OriginalContainer& operator = (const OriginalContainer&) = delete;
	OriginalContainer& operator = (OriginalContainer&&) = delete;
};
