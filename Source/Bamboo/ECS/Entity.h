#pragma once
#include "../Bamboo/Core/UUID.h"
#include "../Bamboo/Scene/Scene.h"
#include "../Bamboo/ECS/Component/Component.h"
#include "../Bamboo/Core/Assert.h"


#include "entt.hpp"
namespace Bamboo
{
    //class Entity
    //{
    //public:
    //    Entity() = default;
    //    Entity(entt::entity handle, Scene* scene);
    //    Entity(const Entity& other) = default;

    //    template <typename T, typename... Args>
    //    T& AddComponent(Args &&...args) 
    //    {
    //        T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward < Args(args)...);
    //        return component;
    //       
    //    }


    //    template <typename T>
    //    T& GetComponent() {
    //        return m_Scene->m_Registry.get<T>(m_EntityHandle);
    //             
    //    }

    //    template <typename T>
    //    bool HasComponent() const 
    //    {
    //        return false;
    //    }

    //    template <typename T>
    //    void RemoveComponent() 
    //    {
    //   
    //    }

    //    bool operator==(const Entity &other) const 
    //    {
    //        return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
    //    }

    //    UUID GetUUID() const { return m_uuid; }

    //    entt::entity GetHandle(){ return m_EntityHandle; }

    //private:
    //    entt::entity m_EntityHandle{entt::null}; 
    //    UUID m_uuid;
    //    Scene* m_Scene;
    //};

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			BAMBOO_ASSESERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			return component;
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			//m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			//HZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			//HZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		//UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		//const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};

}