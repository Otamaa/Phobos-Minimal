#pragma once

#include <string>

// because every project needs an own string implementation...

namespace detail
{
	template <typename T, typename Traits = std::char_traits<T>>
	inline __declspec(noinline) void __fastcall string_copy_n(
		T* const pDest, T const* const pSrc, size_t const cchMax) noexcept
	{
		auto const len = Traits::length(pSrc);
		auto const count = len < cchMax ? len : cchMax;
		Traits::copy(pDest, pSrc, count);
		pDest[count] = T();
	}
}

template <size_t Capacity, typename T = char>
struct FixedString {
	static const size_t Size = Capacity;

	static_assert(Capacity > 0, "Capacity cannot be 0.");

	using data_type = T[Capacity];

	FixedString() noexcept {
		this->chars[0] = 0;
	}

	explicit FixedString(const T* value) noexcept {
		__assume(value != this->chars);
		*this = value;
	}

	/* this is not how the game does it. it most likely has an operator= that
	*  takes a const FixedString&, then memcpys the entire contents over. this
	*  implementation does the same, but gets rid of the temporary object in
	*  case a pointer is passed.
	*/
	FixedString& operator= (FixedString const& value) noexcept = default;

	FixedString& operator= (const T* const value) noexcept {
		if(value) {
			if(value != this->chars) {
				// free function to not templatize on size
				detail::string_copy_n(this->chars, value, Size - 1);
			}
		} else {
			this->chars[0] = 0;
		}
		return *this;
	}

	explicit operator bool() {
		return this->chars[0] != 0;
	}

	explicit operator bool() const {
		return this->chars[0] != 0;
	}

	bool operator !() const {
		return this->chars[0] == 0;
	}

	operator const T* () const {
		return this->chars;
	}

	operator T* () {
		return this->chars;
	}

	data_type& data() {
		return this->chars;
	}

	const data_type& data() const {
		return this->chars;
	}

private:
	data_type chars;
};

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

	inline bool Empty() const { return Buffer[0] == '\0'; }
	inline char* Peek_Buffer() { return Buffer; }
	inline int Get_Length() const { return Length; }

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
	int retval = vsnprintf(temp_buffer, sizeof(temp_buffer), format, arg_list);

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
		std::strcat(Buffer, string);
	}
}

template<int SIZE>
void FixedChar<SIZE>::Clear()
{
	Length = 0;
	Buffer[0] = '\0';
}
