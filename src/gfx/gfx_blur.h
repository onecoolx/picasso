/* Picasso - a vector graphics library
 *
 * Copyright (C) 2014 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_BLUR_H_
#define _GFX_BLUR_H_

#include "common.h"
#include "data_vector.h"

namespace gfx {

extern uint16_t const g_stack_blur8_mul[255];
extern uint8_t const g_stack_blur8_shr[255];

// pixel transformer
template <typename PixFmt>
class pixfmt_transformer
{
public:
    typedef PixFmt pixfmt_type;
    typedef typename pixfmt_type::color_type color_type;

    explicit pixfmt_transformer(pixfmt_type& pixfmt)
        : m_pixfmt(&pixfmt)
    {
    }

    uint32_t width(void) const { return m_pixfmt->height(); }
    uint32_t height(void) const { return m_pixfmt->width(); }

    color_type pixel(int32_t x, int32_t y) const
    {
        return m_pixfmt->pixel(y, x);
    }

    void copy_color_hspan(int32_t x, int32_t y, uint32_t len, const color_type* colors)
    {
        m_pixfmt->copy_color_vspan(y, x, len, colors);
    }

private:
    pixfmt_type* m_pixfmt;
};

// stack blur calc
struct stack_blur_calc_rgba {
    typedef uint32_t value_type;
    value_type r;
    value_type g;
    value_type b;
    value_type a;

    void clear(void)
    {
        r = g = b = a = 0;
    }

    template <typename T> void add(const T& v)
    {
        r += v.r;
        g += v.g;
        b += v.b;
        a += v.a;
    }

    template <typename T> void add(const T& v, uint32_t k)
    {
        r += v.r * k;
        g += v.g * k;
        b += v.b * k;
        a += v.a * k;
    }

    template <typename T> void sub(const T& v)
    {
        r -= v.r;
        g -= v.g;
        b -= v.b;
        a -= v.a;
    }

    template <typename T> void calc_pix(T& v, uint32_t mul, uint32_t shr)
    {
        typedef typename T::value_type value_type;
        v.r = value_type((r * mul) >> shr);
        v.g = value_type((g * mul) >> shr);
        v.b = value_type((b * mul) >> shr);
        v.a = value_type((a * mul) >> shr);
    }
};

// stack blur generator
template <typename ColorType>
class stack_blur
{
public:
    typedef ColorType color_type;
    ALIGNED(16) typedef stack_blur_calc_rgba calculator_type;

    stack_blur()
    {
        m_shading.r = m_shading.g = m_shading.b = m_shading.a = 0;
    }

    void set_shading(const color_type& c)
    {
        m_shading.r = c.r;
        m_shading.g = c.g;
        m_shading.b = c.b;
        m_shading.a = 0;
    }

    template <typename Img>
    void blur_x(Img& img, uint32_t radius)
    {
        if (radius < 1) {
            return;
        }

        uint32_t x, y, xp, i;
        uint32_t stack_ptr;
        uint32_t stack_start;

        color_type pix;
        color_type* stack_pix;
        calculator_type sum;
        calculator_type sum_in;
        calculator_type sum_out;

        uint32_t w = img.width();
        uint32_t h = img.height();
        uint32_t wm = w - 1;
        uint32_t div = (radius << 1) + 1;

        uint32_t mul_sum = 0;
        uint32_t shr_sum = 0;
        uint32_t max_val = color_type::base_mask;

        if (max_val <= 255 && radius < 255) {
            mul_sum = g_stack_blur8_mul[radius];
            shr_sum = g_stack_blur8_shr[radius];
        }

        m_buffer.allocate(w + 128);
        m_stack.allocate(div + 32);

        for (y = 0; y < h; y++) {
            sum.clear();
            sum_in.clear();
            sum_out.clear();

            pix = img.pixel(0, y);
            for (i = 0; i <= radius; i++) {
                m_stack[i] = pix;
                sum.add(pix, i + 1);
                sum_out.add(pix);
            }

            for (i = 1; i <= radius; i++) {
                pix = img.pixel((i > wm) ? wm : i, y);
                m_stack[i + radius] = pix;
                sum.add(pix, radius + 1 - i);
                sum_in.add(pix);
            }

            stack_ptr = radius;
            for (x = 0; x < w; x++) {
                sum.calc_pix(m_buffer[x], mul_sum, shr_sum);
                sum.sub(sum_out);

                stack_start = stack_ptr + div - radius;

                if (stack_start >= div) {
                    stack_start -= div;
                }

                stack_pix = &m_stack[stack_start];

                sum_out.sub(*stack_pix);

                xp = x + radius + 1;

                if (xp > wm) {
                    xp = wm;
                }
                pix = img.pixel(xp, y);

                if ((pix.r == 0) && (pix.g == 0)
                    && (pix.b == 0) && (pix.a == 0)) {
                    pix.r = m_shading.r;
                    pix.g = m_shading.g;
                    pix.b = m_shading.b;
                    pix.a = m_shading.a;
                }

                *stack_pix = pix;

                sum_in.add(pix);
                sum.add(sum_in);

                ++stack_ptr;

                if (stack_ptr >= div) {
                    stack_ptr = 0;
                }

                stack_pix = &m_stack[stack_ptr];

                sum_out.add(*stack_pix);
                sum_in.sub(*stack_pix);
            }
            img.copy_color_hspan(0, y, w, &m_buffer[0]);
        }
    }

    template <typename Img>
    void blur_y(Img& img, uint32_t radius)
    {
        pixfmt_transformer<Img> img2(img);
        blur_x(img2, radius);
    }

    template <typename Img>
    void blur(Img& img, uint32_t radius)
    {
        blur_x(img, radius);
        pixfmt_transformer<Img> img2(img);
        blur_x(img2, radius);
    }

private:
    color_type m_shading;
    pod_vector<color_type> m_buffer;
    pod_vector<color_type> m_stack;
};

}
#endif /*_GFX_BLUR_H_*/
