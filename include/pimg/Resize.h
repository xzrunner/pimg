#pragma once

#include "pimg/FilterType.h"

namespace pimg
{

class ImageData;

class Resize
{
public:
    static bool DownSample(FilterType type, const ImageData& src, ImageData& dst);

}; // Resize

}