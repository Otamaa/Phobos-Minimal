#pragma once

#ifdef ROBIN_HOOD_ENABLED
#include <Lib/robin_hood.h>
#else
#include <Lib/VectorMap.h>
#include <unordered_map>
#endif

#include <CCINIClass.h>
#include <SwizzleManagerClass.h>

#include "PhobosFixedString.h"
#include "Debug.h"
#include "Stream.h"
#include "Swizzle.h"

/*
 * ==========================
 *	It's a kind of magic
 * ==========================

 * These two templates are the basis of the new class extension standard.

 * ==========================

 * Extension<T> is the parent class for the data you want to link with this instance of T
   ( for example, [Warhead]MindControl.Permanent= should be stored in WarheadClassExt::ExtData
	 which itself should be a derivate of Extension<WarheadTypeClass> )

 * ==========================

   Container<TX> is the storage for all the Extension<T> which share the same T,
	where TX is the containing class of the relevant derivate of Extension<T>. // complex, huh?
   ( for example, there is Container<WarheadTypeExt>
	 which contains all the custom data for all WarheadTypeClass instances,
	 and WarheadTypeExt itself contains just statics like the Container itself )

   Requires:
	using base_type = T;
	const DWORD Extension<T>::Canary = (any dword value easily identifiable in a byte stream)
	class TX::ExtData : public Extension<T> { custom_data; }

   Complex? Yes. That's partially why you should be happy these are premade for you.
 *
 */

template <class T>
concept HasEnsureConstanted = requires(T t) { t.InitializeConstants(); };
template <class T>
concept HasInitializeRuled = requires(T t) { t.InitializeRuled(); };
template <class T>
concept HasInitialize = requires(T t) { t.Initialize(); };
template <class T>
concept HasLoadFromRulesFile = requires(T t, CCINIClass * pINI) { t.LoadFromRulesFile(pINI); };
template <class T>
concept HasLoadFromINIFile = requires(T t, CCINIClass * pINI) { t.LoadFromINIFile(pINI); };
template <class T>
concept HasInvalidatePointer = requires(T t, void* ptr, bool removed) { t.InvalidatePointer(ptr, removed); };
template <class T>
concept HasAbsID = requires(T) { T::AbsID; };
template <class T>
concept HasDeriveredAbsID = requires(T) { T::AbsDerivateID; };
template <class T>
concept HasTypeBase = requires(T) { T::AbsTypeBase; };

template <typename T>
class Extension
{
	T* AttachedToObject;
	InitState Initialized;

public:
	static const DWORD Canary;

	Extension(T* const OwnerObject) : AttachedToObject { OwnerObject }
		, Initialized { InitState::Blank }
	{ }

	const InitState GetInitStatus() const {
		return Initialized;
	}

	Extension(const Extension& other) = delete;

	void operator=(const Extension& RHS) = delete;

	virtual ~Extension() = default;

	// the object this Extension expands
	T* const& Get() const {
		return this->AttachedToObject;
	}

	T* const& OwnerObject() const {
		return this->AttachedToObject;
	}

	void EnsureConstanted() {
		if (this->Initialized < InitState::Constanted) {
			this->InitializeConstants();
	
			this->Initialized = InitState::Constanted;
		}
	}

	void LoadFromINI(CCINIClass* pINI)
	{
		if (!pINI)
			return;

		if (this->Initialized == InitState::Constanted) { 
			this->Initialized = InitState::Inited;
			this->Initialize();
			this->LoadFromINIFile(pINI);
			this->Initialized = InitState::Completed;
		} else {

			if(this->Initialized == InitState::Completed)
				this->LoadFromINIFile(pINI);
			else
				Debug::Log("[%s] LoadFrom INI Called When Initilize not done ! \n", typeid(T).name());
		}
	}

	// must be implemented
	virtual void InvalidatePointer(void* ptr, bool bRemoved) { }
	virtual bool InvalidateIgnorable(void* const ptr) const = 0;

	virtual inline void SaveToStream(PhobosStreamWriter& Stm) { 
	  Stm.Save(this->Initialized);
	}

	virtual inline void LoadFromStream(PhobosStreamReader& Stm) { 
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
concept HasOffset = requires(T) { T::ExtOffset; };

template <class T>
concept HasInvalidateExtDataIgnorable = requires(T t, void* const ptr) { t.InvalidateExtDataIgnorable(ptr); };

template <typename T>
class Container
{
private:
	using base_type = typename T::base_type;
	using extension_type = typename T::ExtData;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using extension_type_ptr = extension_type*;
	using extension_type_ref_ptr = extension_type**;
	using const_extension_type_ptr = const extension_type*;

	base_type_ptr SavingObject;
	IStream* SavingStream;
	FixedString<0x100> Name;
	//std::unordered_map<base_type_ptr, extension_type_ptr> ExtMap;
public:

	explicit Container(const char* pName) :
		SavingObject { nullptr },
		SavingStream { nullptr },
		Name {}  //, ExtMap {}
	{
		Name = pName;
	}

	virtual ~Container() = default;

	auto GetName() const
	{
		return this->Name.data();
	}

	base_type_ptr GetSavingObject() const
	{
		return SavingObject;
	}

	IStream* GetStream() const
	{
		return this->SavingStream;
	}

	void PointerGotInvalid(void* ptr, bool bRemoved) {
		if (!this->InvalidateExtDataIgnorable(ptr)) {
			this->InvalidatePointer(ptr, bRemoved);
		}
	}

	void ClearExtAttribute(base_type_ptr key)
	{
		if constexpr (HasOffset<T>)
			(*(uintptr_t*)((char*)key + T::ExtOffset)) = 0;
		else
			(*(uintptr_t*)((char*)key + AbstractExtOffset)) = 0;

		//this->ExtMap.erase(key);
	}

	void SetExtAttribute(base_type_ptr key, extension_type_ptr val)
	{
		if constexpr (HasOffset<T>)
			(*(uintptr_t*)((char*)key + T::ExtOffset)) = (uintptr_t)val;
		else
			(*(uintptr_t*)((char*)key + AbstractExtOffset)) = (uintptr_t)val;

		//this->ExtMap.emplace(key, val);
	}

	extension_type_ptr GetExtAttribute(base_type_ptr key) const
	{
		if constexpr (HasOffset<T>)
			return (extension_type_ptr)(*(uintptr_t*)((char*)key + T::ExtOffset));
		else
			return (extension_type_ptr)(*(uintptr_t*)((char*)key + AbstractExtOffset));
		
		//auto const Iter = this->ExtMap.find(key);
		//return Iter != this->ExtMap.end() ? (*Iter).second : nullptr;
	}

	virtual void Clear() { //ExtMap.clear(); 
	}

protected:

	virtual void InvalidatePointer(void* ptr, bool bRemoved) { }
	virtual bool InvalidateExtDataIgnorable(void* const ptr) const { return true; }

public:

	// using virtual is good way so i can integrate different type of these
	// not sure what is better approach

	extension_type_ptr Allocate(base_type_ptr key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (extension_type_ptr val = new extension_type(key))
		{
			val->EnsureConstanted();
			this->SetExtAttribute(key,val);
			return val;
		}

		return nullptr;
	}

	void JustAllocate(base_type_ptr key, bool bCond, const std::string_view& nMessage)
	{
		if (!key || (!bCond && !nMessage.empty()))
		{
			Debug::Log("%s \n", nMessage.data());
			return;
		}

		this->Allocate(key);
	}

	extension_type_ptr FindOrAllocate(base_type_ptr key)
	{
		// Find Always check for nullptr here
		if (extension_type_ptr const ptr = TryFind(key))
			return ptr;

		return this->Allocate(key);
	}

	extension_type_ptr Find(base_type_ptr key) const
	{
		return this->GetExtAttribute(key);
	}

	extension_type_ptr TryFind(base_type_ptr key) const
	{
		if(!key)
			return nullptr;

		return this->GetExtAttribute(key);
	}

	void Remove(base_type_ptr key)
	{
		if (extension_type_ptr Item = TryFind(key)) {
			delete Item;

			this->ClearExtAttribute(key);
		}
	}

	void LoadFromINI(base_type_ptr key, CCINIClass* pINI)
	{
		if (extension_type_ptr ptr = this->TryFind(key))
			ptr->LoadFromINI(pINI);
	}

	void PrepareStream(base_type_ptr key, IStream* pStm)
	{
		//Debug::Log("[PrepareStream] Next is %p of type '%s'\n", key, this->Name.data());
		this->SavingObject = key;
		this->SavingStream = pStm;
	}

	void SaveStatic()
	{
		if (this->SavingObject && this->SavingStream)
		{
			//Debug::Log("[SaveStatic] Saving object %p as '%s'\n", this->SavingObject, this->Name.data());
			if (!this->Save(this->SavingObject, this->SavingStream))
				Debug::FatalErrorAndExit("[SaveStatic] Saving failed!\n");
		}
		else
		{
			//Debug::Log("[SaveStatic] Object or Stream not set for '%s': %p, %p\n",
			//	this->Name.data(), this->SavingObject, this->SavingStream);
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
	}

	void LoadStatic()
	{
		if (this->SavingObject && this->SavingStream)
		{
			//Debug::Log("[LoadStatic] Loading object %p as '%s'\n", this->SavingObject, this->Name.data());
			if (!this->Load(this->SavingObject, this->SavingStream))
				Debug::FatalErrorAndExit("[LoadStatic] Loading object %p as '%s failed!\n", this->SavingObject, this->Name.data());
		}
		else
		{
			//Debug::Log("[LoadStatic] Object or Stream not set for '%s': %p, %p\n",
			//	this->Name.data(), this->SavingObject, this->SavingStream);
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
			//Debug::Log("[SaveKey] Attempted for a null pointer! WTF!\n");
			return nullptr;
		}

		// get the value data
		extension_type_ptr const buffer = this->Find(key);

		if (!buffer)
		{
			//Debug::Log("[SaveKey] Could not find value.\n");
			return nullptr;
		}

		// write the current pointer, the size of the block, and the canary
		PhobosByteStream saver { sizeof(*buffer) };
		PhobosStreamWriter writer {saver};

		writer.Save(T::Canary);
		writer.Save(buffer);

		// save the data
		buffer->SaveToStream(writer);

		// save the block
		if (!saver.WriteBlockToStream(pStm))
		{
			//Debug::Log("[SaveKey] Failed to save data.\n");
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
			//Debug::Log("[LoadKey] Attempted for a null pointer! WTF!\n");
			return nullptr;
		}

		this->ClearExtAttribute(key);
		extension_type_ptr buffer = new extension_type(key);
		this->SetExtAttribute(key,buffer);

		#ifdef aaaa
		if (buffer) {
			//Debug::Log("[LoadKey] Found an [%x]Ext For [%s - %x] ! \n" , buffer , Name.data() , key);
		} else {
			//Debug::Log("[LoadKey] Could not find value For [%s - %x] Allocating !\n", Name.data(), key);
			buffer = this->Allocate(key);
		}
		#endif

		PhobosByteStream loader { 0 };
		if (!loader.ReadBlockFromStream(pStm))
		{
			//Debug::Log("[LoadKey] Failed to read data from save stream?!\n");
			return nullptr;
		}

		PhobosStreamReader reader { loader };
		if (reader.Expect(T::Canary) && reader.RegisterChange(buffer))
		{
			buffer->LoadFromStream(reader);
			if (reader.ExpectEndOfBlock())
				return buffer;
		}

		return nullptr;
	}

private:
	Container(const Container&) = delete;
	Container& operator = (const Container&) = delete;
	Container& operator = (Container&&) = delete;
};