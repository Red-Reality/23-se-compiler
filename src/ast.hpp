#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <vector>
#include <cassert>
#include "symbollist.h"
using namespace std;
// 所有 AST 的基类

/// 用于记录当前寄存器用到几号
/// %NAME_NUMBER 意思是下一个空着的变量符而不是最后一个已用的变量
static int NAME_NUMBER = 0;
//简化智能指针
template<typename T>
using point = std::unique_ptr<T>;


class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual std::string DumpAST() const {
        return "Not Implement";
    };

    virtual std::string DumpKoopa() const {
        return "Not Implement";
    };
    virtual int Calc() const {
        cerr<<"Not Implied Calc,unexcept situation may be occur"<<endl;
        return 0;
    }
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr <BaseAST> func_def;

    CompUnitAST(std::unique_ptr <BaseAST> &_func_def) {
        func_def = std::move(_func_def);
    }


    std::string DumpAST() const override;

    std::string DumpKoopa() const override;
};

class FuncDefAST : public BaseAST {
public:
    std::unique_ptr <BaseAST> func_type;
    std::string ident;
    std::unique_ptr <BaseAST> block;

    FuncDefAST(std::unique_ptr <BaseAST> &_func_type, const char *_ident, std::unique_ptr <BaseAST> &_block)
            : ident(_ident) {
        func_type = std::move(_func_type);
        block = std::move(_block);
    }


    std::string DumpAST() const override;

    std::string DumpKoopa() const override;
};

class FuncTypeAST : public BaseAST {
public:
    std::string name;

    FuncTypeAST(const char *_name) : name(_name) {}

    std::string DumpAST() const override;

    std::string DumpKoopa() const override;
};


class BlockAST : public BaseAST {
public:
    std::unique_ptr <BaseAST> stmt;
    //插入符号表
    BlockAST(std::unique_ptr <BaseAST> &_stmt) {
        stmt = std::move(_stmt);
    }


    std::string DumpAST() const override;

    std::string DumpKoopa() const override;
};

//用链表的方式来记录block内的所有语句
class BlockItemAST:public BaseAST{
public:
    point<BaseAST> stmt;
    point<BlockItemAST> next;
    void AddItem(point<BlockItemAST> &_next){
        if (next == nullptr) {
            next = std::move(_next);
        } else {
            BlockItemAST* tmp=next.get();
            while (tmp->next!= nullptr){
                tmp=tmp->next.get();
            }
            tmp->next = std::move(_next);
        }
    }

    BlockItemAST(point<BaseAST> &_stmt){
        stmt=std::move(_stmt);
    }
    std::string DumpKoopa() const override;
};

class StmtAST : public BaseAST {
public:

    bool Is_LVal;
    std::unique_ptr <BaseAST> num;
    std::unique_ptr <BaseAST> name;

    StmtAST(std::unique_ptr <BaseAST> &_ret_num) {
        num = std::move(_ret_num);
        Is_LVal= false;
    }

    StmtAST(point<BaseAST>& _num,point<BaseAST>& _lvalname){
        num = std::move(_num);
        name = std::move(_lvalname);
        Is_LVal= true;
    }
    std::string DumpAST() const override;

    std::string DumpKoopa() const override;
};

class NumberAST : public BaseAST {
public:
    int val;

    NumberAST(int _val) : val(_val) {}

    std::string DumpAST() const override;

    std::string DumpKoopa() const override;
    int Calc() const override{
        return val;
    }
};

class ExpAST: public BaseAST{
public:
    std::unique_ptr<BaseAST> unary_exp;
    ExpAST(std::unique_ptr<BaseAST> &_unary_exp){
        unary_exp = std::move(_unary_exp);
    }

    std::string DumpAST() const override;

    std::string DumpKoopa() const override;

    int Calc() const override{
        return unary_exp->Calc();
    }
};

class PrimaryExpAST :public BaseAST{
public:
    std::unique_ptr<BaseAST> p_exp;
    PrimaryExpAST(std::unique_ptr<BaseAST> &_p_exp){
        p_exp = std::move(_p_exp);
    }
    std::string DumpAST() const override;

    std::string DumpKoopa() const override;

    int Calc() const override{
        return p_exp->Calc();
    }
};

enum class UnaryOp{
    Positive=0,
    Negative,
    LogicalFalse
};
class UnaryExpAST :public BaseAST{
public:
    std::unique_ptr<BaseAST> u_exp;

    UnaryOp type;
    //默认是正，因为正不会影响其他的表达式
    UnaryExpAST(std::unique_ptr<BaseAST> &_u_exp,UnaryOp _type = UnaryOp::Positive){
        u_exp = std::move(_u_exp);
        type = _type;
    }

    std::string DumpAST() const override;

    std::string DumpKoopa() const override;

    int Calc() const override{
        switch (type) {
            case UnaryOp::Positive:
                return u_exp->Calc();
            case UnaryOp::Negative:
                return -u_exp->Calc();
            case UnaryOp::LogicalFalse:
                return !u_exp->Calc();
        }
    }
};

enum class MulType{
    NotMul,
    Mul,
    Div,
    Mod
};
class MulExpAST :public BaseAST{
public:
    std::unique_ptr<BaseAST>u_exp;
    std::unique_ptr<BaseAST> m_exp;
    MulType type = MulType::NotMul;
    MulExpAST(point<BaseAST> &_u_exp){
        u_exp=std::move(_u_exp);
    }
    MulExpAST(std::unique_ptr<BaseAST> &_u_exp,std::unique_ptr<BaseAST> &_m_exp,MulType _type ){
        type=_type;
        u_exp=std::move(_u_exp);
        m_exp=std::move(_m_exp);
    }
    std::string DumpAST() const override;

    std::string DumpKoopa() const override;

    int Calc() const override{
        switch (type) {
            case MulType::NotMul:
                return u_exp->Calc();
            case MulType::Mul:
                return m_exp->Calc()*u_exp->Calc();
            case MulType::Div:
                return m_exp->Calc()/u_exp->Calc();
            case MulType::Mod:
                return m_exp->Calc()%u_exp->Calc();
        }
    }

};

enum class AddType{
    NotAdd,
    Add,
    Sub
};

class AddExpAST :public BaseAST{
public:
    point<BaseAST> m_exp;
    point<BaseAST> a_exp;
    AddType type=AddType::NotAdd;
    AddExpAST(point<BaseAST>& _m_exp,point<BaseAST>& _a_exp,AddType _type = AddType::NotAdd){
        m_exp = std::move(_m_exp);
        a_exp = std::move(_a_exp);
        type=_type;
    }
    AddExpAST(point<BaseAST>& _m_exp){
        m_exp=std::move(_m_exp);
    }
    int Calc() const override{
        switch (type) {
            case AddType::NotAdd:
                return m_exp->Calc();
            case AddType::Add:
                return a_exp->Calc()+m_exp->Calc();
            case AddType::Sub:
                return a_exp->Calc()-m_exp->Calc();
        }
    }

    std::string DumpKoopa() const override;
};

enum class RelType{
    NoRel,
    Less,
    Bigger,
    LessEq,
    BiggerEq
};
class RelExpAST :public BaseAST{
public:
    point<BaseAST> a_exp;
    point<BaseAST> r_exp;
    RelType type=RelType::NoRel;
    RelExpAST(point<BaseAST>& _a_exp){
        a_exp = std::move(_a_exp);
    }

    RelExpAST(point<BaseAST>& _a_exp,point<BaseAST>& _r_exp, RelType _type){
        a_exp = std::move(_a_exp);
        r_exp = std::move(_r_exp);
        type=_type;
    }
    int Calc() const override{
        switch (type) {
            case RelType::NoRel:
                return a_exp->Calc();
            case RelType::Less:
                return r_exp->Calc()<a_exp->Calc();
            case RelType::Bigger:
                return r_exp->Calc()>a_exp->Calc();
            case RelType::LessEq:
                return r_exp->Calc()<=a_exp->Calc();
            case RelType::BiggerEq:
                return r_exp->Calc()>=a_exp->Calc();
        }
    }
    std::string DumpKoopa() const override;
};

enum class EqType{
    NoEq,
    Equal,
    NotEqual
};
class EqExpAST :public BaseAST{
public:
    point<BaseAST> e_exp;
    point<BaseAST> r_exp;
    EqType type=EqType::NoEq;

    EqExpAST(point<BaseAST>&_r_exp){
        r_exp = std::move(_r_exp);
    }

    EqExpAST(point<BaseAST>& _e_exp,point<BaseAST>& _r_exp,EqType _type){
        type = _type;
        e_exp = std::move(_e_exp);
        r_exp = std::move(_r_exp);
    }
    int Calc() const override{
        switch (type) {
            case EqType::NoEq:
                return r_exp->Calc();
            case EqType::Equal:
                return e_exp->Calc()==r_exp->Calc();
            case EqType::NotEqual:
                return e_exp->Calc()!=r_exp->Calc();
        }
    }
    std::string DumpKoopa() const override;
};

enum class AndOrType{
    NoLogic,
    And,
    Or
};

class LAndExpAST :public BaseAST{
public:
    point<BaseAST> l_exp;
    point<BaseAST> e_exp;
    AndOrType type = AndOrType::NoLogic;

    LAndExpAST(point<BaseAST>& _e_exp){
        e_exp = std::move(_e_exp);
    }

    LAndExpAST(point<BaseAST>& _l_exp,point<BaseAST>& _e_exp){
        type = AndOrType::And;
        l_exp = std::move(_l_exp);
        e_exp = std::move(_e_exp);
    }

    int Calc() const override{
        switch (type) {
            case AndOrType::NoLogic:
                return e_exp->Calc();
            case AndOrType::And:
                return l_exp->Calc()&&e_exp->Calc();
            default:
                assert(0);
        }
    }
    std::string DumpKoopa() const override;


};

class LOrExpAST:public BaseAST {
public:
    point<BaseAST> And_exp, Or_exp;
    AndOrType type = AndOrType::NoLogic;

    LOrExpAST(point<BaseAST> &_And_exp) {
        And_exp = std::move(_And_exp);
    }

    LOrExpAST(point<BaseAST>& _Or_exp,point<BaseAST>& _And_exp){
        type = AndOrType::Or;
        And_exp = std::move(_And_exp);
        Or_exp = std::move(_Or_exp);
    }

    int Calc() const override{
        switch (type) {
            case AndOrType::NoLogic:
                return And_exp->Calc();
            case AndOrType::Or:
                return Or_exp->Calc()||And_exp->Calc();
            default:
                assert(0);
        }
    }
    std::string DumpKoopa() const override;
};

class DeclAST :public BaseAST{
public:
    bool Is_Const;
    point<BaseAST> decl;
    DeclAST(point<BaseAST>& _decl, bool is_const){
        decl = std::move(_decl);
        Is_Const = is_const;
    }
    string DumpKoopa() const override;
};

//链表记录ConstDef
//初始化时直接插入符号表
class ConstDefAST :public BaseAST{
public:
    point<ConstDefAST> next;
    string name;
    point<BaseAST> exp;
    ConstDefAST(string _name,point<BaseAST>& _exp){
        name = _name;
        exp=std::move(_exp);

    }
    void AddItem(point<ConstDefAST>& _next){
        if (next == nullptr) {
            next = std::move(_next);
        } else {
            ConstDefAST* tmp=next.get();
            while (tmp->next!= nullptr){
                tmp=tmp->next.get();
            }
            tmp->next = std::move(_next);
        }
    }
    string DumpKoopa() const override;
};

class LValAST :public BaseAST{
public:
    string name;
    bool Is_Const;
    LValAST(string _name){
        name = _name;
    }

    //设置属性
    void SetIsConst(bool _is_const){
        Is_Const=_is_const;
    }

    int Calc() const override{
        symboltype reslt= GetLvalValue(VAL_MAP,name);
        //存入的应该是const
        assert(reslt.type==ValType::Const);
        return reslt.num;
    }

    string DumpKoopa() const override;
};

class VarDefAST:public BaseAST{
public:
    string name;
    point<BaseAST> value;
    point<BaseAST> next;

    VarDefAST(string _name, point<BaseAST>& _value){
        name = _name;
        value = std::move(_value);
    }
    VarDefAST(string _name){
        name = _name;
    }
    void AddItem(point<BaseAST>& _next){
        next = std::move(_next);
    }


    string DumpKoopa() const override;
};


class LEVal :public BaseAST{
public:
    string name;
    LEVal(string _name):name(_name){
        ;
    }
    string  DumpKoopa() const override;
};