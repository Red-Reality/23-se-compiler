//
// Created by leon on 2023/10/16.
//

#include "IfElseAst.hpp"
#include "sstream"

int IF_COUNT = 0;
static int WHILE_COUNT = 0;

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
        oss<<endl<<"\t%COMPILER_end_"<<ifcnt<<":"<<endl;

    }else{
        // 说明在这一部分全部值已经返回了
        assert(!BLOCK_RET_RECORDER.empty());
        BLOCK_RET_RECORDER.back() = 1;
    }
    return oss.str();
}
string WhileAST::DumpKoopa() const {
    // 所在块不应该已经有ret，有的话不需要执行下面语句

    if(BLOCK_RET_RECORDER.empty()||BLOCK_RET_RECORDER.back()!=0)
        return "";

    int whl_cnt = WHILE_COUNT++;


    ostringstream oss;
    oss<<"\tjump %WHILE_ENTRY_"<<whl_cnt<<endl;
    oss<<endl<<"%WHILE_ENTRY_"<<whl_cnt<<":"<<endl;
    oss<<condition->DumpKoopa();


    if(BLOCK_RET_RECORDER.back()==0){
        oss<<"\tbr %"<<NAME_NUMBER-1<<", %WHILE_BODY_"<<whl_cnt<<", %WHILE_END_"<<whl_cnt<<endl;
    }

    //进入while块,记录该块所在位置
    BLOCK_RET_RECORDER.push_back(0);
    oss<< endl<<"%WHILE_BODY_"<<whl_cnt<<":"<<endl;
    oss<<stmt->DumpKoopa();
    if(BLOCK_RET_RECORDER.back()==0){
        oss<<"jump %WHILE_ENTRY_"<<whl_cnt<< endl;
    }
    BLOCK_RET_RECORDER.pop_back();
    //离开while块
    //现在当前块是没有返回过的
    if(BLOCK_RET_RECORDER.back()==0)
        oss<<endl<<"%WHILE_END_"<<whl_cnt<<":"<<endl;
    return oss.str();

}

/// TODO：想不到怎么实现判断break和continue不在while内的错误情况
string ConBreStmt::DumpKoopa() const {
    // 一定是推出了当前的语句块
    ostringstream oss;
    BLOCK_RET_RECORDER.back()=1;
    switch (type) {
        case ConOrBre::CONTINUE:
            oss<< "\tjump %WHILE_ENTRY_"<<WHILE_COUNT-1<< endl;
            return oss.str();
        case ConOrBre::BREAK:
            oss<<"\tjump %WHILE_END_"<<WHILE_COUNT-1<< endl;
            return oss.str();
    }
}