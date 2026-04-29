#pragma once
#include "../Bamboo/Core/Ref.h"

#include "../Bamboo/Math/Vector2.h"
#include "Component/UIComponent.h"
#include <vector>

namespace Bamboo::UI
{
    class UIElement : public std::enable_shared_from_this<UIElement>
    {
    public:
        UIElement();
        virtual ~UIElement();
        // transform
        void SetPosition(const Vector2 &position);
        Vector2 GetPosition() const { return m_Position; }

        void SetScale(const Vector2 &scale);
        void SetRotation(float angle);
        void SetSize(const Vector2 &size);
        void SetAnchor(const Vector2 &anchor);

        void RemoveChild(Ref<UIElement> child);
        void ClearChildren();

        template <typename T, typename... Args>
        Ref<T> AddComponent(Args &&...args)
        {
            // 判断类型T是否是UIComponent的子类
            static_assert(std::is_base_of_v<UIComponent, T>, "T must be a subclass of UIComponent!");
            auto component = std::make_shared<T>(std::forward<Args>(args)...);
            m_Components.emplace_back(component);
            return component;
        }

        template <typename T>
        Ref<T> GetComponent() const
        {
            for (auto &component : m_Components)
            {
                if (auto ptr = std::dynamic_pointer_cast<T>(component))
                {
                    return ptr;
                }
            }
            return nullptr;
        }

        template <typename T>
        bool HasComponent() const
        {
            for (auto &component : m_Components)
            {
                if (auto ptr = std::dynamic_pointer_cast<T>(component))
                {
                    return true;
                }
            }
            return false;
        }

        //
        void OnUpdate(float deltaTime);
        virtual void OnRender();

        void SetParent(Ref<UIElement> parent);
        WeakRef<UIElement> GetParent() const { return m_Parent; }

        void SetLayer(int layer);

    protected:
        void AddChild(Ref<UIElement> child);

    protected:
        WeakRef<UIElement> m_Parent;
        std::vector<Ref<UIElement>> m_Children;

        std::vector<Ref<UIComponent>> m_Components;

        Vector2 m_Position;
        Vector2 m_Scale;
        float m_Rotation;
        Vector2 m_Size;
        Vector2 m_Anchor;
    };
}
