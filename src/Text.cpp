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
