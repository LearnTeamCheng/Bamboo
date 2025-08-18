#pragma once

#include "../Bamboo/Event/Event.h"
namespace Bamboo 
{
    class ApplicationClosedEvent : public Event
    {
        public:
        ApplicationClosedEvent() = default;
        EVENT_CLASS_TYPE(WindowClose)
    };


    class  ApplicationResizeEvent:public Event
    {
    public:
        ApplicationResizeEvent(unsigned int width, unsigned int height) :m_Height(height),m_Width(width){};
        unsigned int GetHeight()const { return m_Height; }
        unsigned int GetWidth()const { return m_Width; }
        EVENT_CLASS_TYPE(WindowResize);
    private:
        unsigned int m_Width, m_Height;
    };
}