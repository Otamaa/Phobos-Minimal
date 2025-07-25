// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#include "../core/support.h"
#include "../core/zone.h"
#include "../core/zonebitset_p.h"

ASMJIT_BEGIN_NAMESPACE

// ZoneBitVector - Operations
// ==========================

Error ZoneBitVector::copyFrom(ZoneAllocator* allocator, const ZoneBitVector& other) noexcept {
  BitWord* data = _data;
  size_t newSize = other.size();

  if (!newSize) {
    _size = 0;
    return kErrorOk;
  }

  if (newSize > _capacity) {
    // Realloc needed... Calculate the minimum capacity (in bytes) required.
    size_t minimumCapacityInBits = Support::align_up<size_t>(newSize, Support::bit_size_of<BitWord>);
    if (ASMJIT_UNLIKELY(minimumCapacityInBits < newSize)) {
      return DebugUtils::errored(kErrorOutOfMemory);
    }

    // Normalize to bytes.
    size_t minimumCapacity = minimumCapacityInBits / 8u;
    size_t allocatedCapacity;

    BitWord* newData = static_cast<BitWord*>(allocator->alloc(minimumCapacity, allocatedCapacity));
    if (ASMJIT_UNLIKELY(!newData)) {
      return DebugUtils::errored(kErrorOutOfMemory);
    }

    // `allocatedCapacity` now contains number in bytes, we need bits.
    size_t allocatedCapacityInBits = allocatedCapacity * 8;

    // Arithmetic overflow should normally not happen. If it happens we just
    // change the `allocatedCapacityInBits` to the `minimumCapacityInBits` as
    // this value is still safe to be used to call `_allocator->release(...)`.
    if (ASMJIT_UNLIKELY(allocatedCapacityInBits < allocatedCapacity)) {
      allocatedCapacityInBits = minimumCapacityInBits;
    }

    if (data) {
      allocator->release(data, _capacity / 8);
    }
    data = newData;

    _data = data;
    _capacity = uint32_t(allocatedCapacityInBits);
  }

  _size = uint32_t(newSize);
  _copyBits(data, other.data(), _wordsPerBits(uint32_t(newSize)));

  return kErrorOk;
}

Error ZoneBitVector::_resize(ZoneAllocator* allocator, size_t newSize, size_t idealCapacity, bool newBitsValue) noexcept {
  ASMJIT_ASSERT(idealCapacity >= newSize);

  if (newSize <= _size) {
    // The size after the resize is lesser than or equal to the current size.
    size_t idx = newSize / Support::bit_size_of<BitWord>;
    size_t bit = newSize % Support::bit_size_of<BitWord>;

    // Just set all bits outside of the new size in the last word to zero. There is a case that there are not bits
    // to set if `bit` is zero. This happens when `newSize` is a multiply of `bit_size_of<BitWord>` like 64, 128,
    // and so on. In that case don't change anything as that would mean settings bits outside of the `_size`.
    if (bit) {
      _data[idx] &= (BitWord(1) << bit) - 1u;
    }

    _size = uint32_t(newSize);
    return kErrorOk;
  }

  size_t oldSize = _size;
  BitWord* data = _data;

  if (newSize > _capacity) {
    // Realloc needed, calculate the minimum capacity (in bytes) required.
    size_t minimumCapacityInBits = Support::align_up(idealCapacity, Support::bit_size_of<BitWord>);

    if (ASMJIT_UNLIKELY(minimumCapacityInBits < newSize)) {
      return DebugUtils::errored(kErrorOutOfMemory);
    }

    // Normalize to bytes.
    size_t minimumCapacity = minimumCapacityInBits / 8u;
    size_t allocatedCapacity;

    BitWord* newData = static_cast<BitWord*>(allocator->alloc(minimumCapacity, allocatedCapacity));
    if (ASMJIT_UNLIKELY(!newData)) {
      return DebugUtils::errored(kErrorOutOfMemory);
    }

    // `allocatedCapacity` now contains number in bytes, we need bits.
    size_t allocatedCapacityInBits = allocatedCapacity * 8u;

    // Arithmetic overflow should normally not happen. If it happens we just change the `allocatedCapacityInBits`
    // to the `minimumCapacityInBits` as this value is still safe to be used to call `_allocator->release(...)`.
    if (ASMJIT_UNLIKELY(allocatedCapacityInBits < allocatedCapacity)) {
      allocatedCapacityInBits = minimumCapacityInBits;
    }

    _copyBits(newData, data, _wordsPerBits(oldSize));

    if (data) {
      allocator->release(data, _capacity / 8);
    }
    data = newData;

    _data = data;
    _capacity = uint32_t(allocatedCapacityInBits);
  }

  // Start (of the old size) and end (of the new size) bits
  size_t idx = oldSize / Support::bit_size_of<BitWord>;
  size_t startBit = oldSize % Support::bit_size_of<BitWord>;
  size_t endBit = newSize % Support::bit_size_of<BitWord>;

  // Set new bits to either 0 or 1. The `pattern` is used to set multiple
  // bits per bit-word and contains either all zeros or all ones.
  BitWord pattern = Support::bool_as_mask<BitWord>(newBitsValue);

  // First initialize the last bit-word of the old size.
  if (startBit) {
    size_t nBits = 0;

    if (idx == (newSize / Support::bit_size_of<BitWord>)) {
      // The number of bit-words is the same after the resize. In that case
      // we need to set only bits necessary in the current last bit-word.
      ASMJIT_ASSERT(startBit < endBit);
      nBits = endBit - startBit;
    }
    else {
      // There is be more bit-words after the resize. In that case we don't
      // have to be extra careful about the last bit-word of the old size.
      nBits = Support::bit_size_of<BitWord> - startBit;
    }

    data[idx++] |= pattern << nBits;
  }

  // Initialize all bit-words after the last bit-word of the old size.
  size_t endIdx = _wordsPerBits(newSize);
  while (idx < endIdx) data[idx++] = pattern;

  // Clear unused bits of the last bit-word.
  if (endBit) {
    data[endIdx - 1] = pattern & ((BitWord(1) << endBit) - 1);
  }

  _size = uint32_t(newSize);
  return kErrorOk;
}

Error ZoneBitVector::_append(ZoneAllocator* allocator, bool value) noexcept {
  uint32_t kThreshold = Globals::kGrowThreshold * 8u;
  uint32_t newSize = _size + 1;
  uint32_t idealCapacity = _capacity;

  if (idealCapacity < 128) {
    idealCapacity = 128;
  }
  else if (idealCapacity <= kThreshold) {
    idealCapacity *= 2;
  }
  else {
    idealCapacity += kThreshold;
  }

  if (ASMJIT_UNLIKELY(idealCapacity < _capacity)) {
    if (ASMJIT_UNLIKELY(_size == std::numeric_limits<uint32_t>::max())) {
      return DebugUtils::errored(kErrorOutOfMemory);
    }
    idealCapacity = newSize;
  }

  return _resize(allocator, newSize, idealCapacity, value);
}

// ZoneBitVector - Tests
// =====================

#if defined(ASMJIT_TEST)
static void test_zone_bitvector(ZoneAllocator* allocator) {
  Zone zone(8096);

  uint32_t i, count;
  uint32_t kMaxCount = 100;

  ZoneBitVector vec;
  EXPECT_TRUE(vec.empty());
  EXPECT_EQ(vec.size(), 0u);

  INFO("ZoneBitVector::resize()");
  for (count = 1; count < kMaxCount; count++) {
    vec.clear();
    EXPECT_EQ(vec.resize(allocator, count, false), kErrorOk);
    EXPECT_EQ(vec.size(), count);

    for (i = 0; i < count; i++) {
      EXPECT_FALSE(vec.bitAt(i));
    }

    vec.clear();
    EXPECT_EQ(vec.resize(allocator, count, true), kErrorOk);
    EXPECT_EQ(vec.size(), count);

    for (i = 0; i < count; i++) {
      EXPECT_TRUE(vec.bitAt(i));
    }
  }

  INFO("ZoneBitVector::fillBits() / clearBits()");
  for (count = 1; count < kMaxCount; count += 2) {
    vec.clear();
    EXPECT_EQ(vec.resize(allocator, count), kErrorOk);
    EXPECT_EQ(vec.size(), count);

    for (i = 0; i < (count + 1) / 2; i++) {
      bool value = bool(i & 1);
      if (value) {
        vec.fillBits(i, count - i * 2);
      }
      else {
        vec.clearBits(i, count - i * 2);
      }
    }

    for (i = 0; i < count; i++) {
      EXPECT_EQ(vec.bitAt(i), bool(i & 1));
    }
  }
}

UNIT(zone_bitvector, -1) {
  Zone zone(8096);
  ZoneAllocator allocator(&zone);

  test_zone_bitvector(&allocator);
}
#endif

ASMJIT_END_NAMESPACE
