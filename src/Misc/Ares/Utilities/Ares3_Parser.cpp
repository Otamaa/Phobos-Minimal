#include <Utilities/TemplateDefB.h>
//#include <format>

template<unsigned ...Len>
constexpr auto cat(const char(&...strings)[Len])
{
	constexpr unsigned N = (... + Len) - sizeof...(Len);
	std::array<char, N + 1> result = {};
	result[N] = '\0';

	auto it = result.begin();
	(void)((it = std::copy_n(strings, Len - 1, it), 0), ...);
	return result;
}
