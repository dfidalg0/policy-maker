%require "3.7"

%code requires {
    #include <vector>
    #include <cstdio>

    typedef class Token Token;
    typedef class Action Action;
    typedef class Syscall Syscall;
    typedef class Rule Rule;
    typedef class Stmt Stmt;
    typedef class Policy Policy;
    typedef class Program Program;

    static Program * program;

    Program * parse(const char * filename);

    int yyerror(char const *);

    extern "C" int yylex();
    extern char* yytext;
    extern FILE * yyin;
}

%code provides {
    #include <token.hh>
    #include <syntax.hh>
}

%union {
    Token * token;
    Action * action;
    Syscall * syscall;
    std::vector<Syscall *> * syscall_list;
    Rule * rule;
    std::vector<Rule *> * rule_list;
    Stmt * stmt;
    std::vector<Stmt *> * stmt_list;
    Program * program;
}

%token <token>
    // Basics
    POLICY IDENTIFIER
    // Delimiters
    LBRACE RBRACE LPAREN RPAREN LBRACK RBRACK
    // Actions
    ALLOW KILL TRAP ERROR NOTIFY TRACE LOG TERMINATE
    // Keywords
    IF
    // Separators
    COMMA
    // Binary operators
    ADD SUB MUL DIV MOD AND OR XOR EQ NE GT LT GE LE LSH RSH
    // Logical binary operators
    LOGICAL_AND LOGICAL_OR
    // Unary operators
    NOT
    // Logical unary operators
    LOGICAL_NOT
    // Literals
    INTEGER STRING

%type<token> soft_keyword token_keyword syscall_name token_action token_param_action

%type<action> param_action action

%type<syscall> syscall

%type<syscall_list> syscall_list rule_body

%type<rule> rule

%type<rule_list> policy_body

%type<stmt> stmt policy_decl

%type<stmt_list> stmt_list

%type<program> program

%%
program: stmt_list[stmts] {
    auto begin = (*$stmts)[0]->begin();
    auto end = (*$stmts)[$stmts->size() - 1]->end();

    $$ = new Program(begin, end, $stmts);

    program = $$;
}

stmt_list:
    %empty {
        $$ = new std::vector<Stmt *>();
    }
    | stmt_list[list] stmt[current] {
        $$ = $list;
        $$->push_back($current);
    }

stmt: policy_decl

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
    auto begin = $token->begin();
    auto end = $RPAREN->end();
    auto kind = Action::kind_from_token($token);
    auto param = std::stoi($param->text());

    $$ = new Action(kind, begin, end, param);
}

token_action: ALLOW | KILL  | NOTIFY | LOG | TERMINATE

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

optional_comma: COMMA | %empty

rule_body:
    %empty {
        $$ = new std::vector<Syscall *>();
    }
    | syscall_list optional_comma

expr: %empty

token_keyword: IF | ALLOW | KILL | TRAP | ERROR | NOTIFY | TRACE | LOG

soft_keyword: token_keyword {
    $$ = new Token(yytoken_kind_t::IDENTIFIER, $1->begin(), $1->end(), $1->text());
    delete $1;
}
%%

int yyerror(char const * err) {
    return 0;
}

Program * parse(const char * filename) {
    auto file = fopen(filename, "r");

    if (!file) {
        return nullptr;
    }

    yyin = file;

    yyparse();

    return program;
}
