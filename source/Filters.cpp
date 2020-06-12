#include "pimg/Filters.h"
#include "pimg/ImageData.h"

#include <algorithm>

namespace
{

const int IMAGE_CHANNELS = 3;

}

namespace pimg
{

void Filters::Adaptive4x4(const FilterKernel& kx, const FilterKernel& ky, 
                          const ImageData& src, ImageData& dst)
{
    float final[IMAGE_CHANNELS];

    const float* kernel_table_x = kx.GetTable();
    const float* kernel_table_y = ky.GetTable();

    for (int y = 0; y < dst.GetHeight(); ++y) {
        int start_y = kx.ComputeSampleStart(y);
        for (int x = 0; x < dst.GetWidth(); ++x) {
            int start_x = kx.ComputeSampleStart(x);
                
            const uint8_t* sample = &src.GetPixelData()[(start_y * src.GetWidth() + start_x) * IMAGE_CHANNELS];

            memset(final, 0, sizeof(final));

			unsigned int filter_idx_x = x * 4;
			unsigned int filter_idx_y = y * 4;

            float coeffs_x0 = *(kernel_table_x + filter_idx_x + 0);
            float coeffs_x1 = *(kernel_table_x + filter_idx_x + 1);
            float coeffs_x2 = *(kernel_table_x + filter_idx_x + 2);
            float coeffs_x3 = *(kernel_table_x + filter_idx_x + 3);

            {
				float coeffs_y = *(kernel_table_y + filter_idx_y + 0);
				for (size_t i = 0; i < IMAGE_CHANNELS; ++i) {
					float val = coeffs_x0 * sample[0 * IMAGE_CHANNELS + i] + coeffs_x1 * sample[1 * IMAGE_CHANNELS + i] + coeffs_x2 * sample[2 * IMAGE_CHANNELS + i] + coeffs_x3 * sample[3 * IMAGE_CHANNELS + i];
					final[i] += val * coeffs_y;
				}
				sample += src.GetWidth() * IMAGE_CHANNELS;
            }

			{
                float coeffs_y = *(kernel_table_y + filter_idx_y + 1);
				for (size_t i = 0; i < IMAGE_CHANNELS; ++i) {
                    float val = coeffs_x0 * sample[0 * IMAGE_CHANNELS + i] + coeffs_x1 * sample[1 * IMAGE_CHANNELS + i] + coeffs_x2 * sample[2 * IMAGE_CHANNELS + i] + coeffs_x3 * sample[3 * IMAGE_CHANNELS + i];
					final[i] += val * coeffs_y;
				}
				sample += src.GetWidth() * IMAGE_CHANNELS;
			}

			{
				float coeffs_y = *(kernel_table_y + filter_idx_y + 2);
				for (size_t i = 0; i < IMAGE_CHANNELS; ++i) {
                    float val = coeffs_x0 * sample[0 * IMAGE_CHANNELS + i] + coeffs_x1 * sample[1 * IMAGE_CHANNELS + i] + coeffs_x2 * sample[2 * IMAGE_CHANNELS + i] + coeffs_x3 * sample[3 * IMAGE_CHANNELS + i];
					final[i] += val * coeffs_y;
				}
				sample += src.GetWidth() * IMAGE_CHANNELS;
			}

			{
                float coeffs_y = *(kernel_table_y + filter_idx_y + 3);
				for (size_t i = 0; i < IMAGE_CHANNELS; ++i) {
                    float val = coeffs_x0 * sample[0 * IMAGE_CHANNELS + i] + coeffs_x1 * sample[1 * IMAGE_CHANNELS + i] + coeffs_x2 * sample[2 * IMAGE_CHANNELS + i] + coeffs_x3 * sample[3 * IMAGE_CHANNELS + i];
					final[i] += val * coeffs_y;
				}
				sample += src.GetWidth() * IMAGE_CHANNELS;
			}

            uint8_t* out_sample = const_cast<uint8_t*>(dst.GetPixelData()) + ((dst.GetHeight() - y - 1) * dst.GetWidth() + x) * IMAGE_CHANNELS;
            for (unsigned int i = 0; i < IMAGE_CHANNELS; i++) {
                out_sample[i] = static_cast<uint8_t>(std::min(std::max(final[i], 0.0f), 255.0f));
            }
        }
    }
}

}