#pragma once

#include "../../Core/UUID.h"
namespace Bamboo
{
    struct IDComponent
    {
        UUID id;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
        IDComponent(const UUID& uuid) : id(uuid) {}
    };
}