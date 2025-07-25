// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#include "../core/support.h"
#include "../core/zone.h"
#include "../core/zonevector.h"

ASMJIT_BEGIN_NAMESPACE

// ZoneVectorBase - Memory Management
// ==================================

// Rule based growing strategy - 32 bytes, 128 bytes, 512 bytes, and then grow exponentially until `kGrowThreshold`
// is reached.
static constexpr uint8_t ZoneVector_grow_rule(uint8_t log2_size) noexcept {
  return log2_size < 1u ? uint8_t(0) :
         log2_size < 2u ? uint8_t(2) :
         log2_size < 4u ? uint8_t(4) :
         log2_size < 6u ? uint8_t(6) :
         log2_size < 8u ? uint8_t(8) : uint8_t(log2_size);
}

// The table is never used fully, only indexes up to `ctz(Support::kGrowThreshold) + 1`.
static constexpr uint8_t ZoneVector_grow_table[32] = {
  ZoneVector_grow_rule( 0), ZoneVector_grow_rule( 1), ZoneVector_grow_rule( 2), ZoneVector_grow_rule( 3),
  ZoneVector_grow_rule( 4), ZoneVector_grow_rule( 5), ZoneVector_grow_rule( 6), ZoneVector_grow_rule( 7),
  ZoneVector_grow_rule( 8), ZoneVector_grow_rule( 9), ZoneVector_grow_rule(10), ZoneVector_grow_rule(11),
  ZoneVector_grow_rule(12), ZoneVector_grow_rule(13), ZoneVector_grow_rule(14), ZoneVector_grow_rule(15),
  ZoneVector_grow_rule(16), ZoneVector_grow_rule(17), ZoneVector_grow_rule(18), ZoneVector_grow_rule(19),
  ZoneVector_grow_rule(20), ZoneVector_grow_rule(21), ZoneVector_grow_rule(22), ZoneVector_grow_rule(23),
  ZoneVector_grow_rule(24), ZoneVector_grow_rule(25), ZoneVector_grow_rule(26), ZoneVector_grow_rule(27),
  ZoneVector_grow_rule(28), ZoneVector_grow_rule(29), ZoneVector_grow_rule(30), ZoneVector_grow_rule(31)
};

static ASMJIT_INLINE size_t ZoneVector_expand_byte_size(size_t byte_size) noexcept {
  ASMJIT_ASSERT(byte_size > 0u);

  if (ASMJIT_LIKELY(byte_size <= Globals::kGrowThreshold)) {
    uint32_t grow_table_idx = Support::bit_size_of<size_t> - Support::clz((byte_size - 1u) | 1u);
    uint32_t grow_log2_size = ZoneVector_grow_table[grow_table_idx];

    return size_t(1) << grow_log2_size;
  }
  else {
    return Support::align_up(size_t(byte_size) + 1u, Globals::kGrowThreshold);
  }
}

template<typename ItemSize>
static ASMJIT_NOINLINE Error ZoneVector_reserve_with_byte_size(ZoneVectorBase* self, ZoneAllocator* allocator, size_t byte_size, ItemSize item_size) noexcept {
  size_t allocated_size;
  uint8_t* new_data = static_cast<uint8_t*>(allocator->alloc(byte_size, allocated_size));

  if (ASMJIT_UNLIKELY(!new_data)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  size_t allocated_capacity = Support::item_count_from_byte_size(allocated_size, item_size);

  void* old_data = self->_data;
  uint32_t size = self->_size;

  if (old_data) {
    memcpy(new_data, old_data, Support::byte_size_from_item_count(size, item_size));
    allocator->release(old_data, Support::byte_size_from_item_count(self->_capacity, item_size));
  }

  self->_data = new_data;
  self->_capacity = uint32_t(allocated_capacity);

  return kErrorOk;
}

static ASMJIT_INLINE bool ZoneVector_is_valid_size(size_t size) noexcept {
  if constexpr (sizeof(uint32_t) < sizeof(size_t)) {
    // 64-bit machine - since we store size and capacity as `uint32_t`, we have to check whether
    // the `size_t` argument actually fits `uint32_t`.
    return size < size_t(0xFFFFFFFFu);
  }
  else {
    // 32-bit machine - `uint32_t` is the same as `size_t` - there is no need to do any checks
    // as it's impossible to end up having a container, which data uses the whole address space.
    return true;
  }
}

static ASMJIT_INLINE bool ZoneVector_check_byte_size(uint64_t byte_size) noexcept {
  if constexpr (sizeof(uint32_t) < sizeof(size_t)) {
    return true;
  }
  else {
    return byte_size <= 0x80000000u;
  }
}

template<typename ItemSize>
static ASMJIT_INLINE Error ZoneVector_reserve_fit(ZoneVectorBase* self, ZoneAllocator* allocator, size_t item_count, ItemSize item_size) noexcept {
  size_t capacity = self->_capacity;
  size_t capacity_masked = capacity | Support::bool_as_mask<size_t>(!ZoneVector_is_valid_size(item_count));
  uint64_t byte_size = Support::byte_size_from_item_count<uint64_t>(item_count, item_size);

  if (ASMJIT_UNLIKELY(Support::bool_or(capacity_masked >= item_count, !ZoneVector_check_byte_size(byte_size)))) {
    return capacity >= item_count ? kErrorOk : DebugUtils::errored(kErrorOutOfMemory);
  }

  return ZoneVector_reserve_with_byte_size(self, allocator, size_t(byte_size), item_size);
}

template<typename ItemSize>
static ASMJIT_INLINE Error ZoneVector_reserve_grow(ZoneVectorBase* self, ZoneAllocator* allocator, size_t item_count, ItemSize item_size) noexcept {
  size_t capacity = self->_capacity;
  size_t capacity_masked = capacity | Support::bool_as_mask<size_t>(!ZoneVector_is_valid_size(item_count));
  uint64_t byte_size = Support::byte_size_from_item_count<uint64_t>(item_count, item_size);

  if (ASMJIT_UNLIKELY(Support::bool_or(capacity_masked >= item_count, !ZoneVector_check_byte_size(byte_size)))) {
    return capacity >= item_count ? kErrorOk : DebugUtils::errored(kErrorOutOfMemory);
  }

  size_t expanded_byte_size = ZoneVector_expand_byte_size(size_t(byte_size));
  return ZoneVector_reserve_with_byte_size(self, allocator, expanded_byte_size, item_size);
}

template<typename ItemSize>
static ASMJIT_INLINE Error ZoneVector_grow(ZoneVectorBase* self, ZoneAllocator* allocator, size_t n, ItemSize item_size) noexcept {
  Support::FastUInt8 of {};
  size_t after = Support::add_overflow<size_t>(self->_size, n, &of);

  if (ASMJIT_UNLIKELY(of)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  return ZoneVector_reserve_grow(self, allocator, after, item_size);
}

template<typename ItemSize>
static ASMJIT_INLINE Error ZoneVector_resize_fit(ZoneVectorBase* self, ZoneAllocator* allocator, size_t n, ItemSize item_size) noexcept {
  size_t size = self->_size;
  size_t capacity = self->_capacity;

  if (capacity < n) {
    ASMJIT_PROPAGATE(ZoneVector_reserve_fit(self, allocator, n, item_size));
  }

  if (size < n) {
    memset(static_cast<uint8_t*>(self->_data) + Support::byte_size_from_item_count(size, item_size), 0, Support::byte_size_from_item_count(n - size, item_size));
  }

  self->_size = uint32_t(n);
  return kErrorOk;
}

template<typename ItemSize>
static ASMJIT_INLINE Error ZoneVector_resize_grow(ZoneVectorBase* self, ZoneAllocator* allocator, size_t n, ItemSize item_size) noexcept {
  size_t size = self->_size;
  size_t capacity = self->_capacity;

  if (capacity < n) {
    ASMJIT_PROPAGATE(ZoneVector_reserve_grow(self, allocator, n, item_size));
  }

  if (size < n) {
    memset(static_cast<uint8_t*>(self->_data) + Support::byte_size_from_item_count(size, item_size), 0, Support::byte_size_from_item_count(n - size, item_size));
  }

  self->_size = uint32_t(n);
  return kErrorOk;
}

// Public API wrappers:
Error ZoneVectorBase::_reserve_fit(ZoneAllocator* allocator, size_t n, Support::ByteSize item_size) noexcept {
  return ZoneVector_reserve_fit<Support::ByteSize>(this, allocator, n, item_size);
}

Error ZoneVectorBase::_reserve_fit(ZoneAllocator* allocator, size_t n, Support::Log2Size item_size) noexcept {
  return ZoneVector_reserve_fit<Support::Log2Size>(this, allocator, n, item_size);
}

Error ZoneVectorBase::_reserve_grow(ZoneAllocator* allocator, size_t n, Support::ByteSize item_size) noexcept {
  return ZoneVector_reserve_grow<Support::ByteSize>(this, allocator, n, item_size);
}

Error ZoneVectorBase::_reserve_grow(ZoneAllocator* allocator, size_t n, Support::Log2Size item_size) noexcept {
  return ZoneVector_reserve_grow<Support::Log2Size>(this, allocator, n, item_size);
}

Error ZoneVectorBase::_grow(ZoneAllocator* allocator, size_t n, Support::ByteSize item_size) noexcept {
  return ZoneVector_grow<Support::ByteSize>(this, allocator, n, item_size);
}

Error ZoneVectorBase::_grow(ZoneAllocator* allocator, size_t n, Support::Log2Size item_size) noexcept {
  return ZoneVector_grow<Support::Log2Size>(this, allocator, n, item_size);
}

Error ZoneVectorBase::_resize_fit(ZoneAllocator* allocator, size_t n, Support::ByteSize item_size) noexcept {
  return ZoneVector_resize_fit<Support::ByteSize>(this, allocator, n, item_size);
}

Error ZoneVectorBase::_resize_fit(ZoneAllocator* allocator, size_t n, Support::Log2Size item_size) noexcept {
  return ZoneVector_resize_fit<Support::Log2Size>(this, allocator, n, item_size);
}

Error ZoneVectorBase::_resize_grow(ZoneAllocator* allocator, size_t n, Support::ByteSize item_size) noexcept {
  return ZoneVector_resize_grow<Support::ByteSize>(this, allocator, n, item_size);
}

Error ZoneVectorBase::_resize_grow(ZoneAllocator* allocator, size_t n, Support::Log2Size item_size) noexcept {
  return ZoneVector_resize_grow<Support::Log2Size>(this, allocator, n, item_size);
}

// ZoneVector - Tests
// ==================

#if defined(ASMJIT_TEST)
template<typename T>
static void test_zone_vector(ZoneAllocator* allocator, const char* typeName) {
  constexpr uint32_t kMiB = 1024 * 1024;

  size_t i;
  size_t kMax = 100000;

  ZoneVector<T> vec;

  INFO("ZoneVector<%s> basic tests", typeName);
  EXPECT_EQ(vec.append(allocator, 0), kErrorOk);
  EXPECT_FALSE(vec.empty());
  EXPECT_EQ(vec.size(), 1u);
  EXPECT_GE(vec.capacity(), 1u);
  EXPECT_EQ(vec.index_of(0), size_t(0));
  EXPECT_TRUE(Globals::is_npos(vec.index_of(-11)));

  vec.clear();
  EXPECT_TRUE(vec.empty());
  EXPECT_EQ(vec.size(), 0u);
  EXPECT_TRUE(Globals::is_npos(vec.index_of(0)));

  for (i = 0; i < kMax; i++) {
    EXPECT_EQ(vec.append(allocator, T(i)), kErrorOk);
  }
  EXPECT_FALSE(vec.empty());
  EXPECT_EQ(vec.size(), size_t(kMax));
  EXPECT_EQ(vec.index_of(T(0)), size_t(0));
  EXPECT_EQ(vec.index_of(T(kMax - 1)), uint32_t(kMax - 1));

  EXPECT_EQ(vec.begin()[0], 0);
  EXPECT_EQ(vec.end()[-1], T(kMax - 1));

  EXPECT_EQ(vec.rbegin()[0], T(kMax - 1));
  EXPECT_EQ(vec.rend()[-1], 0);

  int64_t fsum = 0;
  int64_t rsum = 0;

  for (const T& item : vec) {
    fsum += item;
  }

  for (auto it = vec.rbegin(); it != vec.rend(); ++it) {
    rsum += *it;
  }
  EXPECT_EQ(fsum, rsum);

  INFO("ZoneVector<%s>::operator=(ZoneVector<%s>&&)", typeName, typeName);
  ZoneVector<T> movedVec(std::move(vec));
  EXPECT_EQ(vec.data(), nullptr);
  EXPECT_EQ(vec.size(), 0u);
  EXPECT_EQ(vec.capacity(), 0u);

  movedVec.release(allocator);

  INFO("ZoneVector<%s>::reserve_grow()", typeName);
  for (uint32_t j = 8; j < 40 / sizeof(T); j += 8) {
    EXPECT_EQ(vec.reserve_grow(allocator, j * kMiB), kErrorOk);
    EXPECT_GE(vec.capacity(), j * kMiB);
  }
}

template<typename T>
static void test_zone_vector_capacity(ZoneAllocator* allocator, const char* typeName) {
  ZoneVector<T> vec;

  INFO("ZoneVector<%s> capacity (growing) test", typeName);

  for (size_t i = 0; i < 10000000; i++) {
    size_t old_capacity = vec.capacity();
    EXPECT_EQ(vec.append(allocator, T(i)), kErrorOk);

    if (vec.capacity() != old_capacity) {
      INFO("  Increasing capacity from %zu to %zu (vector size=%zu)\n", old_capacity, vec.capacity(), vec.size());
    }
  }
}

UNIT(zone_vector, -1) {
  Zone zone(8096);
  ZoneAllocator allocator(&zone);

  test_zone_vector<int32_t>(&allocator, "int32_t");
  test_zone_vector_capacity<int32_t>(&allocator, "int32_t");

  test_zone_vector<int64_t>(&allocator, "int64_t");
  test_zone_vector_capacity<int64_t>(&allocator, "int64_t");
}
#endif

ASMJIT_END_NAMESPACE
