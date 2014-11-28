/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014 Mislav Blažević <krofnica996@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */
#include "Text.hpp"
#define yyparse xmlparse
#define yy_scan_bytes xml_scan_bytes
#define yy_delete_buffer xml_delete_buffer
#include "flex.hpp"
#include <pango/pangocairo.h>

TextParser::Text* pText;

Text::Text(const string& XML)
{
    ::pText = this;
    YY_BUFFER_STATE buffer = yy_scan_bytes(XML.c_str(), XML.size());
    yyparse();
    yy_delete_buffer(buffer);
}

Text::~Text()
{
}

void Text::SetWrap(int32_t Width)
{
}

void Text::SetString(const string& String)
{
    cairo_surface_t* TempSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 0, 0);
    cairo_t* LayoutContext = cairo_create(TempSurface);
    cairo_surface_destroy(TempSurface);

    PangoLayout* Layout = pango_cairo_create_layout(LayoutContext);
    pango_layout_set_width(Layout, 760 * PANGO_SCALE);
    pango_layout_set_wrap(Layout, PANGO_WRAP_WORD_CHAR);
    pango_layout_set_text(Layout, String.c_str(), -1);

    PangoFontDescription* Desc = pango_font_description_from_string("Sans 18");
    pango_font_description_set_weight(Desc, PANGO_WEIGHT_MEDIUM);
    pango_layout_set_font_description(Layout, Desc);
    pango_font_description_free(Desc);

    pango_layout_get_pixel_size(Layout, &Width, &Height);
    uint8_t* pData = (uint8_t*)calloc(4 * Width * Height, sizeof(uint8_t));
    cairo_surface_t* Surface = cairo_image_surface_create_for_data(pData, CAIRO_FORMAT_ARGB32, Width, Height, 4 * Width);
    cairo_t* RenderContext = cairo_create(Surface);

    cairo_set_source_rgba(RenderContext, 1, 1, 1, 1.0);
    pango_cairo_show_layout(RenderContext, Layout);
    Create(pData, GL_BGRA);

    free(pData);
    g_object_unref(Layout);
    cairo_destroy(LayoutContext);
    cairo_destroy(RenderContext);
    cairo_surface_destroy(Surface);
}
