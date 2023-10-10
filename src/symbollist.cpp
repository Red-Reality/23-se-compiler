//
// Created by leon on 2023/10/10.
//
#include "symbollist.h"
using namespace std;
/// 检查符号表是否有值
bool HasName(vector<unordered_map<string,int>>& sel_map,const string& name){

    for (auto it = sel_map.rbegin(); it != sel_map.rend(); ++it){
        const std::unordered_map<std::string, int>& map = *it;
        auto found = map.find(name);
        if (found != map.end()) {
            return true;
        }
    }
    return false;
}

/// 获取符号表中的值
int GetLvalValue(vector<unordered_map<string,int>>& sel_map,const string& name){
    for (auto it = sel_map.rbegin(); it != sel_map.rend(); ++it){
        const std::unordered_map<std::string, int>& map = *it;
        auto found = map.find(name);
        if (found != map.end()) {
            return found->second;
        }
    }
    assert(0);
}