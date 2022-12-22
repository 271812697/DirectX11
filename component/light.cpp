#include"light.h"
#include"../core/log.h"
#include"../util/math.h"
namespace component {
    void PointLight::SetAttenuation(float linear, float quadratic) {
        CORE_ASERT(linear > 0, "The linear attenuation factor must be positive ...");
        CORE_ASERT(quadratic > 0, "The quadratic attenuation factor must be positive ...");

        this->linear = linear;
        this->quadratic = quadratic;

        // find the max range by solving the quadratic equation when attenuation is <= 0.01
        float a = quadratic;
        float b = linear;
        float c = -100.0f;
        float delta = b * b - 4.0f * a * c;

        CORE_ASERT(delta > 0.0f, "You can never see this line, it is mathematically impossible...");
        range = c / (-0.5f * (b + sqrt(delta)));  // Muller's method
    }

    float PointLight::GetAttenuation(float distance) const {
        CORE_ASERT(distance >= 0.0f, "Distance to the light source cannot be negative ...");
        return distance >= range ? 0.0f
            : 1.0f / (1.0f + linear * distance + quadratic * pow(distance, 2.0f));
    }

    void Spotlight::SetCutoff(float range, float inner_cutoff, float outer_cutoff) {
        CORE_ASERT(range > 0, "The spotlight range must be positive ...");
        CORE_ASERT(inner_cutoff > 0, "The inner cutoff angle must be positive ...");
        CORE_ASERT(outer_cutoff > 0, "The outer cutoff angle must be positive ...");

        this->range = range;
        this->inner_cutoff = inner_cutoff;
        this->outer_cutoff = outer_cutoff;
    }

    float Spotlight::GetInnerCosine() const {
        return cos(util::radians(inner_cutoff));
    }

    float Spotlight::GetOuterCosine() const {
        return cos(util::radians(outer_cutoff));
    }

    float Spotlight::GetAttenuation(float distance) const {

        CORE_ASERT(distance >= 0.0f, "Distance to the light source cannot be negative...");
        return 1.0f - std::clamp(distance / range, 0.0f, 1.0f);
    }
}