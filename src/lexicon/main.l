%{
#include <iostream>
#include <parser.yy.hh>
#include <lexicon/token.hh>
#include <lexicon/context.hh>

using tt = yytokentype;

#define SET_TOKEN_DECL static void set_token_info(void * scanner, YYSTYPE * yylval, lexicon::Context* ctx, tt type)

SET_TOKEN_DECL;

#define MAKE_TOKEN($type) \
    set_token_info(yyscanner, yylval, ctx, tt::$type); \
    return tt::$type;
%}

%option reentrant

id            [a-zA-Z_][a-zA-Z0-9_]*
sc_param      :([0-9]+|{id})
lbrace        \{
rbrace        \}
lparen        \(
rparen        \)
lbrack        \[
rbrack        \]
int           [0-9]+
str           \"([^\"]|\\\")*\"|'([^']|\\')*'

%x COMM M_COM

%%
\/\/        { BEGIN(COMM); }
<COMM>\n    { BEGIN(INITIAL); ++ctx->line_start; ctx->col_start = 1; }
<COMM>.     { ++ctx->col_start; }
\/\*        { BEGIN(M_COM); }
<M_COM>\*\/ { BEGIN(INITIAL); ctx->col_start += 2; }
<M_COM>\n   { ++ctx->line_start; ctx->col_start = 1; }
<M_COM>.    { ++ctx->col_start; }

import      { MAKE_TOKEN(IMPORT); }
from        { MAKE_TOKEN(FROM); }
as          { MAKE_TOKEN(AS); }
policy      { MAKE_TOKEN(POLICY); }
terminate   { MAKE_TOKEN(TERMINATE); }
allow       { MAKE_TOKEN(ALLOW); }
deny|kill   { MAKE_TOKEN(KILL); }
trap        { MAKE_TOKEN(TRAP); }
error       { MAKE_TOKEN(ERROR); }
notify      { MAKE_TOKEN(NOTIFY); }
trace       { MAKE_TOKEN(TRACE); }
log         { MAKE_TOKEN(LOG); }
fn          { MAKE_TOKEN(FUNCTION); }
apply       { MAKE_TOKEN(APPLY); }

true        { MAKE_TOKEN(TRUE); }
false       { MAKE_TOKEN(FALSE); }
null        { MAKE_TOKEN(NIL); }

if          { MAKE_TOKEN(IF); }
or          { MAKE_TOKEN(POLICY_DECL_OR); }

{sc_param}  { MAKE_TOKEN(SYSCALL_PARAM); }
{id}        { MAKE_TOKEN(IDENTIFIER); }
{int}       { MAKE_TOKEN(INTEGER); }
{str}       { MAKE_TOKEN(STRING); }

-\>         { MAKE_TOKEN(ARROW); }
{rbrace}    { MAKE_TOKEN(RBRACE); }
{lbrace}    { MAKE_TOKEN(LBRACE); }
{rparen}    { MAKE_TOKEN(RPAREN); }
{lparen}    { MAKE_TOKEN(LPAREN); }
{rbrack}    { MAKE_TOKEN(RBRACK); }
{lbrack}    { MAKE_TOKEN(LBRACK); }
,           { MAKE_TOKEN(COMMA); }

\=          { MAKE_TOKEN(ASSIGN); }
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
\;          { MAKE_TOKEN(SEMI); }

\n          { ctx->line_start++; ctx->col_start = 1; }
[[:space:]] { ctx->col_start++; }

<<EOF>>     { MAKE_TOKEN(YYEOF); }

.           { MAKE_TOKEN(YYUNDEF); }
%%

SET_TOKEN_DECL {
    auto leng = yyget_leng(scanner);
    auto text = yyget_text(scanner);

    ctx->line_end = ctx->line_start;
    ctx->col_end = ctx->col_start + leng;

    yylval->token = ctx->last_token = new Token(
        type,
        Position(ctx->line_start, ctx->col_start),
        Position(ctx->line_end,   ctx->col_end  ),
        std::string(text)
    );

    if (type == tt::YYEOF) {
        ctx->line_start = ctx->col_start = ctx->line_end = ctx->col_end = 1;
        return;
    }

    ctx->line_start = ctx->line_end;
    ctx->col_start = ctx->col_end;
}
