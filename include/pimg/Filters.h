// from https://github.com/twitter/vireo

#include "pimg/FilterKernel.h"

#pragma once

namespace pimg
{

class ImageData;

class Filters
{
public:
    static void Adaptive4x4(const FilterKernel& kx, const FilterKernel& ky, 
        const ImageData& src, ImageData& dst);

}; // Filters

}