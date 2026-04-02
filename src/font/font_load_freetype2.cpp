/*
 * Copyright (c) 2024, Zhang Ji Peng
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common.h"
#include "picasso_private.h"

#if ENABLE(FREE_TYPE2)
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <string.h>

#if defined(WIN32)
    #define strncasecmp _strnicmp
#endif

#if defined(__ANDROID__)
    #include <expat.h>
#endif

#define MAX_PATH_LEN 512

#define MAX_FONT_PATH_LENGTH MAX_PATH_LEN
#define MAX_FONT_NAME_LENGTH MAX_PATH_LEN

#define MAX_CONFIG_LINE    MAX_PATH_LEN

#if ENABLE(FONT_CONFIG)
    #include <fontconfig/fontconfig.h>
#else //not fontconfig
    #define F(txt)   txt
    #define OPENFILE   fopen
    #define CONFIG_FILE "font_config.cfg"
    #define CONFIG_PATH(path) path
#endif //FONT_CONFIG

namespace picasso {

struct font_item {
    char font_name[MAX_FONT_NAME_LENGTH];
    char font_path[MAX_FONT_PATH_LENGTH];
};

/*
 * Note : _load_fonts function init the global font map at initialize time.
 * it only be call once and not need thread lock.
 */
typedef picasso::pod_bvector<font_item*> font_map;

static font_map g_font_map;

static font_item* get_font_item(const char* name, const char* path)
{
    font_item* f = (font_item*)mem_calloc(1, sizeof(font_item));
    if (f) {
        strncpy(f->font_name, name, MAX_FONT_NAME_LENGTH - 1);
        strncpy(f->font_path, path, MAX_FONT_PATH_LENGTH - 1);
        return f;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

#if ENABLE(FONT_CONFIG)
static FcConfig* g_FcConfig = NULL;

static void load_font_from_fontconfig(void)
{
    g_FcConfig = FcInitLoadConfigAndFonts();
    // load default font
    FcFontSet* fontset = NULL;
    // get first application font as default
    fontset = FcConfigGetFonts(g_FcConfig, FcSetApplication);
    if (fontset) {
        FcValue fvalue, dvalue;
        if (FcResultMatch == FcPatternGet(fontset->fonts[0], FC_FAMILY, 0, &fvalue)) {
            if (FcResultMatch == FcPatternGet(fontset->fonts[0], FC_FILE, 0, &dvalue)) {
                font_item* font = get_font_item((const char*)fvalue.u.s, (const char*)dvalue.u.s);
                g_font_map.add(font);
            }
        }
    }

    // get first system font as default
    fontset = FcConfigGetFonts(g_FcConfig, FcSetSystem);
    if (fontset) {
        FcValue fvalue, dvalue;
        if (FcResultMatch == FcPatternGet(fontset->fonts[0], FC_FAMILY, 0, &fvalue)) {
            if (FcResultMatch == FcPatternGet(fontset->fonts[0], FC_FILE, 0, &dvalue)) {
                font_item* font = get_font_item((const char*)fvalue.u.s, (const char*)dvalue.u.s);
                g_font_map.add(font);
            }
        }
    }
}
#elif defined(__ANDROID__)
// this is only work on android version <= 4.4 not support >= 5.0

#define NO_TAG 0
#define NAMESET_TAG 1
#define FILESET_TAG 2

struct ParserContext {
    ParserContext(XML_Parser* p, font_map* m)
        : parser(p), map(m), remaining_names(0), current_tag(NO_TAG)
    {
    }

    XML_Parser* parser; // The expat parser doing the work
    font_map* map;
    int32_t remaining_names;
    int32_t current_tag;
};

static void text_callback(void* data, const char* s, int32_t len)
{
    ParserContext* context = (ParserContext*)data;

    if (context->current_tag == NAMESET_TAG || context->current_tag == FILESET_TAG) {
        switch (context->current_tag) {
            case NAMESET_TAG: {
                    // add a new font item.
                    font_item* f = (font_item*)mem_calloc(1, sizeof(font_item));
                    strncpy(f->font_name, s, Min(len, MAX_FONT_NAME_LENGTH - 1));
                    context->map->add(f);
                    context->remaining_names++;
                }
                break;
            case FILESET_TAG: {
                    for (int32_t i = 0; i < context->remaining_names; i++) {
                        char* path = context->map->at(context->map->size() - i - 1)->font_path;
                        char buffer[MAX_FONT_PATH_LENGTH] = {0};
                        strncpy(buffer, s, Min(len, MAX_FONT_PATH_LENGTH - 1));
                        snprintf(path, MAX_FONT_PATH_LENGTH - 1, "/system/fonts/%s", buffer);
                    }
                    context->remaining_names = 0;
                }
                break;
            default:
                // do nothing.
                break;
        }
    }
}

static void start_callback(void* data, const char* tag, const char** atts)
{
    ParserContext* context = (ParserContext*)data;

    int32_t len = strlen(tag);
    if (strncmp(tag, "family", len) == 0) {
        context->current_tag = NO_TAG;
        context->remaining_names = 0;
    } else if (len == 7 && strncmp(tag, "nameset", len) == 0) {
        context->current_tag = NAMESET_TAG;
        context->remaining_names = 0;
    } else if (len == 7 && strncmp(tag, "fileset", len) == 0) {
        context->current_tag = FILESET_TAG;
    } else if (strncmp(tag, "name", len) == 0 && context->current_tag == NAMESET_TAG) {
        // add a new font_item to map and set name.
        XML_SetCharacterDataHandler(*context->parser, text_callback);
    } else if (strncmp(tag, "file", len) == 0 && context->current_tag == FILESET_TAG) {
        // set the font file to all remaining items.
        // FIXME: it is only first font file will be set for names.
        if (context->remaining_names > 0) {
            XML_SetCharacterDataHandler(*context->parser, text_callback);
        }
    }
}

static void end_callback(void* data, const char* tag)
{
    ParserContext* context = (ParserContext*)data;
    int32_t len = strlen(tag);
    if (strncmp(tag, "family", len) == 0) {
        context->current_tag = NO_TAG;
        context->remaining_names = 0;
    } else if (len == 7 && strncmp(tag, "nameset", len) == 0) {
        context->current_tag = NO_TAG;
    } else if (len == 7 && strncmp(tag, "fileset", len) == 0) {
        context->current_tag = NO_TAG;
    } else if ((strncmp(tag, "name", len) == 0 && context->current_tag == NAMESET_TAG) ||
               (strncmp(tag, "file", len) == 0 && context->current_tag == FILESET_TAG)) {
        // Disable the arbitrary text handler installed to load Name data
        XML_SetCharacterDataHandler(*context->parser, NULL);
    }
}

static void parse_config_file(const char* file, font_map& map)
{
    FILE* fp = NULL;
    fp = fopen(file, "r");
    if (!fp) {
        return; // file not found.
    }

    XML_Parser parser = XML_ParserCreate(NULL);
    ParserContext* context = new ParserContext(&parser, &map);
    XML_SetUserData(parser, context);
    XML_SetElementHandler(parser, start_callback, end_callback);

    char buffer[512] = {0};
    bool done = false;
    while (!done) {
        fgets(buffer, sizeof(buffer), fp);
        int32_t len = strlen(buffer);
        if (feof(fp) != 0) {
            done = true;
        }
        XML_Parse(parser, buffer, len, done);
    }
    XML_ParserFree(parser);
    delete context;
    fclose(fp);
}

// font config files
#define SYSTEM_FONTS_FILE "/system/etc/system_fonts.xml"
#define FALLBACK_FONTS_FILE "/system/etc/fallback_fonts.xml"
#define VENDOR_FONTS_FILE "/vendor/etc/fallback_fonts.xml"

static void load_font_from_android(void)
{
    parse_config_file(SYSTEM_FONTS_FILE, g_font_map);
    parse_config_file(FALLBACK_FONTS_FILE, g_font_map);
    parse_config_file(VENDOR_FONTS_FILE, g_font_map);
}
#else
static void write_default(void)
{
    FILE* pf = 0;

    if ((pf = OPENFILE(CONFIG_FILE, F("a+")))) {

        fprintf(pf, "[%s]\n", "default");
        fprintf(pf, "path=%s\n", "ZCOOLXiaoWei-Regular.ttf");

        /* Note: add more default fonts like this
         * fprintf(pf, "[%s]\n", "arial");
         * fprintf(pf, "path=%s\n", "arial.ttf");
         */

        fclose(pf);
    }
}

static void load_font_from_file(FILE* f)
{
    char buf[MAX_CONFIG_LINE + 1];
    char* tname = 0;
    char* tpath = 0;
    while (!feof(f)) {
        (void)fgets(buf, MAX_CONFIG_LINE, f);
        char* ps = strchr(buf, '[');
        if (ps) {
            char* pe = strchr(buf, ']');
            if (pe) {
                tname = (char*)mem_malloc(pe - ps);
                strncpy(tname, ps + 1, pe - ps - 1);
                tname[pe - ps - 1] = '\0';

                (void)fgets(buf, MAX_CONFIG_LINE, f);
                ps = strstr(buf, "path=");
                if (ps) {
                    char* pe = strchr(buf, '\n');
                    if (pe) {
                        tpath = (char*)mem_malloc(pe - ps);
                        strncpy(tpath, ps + 5, pe - ps - 5);
                        tpath[pe - ps - 5] = '\0';

                        font_item* font = get_font_item(tname, CONFIG_PATH(tpath));
                        g_font_map.add(font);

                        mem_free(tpath);
                    }
                }
                mem_free(tname);
            }
        }
    }
}
#endif //ENABLE(FONT_CONFIG)

static FT_Library g_library = NULL;

FT_Library _get_ft_library(void)
{
    return g_library;
}

bool _load_fonts(void)
{
    if (g_library) {
        return true;
    }

#if ENABLE(FONT_CONFIG)
    FcInit();
#endif
#if ENABLE(FONT_CONFIG)
    load_font_from_fontconfig();
#elif defined(__ANDROID__)
    load_font_from_android();
#else
    FILE* pf = 0;

    if ((pf = OPENFILE(CONFIG_FILE, F("r")))) {

        load_font_from_file(pf);

        fclose(pf);

    } else {
        // not found config file.
        write_default();

        pf = OPENFILE(CONFIG_FILE, F("r"));
        load_font_from_file(pf);
        fclose(pf);
    }
#endif

    if (!g_font_map.size()) {

        font_item* ansi_font = get_font_item("arial", "arial.ttf");
        font_item* uni_font = get_font_item("default", "default.ttf");

        g_font_map.add(uni_font);
        g_font_map.add(ansi_font);
    }

    if (FT_Init_FreeType(&g_library) == 0) {
        return true;
    } else {
        return false;
    }
}

void _free_fonts(void)
{
    if (g_library) {
        FT_Done_FreeType(g_library);
        g_library = NULL;
    }

    for (uint32_t i = 0; i < g_font_map.size(); i++) {
        mem_free(g_font_map[i]);
    }

    g_font_map.remove_all();
}

#if ENABLE(FONT_CONFIG)
char* _font_by_name(const char* face, float size, float weight, bool italic)
{
    // find from cache
    char tname[68] = {0};
    strncpy(tname, face, 64);
    char font_key[128] = {0};
    sprintf(font_key, "%s-%4.0f-%4.0f-%d", tname, size, weight, italic ? 1 : 0);

    for (uint32_t i = 0; i < g_font_map.size(); i++) {
        if (strncmp(font_key, g_font_map[i]->font_name, MAX_FONT_NAME_LENGTH - 1) == 0) {
            return g_font_map[i]->font_path;
        }
    }

    FcPattern* pattern = FcPatternCreate();
    FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)face);
    if (italic) {
        FcPatternAddInteger(pattern, FC_SLANT, FC_SLANT_ITALIC);
    } else {
        FcPatternAddInteger(pattern, FC_SLANT, FC_SLANT_ROMAN);
    }

    if ((int32_t)round(weight) == 400) {
        FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_REGULAR);
    } else if ((int32_t)round(weight) == 500) {
        FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_DEMIBOLD);
    } else if ((int32_t)round(weight) == 700) {
        FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_BOLD);
    } else if ((int32_t)round(weight) == 900) {
        FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_BLACK);
    } else {
        FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_REGULAR);
    }
    FcPatternAddDouble(pattern, FC_PIXEL_SIZE, size);

    FcConfigSubstitute(g_FcConfig, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result;
    font_item* font = 0;
    FcPattern* p = FcFontMatch(g_FcConfig, pattern, &result);
    if (p) {
        FcChar8* filePath = 0;
        if (FcPatternGetString(p, FC_FILE, 0, &filePath) != FcResultMatch) {
            FcPatternDestroy(p);
            FcPatternDestroy(pattern);
            return g_font_map[0]->font_path;
        }
        font = get_font_item(font_key, (const char*)filePath);
        g_font_map.add(font);
    }
    FcPatternDestroy(p);
    FcPatternDestroy(pattern);
    return font ? font->font_path : g_font_map[0]->font_path;
}
#else
char* _font_by_name(const char* face, float size, float weight, bool italic)
{
    for (uint32_t i = 0; i < g_font_map.size(); i++)
        if (strncasecmp(face, g_font_map[i]->font_name, MAX_FONT_NAME_LENGTH - 1) == 0) {
            return g_font_map[i]->font_path;
        }

    return g_font_map[0]->font_path;
}
#endif

} // namespace picasso

bool platform_font_init(void)
{
    return picasso::_load_fonts();
}

void platform_font_shutdown(void)
{
    picasso::_free_fonts();
#if ENABLE(FONT_CONFIG)
    if (picasso::g_FcConfig) {
        FcConfigDestroy(picasso::g_FcConfig);
        picasso::g_FcConfig = NULL;
        FcFini();
    }
#endif
}

#endif /* FREE_TYPE2 */
