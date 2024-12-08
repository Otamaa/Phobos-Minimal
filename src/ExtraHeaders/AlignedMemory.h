#pragma once

#include <Base/Always.h>
#include <memory>
#include <type_traits>
#include <utility>

template <size_t Size, size_t ByteAlignment>
struct AlignedMemory { };

// AlignedMemory is specialized for all supported alignments.
// Make sure we get a compiler error if someone uses an unsupported alignment.

#define BASE_DECL_ALIGNED_MEMORY(byte_alignment) \
    template <size_t Size> \
    class AlignedMemory<Size, byte_alignment> { \
     public: \
      ALIGNAS(byte_alignment) uint8_t data_[Size]; \
      constexpr void* void_data() { return static_cast<void*>(data_); } \
      constexpr const void* void_data() const { \
        return static_cast<const void*>(data_); \
      } \
      template<typename Type> \
     constexpr  Type* data_as() { return static_cast<Type*>(void_data()); } \
      template<typename Type> \
      constexpr const Type* data_as() const { \
        return static_cast<const Type*>(void_data()); \
      } \
     private: \
      void* operator new(size_t); \
      void operator delete(void*); \
    }

// Specialization for all alignments is required because MSVC (as of VS 2008)
// does not understand ALIGNAS(ALIGNOF(Type)) or ALIGNAS(template_param).
// Greater than 4096 alignment is not supported by some compilers, so 4096 is
// the maximum specified here.
BASE_DECL_ALIGNED_MEMORY(1);
BASE_DECL_ALIGNED_MEMORY(2);
BASE_DECL_ALIGNED_MEMORY(4);
BASE_DECL_ALIGNED_MEMORY(8);
BASE_DECL_ALIGNED_MEMORY(16);
BASE_DECL_ALIGNED_MEMORY(32);
BASE_DECL_ALIGNED_MEMORY(64);
BASE_DECL_ALIGNED_MEMORY(128);
BASE_DECL_ALIGNED_MEMORY(256);
BASE_DECL_ALIGNED_MEMORY(512);
BASE_DECL_ALIGNED_MEMORY(1024);
BASE_DECL_ALIGNED_MEMORY(2048);
BASE_DECL_ALIGNED_MEMORY(4096);

#undef BASE_DECL_ALIGNED_MEMORY