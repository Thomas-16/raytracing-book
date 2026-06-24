#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

#include <iostream>

// color is just an alias of vec3
using color = vec3;

void write_color(std::ostream &out, const color &pixel_color)
{
    double r = pixel_color.x();
    double g = pixel_color.y();
    double b = pixel_color.z();

    // Translate the [0, 1] component values to the byte range [0, 255]
    int rbyte = int(255.999 * r);
    int gbyte = int(255.999 * g);
    int bbyte = int(255.999 * b);

    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif
