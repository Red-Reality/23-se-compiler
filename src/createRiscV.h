//
// Created by leon on 2023/10/3.
//
#pragma once
#ifndef SYSY_MAKE_TEMPLATE_CREATERISCV_H
#define SYSY_MAKE_TEMPLATE_CREATERISCV_H

#endif //SYSY_MAKE_TEMPLATE_CREATERISCV_H
#pragma once
#include "string"
#include "koopa.h"
#include <unordered_map>
using namespace std;
typedef unsigned long long ull;
//读取Koopa程序字符串生产IR内存
void parse_string(const char* str);
//处理二元表达式
void Visit(const koopa_raw_binary_t &oper, unordered_map<ull, int> &value_reg_map, int& stit);

// 处理store语句
void Visit(const koopa_raw_store_t& SaveObj,unordered_map<ull,int>& mymap,int& stit);