#pragma once
#include "../Bamboo/Core/Ref.h"

#include "../Bamboo/Math/Vector2.h"
#include <vector>

namespace Bamboo::UI
{
        class UIElement :public std::enable_shared_from_this<UIElement>
        {
        public:
            UIElement();
            ~UIElement();
            // transform
            void SetPosition(const Vector2& position);
            void SetScale(const Vector2& scale);
            void SetRotation(float angle);
            void SetSize(const Vector2& size);
            void SetAnchor(const Vector2& anchor);

            void RemoveChild(Ref<UIElement> child);
            void UIElement::ClearChildren();

            //
            void OnUpdate(float deltaTime);
            void OnRender();

            void SetParent(Ref<UIElement> parent);
            WeakRef<UIElement> GetParent() const { return m_Parent; }

        protected:
            void AddChild(Ref<UIElement> child);

        protected:
            WeakRef<UIElement> m_Parent;
            std::vector<Ref<UIElement>> m_Children;

            Vector2 m_Position;
            Vector2 m_Scale;
            float m_Rotation;
            Vector2 m_Size;
            Vector2 m_Anchor;
        };
}
