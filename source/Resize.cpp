#include "pimg/Resize.h"
#include "pimg/FilterKernel.h"
#include "pimg/ImageData.h"
#include "pimg/Filters.h"

namespace
{

const int IMAGE_CHANNELS = 3;

}

namespace pimg
{

bool Resize::DownSample(FilterType type, const ImageData& src, ImageData& dst)
{
    if (dst.GetWidth() <= 0 || dst.GetHeight() <= 0) {
        return false;
    }

    auto pixels = new uint8_t[dst.GetWidth() * dst.GetHeight() * IMAGE_CHANNELS];

    const size_t kernel_size = 4;   // 2 4 8 12

    FilterKernel kx(type, kernel_size, src.GetWidth(), dst.GetWidth());
    FilterKernel ky(type, kernel_size, src.GetHeight(), dst.GetHeight());

    Filters::Adaptive4x4(kx, ky, src, dst);

    dst.SetPixelData(pixels);

    return true;
}

}