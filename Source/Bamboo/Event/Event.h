#pragma once
#include "../Bamboo/Core/Base.h"
#include <string>
namespace Bamboo
{
    enum EventType {
        WindowClose,
        WindowResize,
        KeyPressed,
        KeyReleased,
    };


    enum EventCategory
    {
        None = 0,
        EventCategoryApplication = BIT(0),
        EventCategoryInput = BIT(1),
        EventCategoryKeyboard = BIT(2),
        EventCategoryMouse = BIT(3),
        EventCategoryMouseButton = BIT(4)
    };

#define EVENT_CLASS_TYPE(type) EventType GetEventType() const override { return EventType::type; }\
                            const char* GetName() const override { return #type; }\
                            static EventType GetStaticType() { return type; }
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }
    class Event {
        public:
            virtual ~Event() {}
            bool Handled = false;
            virtual EventType GetEventType() const = 0;
            virtual const char* GetName() const = 0;    
            virtual std::string ToString() const { return GetName(); }
            virtual int GetCategoryFlags() const = 0;

    };


    class EventDispatcher {
        public:
            EventDispatcher(Event& event) : m_event(event) {}

            template<typename T, typename F>
            bool Dispatch(const F& func) {
                if (m_event.GetEventType() == T::GetStaticType()) {
                    m_event.Handled |= func(static_cast<T&>(m_event));
                    return true;
                }
                return false;
            }

        private:
            Event& m_event;
    };
};