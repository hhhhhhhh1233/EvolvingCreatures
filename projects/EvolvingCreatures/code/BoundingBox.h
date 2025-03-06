#pragma once

#include "core/math/vec3.h"
#include <vector>

class BoundingBox
{
public:
    vec3 Min;
    vec3 Max;

    BoundingBox(vec3 Position, vec3 Scale);
    BoundingBox();
    bool IsColliding(BoundingBox Other);
    bool PointIsInShape(vec3 Point) const;
    vec3 CalcDistanceToPoint(vec3 Point);
    void Move(vec3 Diff);
    vec3 GetPosition() const;
    vec3 GetScale() const;
};
