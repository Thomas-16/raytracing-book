#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"

// The material class needs to do 2 things:
// 1. Produce a scattered ray (or say it absorbed the incident ray)
// 2. If scattered, say how much the ray should be attnuated
class material
{
public:
    virtual ~material() = default;

    virtual bool scatter(const ray &ray_in, const hit_record &rec, color &attenuation, ray &scattered) const
    {
        return false;
    }
};

class lambertian : public material
{
public:
    lambertian(const color &albedo) : albedo(albedo) {}

    bool scatter(const ray &ray_in, const hit_record &rec, color &attenuation, ray &scattered) const override
    {
        vec3 scatter_direction = rec.normal + random_unit_vector();

        // Catch denenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

private:
    color albedo;
};

class metal : public material
{
public:
    metal(const color &albedo) : albedo(albedo) {}

    bool scatter(const ray &ray_in, const hit_record &rec, color &attenuation, ray &scattered) const override
    {
        vec3 reflected_direction = reflect(ray_in.direction(), rec.normal);
        scattered = ray(rec.p, reflected_direction);
        attenuation = albedo;
        return true;
    }

private:
    color albedo;
};

#endif
