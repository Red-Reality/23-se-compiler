//
// Created by leon on 2023/10/3.
//

/// TODO:重写生成，把map归一成<value_t,int>
#include "koopa.h"
#include <iostream>
#include "sstream"
#include <assert.h>
#include "map"
#include "iomanip"
#include "createRiscV.h"


using namespace std;


//ull代表的是tag==binary和tag==integer的rawvalue的地址,int是存放的 REG_NUMBER
//map<ull, int> value_reg_map;

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

        // 为函数申请特别大的栈空间存
        // 每个语句都可以分到一个4字节空间（无论他用不用)
        int length = 0;
        for (size_t tmp = 0; tmp < func->bbs.len; tmp++) {
            koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[tmp];
            length += bb->insts.len;
        }
        cout << "\taddi sp, sp, -" << length * 4 << endl;
        for (size_t j = 0; j < func->bbs.len; ++j) {
            assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);

            // 符号表，当前变量存在栈上
            unordered_map<koopa_raw_value_t , int> sympol_map;
            // 迭代器，指示当前栈没有数的最小偏移量
            int stackIt = 0;
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
                    BinVisit(value, sympol_map, stackIt);
                } else if (value->kind.tag == KOOPA_RVT_RETURN) {
                    //这部分之后还要改，暂时用特例写死,认为ret的数据一定存在栈底
                    cout << "\tlw a0, " << stackIt - 4 << "(sp)" << endl;
                    //恢复现场
                    cout << "\taddi sp, sp, " << length * 4 << endl;
                    cout << "\tret" << endl;

                } else if (value->kind.tag == KOOPA_RVT_STORE) {
                    // 处理store语句和alloc语句
                    StoreVisit(value, sympol_map, stackIt);
                } else if (value->kind.tag == KOOPA_RVT_LOAD) {
                    // 把该语句指向load的语句的栈偏移量
                        sympol_map[value] = sympol_map[value->kind.data.load.src];
                        cerr << "load addr(value):" << (ull) value << ", stit:" << sympol_map[value] << endl;
                        cerr<<"load addr(value) detail: src="<<(ull)value->kind.data.load.src<<", its stit = "<<sympol_map[value->kind.data.load.src]<<endl;
                        cerr<<"src's type is: "<<value->kind.tag<<endl;
//



                } else if (value->kind.tag == KOOPA_RVT_ALLOC) {
                    //不处理，store的时候才真的存这个数
                    continue;
                } else {
                    cerr << "unexpected statement, type is: " << value->kind.tag << endl;
                    assert(0);
                }

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
void BinVisit(const koopa_raw_value_t &valtmp, unordered_map<koopa_raw_value_t, int> &value_reg_map, int &stit) {

    // 左表达式就存在t0，右表达式就存在t1
    // 如果是0的话就取x0
    koopa_raw_binary_t oper = valtmp->kind.data.binary;
    int lhsreg = 0, rhsreg = 1;

    if (oper.lhs->kind.tag == KOOPA_RVT_INTEGER) {
        //如果是0就用x0寄存器
        if (oper.lhs->kind.data.integer.value == 0) {
            lhsreg = -1;
        } else {
            //调出立即值，不需要存栈里面
            cout << '\t' << setw(6) << "li " << pri_reg_name(lhsreg) << ", " << oper.lhs->kind.data.integer.value
                 << endl;
        }

    } else {
        assert(value_reg_map.find(oper.lhs)!=value_reg_map.end());
        //如果是表达式，取出栈上保存的值放入t0，不会用到其他寄存器
        cerr << "load lhs addr:" << (ull)  (oper.lhs) << ", stit="
             << value_reg_map[oper.lhs] << endl;
        cerr<<"lhs addr is"<<(ull)(oper.lhs)<<endl;
        cout << "\tlw t0," << value_reg_map[oper.lhs]  << "(sp)" << endl;
    }
    if (oper.rhs->kind.tag == KOOPA_RVT_INTEGER) {
        if (oper.rhs->kind.data.integer.value == 0) {
            rhsreg = -1;
        } else {
            cout << '\t' << left << setw(6) << "li " << pri_reg_name(rhsreg) << ", "
                 << oper.rhs->kind.data.integer.value
                 << endl;
        }
    } else {
        assert(value_reg_map.find(oper.rhs)!=value_reg_map.end());
        //如果是表达式，取出栈上保存的值放入t1，不会用到其他寄存器
        cerr << "load rhs addr:" << (ull) (oper.rhs) << ", stit="
             << value_reg_map[oper.rhs] << endl;
        cout << "\tlw t1," << value_reg_map[oper.rhs] << "(sp)" << endl;
    }
//    //在pa3中不应该存在其他可能
//    assert(lhsreg != -2);
//    assert(rhsreg != -2);

    // 存放结果的地方默认在t2
    const int REG_NUMBER = 2;
    //现在找到了两个数的寄存器
    //然后分类处理不同的运算符输出的不同的riscv语句
    switch (oper.op) {
        case koopa_raw_binary_op::KOOPA_RBO_NOT_EQ:
            //暂时只能解决表达式相等（实现了减法的值）
            pri_riscv_bin_order("sub", 3, REG_NUMBER, lhsreg, rhsreg);
            pri_riscv_bin_order("snez", 2, REG_NUMBER, REG_NUMBER);

            break;
        case koopa_raw_binary_op::KOOPA_RBO_EQ:
            //暂时只能解决表达式相等（实现了减法的值）
            pri_riscv_bin_order("sub", 3, REG_NUMBER, lhsreg, rhsreg);
            pri_riscv_bin_order("seqz", 2, REG_NUMBER, REG_NUMBER);
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
    // 把结果存入map中记录

    cout << "\tsw " << pri_reg_name(REG_NUMBER) << ", " << stit << "(sp)" << endl;
    value_reg_map[valtmp] = stit;
//    cerr << "oper addr=" << (ull) &valtmp << ", stackit= " << stit << endl;
    stit += 4;
}

void StoreVisit(const koopa_raw_value_t &obj, unordered_map<koopa_raw_value_t, int> &mymap, int &stit) {
    koopa_raw_store_t SaveObj = obj->kind.data.store;
    koopa_raw_value_t value = SaveObj.value;
    koopa_raw_value_t dest = SaveObj.dest;

    // 先看value，如果是立即数就直接赋值到t0
    // 如果是表达式就取出值
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
        cout << "\tli t0, " << value->kind.data.integer.value << endl;
    } else if (value->kind.tag == KOOPA_RVT_BINARY) {
        assert(mymap.find(value)!=mymap.end());
        cout << "\tlw t0, " << mymap[value] << "(sp)" << endl;
    } else {
        cerr << "unexpected type,type tag is:" << value->kind.tag << endl;
        assert(0);
    }

    //检查dest,看看有没有已经为他分配一个空间
    // 如果没有，就申请一片空间
    if (mymap.find(dest) == mymap.end()) {
        mymap[dest] = stit;
        stit += 4;
    }
//    cerr << "store addr=" << (ull) dest << ", stackit= " << mymap[dest] << endl;
    cout << "\tsw t0, " << mymap[dest] << "(sp)" << endl;
}