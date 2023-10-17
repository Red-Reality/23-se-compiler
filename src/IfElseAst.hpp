//
// Created by leon on 2023/10/16.
//

#pragma once
#include "GlobalVar.h"
#include "ast.hpp"



class IfElseAST:public BaseAST{
public:
    // 条件
    point<BaseAST> sequence;
    // if执行语句
    point<BaseAST> ifexp;
    // else执行语句
    point<BaseAST> elseexp;
    IfElseAST(){
        ;
    }

    string DumpKoopa() const override;
};

class WhileAST:public BaseAST{
public:
    point<BaseAST> condition;
    point<BaseAST> stmt;
    WhileAST(){
        ;
    }
    string DumpKoopa() const override;
};

enum class ConOrBre{
    CONTINUE,
    BREAK
};

class ConBreStmt:public BaseAST{
public:
    ConOrBre type;
    ConBreStmt(){
        ;
    }
    string DumpKoopa() const override;
};


