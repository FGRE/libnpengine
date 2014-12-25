%{
    #include "TextParser.hpp"
    #include <cstdio>
    #include <cstdlib>

    extern TextParser::Text* pText;
    extern int yylex();
    void yyerror(const char* s) { std::printf("Error: %s\n", s); std::abort(); }
%}

%union
{
    TextParser::StringSegment* strseg;
    TextParser::Text* text;
    TextParser::Line* line;
    TextParser::Voice* voice;
    int token;
    std::vector<std::string>* vec;
    std::string* string;
}

%define api.prefix xml

%token <string> TSTRING THEX
%token <token> TPRE TFONT TRUBY TVOICE TLBRACE TRBRACE TLABRACE TRABRACE TQUOTE TEQUAL TSLASH TAT TNEWLINE

%type <text> start text
%type <line> line string
%type <strseg> strseg
%type <vec> args voice
%type <string> arg hexarg

%start start

%%

start : TLBRACE TPRE TAT TSTRING TRBRACE TLABRACE TSTRING TRABRACE text TLBRACE TSLASH TPRE TRBRACE { delete $4; delete $7; }
      ;

text : line { $$ = pText; $$->Lines.push_back(*$1); delete $1; }
     | text line { $1->Lines.push_back(*$2); delete $2; }
     ;

line : string { $$ = $1; }
     | voice string { $$ = $2; $2->VoiceAttrs = *$1; delete $1; }
     ;

string : strseg TNEWLINE { $$ = new TextParser::Line; $$->StringSegs.push_back(*$1); delete $1; }
       | strseg string { $$ = $2; $2->StringSegs.push_back(*$1); delete $1; }
       ;

strseg : TSTRING { $$ = new TextParser::StringSegment; $$->Segment = *$1; delete $1; }
       | TLBRACE TRUBY arg TRBRACE strseg TLBRACE TSLASH TRUBY TRBRACE { $$ = $5; $5->Ruby = *$3; delete $3; }
       | TLBRACE TFONT hexarg hexarg TRBRACE strseg TLBRACE TSLASH TFONT TRBRACE { $$ = $6; $6->InColor = *$3; $6->OutColor = *$4; delete $3; delete $4; }
       ;

voice : TLBRACE TVOICE args TRBRACE { $$ = $3; }
      ;

arg : TSTRING TEQUAL TQUOTE TSTRING TQUOTE { $$ = $4; }
    ;

hexarg : TSTRING TEQUAL TQUOTE THEX TQUOTE { $$ = $4; }
       ;

args : arg { $$ = new TextParser::ArgumentList; $$->push_back(*$1); delete $1; }
     | args arg { $1->push_back(*$2); delete $2; }
     ;

%%
