#include "UIElement.h"
namespace Bamboo::UI
{
    UIElement::UIElement()
    {
    }

    void UIElement::SetPosition(const Vector2& position)
    {
        m_Position = position;
    }

    void UIElement::SetScale(const Vector2& scale)
    {
        m_Scale = scale;
    }

    void UIElement::SetRotation(float angle)
    {
        m_Rotation = angle;
    }

    void UIElement::SetSize(const Vector2& size)
    {
        m_Size = size;
    }

    void UIElement::SetAnchor(const Vector2& anchor)
    {
        m_Anchor = anchor;
    }

    void UIElement::AddChild(Ref<UIElement> child)
    {
        if (!child) {
            return;
        }

        if (std::find(m_Children.begin(), m_Children.end(), child) != m_Children.end()) {
            return;
        }

        m_Children.push_back(child);
    }

    void UIElement::RemoveChild(Ref<UIElement> child)
    {
        auto it = std::find(m_Children.begin(), m_Children.end(), child);
        if (it != m_Children.end())
        {
            m_Children.erase(it);
            child->m_Parent = shared_from_this();
        }
    }

    void UIElement::ClearChildren()
    {
        for (auto &child : m_Children)
        {
            child->m_Parent.reset();
        }
        m_Children.clear();
    }

    void UIElement::SetParent(Ref<UIElement> parent)
    {
        auto self = shared_from_this();
        if (auto oldParent = m_Parent.lock()) {
            oldParent->RemoveChild(self);
        }

        m_Parent = parent;

        if (parent) {
            parent->AddChild(self);
        }
    }

}