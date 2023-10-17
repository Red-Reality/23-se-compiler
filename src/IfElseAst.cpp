//
// Created by leon on 2023/10/16.
//

#include "IfElseAst.hpp"
#include "sstream"

int IF_COUNT = 0;


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

    int ifcnt = IF_COUNT++;
    // br指令跳转
    oss<<"\tbr %"<<NAME_NUMBER-1<<", %COMPILER_then_"<<ifcnt;
    if(elseexp!= nullptr)
        oss<<", %COMPILER_else_"<<ifcnt<<endl;
    else{
        oss<<", %COMPILER_end_"<<ifcnt<<endl;
        output_end_signal= true;
    }

    //处理if内的语句
    oss<<endl<<"%COMPILER_then_"<<ifcnt<<":"<<endl;
    //进入if内部的基本块
    BLOCK_RET_RECORDER.push_back(0);
    oss<<ifexp->DumpKoopa();
    if(BLOCK_RET_RECORDER.back()==0){
        oss<<"\tjump %COMPILER_end_"<<ifcnt<<endl;
        output_end_signal= true;
    }
    BLOCK_RET_RECORDER.pop_back();

    if(elseexp!= nullptr){
        //进入else的基本块
        BLOCK_RET_RECORDER.push_back(0);
        oss<<endl<<"%COMPILER_else_"<<ifcnt<<":"<<endl;
        oss<<elseexp->DumpKoopa();
        if(BLOCK_RET_RECORDER.back()==0){
            oss<<"\tjump %COMPILER_end_"<<ifcnt<<endl;
            output_end_signal= true;
        }
        BLOCK_RET_RECORDER.pop_back();
    }

    if(output_end_signal){
        oss<<endl<<"%COMPILER_end_"<<ifcnt<<":"<<endl;

    }else{
        // 说明在这一部分全部值已经返回了
        assert(!BLOCK_RET_RECORDER.empty());
        BLOCK_RET_RECORDER.back() = 1;
    }
    return oss.str();
}