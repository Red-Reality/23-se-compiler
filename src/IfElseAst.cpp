//
// Created by leon on 2023/10/16.
//

#include "IfElseAst.hpp"
#include "sstream"

int IF_COUNT = 0;

///TODO: IFCOUNT的计数有问题，需要考虑嵌套
vector<int> BLOCK_RET_RECORDER;
string IfElseAST::DumpKoopa() const {
    // 检查有没有输出跳转地址
    bool output_end_signal = false;
    // if所在基本块不应该有ret
    if(BLOCK_RET_RECORDER.back()!=0)
        return "";
    //首先将求出的值存入寄存器
    ostringstream oss;
    oss<<sequence->DumpKoopa();
//    cerr<<oss.str()<<endl;
    // br指令跳转
    oss<<"\tbr %"<<NAME_NUMBER-1<<", %COMPILER_then_"<<IF_COUNT;
    if(elseexp!= nullptr)
        oss<<", %COMPILER_else_"<<IF_COUNT<<endl;
    IF_COUNT++;

    //分行，更好看点
    oss<<endl<<endl;

    //处理if内的语句
    oss<<"%COMPILER_then_"<<IF_COUNT-1<<":"<<endl;
    //进入if内部的基本块
    BLOCK_RET_RECORDER.push_back(0);
    oss<<ifexp->DumpKoopa();
    if(BLOCK_RET_RECORDER.back()==0){
        oss<<"\tjump %COMPILER_end_"<<IF_COUNT-1<<endl<< endl;
        output_end_signal= true;
    }
    BLOCK_RET_RECORDER.pop_back();

    if(elseexp!= nullptr){
        //进入else的基本块
        BLOCK_RET_RECORDER.push_back(0);
        oss<<"%COMPILER_else_"<<IF_COUNT-1<<endl;
        oss<<elseexp->DumpKoopa();
        if(BLOCK_RET_RECORDER.back()==0){
            oss<<"\tjump %COMPILER_end_"<<IF_COUNT-1<<endl<<endl;
            output_end_signal= true;
        }
    }

    if(output_end_signal){
        oss<<"%COMPILER_end_"<<IF_COUNT-1<<endl;
    }
    return oss.str();
}