#pragma once

#include <Utilities/SavegameDef.h>

enum class TInitState
{
	Blank = 0x0,	  // CTOR'd
	Constanted = 0x1, // values that can be set without looking at Rules (i.e. country default loadscreen)
	Ruled = 0x2,	  // Rules has been loaded and props set (i.e. country powerplants taken from [General])
	Inited = 0x3,	  // values that need the object's state (i.e. is object a secretlab? -> load default boons)
	Completed = 0x4	  // INI has been read and values set
};

template <typename T>
class TExtension
{
	T* AttachedToObject;
	TInitState Initialized { TInitState::Blank };

public:

	T* const& OwnerObject() const {
		return this->AttachedToObject;
	}

	void SetOwnerObject(T* const OwnerObject) {
		this->AttachedToObject = OwnerObject;
	}

	virtual OPTIONALINLINE void SaveToStream(PhobosStreamWriter& Stm) {
		Stm.Save(this->Initialized);
	}

	virtual OPTIONALINLINE void LoadFromStream(PhobosStreamReader& Stm) {
		Stm.Load(this->Initialized);
	}

};
