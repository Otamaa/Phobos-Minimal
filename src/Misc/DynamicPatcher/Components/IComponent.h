#pragma once

class PhobosStreamReader;
class PhobosStreamWriter;
class IComponent
{
public:
	virtual void ExtChanged() = 0;
	virtual void Clean() = 0;
	virtual void Awake() = 0;
	virtual void Destroy() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnUpdateEnd() = 0;
	virtual void OnWarpUpdate() = 0;
	virtual void OwnerIsRelease(void* ptr) = 0;
	virtual void OnForeachEnd() = 0;

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange) = 0;
	virtual bool Save(PhobosStreamWriter& stream) const = 0;
};