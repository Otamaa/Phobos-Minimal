#pragma once

#include <string>
#include <Base/Always.h>
#include <CRT.h>
#include <stdexcept>

// because every project needs an own string implementation...

namespace detail
{
	template <typename CharT, typename Traits = std::char_traits<CharT>>
	OPTIONALINLINE void __fastcall string_copy_n(
		CharT* const dest, CharT const* const src, size_t const dest_size) noexcept
	{
		if (dest && src && dest_size > 0) {
			size_t len = std::char_traits<CharT>::length(src);
			size_t copy_len = std::min(len, dest_size);
			std::copy_n(src, copy_len, dest);
			dest[copy_len] = CharT{};
		}
	}

	template <typename CharT, typename Traits = std::char_traits<CharT>>
    OPTIONALINLINE void string_copy_n_unsafe(
        CharT* dest,
        const CharT* src,
        size_t src_len,
        size_t max_size) noexcept
    {
        const size_t copy_count = src_len < max_size ? src_len : max_size;
        Traits::copy(dest, src, copy_count);
        dest[copy_count] = CharT{};
    }
}

template <size_t Capacity, typename CharT = char>
class FixedString {
public:
    using value_type = CharT;
    using size_type = size_t;
    using pointer = CharT*;
    using const_pointer = const CharT*;
    using reference = CharT&;
    using const_reference = const CharT&;
    using iterator = CharT*;
    using const_iterator = const CharT*;
    using traits_type = std::char_traits<CharT>;
	using data_type = CharT[Capacity];

    static constexpr size_type capacity() noexcept { return Capacity - 1; }
    static constexpr size_type max_size() noexcept { return Capacity; }

    static_assert(Capacity > 0, "Capacity must be greater than 0");

    // ========== Constructors ==========

    constexpr FixedString() noexcept : chars_{} {}

    constexpr FixedString(const_pointer str) noexcept : chars_{} {
        if (str) {
            assign(str);
        }
    }

    constexpr FixedString(std::basic_string_view<CharT> sv) noexcept : chars_{} {
        assign(sv.data(), sv.size());
    }

    // Constructor from std::string
    FixedString(const std::basic_string<CharT>& str) noexcept : chars_{} {
        assign(str.data(), str.size());
    }

    constexpr FixedString(const FixedString&) noexcept = default;

    // ========== Assignment Operators ==========

    constexpr FixedString& operator=(const FixedString&) noexcept = default;

    constexpr FixedString& operator=(const_pointer str) noexcept {
        if (str && str != chars_) {
            assign(str);
        } else if (!str) {
            clear();
        }
        return *this;
    }

    constexpr FixedString& operator=(std::basic_string_view<CharT> sv) noexcept {
        assign(sv.data(), sv.size());
        return *this;
    }

    // Assignment from std::string
    FixedString& operator=(const std::basic_string<CharT>& str) noexcept {
        assign(str.data(), str.size());
        return *this;
    }

    // ========== Assign Methods ==========

    constexpr void assign(const_pointer str) noexcept {
        if (str) {
            detail::string_copy_n(chars_, str, capacity());
        } else {
            clear();
        }
    }

    constexpr void assign(const_pointer str, size_type len) noexcept {
        if (str) {
            const size_type copy_len = std::min(len, capacity());
            traits_type::copy(chars_, str, copy_len);
            chars_[copy_len] = CharT{};
        } else {
            clear();
        }
    }

    void assign(const std::basic_string<CharT>& str) noexcept {
        assign(str.data(), str.size());
    }

    constexpr void clear() noexcept {
        chars_[0] = CharT{};
    }

    // ========== Element Access ==========

    constexpr reference operator[](size_type pos) noexcept {
        return chars_[pos];
    }

    constexpr const_reference operator[](size_type pos) const noexcept {
        return chars_[pos];
    }

    constexpr reference at(size_type pos) {
        if (pos >= size()) {
            throw std::out_of_range("FixedString::at");
        }
        return chars_[pos];
    }

    constexpr const_reference at(size_type pos) const {
        if (pos >= size()) {
            throw std::out_of_range("FixedString::at");
        }
        return chars_[pos];
    }

    constexpr reference front() noexcept {
        return chars_[0];
    }

    constexpr const_reference front() const noexcept {
        return chars_[0];
    }

    constexpr reference back() noexcept {
        return chars_[size() > 0 ? size() - 1 : 0];
    }

    constexpr const_reference back() const noexcept {
        return chars_[size() > 0 ? size() - 1 : 0];
    }

    // ========== String Operations ==========

    constexpr const_pointer c_str() const noexcept {
        return chars_;
    }

    constexpr pointer data() noexcept {
        return chars_;
    }

    constexpr const_pointer data() const noexcept {
        return chars_;
    }

    constexpr size_type size() const noexcept {
        return traits_type::length(chars_);
    }

    constexpr size_type length() const noexcept {
        return size();
    }

    constexpr bool empty() const noexcept {
        return chars_[0] == CharT{};
    }

    // ========== Conversions ==========

    constexpr explicit operator bool() const noexcept {
        return !empty();
    }

    constexpr operator const_pointer() const noexcept {
        return chars_;
    }

    constexpr operator std::basic_string_view<CharT>() const noexcept {
        return std::basic_string_view<CharT>(chars_, size());
    }

    // Conversion to std::string
    operator std::basic_string<CharT>() const {
        return std::basic_string<CharT>(chars_, size());
    }

    // Explicit conversion method (more efficient, no copy)
    std::basic_string<CharT> str() const {
        return std::basic_string<CharT>(chars_, size());
    }

	constexpr const data_type& raw() const {
		return this->chars_;
	}

	constexpr data_type& raw() {
		return this->chars_;
	}
    // ========== Iterators ==========

    constexpr iterator begin() noexcept { return chars_; }
    constexpr const_iterator begin() const noexcept { return chars_; }
    constexpr const_iterator cbegin() const noexcept { return chars_; }

    constexpr iterator end() noexcept { return chars_ + size(); }
    constexpr const_iterator end() const noexcept { return chars_ + size(); }
    constexpr const_iterator cend() const noexcept { return chars_ + size(); }

    // ========== String Operations (std::string-like) ==========

    // Append operations
    FixedString& append(const_pointer str) {
        if (str) {
            const size_type current = size();
            const size_type remaining = capacity() - current;
            if (remaining > 0) {
                const size_type str_len = traits_type::length(str);
                const size_type copy_len = std::min(str_len, remaining);
                traits_type::copy(chars_ + current, str, copy_len);
                chars_[current + copy_len] = CharT{};
            }
        }
        return *this;
    }

    FixedString& append(const std::basic_string<CharT>& str) {
        return append(str.data(), str.size());
    }

    FixedString& append(const_pointer str, size_type len) {
        if (str && len > 0) {
            const size_type current = size();
            const size_type remaining = capacity() - current;
            if (remaining > 0) {
                const size_type copy_len = std::min(len, remaining);
                traits_type::copy(chars_ + current, str, copy_len);
                chars_[current + copy_len] = CharT{};
            }
        }
        return *this;
    }

    // Operator +=
    FixedString& operator+=(const_pointer str) {
        return append(str);
    }

    FixedString& operator+=(const std::basic_string<CharT>& str) {
        return append(str);
    }

    FixedString& operator+=(CharT c) {
        const size_type current = size();
        if (current < capacity()) {
            chars_[current] = c;
            chars_[current + 1] = CharT{};
        }
        return *this;
    }

    // Compare
    int compare(const_pointer str) const noexcept {
        return traits_type::compare(chars_, str,
            std::min(size(), traits_type::length(str)) + 1);
    }

    int compare(const std::basic_string<CharT>& str) const noexcept {
        return compare(str.c_str());
    }

    int compare(const FixedString& other) const noexcept {
        return traits_type::compare(chars_, other.chars_, Capacity);
    }

    // ========== Comparison Operators ==========

    constexpr bool operator==(const FixedString& other) const noexcept {
        return traits_type::compare(chars_, other.chars_, Capacity) == 0;
    }

    constexpr bool operator==(const_pointer str) const noexcept {
        return str && traits_type::compare(chars_, str,
            traits_type::length(str) + 1) == 0;
    }

    bool operator==(const std::basic_string<CharT>& str) const noexcept {
        return size() == str.size() &&
               traits_type::compare(chars_, str.data(), size()) == 0;
    }

    constexpr bool operator!=(const FixedString& other) const noexcept {
        return !(*this == other);
    }

    constexpr bool operator!=(const_pointer str) const noexcept {
        return !(*this == str);
    }

    bool operator!=(const std::basic_string<CharT>& str) const noexcept {
        return !(*this == str);
    }

    constexpr bool operator<(const FixedString& other) const noexcept {
        return traits_type::compare(chars_, other.chars_,
            std::min(size(), other.size()) + 1) < 0;
    }

private:
    CharT chars_[Capacity];
};

// ========== Non-member Comparison Operators ==========

template<size_t N, typename CharT>
bool operator==(const std::basic_string<CharT>& lhs, const FixedString<N, CharT>& rhs) noexcept {
    return rhs == lhs;
}

template<size_t N, typename CharT>
bool operator!=(const std::basic_string<CharT>& lhs, const FixedString<N, CharT>& rhs) noexcept {
    return rhs != lhs;
}

template<size_t N, typename CharT>
bool operator==(const CharT* lhs, const FixedString<N, CharT>& rhs) noexcept {
    return rhs == lhs;
}

template<size_t N, typename CharT>
bool operator!=(const CharT* lhs, const FixedString<N, CharT>& rhs) noexcept {
    return rhs != lhs;
}

// ========== Stream Operators ==========

template<size_t N, typename CharT>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const FixedString<N, CharT>& str) {
    return os << str.c_str();
}

template<size_t N, typename CharT>
std::basic_istream<CharT>& operator>>(std::basic_istream<CharT>& is, FixedString<N, CharT>& str) {
    std::basic_string<CharT> temp;
    is >> temp;
    str = temp;
    return is;
}

// ========== Deduction Guides ==========

template<size_t N, typename CharT>
FixedString(const CharT(&)[N]) -> FixedString<N, CharT>;

// ========== Type Aliases ==========

using FixedString8 = FixedString<8>;
using FixedString16 = FixedString<16>;
using FixedString32 = FixedString<32>;
using FixedString64 = FixedString<64>;
using FixedString128 = FixedString<128>;
using FixedString256 = FixedString<256>;

template <size_t Capacity>
using FixedWString = FixedString<Capacity, wchar_t>;

template<int SIZE>
class FixedChar
{
public:
	FixedChar();
	~FixedChar() { }

	void operator+=(const char* string) { Append(string); }
	void operator=(const char* string) { Clear(); Append(string); }

	void Append(const char* string);
	int Format(const char* format, ...);
	void Clear();

	OPTIONALINLINE bool Empty() const { return Buffer[0] == '\0'; }
	OPTIONALINLINE char* Peek_Buffer() { return Buffer; }
	OPTIONALINLINE int Get_Length() const { return Length; }

private:
	int Length;
	static char Buffer[SIZE + 1];
};

template<int SIZE>
char FixedChar<SIZE>::Buffer[SIZE + 1];

template<int SIZE>
FixedChar<SIZE>::FixedChar() :
	Length(0)
{
	Buffer[0] = '\0';
}

template<int SIZE>
int FixedChar<SIZE>::Format(const char* format, ...)
{
	va_list arg_list;
	va_start(arg_list, format);

	char temp_buffer[2048] = { 0 };
	int retval = CRT::vsprintf(temp_buffer,format, arg_list);

	Append(temp_buffer);

	va_end(arg_list);

	return retval;
}

template<int SIZE>
void FixedChar<SIZE>::Append(const char* string)
{
	int src_len = std::strlen(string);
	if (src_len + Length < SIZE - 1)
	{
		Length += src_len;
		CRT::strcat(Buffer, string);
	}
}

template<int SIZE>
void FixedChar<SIZE>::Clear()
{
	Length = 0;
	Buffer[0] = '\0';
}
