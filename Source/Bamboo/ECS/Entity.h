#pragma once
#include "../Core/UUID.h"
#include "World.h"
#include "../ECS/Component/Component.h"
#include "../Core/Assert.h"

#include "entt.hpp"
namespace Bamboo
{

	/// @brief 实体的轻量视图：一个 entt 实体句柄 + 它所属的 World。
	///
	/// 约定：
	///  - 默认构造的 Entity 是**无效**的（m_World == nullptr），组件操作不该在它上面调用；
	///  - 先判 IsValid() / operator bool，再操作组件；
	///  - 组件方法在 Debug 下有前置断言，在 Release 下降级为"不崩"。
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, World *world);
		Entity(const Entity &other) = default;

		template <typename T, typename... Args>
		T &AddComponent(Args &&...args)
		{
			// 断言之外还要判一次：BAMBOO_ASSERT 目前失败只打日志不中断
			//（docs/README.md 缺陷 P0-4），只靠断言的话 Release 下会崩。
			BAMBOO_ASSERT(IsValid(), "AddComponent on invalid entity!");
			BAMBOO_ASSERT(!HasComponent<T>(), "Entity already has component!");
			if (IsValid())
			{
				return m_World->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			}
			return InvalidComponent<T>();
		}

		template <typename T, typename... Args>
		T &AddOrReplaceComponent(Args &&...args)
		{
			BAMBOO_ASSERT(IsValid(), "AddOrReplaceComponent on invalid entity!");
			if (IsValid())
			{
				return m_World->GetRegistry().emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			}
			return InvalidComponent<T>();
		}

		template <typename T>
		T &GetComponent()
		{
			BAMBOO_ASSERT(IsValid(), "GetComponent on invalid entity!");
			BAMBOO_ASSERT(HasComponent<T>(), "Entity does not have component!");
			// registry.get<T>() 找不到时会触发 ENTT_ASSERT，那在 Release 下等于未定义行为，
			// 所以这里先自己判一次，不依赖引擎断言。
			if (HasComponent<T>())
			{
				return m_World->GetRegistry().get<T>(m_EntityHandle);
			}
			return InvalidComponent<T>();
		}

		/// @brief 找不到组件时返回 nullptr 的安全版本（不需要前置检查）
		template <typename T>
		T *TryGetComponent()
		{
			return IsValid() ? m_World->GetRegistry().try_get<T>(m_EntityHandle) : nullptr;
		}

		template <typename T>
		bool HasComponent()
		{
			// 无副作用，且常作为第一道检查，所以无效实体直接返回 false 而不是崩。
			// 注意：用 registry.has<T>(entity)（entt.hpp:8277）—— 这个版本没有 all_of。
			return IsValid() && m_World->GetRegistry().has<T>(m_EntityHandle);
		}

		template <typename T>
		void RemoveComponent()
		{
			BAMBOO_ASSERT(IsValid(), "RemoveComponent on invalid entity!");
			BAMBOO_ASSERT(HasComponent<T>(), "Entity does not have component!");
			if (HasComponent<T>())
			{
				m_World->GetRegistry().remove<T>(m_EntityHandle);
			}
		}

		operator bool() const { return IsValid(); }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		/// @brief 实体是否真的可用：句柄非空 + 所属 registry 里仍存在（未被销毁）。
		///
		/// 用 registry.valid() —— 它比较的是**含版本号的完整 entity 值**，
		/// 所以能检出"已销毁的实体"，而不只是 entt::null。
		bool IsValid() const;

		UUID GetUUID() { return GetComponent<IDComponent>().id; }
		const std::string &GetName() { return GetComponent<TagComponent>().tag; }

		bool operator==(const Entity &other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_World == other.m_World;
		}

		bool operator!=(const Entity &other) const
		{
			return !(*this == other);
		}

	private:
		/// @brief 操作失败时的"哑"组件占位。拿到它说明前面已经打过断言（Debug 下应已暴露）。
		template <typename T>
		static T &InvalidComponent()
		{
			static T dummy{};
			return dummy;
		}

		entt::entity m_EntityHandle{entt::null};
		World *m_World{nullptr};
	};

}
