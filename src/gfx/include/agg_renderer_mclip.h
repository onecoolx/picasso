
#ifndef AGG_RENDERER_MCLIP_INCLUDED
#define AGG_RENDERER_MCLIP_INCLUDED

#include "agg_basics.h"
#include "agg_renderer_base.h"
#include "agg_path_storage.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_bounding_rect.h"

namespace agg
{

    //----------------------------------------------------------renderer_pclip
    template<class PixelFormat> class renderer_pclip
    {
    public:
        typedef PixelFormat pixfmt_type;
        typedef typename pixfmt_type::color_type color_type;
        typedef typename pixfmt_type::row_data row_data;
        typedef renderer_base<pixfmt_type> base_ren_type;

        //--------------------------------------------------------------------
		renderer_pclip() : m_path_clip(false), m_bounds(1, 1, 0, 0) {}
        explicit renderer_pclip(pixfmt_type& pixf) :
            m_ren(pixf),
			m_path_clip(false),
            m_bounds(m_ren.xmin(), m_ren.ymin(), m_ren.xmax(), m_ren.ymax())
        {}

		~renderer_pclip()
		{
			m_path_test.reset_clipping();
		}

        void attach(pixfmt_type& pixf)
        {
            m_ren.attach(pixf);
            reset_clipping(true);
        }
          
        //--------------------------------------------------------------------
        const pixfmt_type& ren() const { return m_ren.ren();  }
        pixfmt_type& ren() { return m_ren.ren();  }

        //--------------------------------------------------------------------
        unsigned width()  const { return m_ren.width();  }
        unsigned height() const { return m_ren.height(); }

        //--------------------------------------------------------------------
        const rect_i& clip_box() const { return m_ren.clip_box(); }
        int           xmin()     const { return m_ren.xmin(); }
        int           ymin()     const { return m_ren.ymin(); }
        int           xmax()     const { return m_ren.xmax(); }
        int           ymax()     const { return m_ren.ymax(); }

        //--------------------------------------------------------------------
        const rect_i& bounding_clip_box() const { return m_bounds;    }
        int           bounding_xmin()     const { return m_bounds.x1; }
        int           bounding_ymin()     const { return m_bounds.y1; }
        int           bounding_xmax()     const { return m_bounds.x2; }
        int           bounding_ymax()     const { return m_bounds.y2; }

        //--------------------------------------------------------------------
        void reset_clipping(bool visibility)
        {
            m_ren.reset_clipping(visibility);
			m_path_test.reset_clipping();
			m_path_clip = false;
            m_bounds = m_ren.clip_box();
        }

		bool inbounds(int x, int y) const
		{
            return x >= m_bounds.x1 && y >= m_bounds.y1 &&
                   x <= m_bounds.x2 && y <= m_bounds.y2;
		}

		bool hit_test(int x, int y) 
		{
			if (!inbounds(x, y))
				return false;

			if (!m_path_test.hit_test(x, y))
				return false;

			return true;
		}
        
        //--------------------------------------------------------------------
        void clear(const color_type& c)
        {
			if (m_path_clip) {
				for (unsigned i=0; i<height(); i++)
					for (unsigned j=0; j<width(); j++)
						if (hit_test(j, i))
							m_ren.copy_pixel(j, i, c);
			} else {
            	m_ren.clear(c);
			}
        }
          
        //--------------------------------------------------------------------
        void copy_pixel(int x, int y, const color_type& c)
        {
			if (m_path_clip) {
				if (hit_test(x, y))	
					m_ren.copy_pixel(x, y, c);
			} else {
				m_ren.copy_pixel(x, y, c);
			}
		}

        //--------------------------------------------------------------------
        void blend_pixel(int x, int y, const color_type& c, cover_type cover)
        {
			if (m_path_clip) {
				if (hit_test(x, y))	
					m_ren.blend_pixel(x, y, c, cover);
			} else {
				m_ren.blend_pixel(x, y, c, cover);
			}
		}

        //--------------------------------------------------------------------
        color_type pixel(int x, int y) const
        {
			if (m_path_clip) {
				if (hit_test(x, y))	
					return m_ren.pixel(x, y);
				return color_type::no_color();
			} else {
				return m_ren.pixel(x, y);
			}
		}

        //--------------------------------------------------------------------
        void copy_hline(int x1, int y, int x2, const color_type& c)
        {
			if (m_path_clip) {
				for (int i=0; i<abs(x2-x1)+1; i++)
					if (hit_test(x1+i, y))
						m_ren.copy_pixel(x1+i, y, c);
			} else {
				m_ren.copy_hline(x1, y, x2, c);
			}
        }

        //--------------------------------------------------------------------
		void copy_vline(int x, int y1, int y2, const color_type& c)
		{
			if (m_path_clip) {
				for (int i=0; i<abs(y2-y1)+1; i++)
					if (hit_test(x, y1+i))
						m_ren.copy_pixel(x, y1+i, c);
			} else {
				m_ren.copy_vline(x, y1, y2, c);
			}
		}

		//--------------------------------------------------------------------
		void blend_hline(int x1, int y, int x2, 
				const color_type& c, cover_type cover)
		{
			if (m_path_clip) {
				for (int i=0; i<abs(x2-x1)+1; i++)
					if (hit_test(x1+i, y))
						m_ren.blend_pixel(x1+i, y, c, cover);
			} else {
				m_ren.blend_hline(x1, y, x2, c, cover);
			}
		}

		//--------------------------------------------------------------------
		void blend_vline(int x, int y1, int y2, 
				const color_type& c, cover_type cover)
		{
			if (m_path_clip) {
				for (int i=0; i<abs(y2-y1)+1; i++)
					if (hit_test(x, y1+i))
						m_ren.blend_pixel(x, y1+i, c, cover);
			} else {
				m_ren.blend_vline(x, y1, y2, c, cover);
			}
		}

		//--------------------------------------------------------------------
		void copy_bar(int x1, int y1, int x2, int y2, const color_type& c)
		{
			if (m_path_clip) {
				for (int i=0; i<abs(y2-y1); i++)
					for (int j=0; j<abs(x2-x1); j++)
						if (hit_test(x1+j, y1+i))
							m_ren.copy_pixel(x1+j, y1+i, c);
			} else {
				m_ren.copy_bar(x1, y1, x2, y2, c);
			}
		}

		//--------------------------------------------------------------------
		void blend_bar(int x1, int y1, int x2, int y2, 
				const color_type& c, cover_type cover)
		{
			if (m_path_clip) {
				for (int i=0; i<abs(y2-y1); i++)
					for (int j=0; j<abs(x2-x1); j++)
						if (hit_test(x1+j, y1+i))
							m_ren.blend_pixel(x1+j, y1+i, c, cover);
			} else {
				m_ren.blend_bar(x1, y1, x2, y2, c, cover);
			}
		}

		//--------------------------------------------------------------------
		void blend_solid_hspan(int x, int y, int len, 
				const color_type& c, const cover_type* covers)
		{
			if (m_path_clip) {
				for (int i=0; i<len; i++)
					if (hit_test(x+i, y))
						m_ren.blend_pixel(x+i, y, c, covers[i]);
			} else {
				m_ren.blend_solid_hspan(x, y, len, c, covers);
			}
		}

		//--------------------------------------------------------------------
		void blend_solid_vspan(int x, int y, int len, 
				const color_type& c, const cover_type* covers)
		{
			if (m_path_clip) {
				for (int i=0; i<len; i++)
					if (hit_test(x, y+i))
						m_ren.blend_pixel(x, y+i, c, covers[i]);
			} else {
				m_ren.blend_solid_vspan(x, y, len, c, covers);
			}
		}

		//--------------------------------------------------------------------
        void copy_color_vspan(int x, int y, int len, const color_type* colors)
		{
			if (m_path_clip) {
				for (int i=0; i<len; i++)
					if (hit_test(x, y+i))
						m_ren.copy_pixel(x, y+i, colors[i]);
			} else {
				m_ren.copy_color_vspan(x, y, len, colors);
			}
		}

		//--------------------------------------------------------------------
		void copy_color_hspan(int x, int y, int len, const color_type* colors)
		{
			if (m_path_clip) {
				for (int i=0; i<len; i++)
					if (hit_test(x+i, y))
						m_ren.copy_pixel(x+i, y, colors[i]);
			} else {
				m_ren.copy_color_hspan(x, y, len, colors);
			}
		}

		//--------------------------------------------------------------------
		void blend_color_hspan(int x, int y, int len, 
				const color_type* colors, 
				const cover_type* covers,
				cover_type cover = cover_full)
		{
			if (m_path_clip) {
				for (int i=0; i<len; i++)
					if (hit_test(x+i, y))
						m_ren.blend_pixel(x+i, y, colors[i], covers[i]);
			} else {
				m_ren.blend_color_hspan(x, y, len, colors, covers, cover);
			}
		}

		//--------------------------------------------------------------------
		void blend_color_vspan(int x, int y, int len, 
				const color_type* colors, 
				const cover_type* covers,
				cover_type cover = cover_full)
		{
			if (m_path_clip) {
				for (int i=0; i<len; i++)
					if (hit_test(x, y+i))
						m_ren.blend_pixel(x, y+i, colors[i], covers[i]);
			} else {
				m_ren.blend_color_vspan(x, y, len, colors, covers, cover);
			}
		}

		//--------------------------------------------------------------------
		void copy_absolute_from(const rendering_buffer& from, 
				const rect_i* rect_src_ptr=0, 
				int dx=0, 
				int dy=0)
		{
			if (m_path_clip) {
				rect_i rsrc(0, 0, from.width(), from.height());
				if(rect_src_ptr)
				{
					rsrc.x1 = rect_src_ptr->x1; 
					rsrc.y1 = rect_src_ptr->y1;
					rsrc.x2 = rect_src_ptr->x2 + 1;
					rsrc.y2 = rect_src_ptr->y2 + 1;
				}

				rect_i rdst(dx, dy, rsrc.x2-rsrc.x1+dx, rsrc.y2-rsrc.y1+dy);
				rect_i rc = m_ren.clip_rect_area(rdst, rsrc, from.width(), from.height());

				if (rc.x2 > 0 && rc.y2 > 0) 
				{
					int incy = 1;
					int incx = 1;

					if(rdst.y1 > rsrc.y1)
					{
						rsrc.y1 += rc.y2 - 1;
						rdst.y1 += rc.y2 - 1;
						incy = -1;
					}

					if(rdst.x1 > rsrc.x1)
					{
						rsrc.x1 += rc.x2 - 1;
						rdst.x1 += rc.x2 - 1;
						incx = -1;
					}

					while(rc.x2 > 0)
					{
						while(rc.y2 > 0)
						{
							if (hit_test(rdst.x1, rdst.y1))
								m_ren.ren().copy_point_from(from, rdst.x1, rdst.y1, rsrc.x1, rsrc.y1);
							rdst.y1 += incy;
							rsrc.y1 += incy;
							--rc.y2;
						}
						rdst.x1 += incx;
						rsrc.x1 += incx;
						--rc.x2;
					}
				}
			} else {
				m_ren.copy_absolute_from(from, rect_src_ptr, dx, dy);
			}
		}

		//--------------------------------------------------------------------
		void copy_from(const rendering_buffer& from, 
				const rect_i* rect_src_ptr=0, 
				int dx=0, 
				int dy=0)
		{
			if (m_path_clip) {
				rect_i rsrc(0, 0, from.width(), from.height());
				if(rect_src_ptr)
				{
					rsrc.x1 = rect_src_ptr->x1; 
					rsrc.y1 = rect_src_ptr->y1;
					rsrc.x2 = rect_src_ptr->x2 + 1;
					rsrc.y2 = rect_src_ptr->y2 + 1;
				}

				rect_i rdst(rsrc.x1 + dx, rsrc.y1 + dy, rsrc.x2 + dx, rsrc.y2 + dy);
				rect_i rc = m_ren.clip_rect_area(rdst, rsrc, from.width(), from.height());

				if (rc.x2 > 0 && rc.y2 > 0) 
				{
					int incy = 1;
					int incx = 1;

					if(rdst.y1 > rsrc.y1)
					{
						rsrc.y1 += rc.y2 - 1;
						rdst.y1 += rc.y2 - 1;
						incy = -1;
					}

					if(rdst.x1 > rsrc.x1)
					{
						rsrc.x1 += rc.x2 - 1;
						rdst.x1 += rc.x2 - 1;
						incx = -1;
					}

					while(rc.x2 > 0)
					{
						while(rc.y2 > 0)
						{
							if (hit_test(rdst.x1, rdst.y1))
								m_ren.ren().copy_point_from(from, rdst.x1, rdst.y1, rsrc.x1, rsrc.y1);
							rdst.y1 += incy;
							rsrc.y1 += incy;
							--rc.y2;
						}
						rdst.x1 += incx;
						rsrc.x1 += incx;
						--rc.x2;
					}
				}
			} else {
				m_ren.copy_from(from, rect_src_ptr, dx, dy);
			}
		}

        //--------------------------------------------------------------------
        template<class SrcPixelFormatRenderer>
        void blend_from(const SrcPixelFormatRenderer& src, 
                        const rect_i* rect_src_ptr = 0, 
                        int dx = 0, 
                        int dy = 0,
                        cover_type cover = cover_full)
        {
			if (m_path_clip) {
				rect_i rsrc(0, 0, src.width(), src.height());
				if(rect_src_ptr)
				{
					rsrc.x1 = rect_src_ptr->x1; 
					rsrc.y1 = rect_src_ptr->y1;
					rsrc.x2 = rect_src_ptr->x2 + 1;
					rsrc.y2 = rect_src_ptr->y2 + 1;
				}

				rect_i rdst(rsrc.x1 + dx, rsrc.y1 + dy, rsrc.x2 + dx, rsrc.y2 + dy);
				rect_i rc = m_ren.clip_rect_area(rdst, rsrc, src.width(), src.height());

				if(rc.x2 > 0 && rc.y2 > 0)
				{
					int incy = 1;
					int incx = 1;

					if(rdst.y1 > rsrc.y1)
					{
						rsrc.y1 += rc.y2 - 1;
						rdst.y1 += rc.y2 - 1;
						incy = -1;
					}

					if(rdst.x1 > rsrc.x1)
					{
						rsrc.x1 += rc.x2 - 1;
						rdst.x1 += rc.x2 - 1;
						incx = -1;
					}

					while(rc.x2 > 0)
					{
						while(rc.y2 > 0)
						{
							if (hit_test(rdst.x1, rdst.y1))
								m_ren.ren().blend_point_from(src, rdst.x1, rdst.y1, rsrc.x1, rsrc.y1, cover);
							rdst.y1 += incy;
							rsrc.y1 += incy;
							--rc.y2;
						}
						rdst.x1 += incx;
						rsrc.x1 += incx;
						--rc.x2;
					}
				}
			} else {
				m_ren.blend_from(src, rect_src_ptr, dx, dy, cover);
			}
        }
        bool clip_box(int x1, int y1, int x2, int y2)
		{
			bool r = m_ren.clip_box(x1, y1, x2, y2);
            m_bounds = m_ren.clip_box();
			return r;
		}
	
		template<class VertexSource>
		void add_clipping(VertexSource & p, filling_rule_e f)
		{
			m_path_clip = true;
			m_path_test.reset_clipping();
			m_path_test.filling_rule(f);
			m_path_test.add_path(p);

			rect_i cb;
			bounding_rect_single(p, 0, &cb.x1, &cb.y1, &cb.x2, &cb.y2);
			if(cb.x1 < m_bounds.x1) m_bounds.x1 = cb.x1;
			if(cb.y1 < m_bounds.y1) m_bounds.y1 = cb.y1;
			if(cb.x2 > m_bounds.x2) m_bounds.x2 = cb.x2;
			if(cb.y2 > m_bounds.y2) m_bounds.y2 = cb.y2;
		}
        
    private:
        renderer_pclip(const renderer_pclip<PixelFormat>&);
        const renderer_pclip<PixelFormat>& 
            operator = (const renderer_pclip<PixelFormat>&);

        base_ren_type          m_ren;
		rasterizer_scanline_aa<> m_path_test;
		bool 				   m_path_clip;
        rect_i                 m_bounds;
    };

}

#endif
