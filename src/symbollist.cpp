//
// Created by leon on 2023/10/10.
//
#include "symbollist.h"
using namespace std;
/// 检查符号表是否有值
bool HasName(vector<unordered_map<string,symboltype>>& sel_map,const string& name){

    for (auto it = sel_map.rbegin(); it != sel_map.rend(); ++it){
        const std::unordered_map<std::string, symboltype>& map = *it;
        auto found = map.find(name);
        if (found != map.end()) {
            return true;
        }
    }
    return false;
}

/// 获取符号表中的值
symboltype GetLvalValue(vector<unordered_map<string,symboltype>>& sel_map,const string& name){
    for (auto it = sel_map.rbegin(); it != sel_map.rend(); ++it){
        const std::unordered_map<std::string, symboltype>& map = *it;
        auto found = map.find(name);
        if (found != map.end()) {
            return found->second;
        }
    }
    cerr << "没有找到指定的参数" << endl;
    cerr << "想要找的参数为：" << name << endl;
    cerr << "当前符号表如下:" << endl;
    for (auto it = sel_map.rbegin(); it != sel_map.rend(); ++it) {
        cerr<<"list"<<endl;
        const std::unordered_map<string, symboltype>& map = *it;
        for (auto mpit = map.begin(); mpit != map.end(); ++mpit) {
            cerr << "name:" << mpit->first << ", num = " << mpit->second.num << ", type = " << int(mpit->second.type) << endl;
        }
    }
    assert(0);
}

void SetValue(vector<unordered_map<string,symboltype>>& sel_map,const string& name,symboltype value){
    for (auto it = sel_map.rbegin(); it != sel_map.rend(); ++it){
        std::unordered_map<std::string, symboltype>& map = *it;
        auto found = map.find(name);
        if (found != map.end()) {
            found->second.type = value.type;
            found->second.num=value.num;
        }
    }
    assert(0);
}