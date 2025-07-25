// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_SUPPORT_P_H_INCLUDED
#define ASMJIT_CORE_SUPPORT_P_H_INCLUDED

#include "../core/support.h"
#include "../core/zonebitset_p.h"

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_utilities
//! \{

namespace Support {

//! Stack with a fixed storage - cannot hold more items than the initial capacity.
template<typename T>
class FixedStack {
  T* _begin = nullptr;
  T* _end = nullptr;
  T* _ptr = nullptr;

public:
  ASMJIT_INLINE_NODEBUG FixedStack(T* buffer, size_t capacity) noexcept
    : _begin(buffer),
      _end(buffer + capacity),
      _ptr(buffer) {}

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isEmpty() const noexcept { return _begin == _ptr; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return (size_t)(_ptr - _begin); }

  //! Pushes an item to the stack.
  ASMJIT_INLINE void push(T item) noexcept {
    ASMJIT_ASSERT(_ptr != _end);
    *_ptr++ = item;
  }

  //! Pops an item from the stack.
  [[nodiscard]]
  ASMJIT_INLINE T pop() noexcept {
    ASMJIT_ASSERT(_ptr != _begin);
    return *--_ptr;
  }
};

//! Queue with a fixed storage - cannot hold more items than the initial capacity.
template<typename T>
struct FixedQueue {
protected:
  T* _begin = nullptr;
  T* _end = nullptr;
  T* _read_ptr = nullptr;
  T* _store_ptr = nullptr;

public:
  ASMJIT_INLINE_NODEBUG FixedQueue(T* buffer, size_t capacity) noexcept
    : _begin(buffer),
      _end(buffer + capacity),
      _read_ptr(buffer),
      _store_ptr(buffer) {}

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isEmpty() const noexcept {
    return _read_ptr == _store_ptr;
  }

  //! Gets an item from the queue (the item is removed).
  [[nodiscard]]
  ASMJIT_INLINE T get() noexcept {
    T item = *_read_ptr++;
    if (ASMJIT_UNLIKELY(_read_ptr == _end)) {
      _read_ptr = _begin;
    }
    return item;
  }

  //! Puts an item to the queue's back.
  ASMJIT_INLINE void put_back(T item) noexcept {
    *_store_ptr++ = item;
    if (ASMJIT_UNLIKELY(_store_ptr == _end)) {
      _store_ptr = _begin;
    }
  }

  //! Puts an item to the queue's front.
  ASMJIT_INLINE void put_front(T item) noexcept {
    if (ASMJIT_UNLIKELY(_read_ptr == _begin)) {
      _read_ptr = _end;
    }
    *--_read_ptr = item;
  }
};

/*

//! A simple FIFO queue with a fixed capacity designed to be used for multiple iterations, where
//! each iteration can use only up the the number of elements that were previously in the queue.
template<typename T>
struct IterativeQueue {
protected:
  T* _begin = nullptr;
  T* _end = nullptr;
  T* _fetch = nullptr;
  T* _store = nullptr;

public:
  ASMJIT_INLINE_NODEBUG IterativeQueue(T* buffer, size_t capacity) noexcept
    : _begin(buffer),
      _end(buffer + capacity),
      _fetch(buffer),
      _store(buffer) {}

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasQueuedItems() const noexcept {
    return _fetch != _end;
  }

  [[nodiscard]]
  ASMJIT_INLINE bool flip() noexcept {
    _end = _store;
    _fetch = _begin;
    _store = _begin;

    return hasQueuedItems();
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t remaining() const noexcept {
    return (size_t)(_end - _fetch);
  }

  //! Puts an item to the queue.
  ASMJIT_INLINE void put(T item) noexcept {
    ASMJIT_ASSERT(_store < _end);
    *_store++ = item;
  }

  //! Gets an item from the queue (the item is removed).
  [[nodiscard]]
  ASMJIT_INLINE T get() noexcept {
    ASMJIT_ASSERT(_fetch < _end);
    return *_fetch++;
  }
};

*/

class BitWordMutator {
public:
  BitWord _bitWord;

  ASMJIT_INLINE explicit BitWordMutator(Span<BitWord> span) noexcept {
    ASMJIT_ASSERT(span.size() == 1u);
    _bitWord = span[0];
  }

  [[nodiscard]]
  ASMJIT_INLINE BitWord bitWord([[maybe_unused]] size_t index) const noexcept {
    ASMJIT_ASSERT(index == 0u);
    return _bitWord;
  }

  ASMJIT_INLINE void setBitWord([[maybe_unused]] size_t index, BitWord bw) noexcept {
    ASMJIT_ASSERT(index == 0u);
    _bitWord = bw;
  }

  template<typename Index>
  [[nodiscard]]
  ASMJIT_INLINE bool bitAt(const Index& index) const noexcept {
    ASMJIT_ASSERT(size_t(index) < bit_size_of<BitWord>);
    return (_bitWord & (BitWord(1) << size_t(index))) != 0u;
  }

  template<typename Index>
  ASMJIT_INLINE void setBit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < bit_size_of<BitWord>);

    BitWord clearMask = BitWord(1u) << size_t(index);
    BitWord bitMask = BitWord(value) << size_t(index);

    _bitWord = (_bitWord & ~clearMask) | bitMask;
  }

  template<typename Index>
  ASMJIT_INLINE void addBit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < bit_size_of<BitWord>);

    BitWord bitMask = BitWord(value) << size_t(index);
    _bitWord |= bitMask;
  }

  template<typename Index>
  ASMJIT_INLINE void clearBit(const Index& index) noexcept {
    ASMJIT_ASSERT(size_t(index) < bit_size_of<BitWord>);

    BitWord bitMask = BitWord(1) << size_t(index);
    _bitWord &= ~bitMask;
  }

  template<typename Index>
  ASMJIT_INLINE void xorBit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < bit_size_of<BitWord>);

    BitWord bitMask = BitWord(value) << size_t(index);
    _bitWord ^= bitMask;
  }

  ASMJIT_INLINE void clearBits(const BitWordMutator& other) noexcept {
    _bitWord &= ~other._bitWord;
  }

  ASMJIT_INLINE void commit(Span<BitWord> span) const noexcept {
    span[0] = _bitWord;
  }
};

class BitVectorMutator {
public:
  template<typename T>
  static ASMJIT_INLINE T bitWordCountOf(const T& n) noexcept {
    return T((n + bit_size_of<BitWord> - 1u) / bit_size_of<BitWord>);
  }

  BitWord* _data;
  size_t _count;

  ASMJIT_INLINE BitVectorMutator(BitWord* data, size_t count) noexcept
    : _data(data),
      _count(count) {}

  ASMJIT_INLINE BitVectorMutator(Span<BitWord> span) noexcept
    : _data(span.data()),
      _count(span.size()) {}

  [[nodiscard]]
  ASMJIT_INLINE BitWord bitWord(size_t index) const noexcept {
    ASMJIT_ASSERT(index < _count);
    return _data[index];
  }

  ASMJIT_INLINE void setBitWord(size_t index, BitWord bw) noexcept {
    ASMJIT_ASSERT(index < _count);
    _data[index] = bw;
  }

  template<typename Index>
  [[nodiscard]]
  ASMJIT_INLINE bool bitAt(const Index& index) const noexcept {
    ASMJIT_ASSERT(size_t(index) < _count * bit_size_of<BitWord>);
    return bitVectorGetBit(_data, size_t(index));
  }

  template<typename Index>
  ASMJIT_INLINE void setBit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < _count * bit_size_of<BitWord>);
    bitVectorSetBit(_data, size_t(index), value);
  }

  template<typename Index>
  ASMJIT_INLINE void addBit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < _count * bit_size_of<BitWord>);
    bitVectorOrBit(_data, size_t(index), value);
  }

  template<typename Index>
  ASMJIT_INLINE void clearBit(const Index& index) noexcept {
    ASMJIT_ASSERT(size_t(index) < _count * bit_size_of<BitWord>);
    bitVectorSetBit(_data, size_t(index), false);
  }

  template<typename Index>
  ASMJIT_INLINE void xorBit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < _count * bit_size_of<BitWord>);
    bitVectorXorBit(_data, size_t(index), value);
  }

  ASMJIT_INLINE void clearBits(const BitVectorMutator& other) noexcept {
    ASMJIT_ASSERT(_count == other._count);

    size_t n = _count;
    const BitWord* otherData = other._data;

    for (size_t i = 0u; i < n; i++) {
      _data[i] &= ~otherData[i];
    }
  }

  ASMJIT_INLINE void commit(Span<BitWord> span) const noexcept {
    // Does nothing - each operation is written to memory.
    DebugUtils::unused(span);
  }
};

} // {Support}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_SUPPORT_P_H_INCLUDED
