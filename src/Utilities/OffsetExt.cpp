#include <iostream>
#include <concepts>

template <typename T>
concept ExtContainer = requires(T a) { T::ExtOffset; };


template<typename T, ExtContainer TExt>
struct ExtMapContainer
{
	static TExt* Find(T* pItem)
	{
		return *(TExt**)((size_t)pItem + TExt::ExtOffset);
	}
};

struct Foo
{
	int Field_0;
	int Field_4;
	int Field_8;
	int Field_C;
	int Field_10;

};

struct FooExt
{
	static constexpr size_t ExtOffset = 0x10;

	int A;
	int B;

	static ExtMapContainer<Foo, FooExt> ExtMap;
};

#define CallNoInline(f) (decltype(&f) ((intptr_t)f - 1 + 1))

void test(Foo* pThis)
{
	auto pExt = FooExt::ExtMap.Find(pThis);
	std::cout << pExt << std::endl;
}

int main()
{
	Foo object;
	FooExt oext;
	object.Field_10 = (size_t)&oext;

	CallNoInline(test)(&object);
	std::cout << &oext << std::endl;
	return 0;
}