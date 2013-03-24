/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "picasso_global.h"

#if ENABLE(FREE_TYPE2)
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <string.h>

#if defined(WINCE) || defined(WIN32)
#define strncasecmp _strnicmp
#endif

#define MAX_PATH_LEN 512

#define MAX_FONT_PATH_LENGTH MAX_PATH_LEN
#define MAX_FONT_NAME_LENGTH MAX_PATH_LEN

#define MAX_CONFIG_LINE	MAX_PATH_LEN 

#if defined(WINCE)
#include <windows.h>
#define CONFIG_FILE GetFilePath(L"font_config.cfg")
#if defined(LOAD_FONT_WITH_PATH)
#define CONFIG_PATH(path) path
#else
#define CONFIG_PATH(path) GetFontPath(path)
#endif
static TCHAR g_path[MAX_PATH_LEN];
static inline LPTSTR GetFilePath(LPTSTR file)
{
	TCHAR *p = 0;
	GetModuleFileName(NULL, g_path, MAX_PATH_LEN);
	p = wcsrchr(g_path, '\\');
	p++; *p = 0;
	lstrcat (g_path, file);
	return g_path;
}

static inline char* GetFontPath(const char* name)
{
	TCHAR *p = 0;
	static char p_path[MAX_PATH_LEN];
	GetModuleFileName(NULL, g_path, MAX_PATH_LEN);
	p = wcsrchr(g_path, '\\');
	p++; *p = 0;

	int len = ::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, g_path, -1, p_path, MAX_PATH_LEN, NULL, NULL);

	if ((len + strlen(name)) > (MAX_PATH_LEN-1)) 
		return 0;
	
	strcat(p_path, name);
	return p_path;
}
#define OPENFILE   _wfopen 
#define F(txt)	L##txt
#else
#define F(txt)   txt
#define OPENFILE   fopen 
#define CONFIG_FILE "font_config.cfg"
#define CONFIG_PATH(path) path
#endif

namespace gfx {

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
	font_item* f = (font_item*)calloc(1, sizeof(font_item));
	if (f) {
		strncpy(f->font_name, name, MAX_FONT_NAME_LENGTH-1);
		strncpy(f->font_path, path, MAX_FONT_PATH_LENGTH-1);
		return f;
	} else { 
        global_status = STATUS_OUT_OF_MEMORY;
		return 0;
	}
}

static void write_default(void)
{
	FILE* pf = 0;

	if ((pf = OPENFILE(CONFIG_FILE, F("a+")))) {

		fprintf(pf, "[%s]\n", "sung");
		fprintf(pf, "path=%s\n", "sung.ttf");
		fprintf(pf, "[%s]\n", "arial");
		fprintf(pf, "path=%s\n", "arial.ttf");
		
		fclose(pf);
	}
}

static void load_font_from_file(FILE * f)
{
	char buf[MAX_CONFIG_LINE+1];
	char* tname = 0;
	char* tpath = 0;
	while (!feof(f)) {
		(void)fgets(buf, MAX_CONFIG_LINE, f);
		char* ps = strchr(buf, '[');	
		if (ps) {
			char* pe = strchr(buf, ']');
			if (pe) {
				tname = (char*)mem_malloc(pe-ps);
				strncpy(tname, ps+1, pe-ps-1);
				tname[pe-ps-1] = '\0';

				(void)fgets(buf, MAX_CONFIG_LINE, f);
				ps = strstr(buf, "path=");
				if (ps) {
					char* pe = strchr(buf, '\n');
					if (pe) {
						tpath = (char*)mem_malloc(pe-ps);
						strncpy(tpath, ps+5, pe-ps-5);
						tpath[pe-ps-5] = '\0';
						
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

FT_Library g_library = 0;

bool _load_fonts(void)
{
	FILE *pf = 0;

	if ((pf = OPENFILE(CONFIG_FILE, F("r")))) {

		load_font_from_file(pf);

		fclose(pf); 

	} else {
		// not found config file.
		write_default();
	}

	if (!g_font_map.size()) {

		font_item* ansi_font = get_font_item("arial", "arial.ttf");
		font_item* uni_font = get_font_item("sung", "sung.ttf");

		g_font_map.add(uni_font);
		g_font_map.add(ansi_font);
	}

    if (FT_Init_FreeType(&g_library) == 0)
        return true;
    else
        return false;
}

void _free_fonts(void)
{
    if (g_library)
        FT_Done_FreeType(g_library);

	for (unsigned int i = 0; i < g_font_map.size(); i++)
		mem_free(g_font_map[i]);

	g_font_map.remove_all();
}

char * _font_by_name(const char* face)
{
	for (unsigned int i = 0; i < g_font_map.size(); i++)
		if (strncasecmp(face, g_font_map[i]->font_name, MAX_FONT_NAME_LENGTH-1) == 0)
			return g_font_map[i]->font_path;

	return g_font_map[0]->font_path;
}

}

bool platform_font_init(void)
{
    return gfx::_load_fonts();
}

void platform_font_shutdown(void)
{
    gfx::_free_fonts();
}

#endif /* FREE_TYPE2 */
