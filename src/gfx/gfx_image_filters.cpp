/* Picasso - a vector graphics library
 *
 * Copyright (C) 2014 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"

#include "gfx_image_filters.h"
#include "picasso.h"

namespace gfx {

// image_filter
template<typename FilterType>
class image_filter : public image_filter_adapter
{
public:
    image_filter()
    {
        calculate(m_filter);
    }

private:
    FilterType m_filter;
};

// filter_bilinear
class image_filter_bilinear
{
public:
    scalar radius(void) const { return FLT_TO_SCALAR(1.0f); }
    scalar calc_weight(scalar x) const
    {
        return FLT_TO_SCALAR(1.0f) - x;
    }
};

// filter_gaussian
class image_filter_gaussian
{
public:
    scalar radius(void) const { return FLT_TO_SCALAR(2.0f); }
    scalar calc_weight(scalar x) const
    {
        return Exp(-FLT_TO_SCALAR(2.0f) * x * x) * Sqrt(FLT_TO_SCALAR(2.0f) * _1divPI);
    }
};

// filter_quadric
class image_filter_quadric
{
public:
    scalar radius(void) const { return FLT_TO_SCALAR(1.5f); }
    scalar calc_weight(scalar x) const
    {
        if (x < FLT_TO_SCALAR(0.5f))
            return FLT_TO_SCALAR(0.75f) - x * x;

        if (x < FLT_TO_SCALAR(1.5f)) {
            scalar t = x - FLT_TO_SCALAR(1.5f); 
            return FLT_TO_SCALAR(0.5f) * t * t;
        }
        return FLT_TO_SCALAR(0.0f);
    }
};

// filter_bicubic
class image_filter_bicubic
{
public:
    scalar radius(void) const { return FLT_TO_SCALAR(2.0f); }
    scalar calc_weight(scalar x) const
    {
        return FLT_TO_SCALAR(1.0f / 6.0f) * (pow3(x + 2) - 4 * pow3(x + 1) + 6 * pow3(x) - 4 * pow3(x - 1));
    }
private:
    static scalar pow3(scalar x)
    {
        return (x <= FLT_TO_SCALAR(0.0f)) ? FLT_TO_SCALAR(0.0f) : x * x * x;
    }
};

// filter creater
image_filter_adapter* create_image_filter(int filter)
{
    switch (filter) {
        case FILTER_BILINEAR:
            return new image_filter<image_filter_bilinear>;
        case FILTER_GAUSSIAN:
            return new image_filter<image_filter_gaussian>;
        case FILTER_BICUBIC:
            return new image_filter<image_filter_bicubic>;
        case FILTER_QUADRIC:
            return new image_filter<image_filter_quadric>;
        default:
            //FILTER_NEAREST: no filter
            return 0;
    }
}

}
