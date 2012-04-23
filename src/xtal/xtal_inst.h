/** \file src/xtal/xtal_inst.h
* \brief src/xtal/xtal_inst.h
*/

#ifndef XTAL_INST_H_INCLUDE_GUARD
#define XTAL_INST_H_INCLUDE_GUARD

#pragma once

namespace xtal{

struct Inst;

typedef u8 inst_t;
typedef i8 inst_i8_t;
typedef u8 inst_u8_t;

template<class T, int Kind>
struct inst_16_t{
	u8 values[2];
	
	inst_16_t(){}

	inst_16_t(int value){
		values[0] = (u8)(value>>8);
		values[1] = (u8)(value>>0);
	}
	
	operator T() const{
		return (T)((values[0]<<8) | values[1]);
	}
};

typedef inst_16_t<i16, 0> inst_i16_t;
typedef inst_16_t<u16, 0> inst_u16_t;
typedef inst_16_t<i16, 1> inst_address_t;


inline int_t inst_inspect(i8 value, Inst*, const CodePtr&){ return (int_t)value; }
inline int_t inst_inspect(u8 value, Inst*, const CodePtr&){ return (int_t)value; }
inline int_t inst_inspect(inst_i16_t value, Inst*, const CodePtr&){ return (int_t)value; }
inline int_t inst_inspect(inst_u16_t value, Inst*, const CodePtr&){ return (int_t)value; }

int_t inst_inspect(inst_address_t& value, Inst* inst, const CodePtr& code);

StringPtr make_inst_string(const LongLivedString& InstName);

StringPtr make_inst_string(const LongLivedString& InstName, 
						const LongLivedString& MemberName1, int_t MemberValue1);

StringPtr make_inst_string(const LongLivedString& InstName, 
						const LongLivedString& MemberName1, int_t MemberValue1,
						const LongLivedString& MemberName2, int_t MemberValue2);

StringPtr make_inst_string(const LongLivedString& InstName, 
						const LongLivedString& MemberName1, int_t MemberValue1,
						const LongLivedString& MemberName2, int_t MemberValue2,
						const LongLivedString& MemberName3, int_t MemberValue3);
						
StringPtr make_inst_string(const LongLivedString& InstName, 
						const LongLivedString& MemberName1, int_t MemberValue1,
						const LongLivedString& MemberName2, int_t MemberValue2,
						const LongLivedString& MemberName3, int_t MemberValue3,
						const LongLivedString& MemberName4, int_t MemberValue4);

StringPtr make_inst_string(const LongLivedString& InstName, 
						const LongLivedString& MemberName1, int_t MemberValue1,
						const LongLivedString& MemberName2, int_t MemberValue2,
						const LongLivedString& MemberName3, int_t MemberValue3,
						const LongLivedString& MemberName4, int_t MemberValue4,
						const LongLivedString& MemberName5, int_t MemberValue5);

StringPtr inspect_range(const CodePtr& code, const inst_t* start, const inst_t* end);

int_t inst_size(uint_t no);

struct Inst{
	inst_t op;

	Inst(inst_t v = 0)
		:op(v){}

	operator const inst_t*() const{
		return &this->op;
	}

	template<class T>
	void checked_assign(T& ref, int v);
};

template<class T>
inline void Inst::checked_assign(T& ref, int v){
	ref = T(v);
	if(static_cast<int>(ref) != v){ 
		op = 0xff; 
	}
}

#define XTAL_DEF_INST_0(N, InstName) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst),\
			ISIZE = SIZE/sizeof(inst_t)\
		};\
		InstName(){\
			op = N;\
		}\
		StringPtr inspect(const CodePtr&){\
			return make_inst_string(\
					XTAL_STRING(#InstName));\
		}\
	}

#define XTAL_DEF_INST_1(N, InstName, MemberType1, MemberName1) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t)\
		};\
		MemberType1 MemberName1;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1){\
			op = N;\
			checked_assign(MemberName1, m1);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code));\
		}\
	}

#define XTAL_DEF_INST_2(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code));\
		}\
	}
	
#define XTAL_DEF_INST_3(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code));\
		}\
	}
	
#define XTAL_DEF_INST_4(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code),\
					XTAL_STRING(#MemberName4), inst_inspect(MemberName4, this, code));\
		}\
	}

#define XTAL_DEF_INST_5(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4, int m5){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
			checked_assign(MemberName5, m5);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code),\
					XTAL_STRING(#MemberName4), inst_inspect(MemberName4, this, code),\
					XTAL_STRING(#MemberName5), inst_inspect(MemberName5, this, code));\
		}\
	}

#define XTAL_DEF_INST_6(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5, MemberType6, MemberName6) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5) + sizeof(MemberType6),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4),\
			OFFSET_##MemberName6 = OFFSET_##MemberName5 + sizeof(MemberType5)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		MemberType6 MemberName6;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4, int m5, int m6){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
			checked_assign(MemberName5, m5);\
			checked_assign(MemberName6, m6);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code),\
					XTAL_STRING(#MemberName4), inst_inspect(MemberName4, this, code),\
					XTAL_STRING(#MemberName5), inst_inspect(MemberName5, this, code));\
		}\
	}

#define XTAL_DEF_INST_7(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5, MemberType6, MemberName6, MemberType7, MemberName7) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5) + sizeof(MemberType6) + sizeof(MemberType7),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4),\
			OFFSET_##MemberName6 = OFFSET_##MemberName5 + sizeof(MemberType5),\
			OFFSET_##MemberName7 = OFFSET_##MemberName6 + sizeof(MemberType6)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		MemberType6 MemberName6;\
		MemberType7 MemberName7;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4, int m5, int m6, int m7){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
			checked_assign(MemberName5, m5);\
			checked_assign(MemberName6, m6);\
			checked_assign(MemberName7, m7);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code),\
					XTAL_STRING(#MemberName4), inst_inspect(MemberName4, this, code),\
					XTAL_STRING(#MemberName5), inst_inspect(MemberName5, this, code));\
		}\
	}

#define XTAL_DEF_INST_8(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5, MemberType6, MemberName6, MemberType7, MemberName7, MemberType8, MemberName8) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5) + sizeof(MemberType6) + sizeof(MemberType7) + sizeof(MemberType8),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4),\
			OFFSET_##MemberName6 = OFFSET_##MemberName5 + sizeof(MemberType5),\
			OFFSET_##MemberName7 = OFFSET_##MemberName6 + sizeof(MemberType6),\
			OFFSET_##MemberName8 = OFFSET_##MemberName7 + sizeof(MemberType7)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		MemberType6 MemberName6;\
		MemberType7 MemberName7;\
		MemberType8 MemberName8;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4, int m5, int m6, int m7, int m8){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
			checked_assign(MemberName5, m5);\
			checked_assign(MemberName6, m6);\
			checked_assign(MemberName7, m7);\
			checked_assign(MemberName8, m8);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code),\
					XTAL_STRING(#MemberName4), inst_inspect(MemberName4, this, code),\
					XTAL_STRING(#MemberName5), inst_inspect(MemberName5, this, code));\
		}\
	}

#define XTAL_DEF_INST_9(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5, MemberType6, MemberName6, MemberType7, MemberName7, MemberType8, MemberName8, MemberType9, MemberName9) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5) + sizeof(MemberType6) + sizeof(MemberType7) + sizeof(MemberType8) + sizeof(MemberType9),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4),\
			OFFSET_##MemberName6 = OFFSET_##MemberName5 + sizeof(MemberType5),\
			OFFSET_##MemberName7 = OFFSET_##MemberName6 + sizeof(MemberType6),\
			OFFSET_##MemberName8 = OFFSET_##MemberName7 + sizeof(MemberType7),\
			OFFSET_##MemberName9 = OFFSET_##MemberName8 + sizeof(MemberType8)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		MemberType6 MemberName6;\
		MemberType7 MemberName7;\
		MemberType8 MemberName8;\
		MemberType8 MemberName9;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4, int m5, int m6, int m7, int m8, int m9){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
			checked_assign(MemberName5, m5);\
			checked_assign(MemberName6, m6);\
			checked_assign(MemberName7, m7);\
			checked_assign(MemberName8, m8);\
			checked_assign(MemberName9, m9);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code),\
					XTAL_STRING(#MemberName4), inst_inspect(MemberName4, this, code),\
					XTAL_STRING(#MemberName5), inst_inspect(MemberName5, this, code));\
		}\
	}

///////////////////////////////////////////////////////////////////////////////////////////////

/**
* \internal
* \brief 行が変わるごとに挟まれる命令
*
*/
XTAL_DEF_INST_0(0, InstLine);

enum{
	LOAD_NULL,
	LOAD_UNDEFINED,
	LOAD_FALSE,
	LOAD_TRUE
};

/**
* \internal
* \brief nullかundefinedかfalseかtrue値を取り出す
*
*/
XTAL_DEF_INST_2(1, InstLoadValue,
	inst_i8_t, result, // 値を代入するローカル変数番号
	inst_u8_t, value // nullかundefinedかfalseかtrue番号
);

/**
* \internal
* \brief 値テーブルから値を取り出す
*
*/
XTAL_DEF_INST_2(2, InstLoadConstant,
		inst_i8_t, result, // 値を代入するローカル変数番号
        inst_u16_t, value_number // 値を取り出すテーブル番号
);

/**
* \internal
* \brief ローカル変数にint値をプッシュする。
*
*/
XTAL_DEF_INST_2(3, InstLoadInt1Byte,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_i8_t, value
);

/**
* \internal
* \brief ローカル変数にfloat値をプッシュする。
*
*/
XTAL_DEF_INST_2(4, InstLoadFloat1Byte,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_i8_t, value
);

/**
* \internal
* \brief ローカル変数に現在実行中の関数をプッシュする。
*
*/
XTAL_DEF_INST_1(5, InstLoadCallee,
	inst_i8_t, result  // 値を代入するローカル変数番号
);

/**
* \internal
* \brief ローカル変数に現在のthisを入れる。
*
*/
XTAL_DEF_INST_1(6, InstLoadThis,
	inst_i8_t, result  // 値を代入するローカル変数番号
);

/**
* \internal
* \brief コピーする
*
*/
XTAL_DEF_INST_2(7, InstCopy,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, target // 値を取り出すローカル変数番号
);

XTAL_DEF_INST_3(8, InstInc,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, target, // 値を取り出すローカル変数番号
	inst_i8_t, stack_base);

XTAL_DEF_INST_3(9, InstDec,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, target, // 値を取り出すローカル変数番号
	inst_i8_t, stack_base);

XTAL_DEF_INST_3(10, InstPos,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, target, // 値を取り出すローカル変数番号
	inst_i8_t, stack_base);

XTAL_DEF_INST_3(11, InstNeg,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, target, // 値を取り出すローカル変数番号
	inst_i8_t, stack_base);

XTAL_DEF_INST_3(12, InstCom,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, target, // 値を取り出すローカル変数番号
	inst_i8_t, stack_base);

XTAL_DEF_INST_5(13, InstAdd,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_5(14, InstSub,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_5(15, InstCat,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_5(16, InstMul,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_5(17, InstDiv,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_5(18, InstMod,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_5(19, InstAnd,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_5(20, InstOr,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_5(21, InstXor,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_5(22, InstShl,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_5(23, InstShr,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_5(24, InstUshr,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base,
	inst_u8_t, assign
	);

XTAL_DEF_INST_4(25, InstAt,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, target, // 値を取り出すローカル変数番号
	inst_i8_t, index,
	inst_i8_t, stack_base
);

XTAL_DEF_INST_4(26, InstSetAt,
	inst_i8_t, target, // 値を取り出すローカル変数番号
	inst_i8_t, index,
	inst_i8_t, value,
	inst_i8_t, stack_base
);

/**
* \internal
* \brief 無条件分岐
*
*/
XTAL_DEF_INST_1(27, InstGoto,
        inst_address_t, address
);

XTAL_DEF_INST_2(28, InstNot,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_i8_t, target);

/**
* \internal
* \brief 条件分岐
*
*/
XTAL_DEF_INST_3(29, InstIf,
		inst_i8_t, target, // 値を取り出すローカル変数番号
        inst_address_t, address_true,
		inst_address_t, address_false
);

XTAL_DEF_INST_3(30, InstIfEq,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base
	);

XTAL_DEF_INST_3(31, InstIfLt,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base
	);

XTAL_DEF_INST_3(32, InstIfRawEq,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base);

XTAL_DEF_INST_3(33, InstIfIs,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base);

XTAL_DEF_INST_3(34, InstIfIn,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base
	);

XTAL_DEF_INST_3(35, InstIfUndefined,
	inst_i8_t, target, // 値を取り出すローカル変数番号
    inst_address_t, address_true,
	inst_address_t, address_false
);

XTAL_DEF_INST_1(36, InstIfDebug,
		inst_address_t, address);


XTAL_DEF_INST_1(37, InstPush,
	inst_i8_t, target // 値を取り出すローカル変数番号
);

XTAL_DEF_INST_1(38, InstPop,
	inst_i8_t, result);

/**
* \internal
* \brief 値の数を調整する
*
*/
XTAL_DEF_INST_3(39, InstAdjustValues,
		inst_u8_t, stack_base,
        inst_u8_t, result_count,
        inst_u8_t, need_result_count
);

/**
* \internal
* \brief ローカル変数を取り出す。
*
*/
XTAL_DEF_INST_3(40, InstLocalVariable,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_u16_t, number,
		inst_u8_t, depth
);

/**
* \internal
* \brief ローカル変数に値を設定する。
*
*/
XTAL_DEF_INST_3(41, InstSetLocalVariable,
		inst_i8_t, target,
		inst_u16_t, number,
		inst_u8_t, depth
);

/**
* \internal
* \brief インスタンス変数を取り出す。
*
*/
XTAL_DEF_INST_3(42, InstInstanceVariable,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_u8_t, number,
        inst_u16_t, info_number
);

/**
* \internal
* \brief インスタンス変数に値を設定する。
*
*/
XTAL_DEF_INST_3(43, InstSetInstanceVariable,
		inst_i8_t, value,
        inst_u8_t, number,
        inst_u16_t, info_number
);

/**
* \internal
* \brief インスタンス変数を取り出す。
*
*/
XTAL_DEF_INST_2(44, InstInstanceVariableByName,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_u16_t, identifier_number
);

/**
* \internal
* \brief インスタンス変数に値を設定する。
*
*/
XTAL_DEF_INST_2(45, InstSetInstanceVariableByName,
		inst_i8_t, value,
        inst_u16_t, identifier_number
);

/**
* \internal
* \brief ファイルローカル変数を取り出す。
*
*/
XTAL_DEF_INST_2(46, InstFilelocalVariable,
		inst_i8_t, result, // 値を代入するローカル変数番号
        inst_u16_t, value_number
);

/**
* \internal
* \brief ファイルローカル変数に値を設定する。
*
*/
XTAL_DEF_INST_2(47, InstSetFilelocalVariable,
		inst_i8_t, value,
        inst_u16_t, value_number
);

/**
* \internal
* \brief ファイルローカル変数を取り出す。
*
*/
XTAL_DEF_INST_2(48, InstFilelocalVariableByName,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_u16_t, identifier_number
);

/**
* \internal
* \brief ファイルローカル変数に値を設定する。
*
*/
XTAL_DEF_INST_2(49, InstSetFilelocalVariableByName,
		inst_i8_t, value,
        inst_u16_t, identifier_number
);

enum{
	CALL_FLAG_ARGS = 0,
	CALL_FLAG_THIS = 1,

	CALL_FLAG_ARGS_BIT = 1<<CALL_FLAG_ARGS,
	CALL_FLAG_THIS_BIT = 1<<CALL_FLAG_THIS
};

enum{
	MEMBER_FLAG_P = 2,
	MEMBER_FLAG_S = 3,
	MEMBER_FLAG_Q = 4,

	MEMBER_FLAG_P_BIT = 1<<MEMBER_FLAG_P,
	MEMBER_FLAG_S_BIT = 1<<MEMBER_FLAG_S,
	MEMBER_FLAG_Q_BIT = 1<<MEMBER_FLAG_Q
};

/**
* \internal
* \brief オブジェクトのメンバを取り出す。
*
*/
XTAL_DEF_INST_3(50, InstMember,
		inst_i8_t, result,  // 値を代入するローカル変数番号
		inst_i8_t, target, // 値を取り出すローカル変数番号
		inst_i16_t, primary
);

XTAL_DEF_INST_5(51, InstMemberEx,
		inst_i8_t, result,  // 値を代入するローカル変数番号
		inst_i8_t, target, // 値を取り出すローカル変数番号
		inst_i16_t, primary,
		inst_i8_t, secondary,
		inst_u8_t, flags
);

XTAL_DEF_INST_5(52, InstCall,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_u8_t, need_result,
		inst_i8_t, target, // 値を取り出すローカル変数番号
		inst_i8_t, stack_base,
        inst_u8_t, ordered
);

XTAL_DEF_INST_8(53, InstCallEx,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_u8_t, need_result,
		inst_i8_t, target, // 値を取り出すローカル変数番号
		inst_i8_t, self,
		inst_i8_t, stack_base,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, flags
);

XTAL_DEF_INST_7(54, InstSend,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_u8_t, need_result,
		inst_i8_t, target, // 値を取り出すローカル変数番号
		inst_i16_t, primary,
		inst_i8_t, secondary,
		inst_i8_t, stack_base,
        inst_u8_t, ordered
);

XTAL_DEF_INST_9(55, InstSendEx,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_u8_t, need_result,
		inst_i8_t, target, // 値を取り出すローカル変数番号
		inst_i16_t, primary,
		inst_i8_t, secondary,
		inst_i8_t, stack_base,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, flags
);

XTAL_DEF_INST_4(56, InstProperty,
		inst_i8_t, result,  // 値を代入するローカル変数番号
		inst_i8_t, target, // 値を取り出すローカル変数番号
        inst_i16_t, primary,
		inst_i8_t, stack_base
);

XTAL_DEF_INST_3(57, InstSetProperty,
		inst_i8_t, target, // 値を取り出すローカル変数番号
        inst_i16_t, primary,
		inst_i8_t, stack_base
);

XTAL_DEF_INST_1(58, InstScopeBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(59, InstScopeEnd);

/**
* \internal
* \brief 関数から抜ける
*
*/
XTAL_DEF_INST_2(60, InstReturn,
		inst_i8_t, base,
        inst_u8_t, result_count
);

/**
* \internal
* \brief fiberの実行を一時中断する
*
*/
XTAL_DEF_INST_4(61, InstYield,
		inst_i8_t, result,  // 値を代入するローカル変数番号
		inst_u8_t, need_result_count,
 		inst_u8_t, base,
	    inst_i8_t, result_count
);

/**
* \internal
* \brief 仮想マシンのループから脱出する。
*
*/
XTAL_DEF_INST_0(62, InstExit);

XTAL_DEF_INST_5(63, InstRange,
	inst_i8_t, result,  // 値を代入するローカル変数番号
	inst_u8_t, kind,
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base
);

XTAL_DEF_INST_3(64, InstOnce,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_address_t, address,
        inst_u16_t, value_number
);

XTAL_DEF_INST_2(65, InstSetOnce,
		inst_i8_t, target, // 値を取り出すローカル変数番号
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(66, InstMakeArray,
		inst_i8_t, result  // 値を代入するローカル変数番号
);

XTAL_DEF_INST_2(67, InstArrayAppend,
		inst_i8_t, target, // 値を取り出すローカル変数番号
		inst_i8_t, value
);

XTAL_DEF_INST_1(68, InstMakeMap,
		inst_i8_t, result  // 値を代入するローカル変数番号
);

XTAL_DEF_INST_3(69, InstMapInsert,
		inst_i8_t, target, // 値を取り出すローカル変数番号
		inst_i8_t, key,
		inst_i8_t, value
);

XTAL_DEF_INST_2(70, InstMapSetDefault,
		inst_i8_t, target, // 値を取り出すローカル変数番号
		inst_i8_t, value
);

XTAL_DEF_INST_2(71, InstClassBegin,
        inst_u16_t, info_number,
		inst_i8_t, mixin_base
);

XTAL_DEF_INST_1(72, InstClassEnd,
		inst_i8_t, result  // 値を代入するローカル変数番号
);

XTAL_DEF_INST_5(73, InstDefineClassMember,
        inst_u16_t, number,
		inst_u16_t, primary,
		inst_i8_t, secondary,
		inst_u8_t, accessibility,
		inst_i8_t, value
);

/**
* \internal
* \brief オブジェクトのメンバを定義する。
*
*/
XTAL_DEF_INST_5(74, InstDefineMember,
		inst_i8_t, target,
		inst_i16_t, primary,
		inst_i8_t, secondary,
		inst_u8_t, flags,
		inst_i8_t, value
);

XTAL_DEF_INST_3(75, InstMakeFun,
		inst_i8_t, result,  // 値を代入するローカル変数番号
        inst_u16_t, info_number,
		inst_address_t, address
);

XTAL_DEF_INST_4(76, InstMakeInstanceVariableAccessor,
		inst_i8_t, result,  // 値を代入するローカル変数番号
		inst_u8_t, type,
		inst_u8_t, number,
        inst_u16_t, info_number
);

XTAL_DEF_INST_1(77, InstTryBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(78, InstTryEnd);

XTAL_DEF_INST_1(79, InstPushGoto,
        inst_address_t, address
);

XTAL_DEF_INST_0(80, InstPopGoto);

XTAL_DEF_INST_0(81, InstThrow);

XTAL_DEF_INST_1(82, InstAssert,
		inst_i8_t, message);

XTAL_DEF_INST_0(83, InstBreakPoint);

XTAL_DEF_INST_0(84, InstMAX);

}

#endif // XTAL_INST_H_INCLUDE_GUARD
