%{
#include <parser.yy.hh>
#include <token.hh>

typedef yytokentype tt;

static int line_start = 1, col_start = 1, line_end = 1, col_end = 1;

inline static void set_token_info(tt type) {
    line_end = line_start;
    col_end = col_start + yyleng;

    yylval.token = new Token(
        type,
        Position(line_start, col_start),
        Position(line_end,   col_end  ),
        std::string(yytext)
    );

    if (type == tt::YYEOF) {
        line_start = col_start = line_end = col_end = 1;
        return;
    }

    line_start = line_end;
    col_start = col_end;
}

#define MAKE_TOKEN($type) \
    set_token_info(tt::$type); \
    return tt::$type;
%}

id            [a-zA-Z_][a-zA-Z0-9_]*
lbrace        \{
rbrace        \}
lparen        \(
rparen        \)
lbrack        \[
rbrack        \]
int           [0-9]+
str           "([^"]|\\")*"|'([^']|\\')*'

%%
policy         { MAKE_TOKEN(POLICY); }
terminate      { MAKE_TOKEN(TERMINATE); }
allow          { MAKE_TOKEN(ALLOW); }
deny|kill      { MAKE_TOKEN(KILL); }
trap           { MAKE_TOKEN(TRAP); }
error          { MAKE_TOKEN(ERROR); }
notify         { MAKE_TOKEN(NOTIFY); }
trace          { MAKE_TOKEN(TRACE); }
log            { MAKE_TOKEN(LOG); }

if    { MAKE_TOKEN(IF); }

{id}        { MAKE_TOKEN(IDENTIFIER); }
{int}       { MAKE_TOKEN(INTEGER); }
{str}       { MAKE_TOKEN(STRING); }

{rbrace}    { MAKE_TOKEN(RBRACE); }
{lbrace}    { MAKE_TOKEN(LBRACE); }
{rparen}    { MAKE_TOKEN(RPAREN); }
{lparen}    { MAKE_TOKEN(LPAREN); }
{rbrack}    { MAKE_TOKEN(RBRACK); }
{lbrack}    { MAKE_TOKEN(LBRACK); }
,           { MAKE_TOKEN(COMMA); }

\+          { MAKE_TOKEN(ADD); }
-           { MAKE_TOKEN(SUB); }
\*          { MAKE_TOKEN(MUL); }
\/          { MAKE_TOKEN(DIV); }
%           { MAKE_TOKEN(MOD); }
&           { MAKE_TOKEN(AND); }
\|          { MAKE_TOKEN(OR); }
\^          { MAKE_TOKEN(XOR); }
==          { MAKE_TOKEN(EQ); }
!=          { MAKE_TOKEN(NE); }
\<          { MAKE_TOKEN(LT); }
\>          { MAKE_TOKEN(GT); }
\<=         { MAKE_TOKEN(LE); }
\>=         { MAKE_TOKEN(GE); }
\<\<        { MAKE_TOKEN(LSH); }
\>\>        { MAKE_TOKEN(RSH); }
&&          { MAKE_TOKEN(LOGICAL_AND); }
\|\|        { MAKE_TOKEN(LOGICAL_OR); }
~           { MAKE_TOKEN(NOT); }
!           { MAKE_TOKEN(LOGICAL_NOT); }

\n          { line_start++; col_start = 1; }
[[:space:]] { col_start++; }

<<EOF>>     { MAKE_TOKEN(YYEOF); }

.           { MAKE_TOKEN(YYerror); }
%%
