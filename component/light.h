#pragma once
#include<DirectXMath.h>
#include <limits>
namespace component {
    class Light  {
    public:
        DirectX::XMFLOAT3 color;
        float intensity;

        Light(DirectX::XMFLOAT3& color, float intensity = 1.0f)
            : color(color), intensity(intensity) {}
    };

    class DirectionLight : public Light {
    public:
        using Light::Light;
    };

    class PointLight : public Light {
    public:
        float linear, quadratic;
        float range = std::numeric_limits<float>::max();

        using Light::Light;

        void SetAttenuation(float linear, float quadratic);
        float GetAttenuation(float distance) const;
    };

    class Spotlight : public Light {
    public:
        float inner_cutoff;  // angle in degrees at the base of the inner cone
        float outer_cutoff;  // angle in degrees at the base of the outer cone
        float range = std::numeric_limits<float>::max();

        using Light::Light;

        void SetCutoff(float range, float inner_cutoff = 15.0f, float outer_cutoff = 30.0f);
        float GetInnerCosine() const;
        float GetOuterCosine() const;
        float GetAttenuation(float distance) const;
    };

    class AreaLight : public Light {
    public:
        using Light::Light;

        // TODO: to be implemented, using Bezier curves sampling and LTC
    };

    class VolumeLight : public Light {
    public:
        using Light::Light;

        // TODO: to be implemented, requires volumetric path tracing
    };

}