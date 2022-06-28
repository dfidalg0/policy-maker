%require "3.7"

%code requires {
    #include <string>
    #include <sstream>
    #include <vector>
    #include <cstdio>
    #include <errors.hh>

    typedef class Token Token;
    typedef class Action Action;
    typedef class Syscall Syscall;
    typedef class Rule Rule;
    typedef class Stmt Stmt;
    typedef class Policy Policy;
    typedef class Program Program;
    typedef class Expr Expr;
    typedef class BinaryExpr BinaryExpr;
    typedef class UnaryExpr UnaryExpr;
    typedef class Constant Constant;
    typedef class Variable Variable;
    typedef class FunctionCall FunctionCall;
    typedef class FunctionDecl FunctionDecl;

    struct ReturnValue {
        Program * program;
        bool success;

        ReturnValue(): program(nullptr), success(true) {}
    };

    static Program * program;

    Program * parse(const char * filename);

    inline BinaryExpr * bin_expr(Expr * left, Expr * right, Token * op);

    int yyerror(ReturnValue *, char const *);

    extern "C" int yylex();
    extern char* yytext;
    extern FILE * yyin;
}

%code provides {
    #include <token.hh>
    #include <syntax.hh>
}

%union {
    Program * program;
    Token * token;
    Action * action;
    Syscall * syscall;
    std::vector<Syscall *> * syscall_list;
    Rule * rule;
    std::vector<Rule *> * rule_list;
    Stmt * stmt;
    std::vector<Stmt *> * stmt_list;
    Expr * expr;
    std::vector<Expr *> * expr_list;
    std::vector<std::string> * args_list;
}

%parse-param { ReturnValue * ret }

%token <token>
    // Basics
    YYEOF POLICY IDENTIFIER
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

%type<token>
    soft_keyword token_keyword syscall_name token_action token_param_action
    // Operators
    p1_op p2_op p3_op p4_op p5_op p6_op p7_op p8_op p9_op p10_op
    unary_op
    // Boolean
    boolean

%type<token> error

%type<action> param_action action

%type<syscall> syscall

%type<syscall_list> syscall_list rule_body

%type<rule> rule

%type<rule_list> policy_body

%type<stmt> stmt policy_decl function_decl

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
    if ($stmts->empty()) {
        $$ = new Program({ 1, 1 }, { 1, 1 }, $stmts);
    }
    else {
        auto begin = (*$stmts)[0]->begin();
        auto end = (*$stmts)[$stmts->size() - 1]->end();

        $$ = new Program(begin, end, $stmts);
    }

    ret->program = $$;
}

stmt_list:
    %empty {
        $$ = new std::vector<Stmt *>();
    }
    | stmt_list[list] stmt[current] {
        $$ = $list;
        $$->push_back($current);
    }

stmt: policy_decl | function_decl

function_decl:
    FUNCTION IDENTIFIER[name] LPAREN function_args[args] RPAREN ARROW expr[body] SEMI {
        $$ = new FunctionDecl(
            $FUNCTION->begin(),
            $SEMI->end(),
            $name->text(),
            $args,
            $body
        );
    }
    | FUNCTION IDENTIFIER[name] LPAREN function_args[args] RPAREN ARROW expr[body] error {
        std::stringstream err;

        err << "Invalid token found at "
            << $error->begin().line() << ":" << $error->begin().col()
            << ": \"" << $error->text() << "\""
            << " (Expected: ';')";

        yyerror(ret, err.str().c_str());

        $$ = new FunctionDecl(
            $FUNCTION->begin(),
            $body->end(),
            $name->text(),
            $args,
            $body
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


policy_decl: POLICY IDENTIFIER[name] LBRACE policy_body[body] RBRACE {
    auto begin = $POLICY->begin();
    auto end = $RBRACE->end();
    auto name = $name->text();
    auto body = $body;
    $$ = new Policy(name, begin, end, body);
}

policy_body:
    %empty {
        $$ = new std::vector<Rule *>();
    }
    | policy_body[body] rule {
        $$ = $body;
        $1->push_back($rule);
    }

token_param_action: ERROR | TRAP | TRACE

param_action: token_param_action[token] LPAREN INTEGER[param] RPAREN {
    $$ = new Action(
        Action::kind_from_token($token),
        $token->begin(),
        $RPAREN->end(),
        std::stoi($param->text())
    );
}

token_action: ALLOW | KILL | NOTIFY | LOG | TERMINATE

action:
    param_action
    | token_action[token] {
        $$ = new Action(
            Action::kind_from_token($token),
            $token->begin(),
            $token->end()
        );
    }

rule: action LBRACE rule_body[body] RBRACE {
    $$ = new Rule($action, $body, $action->begin(), $RBRACE->end());
}

syscall_name: IDENTIFIER | soft_keyword

syscall:
    syscall_name[name] {
        $$ = new Syscall(
            $name->text(),
            $name->begin(),
            $name->end()
        );
    }
    | syscall_name[name] IF expr[condition] {
        $$ = new Syscall(
            $name->text(),
            $name->begin(),
            $name->end(),
            $condition
        );
    }
    | syscall_name[name] IF error {
        std::stringstream err;

        err << "Syscall condition not found at "
            << $error->begin().line() << ":" << $error->begin().col()
            << " (Found: \"" << $error->text() << "\")";

        yyerror(ret, err.str().c_str());

        $$ = new Syscall(
            $name->text(),
            $name->begin(),
            $name->end()
        );
    }

syscall_list:
    syscall[current] {
        $$ = new std::vector<Syscall *>();
        $$->push_back($current);
    }
    | syscall_list[list] COMMA syscall[current] {
        $$ = $list;
        $$->push_back($current);
    }
    | syscall_list[list] error syscall[current] {
        std::stringstream err;

        err << "Invalid token found at "
            << $error->begin().line() << ":" << $error->begin().col()
            << ": \"" << $error->text() << "\""
            << " (Expected: ',')";

        yyerror(ret, err.str().c_str());

        $$ = $list;
        $$->push_back($current);
    }

optional_comma: COMMA | %empty

rule_body:
    %empty {
        $$ = new std::vector<Syscall *>();
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

unary_op: NOT | LOGICAL_NOT

unary_expr:
    unary_op[op] base_expr[expr] {
        $$ = new UnaryExpr(
            $expr,
            UnaryExpr::kind_from_token($op),
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
        $$ = new Variable(
            $id->text(),
            $id->begin(),
            $id->end()
        );
    }
    | function_call

boolean: TRUE | FALSE

constant:
    INTEGER[value] {
        $$ = new Constant(
            Constant::Type::integer,
            $value->text(),
            $value->begin(),
            $value->end()
        );
    }
    | STRING[value] {
        $$ = new Constant(
            Constant::Type::string,
            $value->text(),
            $value->begin(),
            $value->end()
        );
    }
    | boolean[value] {
        $$ = new Constant(
            Constant::Type::boolean,
            $value->text(),
            $value->begin(),
            $value->end()
        );
    }
    | NIL[value] {
        $$ = new Constant(
            Constant::Type::null,
            $value->text(),
            $value->begin(),
            $value->end()
        );
    }

expr_list:
    expr[current] {
        $$ = new std::vector<Expr *>();
        $$->push_back($current);
    }
    | expr_list[list] COMMA expr[current] {
        $$ = $list;
        $$->push_back($current);
    }

function_params:
    %empty {
        $$ = new std::vector<Expr *>();
    }
    | expr_list[params] optional_comma {
        $$ = $params;
    }

function_call:
    IDENTIFIER[name] LPAREN function_params[params] RPAREN {
        $$ = new FunctionCall(
            $name->begin(),
            $RPAREN->end(),
            $name->text(),
            $params
        );
    }

token_keyword: IF | ALLOW | KILL | TRAP | ERROR | NOTIFY | TRACE | LOG | TRUE | FALSE | NIL | FUNCTION

soft_keyword: token_keyword {
    $$ = new Token(yytokentype::IDENTIFIER, $1->begin(), $1->end(), $1->text());
    delete $1;
}
%%

int yyerror(ReturnValue * ret, char const * err) {
    ret->success = false;

    std::cerr << err << std::endl;

    return 0;
}

Program * parse(const char * filename) {
    auto file = fopen(filename, "r");

    if (!file) {
        throw FileNotFoundError(filename);
    }

    yyin = file;

    auto ret = new ReturnValue();

    yyparse(ret);
    auto program = ret->program;

    if (!ret->success) {
        delete program;

        throw ParseError(filename);
    }

    delete ret;

    return program;
}

BinaryExpr * bin_expr(Expr * left, Expr * right, Token * op) {
    return new BinaryExpr(
        left,
        right,
        BinaryExpr::kind_from_token(op),
        left->begin(),
        right->end()
    );
}
