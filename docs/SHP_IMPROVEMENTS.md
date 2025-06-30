# SHP Handling Improvements

This document outlines the improvements made to SHP (Shape) file handling in the Phobos codebase.

## Summary of Changes

### 1. New Utility Classes

#### `SHPUtils` (SHPUtils.h/cpp)
A centralized utility class providing:
- **Better error handling**: Validates SHP files before use
- **Fallback support**: Automatic fallback to generic files when theater-specific files are missing
- **Safety checks**: Prevents crashes from invalid SHP dimensions or frame counts
- **Frame bounds checking**: Safe frame access that clamps to valid ranges

Key functions:
- `LoadSHPFileWithFallback()` - Loads SHP with optional fallback
- `LoadTheaterSHPWithFallback()` - Theater-specific loading with generic fallback
- `IsValidSHP()` - Validates SHP structure
- `GetSafeFrameIndex()` - Bounds-checked frame access

#### `SHPWrapper` (SHPWrapper.h)
A RAII wrapper class providing:
- **Automatic memory management**: No more manual GameDelete calls
- **Move semantics**: Efficient transfer of ownership
- **Safety features**: Built-in validation and bounds checking
- **Convenience methods**: Easy access to common SHP properties

### 2. Improved Existing Classes

#### `TheaterSpecificSHP`
- Now uses `SHPUtils::LoadTheaterSHPWithFallback()` for better error handling
- Improved logging with more descriptive messages
- Automatic fallback from theater-specific to generic files

#### Template System Updates
- `TemplateDef.h` now uses the new utility functions
- Simplified INI reading logic
- Better error reporting for missing SHP files

### 3. Code Cleanup

#### `SHPReference` Extension
- Removed all commented-out alpha support code
- Cleaned up unused container and hook code
- Simplified class structure while keeping it ready for future enhancements

## Benefits

1. **Reduced Memory Leaks**: RAII wrappers ensure automatic cleanup
2. **Better Error Handling**: Graceful fallbacks instead of crashes
3. **Improved Logging**: More descriptive error messages for debugging
4. **Safer Code**: Bounds checking prevents buffer overruns
5. **Cleaner Codebase**: Removed unused/commented code
6. **Easier Maintenance**: Centralized SHP logic reduces code duplication

## Example Usage

### Theater-Specific Loading:
```cpp
// Automatically tries theater-specific first, then generic
auto pSHP = SHPUtils::LoadTheaterSHPWithFallback("building");
// Will try: buildingtem.shp, then building.shp, then return nullptr
```

### RAII Wrapper:
```cpp
SHPWrapper shp("myfile.shp", fallbackSHP);
if (shp) {
    int frameCount = shp.GetFrameCount();
    int safeFrame = shp.GetSafeFrame(requestedFrame);
}
// Automatic cleanup when shp goes out of scope
```

## Compatibility

All changes are backward compatible. Existing code continues to work unchanged, but new code should prefer the improved utilities for better safety and maintainability. 