// from https://github.com/twitter/vireo

#pragma once

#include "pimg/FilterType.h"

namespace pimg
{

class FilterKernel
{
public:
    FilterKernel(FilterType type, size_t kernel_size, size_t in_size, size_t out_size);
    ~FilterKernel();

    int ComputeSampleStart(int out_position) const
    {
        float sample = (out_position + 0.5f) * m_sample_ratio - 0.00001f;
        return (int)(sample - m_window_width + 0.5f);
    }

    const float* GetTable() const { return m_table; }

private:
    size_t m_kernel_size = 0;

    float m_window_width = 0;
    float m_sample_ratio = 0;

    float* m_table = nullptr;

    size_t m_max_samples = 0;

}; // FilterKernel

}