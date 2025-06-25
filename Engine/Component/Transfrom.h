#pragma once

#include "../Math/Vector3.h"
#include "../Math/Quaternion.h"

#include "Component.h"
namespace Bamboo
{

    class Transform : Component
    {
    public:
        setPosition(const Vector3& position) { this->position = position; }
        setScale(const Vector3& scale) { this->scale = scale; }
        setRotation(const Quaternion& rotation) { this->rotation = rotation; }

        const Vector3& getPosition() const { return position; }
        const Vector3& getScale() const { return scale; }
        const Quaternion& getRotation() const { return rotation; }
        
    private:
        Vector3 position;
        Vector3 scale;
        Quaternion rotation;
        Vector3 eulerAngles;
    };
}; // namespace Bamboo