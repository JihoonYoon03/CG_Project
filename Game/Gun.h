#pragma once
#include "pch.h"
#include "Engine/Object.h"
#include "Engine/Camera.h"

class Gun : public Object
{
public:
    using Object::Object;

    void Fire()
    {
        recoilTime_ = 0.0f;
    }

    void Update(float deltaTime) override;
    void FollowCamera(const Camera& camera);

private:
    float recoilTime_ = 1.0f;
};
