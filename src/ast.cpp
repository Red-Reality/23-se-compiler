
//
// Created by leon on 2023/10/3.
//

#include "ast.hpp"
#include <string>
#include "sstream"

using namespace std;
/// 用于记录当前寄存器用到几号
/// %NAME_NUMBER 意思是下一个空着的变量符而不是最后一个已用的变量
int NAME_NUMBER = 0;

/// 用于记录每一层符号表被用过几次，避免变量重名
vector<int> VALMAP_LEVELREC;

//格式化生成运算式
std::string generate_binary_operation(int leftnum, int rightnum, string oper) {
    ostringstream tmp;
    tmp << "\t%" << NAME_NUMBER << " = " << oper << " %" << leftnum << ", %" << rightnum << endl;
    NAME_NUMBER++;
    return tmp.str();
}

std::string CompUnitAST::DumpAST() const {
    return "CompUnitAST { \n" + func_def->DumpAST() + "}";
}

std::string CompUnitAST::DumpKoopa() const {
    ostringstream oss;
    if (func_def != nullptr)
        oss << func_def->DumpKoopa();
    if (next != nullptr)
        oss << next->DumpKoopa();
    return oss.str();
}

std::string FuncDefAST::DumpAST() const {
    return "FuncDefAST { \n" + func_type->DumpAST() + ", " + "ident=" + ident + ", " + block->DumpAST() + " }";
}

std::string FuncDefAST::DumpKoopa() const {
    ostringstream oss;
    // 进入基本块存入一个0
    BLOCK_RET_RECORDER.push_back(0);

    // 为当前函数参数开一个新的变量域
    VALMAP_push();

    // 将函数名存入全局变量组中
    if (func_type->DumpKoopa() == ":i32") {
        VAL_MAP[0][ident] = {-3212345, ValType::IntFuncname};
//        cerr << "funcname is " << ident << endl;
    } else if (func_type->DumpKoopa() == "") {
        VAL_MAP[0][ident] = {-9912345, ValType::VoidFuncname};
//        cerr<<"funcname is "<<ident<<endl;
    } else {
        cerr << "unexpected FuncType in FuncDef" << endl;
        assert(0);
    }
    oss << "fun @" << ident << "(";
    if (FuncFParams != nullptr)
        oss << FuncFParams->DumpKoopa();
    oss << ")" << func_type->DumpKoopa() << "{\n" << "%entry:\n";
    if (FuncFParams != nullptr)
        oss << FuncFParams->ExtraOutput();
    oss << block->DumpKoopa();
    if(BLOCK_RET_RECORDER.back()==0)
        oss<<"\tret\n";
    oss << "}\n";

    BLOCK_RET_RECORDER.pop_back();

    // pop为函数参数开的map
    VAL_MAP.pop_back();
    return oss.str();
}

string FuncFParamsAST::DumpKoopa() const {
    ostringstream oss;
    oss << "@" << name->c_str() << ": i32";
    if (next != nullptr)
        oss <<", "<< next->DumpKoopa();
    return oss.str();
}

string FuncFParamsAST::ExtraOutput() const {
    ostringstream oss;
    int depth = VAL_MAP.size();
    assert(depth > 0);
    string tmpname = *name.get() + "_" + to_string(depth) + "_" + to_string(VALMAP_LEVELREC[depth - 1]);
    //一定是int
    oss << "\t@COMPILER_" << tmpname << " = alloc i32" << endl;
    unordered_map <string, symboltype> &lastMap = VAL_MAP.back();
    // 输出表达式的koopa
    oss << "\tstore @" << *name.get() << ", @COMPILER_" << tmpname << endl;
    //将变量的字符存入表中
    symboltype valstruct = {-7777777, ValType::Var};
    lastMap[*name.get()] = valstruct;
    if(next!= nullptr)
        oss<<next->ExtraOutput();
    return oss.str();
}

std::string FuncTypeAST::DumpAST() const {
    return "FuncTypeAST { \n" + name + " }";
}

std::string FuncTypeAST::DumpKoopa() const {
    if (name == "int")
        return std::string(":i32");
    else
        return "";
}

std::string BlockAST::DumpAST() const {

    string rslt = "BlockAST { \n" + stmt->DumpAST() + " }";

    return rslt;
}

std::string BlockItemAST::DumpKoopa() const {
    ostringstream oss;
    if (stmt != nullptr)
        oss << stmt->DumpKoopa();

    if (next != nullptr)
        oss << next->DumpKoopa();
    return oss.str();
}

std::string BlockAST::DumpKoopa() const {

    VALMAP_push();
    string rslt = "";
    if (stmt != nullptr)
        rslt += stmt->DumpKoopa();
    VAL_MAP.pop_back();

    return rslt;
}

std::string StmtAST::DumpAST() const {
    return "StmtAST { return, \n" + num->DumpAST() + " }";
}

std::string StmtAST::DumpKoopa() const {
    ostringstream oss;
    if (BLOCK_RET_RECORDER.back() != 0) {
        //当前block已经有ret或者jump或者br了
        return "";
    }
    if (type == StmtType::LValEqStmt || type == StmtType::ReturnStmt) {
        if (type == StmtType::ReturnStmt)
            if (BLOCK_RET_RECORDER.back() == 0) {
                BLOCK_RET_RECORDER.back() = 1;
                return num->DumpKoopa() + "\tret %" + to_string(NAME_NUMBER - 1) + '\n';
            } else
                return "";
        else {
            //求值
            oss << num->DumpKoopa();

            //存储
            oss << name->DumpKoopa();

            return oss.str();
        }
    } else if (type == StmtType::BlockStmt) {
        oss << num->DumpKoopa();
        return oss.str();
    }
    else if(type==StmtType::OneExp){
        if(num!= nullptr)
            oss<<num->DumpKoopa();
        return oss.str();
    }
    return "";

}

std::string DeclAST::DumpKoopa() const {
    // 常量的话只需要记录符号表不需要输出
    // 变量的话记录符号并在koopa中alloc
    return decl->DumpKoopa();
}

std::string ConstDefAST::DumpKoopa() const {
    unordered_map <string, symboltype> &lastMap = VAL_MAP.back();
    symboltype record = {exp->Calc(), ValType::Const};
    lastMap[name] = record;
    if (next != nullptr)
        next->DumpKoopa();
    return "";
}

std::string NumberAST::DumpAST() const {

    return "NumberAST { int \n" + std::to_string(val) + " }";
}

std::string NumberAST::DumpKoopa() const {
    //cout<<"Number:"<<NAME_NUMBER<<endl;
    return "\t%" + to_string(NAME_NUMBER++) + "= add 0, " + to_string(val) + "\n";
}

std::string ExpAST::DumpAST() const {
    return "ExpAST{ \n" + unary_exp->DumpAST() + "}";
}

std::string ExpAST::DumpKoopa() const {
    return unary_exp->DumpKoopa();
}

std::string PrimaryExpAST::DumpAST() const {
    return "PrimaryExpAST{  \n" + p_exp->DumpAST() + "}";
}

std::string PrimaryExpAST::DumpKoopa() const {
    return p_exp->DumpKoopa();
}

std::string UnaryExpAST::DumpAST() const {
    std::string oper;
    switch (type) {
        case UnaryOp::Positive:
            oper = "+";
            break;
        case UnaryOp::Negative:
            oper = "-";
            break;
        case UnaryOp::LogicalFalse:
            oper = "!";
            break;
        default:
            cerr << "unexpected unary exp,abort" << endl;
            assert(0);
    }

    return "UnaryExpAST{  \n" + oper + "(" + u_exp->DumpAST() + ")}";
}

std::string UnaryExpAST::DumpKoopa() const {
    std::string rslt = u_exp->DumpKoopa();
    switch (type) {
        case UnaryOp::Positive:
            return rslt;
        case UnaryOp::Negative:
            rslt += "\t%" + to_string(NAME_NUMBER) + " = sub 0, %" + to_string(NAME_NUMBER - 1) + "\n";
            NAME_NUMBER++;
            //cout<<"Negative:"<<NAME_NUMBER<<endl;
            return rslt;
        case UnaryOp::LogicalFalse:
            rslt += "\t%" + to_string(NAME_NUMBER) + " = eq 0, %" + to_string(NAME_NUMBER - 1) + "\n";
            NAME_NUMBER++;
            ///cout<<"LogicalFalse:"<<NAME_NUMBER<<endl;
            return rslt;
        case UnaryOp::FunctionCall:
            return rslt;
        default:
            cerr << "unexpected unary exp,abort" << endl;
            assert(0);

    }
}

std::string MulExpAST::DumpAST() const {

    if (type == MulType::NotMul) {
        return "MulExpAST{  \n" + u_exp->DumpAST() + "}";
    } else {
        string oper;
        switch (type) {
            case MulType::Mul:
                oper = "*";
                break;
            case MulType::Div:
                oper = "/";
                break;
            case MulType::Mod:
                oper = "%";
                break;
            default:
                break;
        }

        return "MulExpAST{  " + m_exp->DumpAST() + "\n" + oper + "\n" + u_exp->DumpAST() + "}";

    }
}

std::string MulExpAST::DumpKoopa() const {
    ostringstream rslt;
    if (type == MulType::NotMul) {
        return u_exp->DumpKoopa();
    } else {
        rslt << m_exp->DumpKoopa();
        int leftnum = NAME_NUMBER - 1;
        rslt << u_exp->DumpKoopa();
        int rightnum = NAME_NUMBER - 1;
        string oper;
        switch (type) {
            case MulType::Mul:
                oper = "mul";
                break;
            case MulType::Div:
                oper = "div";
                break;
            case MulType::Mod:
                oper = "mod";
                break;
            default:
                break;
        }
        rslt << generate_binary_operation(leftnum, rightnum, oper);

        return rslt.str();
    }

}

std::string AddExpAST::DumpKoopa() const {
    ostringstream reslt;
    if (type == AddType::NotAdd) {
        return m_exp->DumpKoopa();
    } else {
        string oper;
        if (type == AddType::Add) {
            oper = "add";
        } else if (type == AddType::Sub) {
            oper = "sub";
        }
        reslt << a_exp->DumpKoopa();
        int leftnum = NAME_NUMBER - 1;
        reslt << m_exp->DumpKoopa();
        int rightnum = NAME_NUMBER - 1;
        reslt << generate_binary_operation(leftnum, rightnum, oper);

        return reslt.str();
    }
}

std::string RelExpAST::DumpKoopa() const {
    string oper = "";
    switch (type) {
        case RelType::Less:
            oper = "lt";
            break;
        case RelType::Bigger:
            oper = "gt";
            break;
        case RelType::LessEq:
            oper = "le";
            break;
        case RelType::BiggerEq:
            oper = "ge";
            break;
        default:
            break;
    }
    ostringstream reslt;
    if (type == RelType::NoRel)
        return a_exp->DumpKoopa();
    else {
        reslt << r_exp->DumpKoopa();
        int leftnum = NAME_NUMBER - 1;
        reslt << a_exp->DumpKoopa();
        int rightnum = NAME_NUMBER - 1;
        reslt << generate_binary_operation(leftnum, rightnum, oper);
        return reslt.str();
    }
}

std::string EqExpAST::DumpKoopa() const {
    ostringstream reslt;
    string oper;
    switch (type) {
        case EqType::Equal:
            oper = "eq";
            break;
        case EqType::NotEqual:
            oper = "ne";
            break;
        default:
            break;
    }

    if (type == EqType::NoEq) {
        return r_exp->DumpKoopa();
    } else {
        reslt << e_exp->DumpKoopa();
        int leftnum = NAME_NUMBER - 1;
        reslt << r_exp->DumpKoopa();
        int rightnum = NAME_NUMBER - 1;
        reslt << generate_binary_operation(leftnum, rightnum, oper);
        return reslt.str();
    }
}

std::string LAndExpAST::DumpKoopa() const {
    ostringstream reslt;
    if (type == AndOrType::NoLogic)
        return e_exp->DumpKoopa();
    else {
        reslt << l_exp->DumpKoopa();

//        if(l_exp->Calc()==0)
//            return reslt.str();
        int leftnum = NAME_NUMBER - 1;
        reslt << e_exp->DumpKoopa();
        int rightnum = NAME_NUMBER - 1;

        reslt << "\t%" << NAME_NUMBER << " = eq 0, %" << leftnum << endl;
        //下一个leftnum需要更新
        leftnum = NAME_NUMBER, NAME_NUMBER++;

        reslt << "\t%" << NAME_NUMBER << " = eq 0, %" << rightnum << endl;
        //下一个rightnum需要更新
        rightnum = NAME_NUMBER, NAME_NUMBER++;

        //and 说明两个都为真才是真（在这里两个值都为0）
        //所以是left or right取反

        reslt << generate_binary_operation(leftnum, rightnum, "or");
        reslt << "\t%" << NAME_NUMBER << " = eq 0, %" << NAME_NUMBER - 1 << endl;
        NAME_NUMBER++;

        return reslt.str();
    }
}

std::string LOrExpAST::DumpKoopa() const {
    ostringstream reslt;
    if (type == AndOrType::NoLogic)
        return And_exp->DumpKoopa();
    else {

        reslt << Or_exp->DumpKoopa();
//        if(Or_exp->Calc()!=0)
//            return reslt.str();
        int leftnum = NAME_NUMBER - 1;
        reslt << And_exp->DumpKoopa();
        int rightnum = NAME_NUMBER - 1;

        //和0比较取布尔值
        reslt << "\t%" << NAME_NUMBER << " = eq 0, %" << leftnum << endl;
        //下一个leftnum需要更新
        leftnum = NAME_NUMBER, NAME_NUMBER++;

        reslt << "\t%" << NAME_NUMBER << " = eq 0, %" << rightnum << endl;
        //下一个rightnum需要更新
        rightnum = NAME_NUMBER, NAME_NUMBER++;

        //or即如果两个都为假（在这里值为1）则假否则为真
        //所以是leftnum and rightnum 然后取反

        reslt << generate_binary_operation(leftnum, rightnum, "and");
        reslt << "\t%" << NAME_NUMBER << " = eq 0, %" << NAME_NUMBER - 1 << endl;
        NAME_NUMBER++;
        return reslt.str();
    }
}

string LValAST::DumpKoopa() const {
    ostringstream oss;
    if (!HasName(VAL_MAP, name)) {
        cerr << "不存在的变量名" << endl;
        cerr << "现在的变量名为\"" << name << "\"" << endl;

        assert(0);
    }
    symboltype rslt = GetLvalValue(VAL_MAP, name);
    if (rslt.type == ValType::Const) {
        oss << "\t%" << NAME_NUMBER++ << "= add 0, " << Calc() << endl;
    } else if(rslt.type==ValType::Var) {
        int depth = retValDepth(name);
        assert(depth > 0);
        oss << "\t%" << NAME_NUMBER << "= load @COMPILER_" << name << "_" << depth << "_"
            << VALMAP_LEVELREC[depth - 1] << endl;
//        cerr<<"NAME_NUMBER is "<<NAME_NUMBER<<endl<<"Current Statment is"<<oss.str()<<endl;
        NAME_NUMBER++;
    }
    else{
        cerr<<"unexpected lval_type"<<endl;
        assert(0);
    }

    return oss.str();
}


string VarDefAST::DumpKoopa() const {
    ostringstream oss;
    // 记录当前深度用于处理变量名
    int depth = VAL_MAP.size();
    assert(depth > 0);
    if(IsGlobal)
        oss<<"global";
    string tmpname = name + "_" + to_string(depth) + "_" + to_string(VALMAP_LEVELREC[depth - 1]);
    //一定是int,默认初始化0(兼容全局变量)
    oss << "\t@COMPILER_" << tmpname << " = alloc i32";
    unordered_map <string, symboltype> &lastMap = VAL_MAP.back();
    if(!IsGlobal){
        if (value != nullptr) {
            oss<<endl;
            // 输出表达式的koopa
            oss << value->DumpKoopa();

            oss << "\tstore %" << NAME_NUMBER - 1 << ", @COMPILER_" << tmpname<<endl;
            //将变量的字符存入表中
            symboltype valstruct = {-7777777, ValType::Var};
            lastMap[name] = valstruct;
        } else {
            //默认值，用于报警
            symboltype valstruct = {-7777777, ValType::Var};
            lastMap[name] = valstruct;
        }
    } else{
        /// TODO: 可能存在bug

        if(value!= nullptr){
            oss<<", "<<value->Calc()<<endl;
            symboltype valstruct = {value->Calc(), ValType::Var};
            lastMap[name] = valstruct;
        }
        else{
            oss<<", zeroinit"<<endl;
            symboltype valstruct = {0, ValType::Var};
            lastMap[name] = valstruct;
        }
    }
    if (next != nullptr)
        oss << next->DumpKoopa();
    return oss.str();
}

string LEVal::DumpKoopa() const {
    // 在表中查找该值在第几层
    int depth = retValDepth(name);
    assert(depth > 0);
    //不用存值，直接输出koopa
    ostringstream oss;
    oss << "\tstore %" << NAME_NUMBER - 1 << ", @COMPILER_" << name << "_" << depth << "_" << VALMAP_LEVELREC[depth - 1]
        << endl;
    return oss.str();
}

string FuncCallAST::DumpKoopa() const {
    ostringstream oss;
    vector<int> regnum;
    if(paramlist!= nullptr){
        oss << paramlist->DumpKoopa();
        regnum= paramlist->CommunicateIntVector();
    }
    // 找到函数声明
    unordered_map <string, symboltype> &glob = VAL_MAP[0];
//    string selfname = *name.get();
//    cerr << selfname << endl << endl;
    assert(glob.count(*name.get()));
    symboltype tmp = glob[*name.get()];
    if (tmp.type == ValType::IntFuncname) {
        oss << "\t%" << NAME_NUMBER << " = call @" << name->c_str();
        NAME_NUMBER++;
    } else if (tmp.type == ValType::VoidFuncname) {
        oss << "\tcall @" << name->c_str();
    } else {
        cerr << "invalid operation in funccall,abort" << endl;
        assert(0);
    }

    oss << "(";
    for (int i = 0; i < regnum.size(); i++) {
        oss << "%" << regnum[i];
        if (i != regnum.size() - 1) {
            oss << ", ";
        }
    }
    oss << ")" << endl;
    return oss.str();
}

string FuncRParamsAST::DumpKoopa() const {
    ostringstream oss;
    oss << exp->DumpKoopa();
    this->paramregnum.push_back(NAME_NUMBER - 1);
    vector<int> otherregnum;
    if (next != nullptr) {
        oss << next->DumpKoopa();
        otherregnum = next->CommunicateIntVector();
    }

    paramregnum.insert(paramregnum.end(), otherregnum.begin(), otherregnum.end());
    return oss.str();
}

string GlobDeclAST::DumpKoopa() const {
    ostringstream oss;
    oss<<decllist->DumpKoopa();
    return oss.str();
}