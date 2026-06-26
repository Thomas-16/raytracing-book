#ifndef CAMERA_H
#define CAMERA_H

#include <chrono>
#include <iomanip>
#include <string>

#include "hittable.h"
#include "material.h"

// Responsible for:
// 1. Constructing and dispatching rays into the world
// 2. Using the results of these rays to construct the rendered image
class camera
{
public:
    double aspect_ratio = 1.0;
    int image_width = 100;
    int samples_per_pixel = 10;
    int max_bounces = 10;

    double vfov = 90.0;
    point3 lookfrom = point3(0, 0, 0); // Point camera is looking from
    point3 lookat = point3(0, 0, -1);  // Point camera is looking at
    vec3 vup = vec3(0, 1, 0);          // Camera-relative "up" direction

    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 19;   // Distance from camera lookfrom point to plane of perfect focus

    void render(const hittable &world)
    {
        initialize();
        const auto start_time = std::chrono::steady_clock::now();
        const auto show_progress = [&](int completed)
        {
            const double progress = static_cast<double>(completed) / image_height;
            const int filled = static_cast<int>(40 * progress);
            const double seconds = std::chrono::duration<double>(
                                       std::chrono::steady_clock::now() - start_time)
                                       .count();
            std::clog << "\rRendering [" << std::string(filled, '=')
                      << std::string(40 - filled, ' ') << "] "
                      << std::setw(3) << static_cast<int>(100 * progress) << "%  "
                      << "Elapsed: " << std::fixed << std::setprecision(1)
                      << seconds << "s" << std::flush;
        };

        std::cout << "P3\n"
                  << image_width << ' ' << image_height << "\n255\n";

        show_progress(0);

        for (int j = 0; j < image_height; j++)
        {
            for (int i = 0; i < image_width; i++)
            {
                color pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; sample++)
                {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_bounces, world);
                }
                write_color(std::cout, pixel_color / samples_per_pixel);
            }

            show_progress(j + 1);
        }

        const double seconds = std::chrono::duration<double>(
                                   std::chrono::steady_clock::now() - start_time)
                                   .count();
        std::clog << "\nRender complete. Elapsed time: " << seconds << "s\n";
    }

private:
    int image_height;           // Rendered image height
    double pixel_samples_scale; // Color scale factor for a sum of pixel samples
    point3 center;              // Camera center
    point3 pixel00_loc;         // Location of pixel 0, 0
    vec3 pixel_delta_u;         // Offset to pixel to the right
    vec3 pixel_delta_v;         // Offset to pixel below
    vec3 u, v, w;               // Camera frame basis vectors
    vec3 defocus_disk_u;        // Defocus disk horizontal radius
    vec3 defocus_disk_v;        // Defocus disk vertical radius

    void initialize()
    {
        // Calculate the image height
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        center = lookfrom;

        // Determine viewport dimensions
        double theta = degrees_to_radians(vfov);
        double h = std::tan(theta / 2);
        double viewport_height = 2 * h * focus_dist;
        double viewport_width = viewport_height * (double(image_width) / image_height);

        // Calculate the u,v,w unit basis vecetors for the camera coordinate frame
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges
        vec3 viewport_u = viewport_width * u;
        vec3 viewport_v = viewport_height * -v;

        // Calculate the horizontal and vertical delta vectors from pixel to pixel
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the position of the upperleft pixel
        vec3 viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors
        double defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    // Construct a camera ray originating from the defocus disk and directed
    // at a randomly sampled point around the pixel location i, j
    ray get_ray(int i, int j) const
    {
        vec3 offset = sample_square();
        vec3 pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

        // Instead of originating all rays from one point, originate them from
        // a disk around the lookfrom point to create defocus blur or depth of field
        vec3 ray_origin = defocus_angle <= 0 ? center : defocus_disk_sample();
        vec3 ray_direction = pixel_sample - ray_origin;
        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const
    {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const
    {
        // Returns a random point in the camera defocus disk
        vec3 p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    color ray_color(const ray &r, int bounces_left, const hittable &world) const
    {
        // If we've exceeded the ray bounce limit, no more light is gathered
        if (bounces_left <= 0)
            return color(0, 0, 0);

        hit_record rec;
        if (world.hit(r, interval(0.001, infinity), rec))
        {
            // If the ray hit something, calculate the direction it's reflected to and fire a new ray
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, bounces_left - 1, world);
            return color(0, 0, 0);
        }

        // The ray is going into the sky
        // A simple blue-to-white sky gradient depending on ray y coordinate
        vec3 unit_direction = unit_vector(r.direction());
        double a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
};

#endif
