#pragma once
#include <string
#include "../Bamboo/Event/Event.h"
#include "../Bamboo/Core/KeyCodes.h"

namespace Bamboo
{

    class KeyEvent : public Event
    {
    public:
        KeyEvent() = default;
        KeyCode GetKeyCode() const { return m_keyCode; }
        EVENT_CLASS_TYPE(KeyEvent);

    protected:
        KeyEvent(KeyCode keyCode) : m_keyCode(keyCode) {}
        KeyCode m_keyCode;
    };

    ///@brief 按键按下事件
    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(KeyCode keyCode, bool isRepeat = false) : KeyEvent(keyCode), m_Repeat(isRepeat) {}
        EVENT_CLASS_TYPE(KeyPressedEvent);

        std::string ToString() const override
        {
            std::stringstream ss;
            ss<< "KeyPressedEvent: " << m_keyCode << " (" << (m_Repeat? "repeat" : "press") << ")";
            return ss.str();
        }

    private:
        /// @brief   是否重复按键
        bool m_Repeat;
    };

    ///@brief 按键释放事件
    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(KeyCode keyCode) : KeyEvent(keyCode) {}
        EVENT_CLASS_TYPE(KeyReleasedEvent);

        std::string ToString() const override
        {
            std::stringstream ss;
            ss<< "KeyReleasedEvent: " << m_keyCode;
            return ss.str();
        }
    };
}