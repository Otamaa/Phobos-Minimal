#pragma once

//#ifdef ROBIN_HOOD_ENABLED
//#include <Lib/robin_hood.h>
//#else
//#include <Lib/VectorMap.h>
//#include <unordered_map>
//#endif

#include <CCINIClass.h>
#include <SwizzleManagerClass.h>

#include "PhobosFixedString.h"
#include "Debug.h"
#include "Stream.h"
#include "Swizzle.h"
#include "Concepts.h"

static constexpr size_t AbstractExtOffset = 0x18;

struct IExtension
{
	virtual void LoadFromStream(PhobosStreamReader& Stm) = 0;
	virtual void SaveToStream(PhobosStreamWriter& Stm) = 0;
};

template <typename T>
struct TExtension : public IExtension
{
	static const DWORD Canary;

	T* AttachedToObject {};
	InitState Initialized { InitState::Blank };

	virtual T* GetAttachedObject() const {
		return this->AttachedToObject;
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) {
		Stm.Process(this->Initialized);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) {
		Stm.Process(this->Initialized);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize() {
		//AttachedToObject
		return  -4;
	}
};


// container that do lookup,Cleanup
template <typename T>
class PassiveContainer
{
public:
	using base_type = typename T::base_type;
	using extension_type = typename T;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using extension_type_ptr = extension_type*;
	using extension_type_ref_ptr = extension_type**;
	using const_extension_type_ptr = const extension_type*;

	constexpr FORCEINLINE const char* GetName() const
	{
		return typeid(T).name();
	}

	constexpr FORCEINLINE extension_type_ptr GetExtAttribute(base_type_ptr key)
	{
		return (extension_type_ptr)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	// Find ExtAttribute with key checking
	extension_type_ptr TryFind(base_type_ptr key)
	{
		if (!key)
			return nullptr;

		return this->GetExtAttribute(key);
	}

	// Find ExtAttribute without key checking
	extension_type_ptr Find(base_type_ptr key)
	{
		return this->GetExtAttribute(key);
	}

	// Read ExtAttibute data from INI
	virtual void LoadFromINI(base_type_ptr key, CCINIClass* pINI, bool parseFailAddr)
	{
		if constexpr (CanLoadFromINIFile<T>)
		{
			if (extension_type_ptr ptr = this->TryFind(key))
			{
				if (!pINI)
				{
					Debug::Log("[%s] LoadFrom INI Called WithInvalid CCINIClass ptr ! \n", typeid(T).name());
					return;
				}

				switch (ptr->Initialized)
				{
				case InitState::Blank:
				{
					if constexpr (Initable<T>)
						ptr->Initialize();

					ptr->Initialized = InitState::Inited;

					if constexpr (CanLoadFromRulesFile<T>)
					{
						if (pINI == CCINIClass::INI_Rules)
						{
							ptr->LoadFromRulesFile(pINI);
						}
					}

					//Load from rules INI File
					ptr->LoadFromINIFile(pINI, parseFailAddr);
					ptr->Initialized = InitState::Ruled;
				}
				break;
				case InitState::Ruled:
				case InitState::Constanted:
				{
					//load anywhere other than rules
					ptr->LoadFromINIFile(pINI, parseFailAddr);
					//this function can be called again multiple time but without need to re-init the data
					ptr->Initialized = InitState::Ruled;
				}
				break;
				{
				default:
					break;
				}
				}
			}
		}
	}

	// Clear contaier data if any
	virtual void Clear() { }

	// Invalidate pointer ExtAttribute
	virtual void InvalidatePointerFor(base_type_ptr key, AbstractClass* const ptr, bool bRemoved)
	{
		if constexpr (ThisPointerInvalidationSubscribable<T>)
		{
			if (Phobos::Otamaa::ExeTerminated || !ptr)
				return;

			extension_type_ptr Extptr = this->TryFind(key);

			if (!Extptr)
				return;

			if constexpr (PointerInvalidationIgnorAble<T>)
			{
				if (!extension_type::InvalidateIgnorable(ptr))
					Extptr->InvalidatePointer(ptr, bRemoved);
			}
			else
			{
				Extptr->InvalidatePointer(ptr, bRemoved);
			}
		}
	}

};

//Container that do lookup,Allocate,Cleaup
template <typename T>
class Container : public PassiveContainer<T>
{
private:
	using base_type = typename T::base_type;
	using extension_type = typename T;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using extension_type_ptr = extension_type*;
	using extension_type_ref_ptr = extension_type**;
	using const_extension_type_ptr = const extension_type*;

	base_type_ptr SavingObject;
	IStream* SavingStream;

public:

	constexpr FORCEINLINE base_type_ptr GetSavingObject() const
	{
		return SavingObject;
	}

	constexpr FORCEINLINE IStream* GetStream() const
	{
		return this->SavingStream;
	}

	constexpr FORCEINLINE void ClearExtAttribute(base_type_ptr key)
	{
		(*(uintptr_t*)((char*)key + AbstractExtOffset)) = 0;
	}

	constexpr FORCEINLINE void SetExtAttribute(base_type_ptr key, extension_type_ptr val)
	{
		(*(uintptr_t*)((char*)key + AbstractExtOffset)) = (uintptr_t)val;
	}

	// Allocate extensionptr without any checking
	extension_type_ptr AllocateUnchecked(base_type_ptr key)
	{
		if (extension_type_ptr val = new extension_type())
		{
			val->AttachedToObject = key;
			// i dont want to declare ay CTOR
			// so this function use to replace that functionality
			if constexpr (CTORInitable<T>) {
				if(!Phobos::Otamaa::DoingLoadGame) {
					val->InitializeConstant();
				}
			}

			return val;
		}

		return nullptr;
	}

	// Allocate extensionptr with checking
	extension_type_ptr Allocate(base_type_ptr key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (extension_type_ptr val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	// TryFind the ExtAttribute if not found Allocate
	extension_type_ptr FindOrAllocate(base_type_ptr key)
	{
		// Find Always check for nullptr here
		if (extension_type_ptr const ptr = TryFind(key))
			return ptr;

		return this->Allocate(key);
	}

	// Remove ExtAttibute from key
	void Remove(base_type_ptr key)
	{
		if (extension_type_ptr Item = TryFind(key))
		{
			delete Item;
			this->ClearExtAttribute(key);
		}
	}

	// Read ExtAttibute data from INI
	virtual void LoadFromINI(base_type_ptr key, CCINIClass* pINI, bool parseFailAddr)
	{
		if constexpr (CanLoadFromINIFile<T>)
		{
			if (extension_type_ptr ptr = this->TryFind(key))
			{
				if (!pINI) {
					Debug::Log("[%s] LoadFrom INI Called WithInvalid CCINIClass ptr ! \n", typeid(T).name());
					return;
				}

				switch (ptr->Initialized) {
					case InitState::Blank:
					{
						if constexpr (Initable<T>)
							ptr->Initialize();

						ptr->Initialized = InitState::Inited;

						if constexpr (CanLoadFromRulesFile<T>) {
							if (pINI == CCINIClass::INI_Rules) {
								ptr->LoadFromRulesFile(pINI);
							}
						}

						//Load from rules INI File
						ptr->LoadFromINIFile(pINI, parseFailAddr);
						ptr->Initialized = InitState::Ruled;
					}
					break;
					case InitState::Ruled:
					case InitState::Constanted:
					{
						//load anywhere other than rules
						ptr->LoadFromINIFile(pINI, parseFailAddr);
						//this function can be called again multiple time but without need to re-init the data
						ptr->Initialized = InitState::Ruled;
					}
					break;
					{
					default:
						break;
					}
				}
			}
		}
	}

	// Clear contaier data if any
	virtual void Clear() override {	};

	// Invalidate pointer ExtAttribute
	virtual void InvalidatePointerFor(base_type_ptr key, AbstractClass* const ptr, bool bRemoved)
	{
		if constexpr (ThisPointerInvalidationSubscribable<T>){
			if (Phobos::Otamaa::ExeTerminated || !ptr)
				return;

			extension_type_ptr Extptr = this->TryFind(key);

			if (!Extptr)
				return;

			if constexpr (PointerInvalidationIgnorAble<T>){
				if (!extension_type::InvalidateIgnorable(ptr))
					Extptr->InvalidatePointer(ptr, bRemoved);
			} else {
					Extptr->InvalidatePointer(ptr, bRemoved);
			}
		}
	}

	// Prepare container data before S/L
	void PrepareStream(base_type_ptr key, IStream* pStm)
	{
		static_assert(T::Canary < std::numeric_limits<size_t>::max(), "Canary Too Big !");
		Debug::Log("[PrepareStream] Next is %p of type '%s'\n", key, typeid(T).name());
		this->SavingObject = key;
		this->SavingStream = pStm;
	}

	// Do Saave
	void SaveStatic()
	{
		Debug::Log("[SaveStatic] For object %p as '%s\n", this->SavingObject, typeid(T).name());
		if (this->SavingObject && this->SavingStream) {
			if (!this->Save(this->SavingObject, this->SavingStream))
				Debug::FatalErrorAndExit("[SaveStatic] Saving failed!\n");
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
	}

	// Do Load
	bool LoadStatic()
	{
		Debug::Log("[LoadStatic] For object %p as '%s\n", this->SavingObject, typeid(T).name());
		if (this->SavingObject && this->SavingStream)
		{
			if (!this->Load(this->SavingObject, this->SavingStream)){
				Debug::FatalErrorAndExit("[LoadStatic] Loading object %p as '%s failed!\n", this->SavingObject, typeid(T).name());
				return false;
			}
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
		return true;
	}

protected:

	// override this method to do type-specific stuff
	virtual bool Save(base_type_ptr key, IStream* pStm)
	{
		if constexpr (CanThisSaveToStream<T>)
		{
			// this really shouldn't happen
			if (!key)
			{
				//Debug::Log("[SaveKey] Attempted for a null pointer! WTF!\n");
				return false;
			}

			// get the value data
			extension_type_ptr const buffer = this->Find(key);

			if (!buffer)
			{
				//Debug::Log("[SaveKey] Could not find value.\n");
				return false;
			}

			// write the current pointer, the size of the block, and the canary
			PhobosByteStream saver { sizeof(extension_type) - extension_type::GetSavedOffsetSize() };
			PhobosStreamWriter writer { saver };

			writer.Save(extension_type::Canary);
			writer.Save(buffer);

			// save the data
			buffer->SaveToStream(writer);

			// save the block
			if (saver.WriteBlockToStream(pStm))
			{
				//Debug::Log("[SaveKey] Save used up 0x%X bytes\n", saver.Size());
				return true;
			}
		}

		//Debug::Log("[SaveKey] Failed to save data.\n");
		return false;
	}

	// override this method to do type-specific stuff
	virtual bool Load(base_type_ptr key, IStream* pStm)
	{
		if constexpr (CanThisLoadFromStream<T>)
		{
			// this really shouldn't happen
			if (!key)
			{
				//Debug::Log("[LoadKey] Attempted for a null pointer! WTF!\n");
				return false;
			}

			this->ClearExtAttribute(key);
			auto buffer = this->AllocateUnchecked(key);
			this->SetExtAttribute(key, buffer);

			PhobosByteStream loader { 0 };
			if (!loader.ReadBlockFromStream(pStm))
			{
				//Debug::Log("[LoadKey] Failed to read data from save stream?!\n");
				return false;
			}

			PhobosStreamReader reader { loader };
			if (reader.Expect(extension_type::Canary) && reader.RegisterChange(buffer))
			{
				buffer->LoadFromStream(reader);
				if (reader.ExpectEndOfBlock())
					return true;
			}
		}

		return false;
	}

};

template<typename T , typename Tkey , bool check = false>
static FORCEINLINE T GetExtPtr(Tkey who) {

	if constexpr (!check)
		return dynamic_cast<T>((IExtension*)(((DWORD)who) + AbstractExtOffset));
	else
		return !who ? nullptr : dynamic_cast<T>((IExtension*)(((DWORD)who) + AbstractExtOffset));
}
