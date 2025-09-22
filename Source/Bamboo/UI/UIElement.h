#pragma once
#include "../Bamboo/Core/Ref.h"
#include <vector>
namespace Bamboo {

    class UIElement
    {
    public:
        UIElement();
        ~UIElement();


        //transform
        void SetPosition(float x, float y);
        void SetScale(float x, float y);
        void SetRotation(float angle);
        void SetSize(float width, float height);
        void SetAnchor(float x, float y);


        void AddChild(Ref<UIElement> child);
        void RemoveChild(Ref<UIElement> child);

        //
        void OnUpdate(float deltaTime);
        void OnRender();

        void SetParent(Ref<UIElement> parent);
        Ref<UIElement> GetParent() const { return m_Parent; }

    protected:
        Ref<UIElement> m_Parent;
        std::vector<Ref<UIElement>> m_Children;
    
    };
}


