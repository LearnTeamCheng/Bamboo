#pragma once

#include <iostream>
#include <string>
#include <functional>
#include "../Bamboo/Math/Vector2.h"

#include "UI/UIElement.h"
namespace Bamboo::UI
{
    class Button : public UIElement
    {
    public:
        using ButtonClickListener = std::function<void()>;
        enum class ButtonTransition
        {
            None,
            Scale,
            Color,
        };

    public:
        Button() = default;
        Button(const std::string &text);
        /// @brief 构造函数
        /// @param pressedTexture 按下时的贴图
        /// @param normalTexture  正常时的贴图
        /// @param hoverTexture   鼠标悬停时的贴图
        Button(const std::string &pressedTexture, const std::string &normalTexture, const std::string &hoverTexture);
        ~Button();

        void SetText(const std::string &text) { m_Text = text; }
        const std::string& GetText() const { return m_Text; }

        void SetNormalTexture(const std::string &texture);
        void SetPressedTexture(const std::string &texture);
        void SetHoverTexture(const std::string &texture);

        /// @brief 设置按钮的过渡效果
        void SetTransition(ButtonTransition transition) { m_Transition = transition; }
        ButtonTransition GetTransition() const {return m_Transition; }

        /// @brief 设置按钮是否可以交互
        void SetInteractable(bool interactable) { m_IsInteractable = interactable; }
        bool IsInteractable() const { return m_IsInteractable; }

        void SetClickListener(ButtonClickListener &listener) { m_ClickListener = listener; }
        void RemoveClickListener() { m_ClickListener = nullptr; }

    private:
        ButtonTransition m_Transition;
        bool m_IsInteractable;
        ButtonClickListener m_ClickListener;

        std::string m_Text;
        bool m_isClicked;
    };
}