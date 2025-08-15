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
}