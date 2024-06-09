#pragma once

#include <Utilities/SavegameDef.h>

class IComponent
{
public:

	virtual void ExtChanged() { };
	virtual void Awake() { };
	virtual void OnUpdate() { };
	virtual void OnUpdateEnd() { };
	virtual void OnWarpUpdate() { };
	virtual void Destroy() { };
	virtual void OwnerIsRelease(void* ptr) { };
	virtual void OnForeachEnd() { };

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange) = 0;
	virtual bool Save(PhobosStreamWriter& stream) const = 0;
};
