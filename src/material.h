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

// Scatters rays in random directions following lambertian reflectance
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

// Reflects light following the law of reflection
// Has a fuzziness index that affects the reflected direction
class metal : public material
{
public:
    metal(const color &albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray &ray_in, const hit_record &rec, color &attenuation, ray &scattered) const override
    {
        vec3 reflected_direction = reflect(ray_in.direction(), rec.normal);
        reflected_direction = unit_vector(reflected_direction) + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected_direction);
        attenuation = albedo;
        return dot(scattered.direction(), rec.normal) > 0;
    }

private:
    color albedo;
    double fuzz;
};

// A dielectric material sometimes refracts and sometimes reflects
class dielectric : public material
{
public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray &ray_in, const hit_record &rec, color &attenuation, ray &scattered) const override
    {
        attenuation = color(1.0, 1.0, 1.0); // Glass surface obsorbs nothing
        double ri = rec.front_face ? (1.0 / refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(ray_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;

        vec3 ray_direction;
        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            ray_direction = reflect(unit_direction, rec.normal);
        else
            ray_direction = refract(unit_direction, rec.normal, ri);

        scattered = ray(rec.p, ray_direction);
        return true;
    }

private:
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    double refraction_index;

    static double reflectance(double cosine, double refraction_index)
    {
        // Use Schlick's approximation for reflectance
        double r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }
};

#endif
