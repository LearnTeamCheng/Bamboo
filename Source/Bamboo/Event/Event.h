#pragma once
#include <string>
namespace Bamboo
{
    enum EventType {
        WindowClose,
        WindowResize,
    };

#define EVENT_CLASS_TYPE(type) EventType GetEventType() const override { return EventType::type; }\
                            const char* GetName() const override { return #type; }\
                            static EventType GetStaticType() { return type; }

    class Event {
        public:
            virtual ~Event() {}
            bool Handled = false;
            virtual EventType GetEventType() const = 0;
            virtual const char* GetName() const = 0;    
            virtual std::string ToString() const { return GetName(); }


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