#pragma once
#include "Macro.h"

/*
	Author By : Passer By
	Links : https://stackoverflow.com/a/45365798
	Modified By : Otamaa
*/

template<typename Callable>
union storage
{
	storage() { }
	std::decay_t<Callable> callable;
};

template<int, typename Callable, typename Ret, typename... Args>
static auto fnptr_impl(Callable&& c, Ret(*)(Args...))
{
	using calltype = Ret(__fastcall*)(Args...);
	static bool used = false;
	static storage<Callable> s;
	using type = decltype(s.callable);

	if (used)
		s.callable.~type();
	new (&s.callable) type(std::forward<Callable>(c));
	used = true;

	return [](Args... args) -> Ret {
		return Ret(s.callable(std::forward<Args>(args)...));
	};
}

template<typename Fn,  int N = 0, typename Callable>
static Fn* fnptr(Callable&& c) {
	return fnptr_impl<N>(std::forward<Callable>(c), (Fn*)nullptr);
}