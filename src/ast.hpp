#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <cstdlib>
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

    BlockAST(std::unique_ptr <BaseAST> &_stmt) {
        stmt = std::move(_stmt);
    }

    std::string DumpAST() const override;


    std::string DumpKoopa() const override;
};

class StmtAST : public BaseAST {
public:
    std::unique_ptr <BaseAST> ret_num;

    StmtAST(std::unique_ptr <BaseAST> &_ret_num) {
        ret_num = std::move(_ret_num);
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
};

class ExpAST: public BaseAST{
public:
    std::unique_ptr<BaseAST> unary_exp;
    ExpAST(std::unique_ptr<BaseAST> &_unary_exp){
        unary_exp = std::move(_unary_exp);
    }

    std::string DumpAST() const override;

    std::string DumpKoopa() const override;
};

class PrimaryExpAST :public BaseAST{
public:
    std::unique_ptr<BaseAST> p_exp;
    PrimaryExpAST(std::unique_ptr<BaseAST> &_p_exp){
        p_exp = std::move(_p_exp);
    }
    std::string DumpAST() const override;

    std::string DumpKoopa() const override;
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
    std::string DumpKoopa() const override;
};