#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include "iostream"
#include <fstream>
#include "cstring"
#include "sstream"
#include "ast.hpp"
#include "createRiscV.h"
#include "symbollist.h"
using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
extern FILE *yyin;
extern int yyparse(std::unique_ptr<BaseAST> &ast);

string prepare(int t){
    ostringstream oss;
    // 初始化时先存入全局符号表
    VALMAP_push();

    if(t){
        cout<<"decl @getint(): i32"<<endl;
        cout<<"decl @getch(): i32"<<endl;
        cout<<"decl @getarray(*i32): i32"<<endl;
        cout<<"decl @putint(i32)"<<endl;
        cout<<"decl @putch(i32)"<<endl;
        cout<<"decl @putarray(i32, *i32)"<<endl;
        cout<<endl<<endl;
    }
    else{
        oss<<"decl @getint(): i32"<<endl;
        oss<<"decl @getch(): i32"<<endl;
        oss<<"decl @getarray(*i32): i32"<<endl;
        oss<<"decl @putint(i32)"<<endl;
        oss<<"decl @putch(i32)"<<endl;
        oss<<"decl @putarray(i32, *i32)"<<endl;
        oss<<endl<<endl;
    }
    //初始化库函数

    //插入库函数名
    unordered_map<string,symboltype>& last = VAL_MAP[0];
    last["getint"] = {-3333333,ValType::IntFuncname};
    last["getch"] = {-3333333,ValType::IntFuncname};
    last["getarray"] = {-3333333,ValType::IntFuncname};
    last["putint"] = {-3333333,ValType::VoidFuncname};
    last["putch"] = {-3333333,ValType::VoidFuncname};
    last["putarray"] = {-3333333,ValType::VoidFuncname};
    if(t)
        return "";
    else
        return oss.str();
}
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

        prepare(1);
        cout<<ast->DumpKoopa()<<endl;

        return 0;
    }
    else if (strcmp(mode,"-riscv")==0){

        freopen(output,"w",stdout);
        ostringstream oss;
        oss<<prepare(0)<<ast->DumpKoopa().c_str();
        parse_string(oss.str().c_str());

        return 0;
    }
    else if (strcmp(mode,"-deb")==0){
        freopen("debug.out","w",stdout);
        cout<<ast->DumpAST()<<endl;
        return 0;
    }

}