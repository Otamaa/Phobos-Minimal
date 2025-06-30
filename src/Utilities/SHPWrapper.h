#pragma once

#include <FileFormats/SHP.h>
#include <Utilities/GameUniquePointers.h>
#include "SHPUtils.h"

// Simple RAII wrapper for SHP files with automatic memory management
class SHPWrapper
{
public:
	SHPWrapper() = default;
	
	explicit SHPWrapper(SHPStruct* pSHP) 
		: shp_(SHPUtils::MakeUniqueSHP(pSHP)) 
	{
	}
	
	explicit SHPWrapper(const char* filename, SHPStruct* fallback = nullptr)
		: shp_(SHPUtils::MakeUniqueSHP(SHPUtils::LoadSHPFileWithFallback(filename, fallback)))
	{
	}
	
	// Non-copyable but movable
	SHPWrapper(const SHPWrapper&) = delete;
	SHPWrapper& operator=(const SHPWrapper&) = delete;
	
	SHPWrapper(SHPWrapper&&) = default;
	SHPWrapper& operator=(SHPWrapper&&) = default;
	
	// Access operators
	SHPStruct* get() const { return shp_.get(); }
	SHPStruct* operator->() const { return shp_.get(); }
	SHPStruct& operator*() const { return *shp_.get(); }
	
	// Boolean conversion
	explicit operator bool() const { return shp_ && SHPUtils::IsValidSHP(shp_.get()); }
	
	// Release ownership
	SHPStruct* release() { return shp_.release(); }
	
	// Reset with new SHP
	void reset(SHPStruct* pSHP = nullptr) { shp_.reset(pSHP); }
	
	// Safe frame access
	int GetFrameCount() const { return SHPUtils::GetSafeFrameCount(shp_.get()); }
	int GetSafeFrame(int requestedFrame) const { return SHPUtils::GetSafeFrameIndex(shp_.get(), requestedFrame); }
	
	// Convenience methods
	bool IsValid() const { return SHPUtils::IsValidSHP(shp_.get()); }
	int GetWidth() const { return shp_ ? shp_->Width : 0; }
	int GetHeight() const { return shp_ ? shp_->Height : 0; }

private:
	UniqueGamePtrC<SHPStruct> shp_;
}; 