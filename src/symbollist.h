//
// Created by leon on 2023/10/10.
//


#pragma once
#include <unordered_map>
#include <vector>
#include <iostream>
#include <cassert>
#include "string"
#include "GlobalVar.h"
using namespace std;


/// 记录符号是常量还是变量的结构体


/// 检查符号表是否有值
bool HasName(vector<unordered_map<string,symboltype>>& sel_map,const string& name);
/// 获取符号表中的值
symboltype GetLvalValue(vector<unordered_map<string,symboltype>>& sel_map,const string& name);

void SetValue(vector<unordered_map<string,symboltype>>& sel_map,const string& name,symboltype value);

void warnerror(vector <unordered_map<string, symboltype>> &sel_map, const string &name);

void VALMAP_push();

int retValDepth(string name) ;