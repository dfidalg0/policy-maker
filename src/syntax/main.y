%require "3.7"

%code requires {
    #include <string>
    #include <sstream>
    #include <vector>
    #include <cstdio>
    #include <memory>
    #include <syntax/nodes.hh>
    #include <lexicon/token.hh>
    #include <lexicon/context.hh>
    #include <stdexcept>
    #include <sstream>
    #include <errors.hh>

    struct ReturnValue {
        syntax::Program * program;
        std::stringstream error;

        ReturnValue(): program(nullptr) {}
    };

    static syntax::Program * program;

    namespace syntax {
        std::unique_ptr<syntax::Program> parse(std::string filename);
    }

    inline syntax::BinaryExpr * bin_expr(syntax::Expr * left, syntax::Expr * right, Token * op);

    #define YYERROR_DECL void \
    yyerror(void * scanner, lexicon::Context * ctx, std::string filename, ReturnValue * ret, char const * err)

    YYERROR_DECL;
}

%code provides {
    #define YY_DECL int yylex(YYSTYPE * yylval, void * yyscanner, lexicon::Context * ctx)

    extern "C" YY_DECL;
    extern "C" int yylex_init(void **);
    extern "C" int yylex_destroy(void *);
    extern "C" void yyset_in(FILE *, void *);
}

%code top {
    #include <filesystem>
    namespace fs = std::filesystem;
}

%define parse.error detailed

%union {
    syntax::Program * program;
    Token * token;
    syntax::Action * action;
    syntax::Syscall * syscall;
    syntax::SyscallsList * syscall_list;
    syntax::Rule * rule;
    syntax::RulesList * rule_list;
    syntax::Stmt * stmt;
    std::vector<std::shared_ptr<syntax::Stmt>> * stmt_list;
    syntax::Expr * expr;
    syntax::ExprList * expr_list;
    std::vector<std::string> * args_list;
    syntax::ImportArg * import_arg;
    syntax::ImportList * import_list;
}

%define api.pure full

%parse-param { void * yyscanner }
%parse-param { lexicon::Context * ctx }
%parse-param { std::string filename }
%parse-param { ReturnValue * ret }
%lex-param { void * yyscanner }
%lex-param { lexicon::Context * ctx }

%destructor { delete $$; } <token>

%token <token>
    // Basics
    YYEOF POLICY IDENTIFIER SYSCALL_PARAM POLICY_DECL_OR
    // Import statements
    IMPORT AS FROM
    // Delimiters
    LBRACE RBRACE LPAREN RPAREN LBRACK RBRACK ARROW
    // Actions
    ALLOW KILL TRAP ERROR NOTIFY TRACE LOG TERMINATE
    // Keywords
    IF FUNCTION
    // Separators
    COMMA SEMI
    // Binary operators
    ADD SUB MUL DIV MOD AND OR XOR LSH RSH
    // Logical binary operators
    EQ NE GT LT GE LE LOGICAL_AND LOGICAL_OR
    // Unary operators
    NOT
    // Logical unary operators
    LOGICAL_NOT
    // Literals
    INTEGER STRING TRUE FALSE NIL
    // Assignment
    ASSIGN

%type<token>
    soft_keyword token_keyword syscall_name token_action token_param_action
    // Operators
    p1_op p2_op p3_op p4_op p5_op p6_op p7_op p8_op p9_op p10_op
    unary_op
    // Boolean
    boolean

%type<action> param_action action

%type<syscall> syscall

%type<syscall_list> syscall_list rule_body

%type<rule> rule

%type<rule_list> policy_body

%type<stmt> stmt policy_decl function_decl variable_decl import_stmt

%type<import_list> import_list
%type<import_arg> import_arg

%type<stmt_list> stmt_list

%type<program> program

%type<expr>
    // Root expression
    expr
    // Expression groups
    p1_expr p2_expr p3_expr p4_expr p5_expr p6_expr p7_expr p8_expr p9_expr p10_expr
    // Base expressions
    unary_expr base_expr constant function_call

%type<expr_list> expr_list function_params

%type<args_list> function_args args_list

%%
program: stmt_list[stmts] YYEOF {
    auto stmts = std::shared_ptr<std::vector<std::shared_ptr<syntax::Stmt>>>($stmts);

    if ($stmts->empty()) {
        $$ = new syntax::Program(filename, stmts, { 1, 1 }, { 1, 1 });
    }
    else {
        auto begin = $stmts->at(0)->begin();
        auto end = $stmts->at($stmts->size() - 1)->end();

        $$ = new syntax::Program(filename, stmts, begin, end);
    }

    ret->program = $$;
}

stmt_list:
    %empty {
        $$ = new std::vector<std::shared_ptr<syntax::Stmt>>();
    }
    | stmt_list[list] stmt[current] {
        $$ = $list;
        $$->push_back(std::shared_ptr<syntax::Stmt>($current));
    }

stmt: policy_decl | function_decl | variable_decl | import_stmt

import_stmt:
    FROM STRING[module] IMPORT import_list[args] SEMI {
        $$ = new syntax::ImportStmt(
            $module->text().substr(1, $module->text().size() - 2),
            std::shared_ptr<syntax::ImportList>($args),
            $FROM->begin(),
            $SEMI->end()
        );
    }
    | FROM STRING[module] IMPORT LPAREN import_list[args] RPAREN SEMI {
        $$ = new syntax::ImportStmt(
            $module->text().substr(1, $module->text().size() - 2),
            std::shared_ptr<syntax::ImportList>($args),
            $FROM->begin(),
            $SEMI->end()
        );
    }

import_arg:
    IDENTIFIER[name] {
        $$ = syntax::ImportArg::Symbol($name->text(), $name->begin(), $name->end());
    }
    | IDENTIFIER[name] AS IDENTIFIER[alias] {
        $$ = syntax::ImportArg::Symbol($name->text(), $alias->text(), $name->begin(), $alias->end());
    }
    | POLICY IDENTIFIER[name] {
        $$ = syntax::ImportArg::Policy($name->text(), $POLICY->begin(), $name->end());
    }
    | POLICY IDENTIFIER[name] AS IDENTIFIER[alias] {
        $$ = syntax::ImportArg::Policy($name->text(), $alias->text(), $POLICY->begin(), $alias->end());
    }

import_list:
    import_arg[arg] {
        $$ = new syntax::ImportList();
        $$->push_back(std::shared_ptr<syntax::ImportArg>($arg));
    }
    | import_list[list] COMMA import_arg[arg] {
        $$ = $list;
        $$->push_back(std::shared_ptr<syntax::ImportArg>($arg));
    }

variable_decl: IDENTIFIER[name] ASSIGN expr[value] SEMI {
    $$ = new syntax::VariableDecl(
        $name->text(),
        std::shared_ptr<syntax::Expr>($value),
        $name->begin(),
        $SEMI->end()
    );
}

function_decl:
    FUNCTION IDENTIFIER[name] LPAREN function_args[args] RPAREN ARROW expr[body] SEMI {
        $$ = new syntax::FunctionDecl(
            $name->text(),
            std::shared_ptr<std::vector<std::string>>($args),
            std::shared_ptr<syntax::Expr>($body),
            $FUNCTION->begin(),
            $SEMI->end()
        );
    }
    | FUNCTION IDENTIFIER[name] LPAREN function_args[args] RPAREN ARROW expr[body] error {
        $$ = new syntax::FunctionDecl(
            $name->text(),
            std::shared_ptr<std::vector<std::string>>($args),
            std::shared_ptr<syntax::Expr>($body),
            $FUNCTION->begin(),
            $body->end()
        );
    }

function_args:
    %empty {
        $$ = new std::vector<std::string>();
    }
    | args_list optional_comma

args_list:
    IDENTIFIER[current] {
        $$ = new std::vector<std::string>();
        $$->push_back($current->text());
    }
    | args_list[list] COMMA IDENTIFIER[current] {
        $$ = $list;
        $$->push_back($current->text());
    }


policy_decl:
    POLICY IDENTIFIER[name] action POLICY_DECL_OR LBRACE policy_body[body] RBRACE {
        auto begin = $POLICY->begin();
        auto end = $RBRACE->end();
        auto body = std::shared_ptr<syntax::RulesList>($body);
        auto name = $name->text();
        auto action = std::shared_ptr<syntax::Action>($action);
        $$ = new syntax::Policy(name, body, action, begin, end);
    }
    | POLICY IDENTIFIER[name] LBRACE policy_body[body] RBRACE {
        auto begin = $POLICY->begin();
        auto end = $RBRACE->end();
        auto name = $name->text();
        auto body = std::shared_ptr<syntax::RulesList>($body);
        auto action = std::make_shared<syntax::Action>(
            syntax::Action::Kind::error, EPERM, begin, end
        );
        $$ = new syntax::Policy(name, body, action, begin, end);
    }

policy_body:
    %empty {
        $$ = new std::vector<std::shared_ptr<syntax::Rule>>();
    }
    | policy_body[body] rule {
        $$ = $body;
        $$->push_back(std::shared_ptr<syntax::Rule>($rule));
    }

token_param_action: ERROR | TRAP | TRACE

param_action: token_param_action[token] LPAREN INTEGER[param] RPAREN {
    $$ = new syntax::Action(
        syntax::Action::kind_from_token($token),
        std::stoi($param->text()),
        $token->begin(),
        $RPAREN->end()
    );
}

token_action: ALLOW | KILL | NOTIFY | LOG | TERMINATE

action:
    param_action
    | token_action[token] {
        $$ = new syntax::Action(
            syntax::Action::kind_from_token($token),
            -1,
            $token->begin(),
            $token->end()
        );
    }

rule: action LBRACE rule_body[body] RBRACE {
    $$ = new syntax::Rule(
        std::shared_ptr<syntax::Action>($action),
        std::shared_ptr<std::vector<std::shared_ptr<syntax::Syscall>>>($body),
        $action->begin(),
        $RBRACE->end()
    );
}

syscall_name: IDENTIFIER | soft_keyword

syscall:
    LBRACK INTEGER[nr] RBRACK {
        $$ = new syntax::Syscall($nr->text(), nullptr, $LBRACK->begin(), $RBRACK->end());
    }
    | LBRACK INTEGER[nr] RBRACK IF expr[condition] {
        $$ = new syntax::Syscall(
            $nr->text(),
            std::shared_ptr<syntax::Expr>($condition),
            $LBRACK->begin(),
            $condition->end()
        );
    }
    | syscall_name[name] {
        $$ = new syntax::Syscall(
            $name->text(),
            nullptr,
            $name->begin(),
            $name->end()
        );
    }
    | syscall_name[name] IF expr[condition] {
        $$ = new syntax::Syscall(
            $name->text(),
            std::shared_ptr<syntax::Expr>($condition),
            $name->begin(),
            $condition->end()
        );
    }
    | syscall_name[name] IF error {
        $$ = new syntax::Syscall(
            $name->text(),
            nullptr,
            $name->begin(),
            $IF->end()
        );
    }

syscall_list:
    syscall[current] {
        $$ = new std::vector<std::shared_ptr<syntax::Syscall>>();
        $$->push_back(std::shared_ptr<syntax::Syscall>($current));
    }
    | syscall_list[list] COMMA syscall[current] {
        $$ = $list;
        $$->push_back(std::shared_ptr<syntax::Syscall>($current));
    }
    | syscall_list[list] error syscall[current] {
        $$ = $list;
        $$->push_back(std::shared_ptr<syntax::Syscall>($current));
    }

optional_comma: COMMA | %empty

rule_body:
    %empty {
        $$ = new std::vector<std::shared_ptr<syntax::Syscall>>();
    }
    | syscall_list optional_comma

expr: p1_expr

p1_op: LOGICAL_OR
p2_op: LOGICAL_AND
p3_op: EQ | NE
p4_op: LT | LE | GT | GE
p5_op: OR
p6_op: XOR
p7_op: AND
p8_op: LSH | RSH
p9_op: ADD | SUB
p10_op: MUL | DIV | MOD

p1_expr:
    p1_expr[left] p1_op[op] p2_expr[right] {
        $$ = bin_expr($left, $right, $op);
    }
    | p2_expr

p2_expr:
    p2_expr[left] p2_op[op] p3_expr[right] {
        $$ = bin_expr($left, $right, $op);
    }
    | p3_expr

p3_expr:
    p3_expr[left] p3_op[op] p4_expr[right] {
        $$ = bin_expr($left, $right, $op);
    }
    | p4_expr

p4_expr:
    p4_expr[left] p4_op[op] p5_expr[right] {
        $$ = bin_expr($left, $right, $op);
    }
    | p5_expr

p5_expr:
    p5_expr[left] p5_op[op] p6_expr[right] {
        $$ = bin_expr($left, $right, $op);
    }
    | p6_expr

p6_expr:
    p6_expr[left] p6_op[op] p7_expr[right] {
        $$ = bin_expr($left, $right, $op);
    }
    | p7_expr

p7_expr:
    p7_expr[left] p7_op[op] p8_expr[right] {
        $$ = bin_expr($left, $right, $op);
    }
    | p8_expr

p8_expr:
    p8_expr[left] p8_op[op] p9_expr[right] {
        $$ = bin_expr($left, $right, $op);
    }
    | p9_expr

p9_expr:
    p9_expr[left] p9_op[op] p10_expr[right] {
        $$ = bin_expr($left, $right, $op);
    }
    | p10_expr

p10_expr:
    p10_expr[left] p10_op[op] base_expr[right] {
        $$ = bin_expr($left, $right, $op);
    }
    | unary_expr

unary_op: NOT | LOGICAL_NOT | ADD | SUB

unary_expr:
    unary_op[op] base_expr[expr] {
        $$ = new syntax::UnaryExpr(
            std::shared_ptr<syntax::Expr>($expr),
            syntax::UnaryExpr::kind_from_token($op),
            $op->begin(),
            $expr->end()
        );
    }
    | base_expr

base_expr:
    LPAREN expr RPAREN {
        $$ = $expr;
    }
    | constant
    | IDENTIFIER[id] {
        $$ = new syntax::Variable(
            $id->text(),
            $id->begin(),
            $id->end()
        );
    }
    | SYSCALL_PARAM[id] {
        $$ = new syntax::SyscallParam(
            $id->text().substr(1),
            $id->begin(),
            $id->end()
        );
    }
    | function_call

boolean: TRUE | FALSE

constant:
    INTEGER[value] {
        $$ = new syntax::Constant(
            std::stoi($value->text()),
            $value->begin(),
            $value->end()
        );
    }
    | STRING[value] {
        $$ = new syntax::Constant(
            $value->text().substr(1, $value->text().size() - 2),
            $value->begin(),
            $value->end()
        );
    }
    | boolean[value] {
        $$ = new syntax::Constant(
            $value->text() == "true",
            $value->begin(),
            $value->end()
        );
    }
    | NIL[value] {
        $$ = new syntax::Constant(
            $value->begin(),
            $value->end()
        );
    }

expr_list:
    expr[current] {
        $$ = new syntax::ExprList();
        $$->push_back(std::shared_ptr<syntax::Expr>($current));
    }
    | expr_list[list] COMMA expr[current] {
        $$ = $list;
        $$->push_back(std::shared_ptr<syntax::Expr>($current));
    }

function_params:
    %empty {
        $$ = new syntax::ExprList();
    }
    | expr_list[params] optional_comma {
        $$ = $params;
    }

function_call:
    IDENTIFIER[name] LPAREN function_params[params] RPAREN {
        $$ = new syntax::FunctionCall(
            $name->text(),
            std::shared_ptr<syntax::ExprList>($params),
            $name->begin(),
            $RPAREN->end()
        );
    }

token_keyword:
    IF | ALLOW | KILL | TRAP | ERROR | NOTIFY | TRACE | LOG |
    TRUE | FALSE | NIL | FUNCTION | POLICY_DECL_OR |
    IMPORT | AS | FROM

soft_keyword: token_keyword[token] {
    $$ = new Token(yytokentype::IDENTIFIER, $token->begin(), $token->end(), $token->text());
    delete $token;
}
%%
YYERROR_DECL {
    auto token = ctx->last_token;

    ret->error
        << "Syntax error: " << (err + sizeof("sintax error,")) << '\n'
        << "    at " << filename << ':' << token->begin();
}

std::unique_ptr<syntax::Program> syntax::parse(std::string filename) {
    filename = fs::absolute(filename).string();

    auto file = fopen(filename.c_str(), "r");

    if (!file) {
        throw FileNotFoundError(filename);
    }

    auto ret = new ReturnValue();
    auto ctx = new lexicon::Context();

    void * yyscanner;

    yylex_init(&yyscanner);
    yyset_in(file, yyscanner);
    yyparse(yyscanner, ctx, filename, ret);

    auto program = ret->program;
    auto err = ret->error.str();

    yylex_destroy(yyscanner);
    fclose(file);
    delete ret;
    delete ctx;

    if (err.size()) {
        delete program;

        throw std::runtime_error(err);
    }

    return std::unique_ptr<syntax::Program>(program);
}

syntax::BinaryExpr * bin_expr(syntax::Expr * left, syntax::Expr * right, Token * op) {
    return new syntax::BinaryExpr(
        std::shared_ptr<syntax::Expr>(left),
        std::shared_ptr<syntax::Expr>(right),
        syntax::BinaryExpr::kind_from_token(op),
        left->begin(),
        right->end()
    );
}
