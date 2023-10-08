#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include "iostream"
#include <fstream>
#include "cstring"

#include "ast.hpp"
#include "createRiscV.h"
using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
extern FILE *yyin;
extern int yyparse(std::unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[])
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
    // compiler 模式 输入文件 -o 输出文件
    assert(argc == 5);
    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
    yyin = fopen(input, "r");
    assert(yyin);

    // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
    unique_ptr<BaseAST> ast;
    auto ret = yyparse(ast);
    assert(!ret);
    if(strcmp(mode,"-koopa")==0)
    {
        freopen(output,"w",stdout);
        cout<<ast->DumpKoopa()<<endl;
        return 0;
    }
    else if (strcmp(mode,"-riscv")==0){

        freopen(output,"w",stdout);

        parse_string(ast->DumpKoopa().c_str());

        return 0;
    }
    else if (strcmp(mode,"-deb")==0){
        freopen("debug.out","w",stdout);
        cout<<ast->DumpAST()<<endl;
        return 0;
    }

}