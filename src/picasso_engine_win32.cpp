/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "pconfig.h"
#include "picasso_utils.h"
#if defined(WIN32) && !ENABLE(FREE_TYPE2)
#include <windows.h>

#include "picasso.h"
#include "picasso_p.h"

namespace picasso {

font_engine::font_engine()
	: m_engine(new font_engine_win32_tt_int32(::GetDC(0)))
{
}


font_engine::~font_engine()
{
	m_engine->release_dc0(); // free GetDC(0)'s handle
	delete m_engine;
}

void font_engine::resolution(unsigned d)
{
	m_engine->resolution(d);
}

unsigned font_engine::resolution() const
{
	return m_engine->resolution();
}

void font_engine::weight(int w)
{
	m_engine->weight(w);
}	

int font_engine::weight() const
{
	return m_engine->weight();
}	

void font_engine::hinting(bool h)
{
	m_engine->hinting(h);
}

bool font_engine::hinting() const
{
	return m_engine->hinting();
}

void font_engine::height(float h)
{
	m_engine->height(h);
}

float font_engine::height() const
{
	return m_engine->height();
}

void font_engine::width(float w)
{
	m_engine->width(w);
}

float font_engine::width() const
{
	return m_engine->width();
}

void font_engine::italic(bool b) 
{
	m_engine->italic(b);
}

bool font_engine::italic() const 
{
	return m_engine->italic();
}

void font_engine::flip_y(bool b) 
{
	m_engine->flip_y(b);
}

bool font_engine::flip_y() const  
{
	return m_engine->flip_y();
}

void font_engine::set_antialias(bool b)
{
	m_engine->set_antialias(b);
}

bool font_engine::antialias(void)
{
	return m_engine->antialias();
}

bool font_engine::create_font(const char* face, glyph_rendering g)
{
	return m_engine->create_font(face, g);
}

void font_engine::transform(const trans_affine& mtx)
{
	m_engine->transform(mtx);
}

bool font_engine::prepare_glyph(unsigned g)
{
	return m_engine->prepare_glyph(g);
}

unsigned font_engine::glyph_index() const
{
	return m_engine->glyph_index();
}

unsigned font_engine::data_size() const
{
	return m_engine->data_size();
}

glyph_data_type font_engine::data_type()
{
	return m_engine->data_type();
}

const rect_i& font_engine::bounds()
{
	return m_engine->bounds();
}

float font_engine::advance_x() const
{
	return m_engine->advance_x();
}

float font_engine::advance_y() const
{
	return m_engine->advance_y();
}

float font_engine::ascent() const
{
	return m_engine->ascent();
}

float font_engine::descent() const
{
	return m_engine->descent();
}

float font_engine::leading() const
{
	return m_engine->leading();
}

unsigned font_engine::units_per_em() const
{
	return m_engine->units_per_em();
}

void font_engine::write_glyph_to(int8u* data) const
{
	m_engine->write_glyph_to(data);
}

bool font_engine::add_kerning(unsigned f, unsigned t, float* x, float* y)
{
	return m_engine->add_kerning(f, t, x, y);
}

const char* font_engine::font_signature()
{
	return m_engine->font_signature();
}

const char* font_engine::name() const
{
	return m_engine->typeface();
}

int font_engine::change_stamp() const
{
	return m_engine->change_stamp();
}

void font_engine::char_set(ps_charset s)
{
	switch (s) {
		case CHARSET_ANSI:
			m_engine->char_set(ANSI_CHARSET);
			break;
		case CHARSET_UNICODE:
			m_engine->char_set(DEFAULT_CHARSET);
			break;
	}
}

ps_charset font_engine::char_set() const
{
	switch (m_engine->char_set()) {
		case ANSI_CHARSET:
		  	return CHARSET_ANSI;
		case DEFAULT_CHARSET:
			return CHARSET_UNICODE;
		default:
			return CHARSET_ANSI;
	}
}

}

#endif /* WIN32 */
