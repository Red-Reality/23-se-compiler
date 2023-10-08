//
// Created by leon on 2023/10/3.
//
#include "koopa.h"
#include <iostream>
#include "sstream"
#include <assert.h>
#include "map"
#include "iomanip"
#include "createRiscV.h"

using namespace std;
#define MAXREG 13
//指向下一个可用的寄存器，范围是0-12
static int REG_NUMBER = 0;
typedef unsigned long long ull;
//ull代表的是tag==binary和tag==integer的rawvalue的地址,int是存放的 REG_NUMBER
map<ull, int> value_reg_map;

// 输出寄存器号码
string pri_reg_name(int regnum) {
    ostringstream oss;
    if (regnum == -1) {
        //运算0
        return "x0";
    } else if (regnum < 7) {
        //t0-t6
        oss << "t" << regnum;
        return oss.str();
    } else if (regnum < 13) {
        //a2-a7
        oss << "a" << regnum - 6;
        return oss.str();
    } else {
        assert(0);
    }
}

// 在pa3中，暂时不考虑临时寄存器用完的问题
/// 将值存入map中的函数，返回需要输出的riscv语句
string set_value_in_register(koopa_raw_value_t l) {
    ostringstream oss;
    if (l->kind.tag == KOOPA_RVT_INTEGER) {
        //如果是0，那么用x0代表这个寄存器
        if (l->kind.data.integer.value == 0) {
            value_reg_map[(ull) l] = -1;
            return "";
        } else {
            value_reg_map[(ull) l] = REG_NUMBER;
            oss << "\tli  " << pri_reg_name(REG_NUMBER) << ", " << l->kind.data.integer.value << endl;
            return oss.str();
        }

    }
    return "";


}

void parse_string(const char *str) {

    // 解析字符串 str, 得到 Koopa IR 程序
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);
    assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
    // 创建一个 raw program builder, 用来构建 raw program
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    // 将 Koopa IR 程序转换为 raw program
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    // 释放 Koopa IR 程序占用的内存
    koopa_delete_program(program);

    cout << "   .text" << endl;

    for (size_t i = 0; i < raw.funcs.len; ++i) {
        // 正常情况下, 列表中的元素就是函数, 我们只不过是在确认这个事实
        // 当然, 你也可以基于 raw slice 的 kind, 实现一个通用的处理函数
        assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
        // 获取当前函数
        koopa_raw_function_t func = (koopa_raw_function_t) raw.funcs.buffer[i];

        cout << "   .globl " << func->name + 1 << endl;
        //排除掉第一个@号
        cout << func->name + 1 << ":" << endl;

        for (size_t j = 0; j < func->bbs.len; ++j) {
            assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
            koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[j];
            for (size_t k = 0; k < bb->insts.len; ++k) {
                koopa_raw_value_t value = (koopa_raw_value_t) bb->insts.buffer[k];
//                cerr << value->kind.tag << endl;

                //表达式类型：binary
                if (value->kind.tag == KOOPA_RVT_BINARY) {
//                    cerr << "lh kind:" << value->kind.data.binary.lhs->kind.tag << "\trh kind:"
//                         << value->kind.data.binary.rhs->kind.tag << endl;
//                    cerr << "lh:" << value->kind.data.binary.lhs->kind.data.integer.value << "\top:"
//                         << value->kind.data.binary.op << "\trhs:"
//                         << value->kind.data.binary.rhs->kind.data.integer.value << endl;
                    Visit(value->kind.data.binary);
                } else if (value->kind.tag == KOOPA_RVT_RETURN) {
                    //这部分之后还要改，暂时用特例写死,认为ret的上一步的寄存器一定是上一个

                    cout << "\t" << left << setw(6) << "mv" << "a0," << pri_reg_name((REG_NUMBER + MAXREG - 1) % MAXREG)
                         << endl;
                    cout<<"\tret"<<endl;

                }
                // 示例程序中, 你得到的 value 一定是一条 return 指令
                //assert(value->kind.tag == KOOPA_RVT_RETURN);
                // 于是我们可以按照处理 return 指令的方式处理这个 value
                // return 指令中, value 代表返回值
                //koopa_raw_value_t ret_value = value->kind.data.ret.value;
                // 示例程序中, ret_value 一定是一个 integer
                //assert(ret_value->kind.tag == KOOPA_RVT_INTEGER);
                // 于是我们可以按照处理 integer 的方式处理 ret_value
                // integer 中, value 代表整数的数值
                //int32_t int_val = ret_value->kind.data.integer.value;
                // 示例程序中, 这个数值一定是 0
                //assert(int_val == 0);
                //cout<<"   li "<<"a0 , "<<int_val<<endl;
                //cout<<"   ret"<<endl;
            }
            // ...
        }
        // ...
    }

    // 处理完成, 释放 raw program builder 占用的内存
    // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
    // 所以不要在 raw program builder 处理完毕之前释放 builder
    koopa_delete_raw_program_builder(builder);
}

//格式化输出运算式汇编指令
//order:指令名,ordernum：指令需要的寄存器个数，reg123:用到的寄存器
void pri_riscv_bin_order(const char *order, int ordernum, int reg1, int reg2 = -2, int reg3 = -2) {
    assert(ordernum < 4);
    cout << "\t" << left << setw(6) << order << pri_reg_name(reg1);

    if (ordernum >= 2) {
        cout << ", " << pri_reg_name(reg2);

    }
    if (ordernum == 3) {
        cout << ", " << pri_reg_name(reg3);

    }
    cout << endl;


}

//处理表达式
//要处理的是lhs,rhs,op
void Visit(const koopa_raw_binary_t &oper) {
    //首先获取lhs，rhs
    //如果是立即数，就存入寄存器里
    //注意，可能存在过深的调用导致覆盖了寄存器导致出错，但pa3暂时不考虑
    //clog << "start Visit" << endl;

    int lhsreg = -2, rhsreg = -2;
    if (oper.lhs->kind.tag == KOOPA_RVT_INTEGER) {
        //如果是0就用x0寄存器
        if (oper.lhs->kind.data.integer.value == 0) {
            value_reg_map[(ull) oper.lhs] = -1;
        } else {
            //申请一个寄存器
            cout << '\t' << setw(6) << "li " << pri_reg_name(REG_NUMBER) << ", " << oper.lhs->kind.data.integer.value
                 << endl;
            value_reg_map[(ull) oper.lhs] = REG_NUMBER;
            REG_NUMBER = (REG_NUMBER + 1) % MAXREG;
        }

        lhsreg = value_reg_map[(ull) oper.lhs];

    } else if (oper.lhs->kind.tag == KOOPA_RVT_BINARY) {
        //如果是表达式，取binary_t的地址来找出用的寄存器
        lhsreg = value_reg_map[(ull) (&oper.lhs->kind.data.binary)];
    }
    if (oper.rhs->kind.tag == KOOPA_RVT_INTEGER) {
        if (oper.rhs->kind.data.integer.value == 0) {
            value_reg_map[(ull) oper.rhs] = -1;
        } else {
            cout << '\t' << left << setw(6) << "li " << pri_reg_name(REG_NUMBER) << ", "
                 << oper.rhs->kind.data.integer.value
                 << endl;
            value_reg_map[(ull) oper.rhs] = REG_NUMBER;
            REG_NUMBER = (REG_NUMBER + 1) % MAXREG;
        }
        rhsreg = value_reg_map[(ull) oper.rhs];
    } else if (oper.rhs->kind.tag == KOOPA_RVT_BINARY) {
        rhsreg = value_reg_map[(ull) (&oper.rhs->kind.data.binary)];
    }
    //在pa3中不应该存在其他可能
    assert(lhsreg != -2);
    assert(rhsreg != -2);

    //现在找到了两个数的寄存器
    //然后分类处理不同的运算符输出的不同的riscv语句
    switch (oper.op) {
        case koopa_raw_binary_op::KOOPA_RBO_NOT_EQ:
            //暂时只能解决表达式相等（实现了减法的值）
            pri_riscv_bin_order("sub",3,REG_NUMBER,lhsreg,rhsreg);
            pri_riscv_bin_order("snez",2,REG_NUMBER,REG_NUMBER);

            break;
        case koopa_raw_binary_op::KOOPA_RBO_EQ:
            //暂时只能解决表达式相等（实现了减法的值）
            pri_riscv_bin_order("sub",3,REG_NUMBER,lhsreg,rhsreg);
            pri_riscv_bin_order("seqz",2,REG_NUMBER,REG_NUMBER);
            break;
        case koopa_raw_binary_op::KOOPA_RBO_GT:
            pri_riscv_bin_order("sgt", 3, REG_NUMBER, lhsreg, rhsreg);
            break;
        case koopa_raw_binary_op::KOOPA_RBO_LT:
            pri_riscv_bin_order("slt", 3, REG_NUMBER, lhsreg, rhsreg);
            break;
        case koopa_raw_binary_op::KOOPA_RBO_GE:
            //判断是不是小于，然后把值写入左式
            //如果是小于，那么eq 0命令就失效
            pri_riscv_bin_order("slt", 3, lhsreg, lhsreg, rhsreg);
            pri_riscv_bin_order("seqz", 2, REG_NUMBER, lhsreg);

            break;
        case koopa_raw_binary_op::KOOPA_RBO_LE:
            pri_riscv_bin_order("sgt", 3, lhsreg, lhsreg, rhsreg);
            pri_riscv_bin_order("seqz", 2, REG_NUMBER, lhsreg);
            break;
        case koopa_raw_binary_op::KOOPA_RBO_ADD:
            pri_riscv_bin_order("add", 3, REG_NUMBER, lhsreg, rhsreg);

            break;
        case koopa_raw_binary_op::KOOPA_RBO_SUB:
            pri_riscv_bin_order("sub", 3, REG_NUMBER, lhsreg, rhsreg);

            break;
        case koopa_raw_binary_op::KOOPA_RBO_MUL:
            pri_riscv_bin_order("mul", 3, REG_NUMBER, lhsreg, rhsreg);

            break;
        case koopa_raw_binary_op::KOOPA_RBO_DIV:
            pri_riscv_bin_order("div", 3, REG_NUMBER, lhsreg, rhsreg);

            break;
        case koopa_raw_binary_op::KOOPA_RBO_MOD:
            pri_riscv_bin_order("rem", 3, REG_NUMBER, lhsreg, rhsreg);

            break;
        case koopa_raw_binary_op::KOOPA_RBO_AND:
            pri_riscv_bin_order("and", 3, REG_NUMBER, lhsreg, rhsreg);

            break;
        case koopa_raw_binary_op::KOOPA_RBO_OR:
            pri_riscv_bin_order("or", 3, REG_NUMBER, lhsreg, rhsreg);

            break;
        default:
            //没实现的运算符
            assert(0);

    }
    //存储表达式结果至下一个寄存器中,用地址映射
    value_reg_map[(ull) (&oper)] = REG_NUMBER;
    REG_NUMBER = (REG_NUMBER + 1) % MAXREG;

    //clog << "end Visit" << endl;
}