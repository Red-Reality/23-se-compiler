//
// Created by leon on 2023/10/10.
//


#pragma once
#include <unordered_map>
#include <vector>
#include <iostream>
#include <cassert>
#include "string"
using namespace std;

enum class ValType{
    Const,
    Var,
};
/// 记录符号是常量还是变量的结构体
struct symboltype{
    int num;
    ValType type;
};
/// 常量符号表，用于记录已声明的常量符号
/// 为了保证第五部分的作用域，每个block拥有一个unorder map
static vector<unordered_map<string,symboltype>> VAL_MAP;
/// 检查符号表是否有值
bool HasName(vector<unordered_map<string,symboltype>>& sel_map,const string& name);
/// 获取符号表中的值
symboltype GetLvalValue(vector<unordered_map<string,symboltype>>& sel_map,const string& name);

void SetValue(vector<unordered_map<string,symboltype>>& sel_map,const string& name,symboltype value);

void warnerror(vector <unordered_map<string, symboltype>> &sel_map, const string &name);