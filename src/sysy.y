%code requires {
    #include <memory>
    #include <string>
    #include<cstring>
    #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include<cstring>
#include "ast.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

%}

// 定义 parser 函数和错误处理函数的附加参数
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的


%union {
    std::string *str_val;
    int int_val;
    BaseAST *ast_val;


}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT LOR LAND EQ NEQ GEQ LEQ
%token <int_val> INT_CONST



// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Number Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
    : FuncDef {
        auto func = std::unique_ptr<BaseAST>($1);
        ast = std::unique_ptr<BaseAST>(new CompUnitAST(func));
    }
    ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
FuncDef
    : FuncType IDENT '(' ')' Block {
        auto type = std::unique_ptr<BaseAST>($1);
        auto ident = std::unique_ptr<std::string>($2);
        auto block = std::unique_ptr<BaseAST>($5);
        $$ = new FuncDefAST(type, ident->c_str(), block);
    }
    ;

// 同上, 不再解释
FuncType
    : INT {
        $$ = new FuncTypeAST("int");
    }
    ;

Block
    : '{' Stmt '}' {
        auto stmt = std::unique_ptr<BaseAST>($2);
        $$ = new BlockAST(stmt);
    }
    ;

Stmt
    : RETURN Exp ';' {
        auto exp = std::unique_ptr<BaseAST>($2);
        $$ = new StmtAST(exp);
    }
    ;

Exp
    : LOrExp{
        auto lorexp = std::unique_ptr<BaseAST>($1);
        $$ = new ExpAST(lorexp);
    }
    ;

PrimaryExp
    : '(' Exp ')'{
        auto exp = std::unique_ptr<BaseAST>($2);
        $$ = new PrimaryExpAST(exp);
    }
    | Number{
        auto number = std::unique_ptr<BaseAST>($1);
        $$ = new PrimaryExpAST(number);
    }
    ;

UnaryExp
    : PrimaryExp{
        auto primaryexp = std::unique_ptr<BaseAST>($1);
        $$ = new PrimaryExpAST(primaryexp);
    }
    | '+' UnaryExp{
        auto unaryexp = std::unique_ptr<BaseAST>($2);
        $$ = new UnaryExpAST(unaryexp,UnaryOp::Positive);
    }
    | '-' UnaryExp{
        auto unaryexp = std::unique_ptr<BaseAST>($2);
        $$ = new UnaryExpAST(unaryexp,UnaryOp::Negative);
    }
    | '!' UnaryExp{
    auto unaryexp = std::unique_ptr<BaseAST>($2);
    $$ = new UnaryExpAST(unaryexp,UnaryOp::LogicalFalse);
    }
    ;

MulExp
    : UnaryExp{
        auto unaryexp = std::unique_ptr<BaseAST>($1);
        $$ = new MulExpAST(unaryexp);
    }
    | MulExp '*' UnaryExp{
        auto mulexp = std::unique_ptr<BaseAST>($1);
        auto unaryexp = std::unique_ptr<BaseAST>($3);
        $$ = new MulExpAST(unaryexp,mulexp,MulType::Mul);
    }
    | MulExp '/' UnaryExp{
    auto mulexp = std::unique_ptr<BaseAST>($1);
    auto unaryexp = std::unique_ptr<BaseAST>($3);
    $$ = new MulExpAST(unaryexp,mulexp,MulType::Div);
    }
    | MulExp '%' UnaryExp{
     auto mulexp = std::unique_ptr<BaseAST>($1);
     auto unaryexp = std::unique_ptr<BaseAST>($3);
     $$ = new MulExpAST(unaryexp,mulexp,MulType::Mod);
    }
    ;

AddExp
    : MulExp{
        auto mulexp = std::unique_ptr<BaseAST>($1);
        $$ = new AddExpAST(mulexp);
    }
    | AddExp '+' MulExp{
        auto addexp = std::unique_ptr<BaseAST>($1);
        auto mulexp = std::unique_ptr<BaseAST>($3);
        $$ = new AddExpAST(mulexp,addexp,AddType::Add);
    }
    | AddExp '-' MulExp{
     auto addexp = std::unique_ptr<BaseAST>($1);
     auto mulexp = std::unique_ptr<BaseAST>($3);
     $$ = new AddExpAST(mulexp,addexp,AddType::Sub);
     }
    ;

RelExp
    :AddExp{
        auto addexp =std::unique_ptr<BaseAST>($1);
        $$ = new RelExpAST(addexp);
    }
    | RelExp '<' AddExp{
        auto relexp = std::unique_ptr<BaseAST>($1);
        auto addexp = std::unique_ptr<BaseAST>($3);
        $$ = new RelExpAST(addexp,relexp,RelType::Less);
    }
    | RelExp '>' AddExp{
        auto relexp = std::unique_ptr<BaseAST>($1);
        auto addexp = std::unique_ptr<BaseAST>($3);
        $$ = new RelExpAST(addexp,relexp,RelType::Bigger);
    }
    | RelExp LEQ AddExp{
        auto relexp = std::unique_ptr<BaseAST>($1);
        auto addexp = std::unique_ptr<BaseAST>($3);
        $$ = new RelExpAST(addexp,relexp,RelType::LessEq);
    }
    |RelExp GEQ AddExp{
        auto relexp = std::unique_ptr<BaseAST>($1);
        auto addexp = std::unique_ptr<BaseAST>($3);
        $$ = new RelExpAST(addexp,relexp,RelType::BiggerEq);
    }
    ;

EqExp
    : RelExp{
        auto relexp = std::unique_ptr<BaseAST>($1);
        $$ = new EqExpAST(relexp);
    }
    | EqExp EQ RelExp{
        auto eqexp = std::unique_ptr<BaseAST>($1);
        auto relexp = std::unique_ptr<BaseAST>($3);
        $$ = new EqExpAST(eqexp,relexp,EqType::Equal);
    }
    | EqExp NEQ RelExp{
        auto eqexp = std::unique_ptr<BaseAST>($1);
        auto relexp = std::unique_ptr<BaseAST>($3);
        $$ = new EqExpAST(eqexp,relexp,EqType::NotEqual);
    }
    ;

LAndExp
    : EqExp{
        auto eqexp = std::unique_ptr<BaseAST>($1);
        $$ = new LAndExpAST(eqexp);
    }
    | LAndExp LAND EqExp{
        auto landexp = std::unique_ptr<BaseAST>($1);
        auto eqexp = std::unique_ptr<BaseAST>($3);
        $$ = new LAndExpAST(landexp,eqexp);
    }
    ;

LOrExp
    : LAndExp{
        auto landexp =std::unique_ptr<BaseAST>($1);
        $$ = new LOrExpAST(landexp);
    }
    | LOrExp LOR LAndExp{
        auto lorexp = std::unique_ptr<BaseAST>($1);
        auto landexp = std::unique_ptr<BaseAST>($3);
        $$ = new LOrExpAST(lorexp,landexp);
    }
Number
    : INT_CONST {
        $$ = new NumberAST($1);
    }
    ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
      extern int yylineno;    // defined and maintained in lex
        extern char *yytext;    // defined and maintained in lex
        int len=strlen(yytext);
        int i;
        char buf[512]={0};
        for (i=0;i<len;++i)
        {
            sprintf(buf,"%s%d ",buf,yytext[i]);
        }
        fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);
}