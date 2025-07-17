#include "Quaternion.h"
namespace Bamboo{


    void Quaternion::FromAngleAxis(const Vector3& axis, float angle){
        float halfAngle = angle * 0.5f;
        float sinHalfAngle = sin(halfAngle);
        w = cos(halfAngle);
        x = sinHalfAngle * axis.x;
        y = sinHalfAngle * axis.y;
        z = sinHalfAngle * axis.z;
    }

}