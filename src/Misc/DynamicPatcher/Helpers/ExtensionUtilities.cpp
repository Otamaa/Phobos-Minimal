#include "ExtensionUtilities.h"

#include <memory>
#include <unordered_map>

template<typename T>
struct Extension {
	Extension() = default;
	Extension(T* pWho) : Object { pWho } { }
	virtual ~Extension() = default;

	T* Object;
};

template<typename T>
class ExtMap
{
	using SharedExtPtr = std::shared_ptr<Extension<T>>;
	static std::unordered_map<const T*, SharedExtPtr> Map;
public :
	ExtMap() = default;
	ExtMap(T* ptr) noexcept{
		Map.emplace(ptr, std::make_shared<Extension<T>>{ ptr });
	}
	virtual ~ExtMap() = default;

	static SharedExtPtr& FindByPointer(T* ptr) const noexcept {
		auto const iter = Map.find(ptr);
		if (iter != Map.end())
			return iter->second;

		return SharedExtPtr();
	}
};

template<typename T>
struct ExtensionReference
{
	using SharedExtPtr = std::shared_ptr<Extension<T>>;
	std::weak_ptr<Extension<T>> weakReference;

	ExtensionReference(const SharedExtPtr& ext) : weakReference { ext }
	{ }

	ExtensionReference(T* ptr) : weakReference {}
		, storedObject { nullptr }
	{
		weakReference = std::weak_ptr<Extension<T>> { ExtMap<T>::FindByPointer(ptr) };
	}
};

void Main()
{
}