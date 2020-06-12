#include "pimg/FilterKernel.h"

#include <sm_const.h>

#include <algorithm>
#include <functional>

#include <string.h>
#include <assert.h>

namespace
{

// From Numerical Recipes in C: The Art of Scientific Computing
// Zeroth order modified bessel function of the first kind.
float bessel_i0(float x)
{
    float ax;
    double ans;
	double y;
	if ((ax=fabs(x)) < 3.75) {
		y=x/3.75;
		y*=y;
		ans=1.0+y*(3.5156229+y*(3.0899424+y*(1.2067492
											 +y*(0.2659732+y*(0.360768e-1+y*0.45813e-2)))));
	} else {
		y=3.75/ax;
		ans=(exp(ax)/sqrt(ax))*(0.39894228+y*(0.1328592e-1
											  +y*(0.225319e-2+y*(-0.157565e-2+y*(0.916281e-2
																				 +y*(-0.2057706e-1+y*(0.2635537e-1+y*(-0.1647633e-1
																												+y*0.392377e-2))))))));
	}
	return static_cast<float>(ans);
}

const float KaiserAlpha = 7.0f;
const float KaiserInvI0Alpha = 1.0f / bessel_i0(KaiserAlpha);

float kaiser(const float x, const float w)
{
	double xSq = x * x;
	if( xSq < 1.0 ) {
		return KaiserInvI0Alpha * bessel_i0(static_cast<float>(KaiserAlpha * std::sqrt(1.0 - xSq)));
	}
	return 0.0f;
}

float sinc(const float x)
{
	if( x == 0.0f ) {
		return 1.0f;
	}
	double xpi = x * SM_PI;
	return (float)(sin(xpi) / xpi);
}

float lanczos(const float x, const float invSize)
{
	return sinc(x) * sinc(x * invSize);
}

}

namespace pimg
{

FilterKernel::FilterKernel(FilterType type, size_t kernel_size, size_t in_size, size_t out_size)
    : m_kernel_size(kernel_size)
{
    float ratio = static_cast<float>(out_size) / static_cast<float>(in_size);
    float inv_ratio = static_cast<float>(in_size) / static_cast<float>(out_size);
    m_sample_ratio = inv_ratio;

    size_t table_alloc_sz = out_size * kernel_size;
    m_table = new float[table_alloc_sz];
    memset(m_table, 0, table_alloc_sz * sizeof(float));

    float max_window_size = (kernel_size * 0.5f) - 0.00001f;
    m_window_width = (std::min(max_window_size, kernel_size * 0.25f * inv_ratio));
    float inv_filter_scale = 1.0f / (kernel_size * 0.25f);
    int max_samples = 0;

    if (type == FilterType::Linear)
    {
        for (size_t i = 0; i < out_size; ++i) 
        {
            float sample = (i + 0.5f) * inv_ratio;
            float w0 = sample - floorf(sample);
            m_table[i * kernel_size + 0] = 1.0f - w0;
            m_table[i * kernel_size + 1] = w0;
        }
        //m_WindowWidth = 0.5f;
        max_samples = 2;
    }
    else
    {
        std::function<float(const float, const float)> filter_func = nullptr;
        switch (type)
        {
        case FilterType::Kaiser:
            filter_func = kaiser;
            break;
        case FilterType::Lanczos:
            filter_func = lanczos;
            break;
        default:
            assert(0);
        }

        for (size_t i = 0; i < out_size; ++i)
        {
			float sample = (i + 0.5f) * inv_ratio - 0.00001f;
			int start = static_cast<int>(sample - m_window_width + 0.5f);
			int end = static_cast<int>(sample + m_window_width + 0.5f);
			int num_samples = end - start;
			max_samples = std::max(max_samples, num_samples);
			float sum = 0.0f;
			for( int k = 0; k < num_samples; k++ ) {
				float sample_pos = ratio * ((float)(start + k) - sample + 0.5f);
				float weight = filter_func(sample_pos, inv_filter_scale);
				m_table[i * kernel_size + k] = weight;
				sum += weight;
			}
			for( unsigned int k = num_samples; k < kernel_size; k++ ) {
				m_table[i * kernel_size + k] = 0.0f;
			}
			float invSum = 1.0f / sum;
			for (int k = 0; k < num_samples; k++) {
				m_table[i * kernel_size + k] *= invSum;
			}
        }
    }

    m_max_samples = max_samples;
}

FilterKernel::~FilterKernel()
{
    delete[] m_table;
}

}