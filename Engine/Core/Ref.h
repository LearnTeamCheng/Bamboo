#pragma once
#include <memory>
#include <utility>
namespace Bamboo {
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using WeakRef = std::weak_ptr<T>;

	template<typename T>
	Scope<T> Lock(const WeakRef<T>& weakRef) {
		return weakRef.lock();
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T,typename... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

};

