//
// Created by leon on 2023/10/16.
//

#pragma once
#include <vector>
using namespace std;
//用于命名if跳转标签
extern int IF_COUNT ;

/// 用于记录当前寄存器用到几号
/// %NAME_NUMBER 意思是下一个空着的变量符而不是最后一个已用的变量
extern int NAME_NUMBER;

/// 用于记录当前块是否已经有ret
extern vector<int> BLOCK_RET_RECORDER;


/// 用于记录每一层符号表被用过几次，避免变量重名
extern vector<int> VALMAP_LEVELREC;