// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_ZONEBITSET_P_H_INCLUDED
#define ASMJIT_CORE_ZONEBITSET_P_H_INCLUDED

#include "../core/span.h"
#include "../core/support.h"
#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_zone
//! \{

using BitWord = Support::BitWord;

namespace BitOps {
namespace {

template<typename T>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG size_t size_in_bits(const Span<T>& span) noexcept { return span.size() * Support::bit_size_of<T>; }

template<typename T>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG size_t size_in_words(size_t num_bits) noexcept { return (num_bits + Support::bit_size_of<T> - 1u) / Support::bit_size_of<T>; }

template<typename T, typename Index>
[[nodiscard]]
ASMJIT_INLINE bool bit_at(const Span<T>& span, const Index& index) noexcept {
  size_t i = Support::as_basic_uint(index);
  size_t word_index = i / Support::bit_size_of<T>;
  size_t bit_index = i % Support::bit_size_of<T>;

  return bool((span[word_index] >> bit_index) & 0x1u);
}

template<typename T, typename Index>
ASMJIT_INLINE void set_bit(const Span<T>& span, const Index& index, bool value) noexcept {
  size_t i = Support::as_basic_uint(index);
  size_t word_index = i / Support::bit_size_of<T>;
  size_t bit_index = i % Support::bit_size_of<T>;

  T and_mask = T(~(T(1u) << bit_index));
  T bit_mask = T(T(value) << bit_index);

  T& bit_word = span[word_index];
  bit_word = T((bit_word & and_mask) | bit_mask);
}

template<typename T, typename Index>
ASMJIT_INLINE void clear_bit(const Span<T>& span, const Index& index) noexcept {
  size_t i = Support::as_basic_uint(index);

  size_t word_index = i / Support::bit_size_of<T>;
  size_t bit_index = i % Support::bit_size_of<T>;

  T and_mask = T(~(T(1u) << bit_index));

  T& bit_word = span[word_index];
  bit_word = T(bit_word & and_mask);
}

template<typename T, typename Index>
ASMJIT_INLINE void or_bit(const Span<T>& span, const Index& index, bool value) noexcept {
  size_t i = Support::as_basic_uint(index);

  size_t word_index = i / Support::bit_size_of<T>;
  size_t bit_index = i % Support::bit_size_of<T>;

  T bit_mask = T(T(value) << bit_index);

  T& bit_word = span[word_index];
  bit_word = T(bit_word | bit_mask);
}

template<typename T, typename Index>
ASMJIT_INLINE void xor_bit(const Span<T>& span, const Index& index, bool value) noexcept {
  size_t i = Support::as_basic_uint(index);
  size_t word_index = i / Support::bit_size_of<T>;
  size_t bit_index = i % Support::bit_size_of<T>;

  T bit_mask = T(T(value) << bit_index);

  T& bit_word = span[word_index];
  bit_word = T(bit_word ^ bit_mask);
}

template<typename Op, typename T, typename... Args>
ASMJIT_INLINE void combine_spans(Span<T> dst, Args&&... args) noexcept {
  size_t size = dst.size();

  for (size_t i = 0u; i < size; i++) {
    dst[i] = Op::op(args[i]...);
  }
}

template<typename T, typename... Args>
ASMJIT_INLINE void or_(Span<T> dst, Args&&... args) noexcept {
  return combine_spans<Support::Or>(dst, std::forward<Args>(args)...);
}

} // {anonymous}
} // {BitOps}

//! Zone-allocated bit vector.
class ZoneBitVector {
public:
  ASMJIT_NONCOPYABLE(ZoneBitVector)

  //! \name Members
  //! \{

  //! Bits.
  BitWord* _data {};
  //! Size of the bit-vector (in bits).
  uint32_t _size {};
  //! Capacity of the bit-vector (in bits).
  uint32_t _capacity {};

  //! \}

  //! \cond INTERNAL
  //! \name Internal
  //! \{

  static ASMJIT_INLINE_NODEBUG size_t _wordsPerBits(size_t nBits) noexcept {
    return ((nBits + Support::bit_size_of<BitWord> - 1u) / Support::bit_size_of<BitWord>);
  }

  static ASMJIT_INLINE_NODEBUG void _zeroBits(BitWord* dst, size_t nBitWords) noexcept {
    for (size_t i = 0; i < nBitWords; i++) {
      dst[i] = 0;
    }
  }

  static ASMJIT_INLINE_NODEBUG void _fillBits(BitWord* dst, size_t nBitWords) noexcept {
    for (size_t i = 0; i < nBitWords; i++) {
      dst[i] = ~BitWord(0);
    }
  }

  static ASMJIT_INLINE_NODEBUG void _copyBits(BitWord* dst, const BitWord* src, size_t nBitWords) noexcept {
    for (size_t i = 0; i < nBitWords; i++) {
      dst[i] = src[i];
    }
  }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG ZoneBitVector() noexcept {}

  ASMJIT_INLINE_NODEBUG ZoneBitVector(ZoneBitVector&& other) noexcept
    : _data(other._data),
      _size(other._size),
      _capacity(other._capacity) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const ZoneBitVector& other) const noexcept { return  equals(other); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const ZoneBitVector& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the bit-vector is empty (has no bits).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool empty() const noexcept { return _size == 0; }

  //! Returns the size of this bit-vector (in bits).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }

  //! Returns the capacity of this bit-vector (in bits).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t capacity() const noexcept { return _capacity; }

  //! Returns the size of the `BitWord[]` array in `BitWord` units.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t sizeInBitWords() const noexcept { return _wordsPerBits(_size); }

  //! Returns the capacity of the `BitWord[]` array in `BitWord` units.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t capacityInBitWords() const noexcept { return _wordsPerBits(_capacity); }

  //! Returns bit-vector data as `BitWord[]`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BitWord* data() noexcept { return _data; }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const BitWord* data() const noexcept { return _data; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<BitWord> as_span() noexcept { return Span<BitWord>(_data, sizeInBitWords()); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<const BitWord> as_span() const noexcept { return Span<BitWord>(_data, sizeInBitWords()); }

  //! \}

  //! \name Utilities
  //! \{

  ASMJIT_INLINE_NODEBUG void swap(ZoneBitVector& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
  }

  ASMJIT_INLINE_NODEBUG void clear() noexcept {
    _size = 0;
  }

  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _data = nullptr;
    _size = 0u;
    _capacity = 0u;
  }

  ASMJIT_INLINE_NODEBUG void truncate(uint32_t newSize) noexcept {
    _size = Support::min(_size, newSize);
    _clearUnusedBits();
  }

  template<typename Index>
  [[nodiscard]]
  inline bool bitAt(const Index& index) const noexcept {
    ASMJIT_ASSERT(Support::as_basic_uint(index) < _size);
    return Support::bitVectorGetBit(_data, Support::as_basic_uint(index));
  }

  template<typename Index>
  inline void setBit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(Support::as_basic_uint(index) < _size);
    Support::bitVectorSetBit(_data, Support::as_basic_uint(index), value);
  }

  template<typename Index>
  inline void addBit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(Support::as_basic_uint(index) < _size);
    Support::bitVectorOrBit(_data, Support::as_basic_uint(index), value);
  }

  template<typename Index>
  inline void clearBit(const Index& index) noexcept {
    ASMJIT_ASSERT(Support::as_basic_uint(index) < _size);
    Support::bitVectorSetBit(_data, Support::as_basic_uint(index), false);
  }

  template<typename Index>
  inline void xorBit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(Support::as_basic_uint(index) < _size);
    Support::bitVectorXorBit(_data, Support::as_basic_uint(index), value);
  }

  ASMJIT_INLINE Error append(ZoneAllocator* allocator, bool value) noexcept {
    uint32_t index = _size;
    if (ASMJIT_UNLIKELY(index >= _capacity))
      return _append(allocator, value);

    uint32_t idx = index / Support::bit_size_of<BitWord>;
    uint32_t bit = index % Support::bit_size_of<BitWord>;

    if (bit == 0)
      _data[idx] = BitWord(value) << bit;
    else
      _data[idx] |= BitWord(value) << bit;

    _size++;
    return kErrorOk;
  }

  Error copyFrom(ZoneAllocator* allocator, const ZoneBitVector& other) noexcept;

  ASMJIT_INLINE void clearAll() noexcept {
    _zeroBits(_data, _wordsPerBits(_size));
  }

  ASMJIT_INLINE void fillAll() noexcept {
    _fillBits(_data, _wordsPerBits(_size));
    _clearUnusedBits();
  }

  ASMJIT_INLINE void clearBits(size_t start, size_t count) noexcept {
    ASMJIT_ASSERT(start <= size_t(_size));
    ASMJIT_ASSERT(size_t(_size) - start >= count);

    Support::bitVectorClear(_data, start, count);
  }

  ASMJIT_INLINE void fillBits(size_t start, size_t count) noexcept {
    ASMJIT_ASSERT(start <= size_t(_size));
    ASMJIT_ASSERT(size_t(_size) - start >= count);

    Support::bitVectorFill(_data, start, count);
  }

  //! Performs a logical bitwise AND between bits specified in this array and bits in `other`. If `other` has less
  //! bits than `this` then all remaining bits are set to zero.
  //!
  //! \note The size of the BitVector is unaffected by this operation.
  ASMJIT_INLINE void and_(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    size_t thisBitWordCount = sizeInBitWords();
    size_t otherBitWordCount = other.sizeInBitWords();
    size_t commonBitWordCount = Support::min(thisBitWordCount, otherBitWordCount);

    size_t i = 0;
    while (i < commonBitWordCount) {
      dst[i] = dst[i] & src[i];
      i++;
    }

    while (i < thisBitWordCount) {
      dst[i] = 0;
      i++;
    }
  }

  //! Performs a logical bitwise AND between bits specified in this array and negated bits in `other`. If `other`
  //! has less bits than `this` then all remaining bits are kept intact.
  //!
  //! \note The size of the BitVector is unaffected by this operation.
  ASMJIT_INLINE void andNot(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    size_t commonBitWordCount = _wordsPerBits(Support::min(_size, other._size));
    for (size_t i = 0; i < commonBitWordCount; i++) {
      dst[i] = dst[i] & ~src[i];
    }
  }

  //! Performs a logical bitwise OP between bits specified in this array and bits in `other`. If `other` has less
  //! bits than `this` then all remaining bits are kept intact.
  //!
  //! \note The size of the BitVector is unaffected by this operation.
  ASMJIT_INLINE void or_(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    size_t commonBitWordCount = _wordsPerBits(Support::min(_size, other._size));
    for (size_t i = 0; i < commonBitWordCount; i++) {
      dst[i] = dst[i] | src[i];
    }
    _clearUnusedBits();
  }

  ASMJIT_INLINE void _clearUnusedBits() noexcept {
    uint32_t idx = _size / Support::bit_size_of<BitWord>;
    uint32_t bit = _size % Support::bit_size_of<BitWord>;

    if (!bit) {
      return;
    }

    _data[idx] &= (BitWord(1) << bit) - 1u;
  }

  [[nodiscard]]
  ASMJIT_INLINE bool equals(const ZoneBitVector& other) const noexcept {
    if (_size != other._size) {
      return false;
    }

    const BitWord* aData = _data;
    const BitWord* bData = other._data;
    size_t numBitWords = _wordsPerBits(_size);

    for (size_t i = 0; i < numBitWords; i++) {
      if (aData[i] != bData[i]) {
        return false;
      }
    }
    return true;
  }

  //! \}

  //! \name Memory Management
  //! \{

  inline void release(ZoneAllocator* allocator) noexcept {
    if (!_data) {
      return;
    }

    allocator->release(_data, _capacity / 8u);
    reset();
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Error resize(ZoneAllocator* allocator, size_t newSize, bool newBitsValue = false) noexcept {
    return _resize(allocator, newSize, newSize, newBitsValue);
  }

  Error _resize(ZoneAllocator* allocator, size_t newSize, size_t idealCapacity, bool newBitsValue) noexcept;
  Error _append(ZoneAllocator* allocator, bool value) noexcept;

  //! \}

  //! \name Iterators
  //! \{

  class ForEachBitSet : public Support::BitVectorIterator<BitWord> {
  public:
    inline explicit ForEachBitSet(const ZoneBitVector& bitVector) noexcept
      : Support::BitVectorIterator<BitWord>(bitVector.data(), bitVector.sizeInBitWords()) {}
  };

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_ZONEBITSET_P_H_INCLUDED
