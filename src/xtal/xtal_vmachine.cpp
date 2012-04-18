#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_stringspace.h"
#include "xtal_details.h"

namespace xtal{

#undef XTAL_USE_COMPUTED_GOTO

//#define XTAL_VM_DEF_INST(key) typedef const Inst##key InstType
//#define inst (*(InstType*)pc)
#define XTAL_VM_DEF_INST(key) const key& inst = *(const key*)pc; XTAL_UNUSED_VAR(inst)

#define XTAL_VM_RETURN return
#define XTAL_VM_THROW_EXCEPT(e) XTAL_VM_CONTINUE(push_except(pc, e))
#define XTAL_VM_CHECK_EXCEPT if(except_[0]){ XTAL_VM_CONTINUE(push_except(pc)); }

#ifdef XTAL_USE_COMPUTED_GOTO
#	define XTAL_COPY_LABEL_ADDRESS(key) &&Label##key
#	define XTAL_VM_CASE_FIRST(key) Label##key: { XTAL_VM_DEF_INST(key);
#	define XTAL_VM_CASE(key) } Label##key: { XTAL_VM_DEF_INST(key);
#	define XTAL_VM_LOOP vmloopbegin: goto *labels[*pc];
#	define XTAL_VM_LOOP_END }
#	define XTAL_VM_CONTINUE(x) { pc = (x); goto *labels[*pc]; }
#	define XTAL_VM_CONTINUE0 goto *labels[*pc]
#else
#	define XTAL_VM_CASE_FIRST(key) case key::NUMBER: { XTAL_VM_DEF_INST(key);
#	define XTAL_VM_CASE(key) } case key::NUMBER: /*printf("%s\n", #key);*/ { XTAL_VM_DEF_INST(key);
#	define XTAL_VM_LOOP switch(xop){
#	define XTAL_VM_LOOP_END } XTAL_NODEFAULT; }
#	define XTAL_VM_CONTINUE(x) { pc = (x); goto vmloopbegin; }
#	define XTAL_VM_CONTINUE0 goto vmloopbegin
#endif

#if defined(XTAL_NO_THREAD) || defined(XTAL_USE_THREAD_MODEL2)
#	define XTAL_CHECK_YIELD
#else
#	define XTAL_CHECK_YIELD if(--thread_yield_count_<0){ yield_thread(); thread_yield_count_ = 1000; }
#endif

#define XTAL_VM_FUN

const ClassPtr& Any::get_class_except_base() const{
	int_t t = XTAL_detail_type(*this);
	//if(t==TYPE_BASE){ return XTAL_detail_pvalue(*this)->get_class(); }
	if(t==TYPE_POINTER){ return cpp_class(value_.cpp_class_index()); }

	static CppClassSymbolData* const data[] = {
		&CppClassSymbol<Null>::value,
		&CppClassSymbol<Undefined>::value,
		&CppClassSymbol<Bool>::value,
		&CppClassSymbol<Bool>::value,
		&CppClassSymbol<Int>::value,
		&CppClassSymbol<Float>::value,
		&CppClassSymbol<ImmediateValue>::value,
		&CppClassSymbol<Any>::value,
		&CppClassSymbol<StatelessNativeMethod>::value,
		&CppClassSymbol<InstanceVariableGetter>::value,
		&CppClassSymbol<InstanceVariableSetter>::value,
		&CppClassSymbol<String>::value,
		&CppClassSymbol<String>::value,
		&CppClassSymbol<String>::value,
		&CppClassSymbol<Any>::value, //pading 0
		&CppClassSymbol<Any>::value, //pading 1
		&CppClassSymbol<Any>::value,
		&CppClassSymbol<String>::value,
		&CppClassSymbol<Array>::value,
		&CppClassSymbol<Values>::value,
		&CppClassSymbol<TreeNode>::value,
		&CppClassSymbol<NativeMethod>::value,
		&CppClassSymbol<NativeFun>::value,
		&CppClassSymbol<Method>::value,
		&CppClassSymbol<Fun>::value,
		&CppClassSymbol<Lambda>::value,
		&CppClassSymbol<Fiber>::value,
	};

	// 型を増やしたら変更すること
	XTAL_STATIC_ASSERT(sizeof(data)/sizeof(*data) == TYPE_MAX);

	return cpp_class(data[t]);
	
	/*
	CppClassSymbolData* data;
	switch(XTAL_detail_type(*this)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_NULL){ data = &CppClassSymbol<Null>::value; }
		XTAL_CASE(TYPE_UNDEFINED){ data = &CppClassSymbol<Undefined>::value; }
		XTAL_CASE(TYPE_FALSE){ data = &CppClassSymbol<Bool>::value; }
		XTAL_CASE(TYPE_TRUE){ data = &CppClassSymbol<Bool>::value; }
		XTAL_CASE(TYPE_INT){ data = &CppClassSymbol<Int>::value; }
		XTAL_CASE(TYPE_FLOAT){ data = &CppClassSymbol<Float>::value; }
		XTAL_CASE(TYPE_IMMEDIATE_VALUE){ data = &CppClassSymbol<ImmediateValue>::value; }
		XTAL_CASE(TYPE_POINTER){ return cpp_class(value_.cpp_class_index()); }
		XTAL_CASE(TYPE_STATELESS_NATIVE_METHOD){ data = &CppClassSymbol<StatelessNativeMethod>::value; }
		XTAL_CASE(TYPE_IVAR_GETTER){ data = &CppClassSymbol<InstanceVariableGetter>::value; }
		XTAL_CASE(TYPE_IVAR_SETTER){ data = &CppClassSymbol<InstanceVariableSetter>::value; }
		XTAL_CASE(TYPE_SMALL_STRING){ data = &CppClassSymbol<String>::value; }
		XTAL_CASE(TYPE_LONG_LIVED_STRING){ data = &CppClassSymbol<String>::value; }
		XTAL_CASE(TYPE_INTERNED_STRING){ data = &CppClassSymbol<String>::value; }
		XTAL_CASE(TYPE_PADDING_0){ data = &CppClassSymbol<Any>::value; }
		XTAL_CASE(TYPE_PADDING_1){ data = &CppClassSymbol<Any>::value; }
		XTAL_CASE(TYPE_BASE){ return XTAL_detail_pvalue(*this)->get_class(); }
		XTAL_CASE(TYPE_STRING){ data = &CppClassSymbol<String>::value; }
		XTAL_CASE(TYPE_ARRAY){ data = &CppClassSymbol<Array>::value; }
		XTAL_CASE(TYPE_VALUES){ data = &CppClassSymbol<Values>::value; }
		XTAL_CASE(TYPE_TREE_NODE){ data = &CppClassSymbol<TreeNode>::value; }
		XTAL_CASE(TYPE_NATIVE_METHOD){ data = &CppClassSymbol<NativeMethod>::value; }
		XTAL_CASE(TYPE_NATIVE_FUN){ data = &CppClassSymbol<NativeFun>::value; }
		XTAL_CASE(TYPE_METHOD){ data = &CppClassSymbol<Method>::value; }
		XTAL_CASE(TYPE_FUN){ data = &CppClassSymbol<Fun>::value; }
		XTAL_CASE(TYPE_LAMBDA){ data = &CppClassSymbol<Lambda>::value; }
		XTAL_CASE(TYPE_FIBER){ data = &CppClassSymbol<Fiber>::value; }
	}

	return cpp_class(data);
	*/
}

bool Any::is(const AnyPtr& klass) const{
	const ClassPtr& my_class = get_class();
	if(XTAL_detail_raweq(my_class, klass)) return true;
	return environment_->is_cache_table_.cache(my_class, klass);
}

bool Any::is(CppClassSymbolData* key) const{
	return is(cpp_class(key));
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const SmartPtr<Any>& p){
	XTAL_detail_inc_ref_count(p);
	XTAL_detail_dec_ref_count(*this);
	XTAL_detail_copy(*this, p);
	return *this;
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const NullPtr& p){
	XTAL_detail_dec_ref_count(*this);
	value_.init_primitive(TYPE_NULL);
	return *this;
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const UndefinedPtr& p){
	XTAL_detail_dec_ref_count(*this);
	value_.init_primitive(TYPE_UNDEFINED);
	return *this;
}

////////////////////////////////////////////////

void VMachine::carry_over(Method* fun, bool adjust_arguments){
	const inst_t* next_pc =  fun->source();
	if(*hook_setting_bit_!=0){
		check_breakpoint_hook(next_pc, to_smartptr(fun), BREAKPOINT_CALL_PROFILE);
		check_breakpoint_hook(next_pc, to_smartptr(fun), BREAKPOINT_CALL_LIGHT_WEIGHT);
		check_breakpoint_hook(next_pc, to_smartptr(fun), BREAKPOINT_INNER_CALL);
		check_breakpoint_hook(next_pc, to_smartptr(fun), BREAKPOINT_CALL);
	}

	FunFrame& f = XTAL_VM_ff();

	f.fun = fun; 
	f.outer = fun->outer_;
	f.code = fun->code_.get();
	f.identifiers = f.code->identifier_data(); 
	f.values = f.code->value_data();

	f.next_pc = next_pc;
	f.yieldable = f.poped_pc==&end_code_ ? false : XTAL_VM_prev_ff().yieldable;
	f.is_executed = 1;

	FunInfo* info = fun->info();
	
	if(adjust_arguments){
		int_t size = info->variable_size;
		if(f.ordered_arg_count!=size){
			adjust_values2(0, f.ordered_arg_count, size);
		}

		f.ordered_arg_count = size;
		f.named_arg_count = 0;
	}
	else{
		if(fun->extendable_param()){
			set_local_variable(info->variable_size-1, inner_make_arguments(fun));
		}
		else{
			if(f.ordered_arg_count!=fun->param_size()){
				adjust_args(fun);
			}
		}
		f.ordered_arg_count = f.named_arg_count = 0;
	}

	upsize_variables(info->max_variable);
	push_scope(info);
}

void VMachine::adjust_values2(int_t stack_base, int_t n, int_t need_result_count){
	adjust_values3(&XTAL_VM_local_variable(stack_base), n, need_result_count);
}

void VMachine::adjust_values3(AnyPtr* values, int_t src_count, int_t dest_count){

	// 関数が返す戻り値の数と要求している戻り値の数が等しい
	if(dest_count==src_count){
		return;
	}

	// 戻り値なんて要求してない
	if(dest_count==0){
		return;
	}

	// 関数が返す戻り値が一つも無いのでundefinedで埋める
	if(src_count==0){
		for(int_t i = 0; i<dest_count; ++i){
			values[i] = undefined;
		}
		return;
	}

	// この時点で、src_countもdest_countも1以上

	// 要求している戻り値の数の方が、関数が返す戻り値より少ない
	if(dest_count<src_count){

		// 余った戻り値を多値に直す。
		int_t size = src_count-dest_count+1;
		ValuesPtr ret;
		AnyPtr top = values[src_count-1];
		if(XTAL_detail_type(top)==TYPE_VALUES){
			ret = unchecked_ptr_cast<Values>(top);
		}
		else{
			ret = XNew<Values>(top);
		}

		for(int_t i=1; i<size; ++i){
			ret = XNew<Values>(values[src_count-1-i], ret);
		}

		values[dest_count-1] = ret;
	}
	else{
		// 要求している戻り値の数の方が、関数が返す戻り値より多い

		if(XTAL_detail_type(values[src_count-1])==TYPE_VALUES){
			// 最後の要素の多値を展開し埋め込む
			ValuesPtr mv = unchecked_ptr_cast<Values>(values[src_count-1]);

			const ValuesPtr* cur = &mv;
			int_t size = 0;
			while(true){
				if((*cur)->tail()){
					if(dest_count==src_count+size){
						values[src_count-1+size] = *cur;
						++size;
						break;
					}

					values[src_count-1+size] = (*cur)->head();
					++size;		
					cur = &(*cur)->tail();
				}
				else{
					values[src_count-1+size] = (*cur)->head();
					++size;		
					while(dest_count>=src_count+size){
						values[src_count-1+size] = undefined;
						++size;
					}
					break;
				}
			}
		}
		else{
			// 最後の要素が多値ではないので、undefinedで埋めとく
			for(int_t i = src_count; i<dest_count; ++i){
				values[i] = undefined;
			}
		}
	}
}

void VMachine::upsize_variables_detail(uint_t upsize){
	int_t top = XTAL_VM_variables_top();

	variables_.resize(top+upsize+130);
	variables_.resize(variables_.capacity());
	XTAL_VM_set_variables_top(top);

	for(uint_t i=0; i<scopes_.size(); ++i){
		Scope& scope = scopes_[i]; 
		if(scope.flags!=Scope::CLASS){
			XTAL_ASSERT(scope.pos+scope.size<=variables_.size());
			scopes_[i].frame->members_.attach((AnyPtr*)variables_.data()+scope.pos, scope.size);
		}
	}
}
	
FunFrame* VMachine::reserve_ff(){
	FunFrame* fp = fun_frames_.push();
	if(XTAL_UNLIKELY(!fp)){ 
		fun_frames_.top() = fp = new_object_xmalloc<FunFrame>();
	}
	fun_frames_.downsize(1);
	return fp;
}

void VMachine::push_ff(CallState& call_state){
	FunFrame* fp = fun_frames_.push();
	if(XTAL_UNLIKELY(!fp)){ 
		fun_frames_.top() = fp = new_object_xmalloc<FunFrame>();
	}

	FunFrame& f = *fp;
	f.is_executed = 0;
	f.need_result_count = call_state.need_result_count;
	f.ordered_arg_count = call_state.ordered;
	f.named_arg_count = call_state.named;
	f.next_pc = 0;
	f.poped_pc = call_state.next_pc;
	f.result = call_state.result;
	f.prev_stack_base = XTAL_VM_variables_top();
	f.scope_lower = scopes_.size();
	f.self = ap(call_state.self);
	f.fun = null;
	f.outer = null;

	result_base_ = f.result;

	// 引数がXTAL_VM_local_variable(0)で取得できるようにする
	upsize_variables(call_state.stack_base);
}

const inst_t* VMachine::pop_ff(int_t base, int_t result_count){
	FunFrame& f = *fun_frames_.top();
	int_t src_base = base + XTAL_VM_variables_top();
	XTAL_VM_set_variables_top(f.prev_stack_base);
	result_base_ = f.result;

	// 戻り値の値を調節する
	if(f.need_result_count!=result_count){
		XTAL_VM_LOCK{
			adjust_values3(variables_.data() + src_base, result_count, f.need_result_count);
		}
	}

	// 戻り値をコピーする
	for(uint_t i=0, sz=f.need_result_count; i<sz; ++i){
		XTAL_VM_LOCK{
			set_local_variable(result_base_ + i, variables_.at(src_base + i));
		}
	}

	XTAL_VM_LOCK{
		f.fun = null;
		f.outer = null;
		f.self = null;
	}

	fun_frames_.downsize(1);
	fun_frames_.top()->is_executed = 2;
	return fun_frames_.top()->next_pc = f.poped_pc; 
}

void VMachine::pop_ff_non(){
	FunFrame& f = *fun_frames_.top();

	f.fun = null;
	f.outer = null;
	f.self = null;

	fun_frames_.downsize(1);
	fun_frames_.top()->is_executed = 2;
	fun_frames_.top()->next_pc = f.poped_pc; 
}

void VMachine::push_scope(ScopeInfo* info){
	Scope& scope = scopes_.push();
	if(!scope.frame){
		scope.frame = xnew<Frame>();
		scope.frame->unset_orphan();
	}

	scope.pos = XTAL_VM_variables_top() - info->variable_offset;
	scope.size = info->variable_size;
	scope.flags = Scope::NONE;

	scope.frame->attach(info, XTAL_VM_ff().code, (AnyPtr*)variables_.data()+scope.pos, scope.size);
}

void VMachine::pop_scope(){
	Scope& scope = scopes_.top();
	if(scope.flags==Scope::NONE){
		scope.frame->detach();
	}
	else{
		if(scope.flags==Scope::FRAME){
			scope.frame->members_.reflesh();
		}

		scope.frame->set_orphan();
		scope.frame = null;
	}

	scopes_.downsize(1);
}

const FramePtr& VMachine::make_outer_outer(uint_t i, uint_t call_n, bool force){
	int_t scope_upper = call_n==0 ? scopes_.size() : fun_frames_[call_n-1]->scope_lower;

	FunFrame& ff = *fun_frames_[call_n];
	if(i < scope_upper-ff.scope_lower){
		Scope& scope = scopes_.reverse_at(scope_upper - i - 1);
		if(scope.flags==Scope::NONE){
			scope.flags = Scope::FRAME;
			if(force || scope.frame->info()->flags&ScopeInfo::FLAG_SCOPE_CHAIN){
				scope.frame->set_outer(make_outer_outer(i+1, call_n, force));
			}
		}
		return scope.frame;
	}

	if(ff.fun){
		return ff.outer;
	}

	return nul<Frame>();
}

const FramePtr& VMachine::make_outer(ScopeInfo* scope){
	if(debug::is_enabled() || scope->flags&ScopeInfo::FLAG_SCOPE_CHAIN){
		return make_outer_outer(0, 0, debug::is_enabled());
	}
	return nul<Frame>();
}

void VMachine::set_local_variable_out_of_fun(uint_t pos, uint_t depth, const AnyPtr& value){
	uint_t size = scopes_.size()-XTAL_VM_ff().scope_lower;
	if(depth<size){
		scopes_[depth].frame->set_member_direct(pos, value);
		return;
	}

	depth -= size;

	Frame* out = XTAL_VM_ff().outer.get();
	while(depth){
		out = out->outer().get();
		depth--;
	}

	out->set_member_direct(pos, value);
}

AnyPtr& VMachine::local_variable_out_of_fun(uint_t pos, uint_t depth){
	uint_t size = scopes_.size()-XTAL_VM_ff().scope_lower;
	if(depth<size){
		return scopes_[depth].frame->member_direct(pos);
	}

	depth -= size;

	Frame* out = XTAL_VM_ff().outer.get();
	while(depth){
		out = out->outer().get();
		depth--;
	}

	return out->member_direct(pos);
}

void VMachine::execute_inner(const inst_t* start, int_t eval_n){
	XTAL_VM_UNLOCK{
		ExceptFrame cur;
		cur.info = 0;
		cur.stack_size = stack_.size() - (XTAL_VM_ff().ordered_arg_count+(XTAL_VM_ff().named_arg_count<<1));
		cur.fun_frame_size = fun_frames_.size();
		cur.scope_size = scopes_.size();
		cur.variables_top = XTAL_VM_variables_top();

		hook_setting_bit_ = debug::hook_setting_bit_ptr();

		execute_inner2(start, eval_n, cur);
	}
}

void VMachine::execute_inner2(const inst_t* start, int_t eval_n, ExceptFrame& cur){
	register const inst_t* pc = start;
	const void* next_pc = 0;

	static Any values[4] = { null, undefined, Bool(false), Bool(true) };

	CallState call_state;

	int_t eval_base_n = fun_frames_.size();

	inst_t xop = 0;

	int_t iprimary = 0;

	uint_t common_flag = 0;
	int_t accessibility = 0;

	XTAL_ASSERT(cur.stack_size>=0);
	
#ifdef XTAL_USE_COMPUTED_GOTO
	static const void* labels[] = {
	//{LABELS{{
		XTAL_COPY_LABEL_ADDRESS(InstLine),
		XTAL_COPY_LABEL_ADDRESS(InstLoadValue),
		XTAL_COPY_LABEL_ADDRESS(InstLoadConstant),
		XTAL_COPY_LABEL_ADDRESS(InstLoadInt1Byte),
		XTAL_COPY_LABEL_ADDRESS(InstLoadFloat1Byte),
		XTAL_COPY_LABEL_ADDRESS(InstLoadCallee),
		XTAL_COPY_LABEL_ADDRESS(InstLoadThis),
		XTAL_COPY_LABEL_ADDRESS(InstCopy),
		XTAL_COPY_LABEL_ADDRESS(InstMember),
		XTAL_COPY_LABEL_ADDRESS(InstUna),
		XTAL_COPY_LABEL_ADDRESS(InstArith),
		XTAL_COPY_LABEL_ADDRESS(InstBitwise),
		XTAL_COPY_LABEL_ADDRESS(InstAt),
		XTAL_COPY_LABEL_ADDRESS(InstSetAt),
		XTAL_COPY_LABEL_ADDRESS(InstGoto),
		XTAL_COPY_LABEL_ADDRESS(InstNot),
		XTAL_COPY_LABEL_ADDRESS(InstIf),
		XTAL_COPY_LABEL_ADDRESS(InstIfComp),
		XTAL_COPY_LABEL_ADDRESS(InstIfRawEq),
		XTAL_COPY_LABEL_ADDRESS(InstIfIs),
		XTAL_COPY_LABEL_ADDRESS(InstIfUndefined),
		XTAL_COPY_LABEL_ADDRESS(InstIfDebug),
		XTAL_COPY_LABEL_ADDRESS(InstPush),
		XTAL_COPY_LABEL_ADDRESS(InstPop),
		XTAL_COPY_LABEL_ADDRESS(InstAdjustValues),
		XTAL_COPY_LABEL_ADDRESS(InstLocalVariable),
		XTAL_COPY_LABEL_ADDRESS(InstSetLocalVariable),
		XTAL_COPY_LABEL_ADDRESS(InstInstanceVariable),
		XTAL_COPY_LABEL_ADDRESS(InstSetInstanceVariable),
		XTAL_COPY_LABEL_ADDRESS(InstInstanceVariableByName),
		XTAL_COPY_LABEL_ADDRESS(InstSetInstanceVariableByName),
		XTAL_COPY_LABEL_ADDRESS(InstFilelocalVariable),
		XTAL_COPY_LABEL_ADDRESS(InstSetFilelocalVariable),
		XTAL_COPY_LABEL_ADDRESS(InstFilelocalVariableByName),
		XTAL_COPY_LABEL_ADDRESS(InstSetFilelocalVariableByName),
		XTAL_COPY_LABEL_ADDRESS(InstCall),
		XTAL_COPY_LABEL_ADDRESS(InstSend),
		XTAL_COPY_LABEL_ADDRESS(InstProperty),
		XTAL_COPY_LABEL_ADDRESS(InstSetProperty),
		XTAL_COPY_LABEL_ADDRESS(InstScopeBegin),
		XTAL_COPY_LABEL_ADDRESS(InstScopeEnd),
		XTAL_COPY_LABEL_ADDRESS(InstReturn),
		XTAL_COPY_LABEL_ADDRESS(InstYield),
		XTAL_COPY_LABEL_ADDRESS(InstExit),
		XTAL_COPY_LABEL_ADDRESS(InstRange),
		XTAL_COPY_LABEL_ADDRESS(InstOnce),
		XTAL_COPY_LABEL_ADDRESS(InstSetOnce),
		XTAL_COPY_LABEL_ADDRESS(InstMakeArray),
		XTAL_COPY_LABEL_ADDRESS(InstArrayAppend),
		XTAL_COPY_LABEL_ADDRESS(InstMakeMap),
		XTAL_COPY_LABEL_ADDRESS(InstMapInsert),
		XTAL_COPY_LABEL_ADDRESS(InstMapSetDefault),
		XTAL_COPY_LABEL_ADDRESS(InstClassBegin),
		XTAL_COPY_LABEL_ADDRESS(InstClassEnd),
		XTAL_COPY_LABEL_ADDRESS(InstDefineClassMember),
		XTAL_COPY_LABEL_ADDRESS(InstDefineMember),
		XTAL_COPY_LABEL_ADDRESS(InstMakeFun),
		XTAL_COPY_LABEL_ADDRESS(InstMakeInstanceVariableAccessor),
		XTAL_COPY_LABEL_ADDRESS(InstTryBegin),
		XTAL_COPY_LABEL_ADDRESS(InstTryEnd),
		XTAL_COPY_LABEL_ADDRESS(InstPushGoto),
		XTAL_COPY_LABEL_ADDRESS(InstPopGoto),
		XTAL_COPY_LABEL_ADDRESS(InstThrow),
		XTAL_COPY_LABEL_ADDRESS(InstAssert),
		XTAL_COPY_LABEL_ADDRESS(InstBreakPoint),
		XTAL_COPY_LABEL_ADDRESS(InstMAX),
//}}LABELS}
		};
#endif

	XTAL_CHECK_YIELD;

XTAL_VM_CONTINUE0;

	{
zerodiv:
		XTAL_VM_LOCK{
			set_runtime_error(Xt("XRE1024"), to_smartptr(this));
			XTAL_VM_CONTINUE(&throw_code_);
		}
	}

	enum{
		SEND = 0,
		PROPERTY = 1,
		SET_PROPERTY = 2,
		MEMBER = 3
	};

	{
send_comp:
		{
			const InstIfEq& inst = *(const InstIfEq*)next_pc;
			typedef InstIf InstType2; 
			InstType2& inst2 = *(InstType2*)((u8*)next_pc+inst.ISIZE);
			set_local_variable(inst.stack_base, XTAL_VM_local_variable(inst.rhs));
			call_state.set(pc, pc+inst.ISIZE, inst2.target, 1, inst.stack_base, 1, 0, MEMBER_FLAG_Q_BIT);
			call_state.target = XTAL_VM_local_variable(inst.lhs);
			goto send_common_iprimary_nosecondary;
		}

send_una:
		{
			const InstInc& inst = *(const InstInc*)next_pc;
			call_state.set(pc, pc+inst.ISIZE, inst.result, 1, inst.stack_base, 0, 0, 0);
			call_state.target = XTAL_VM_local_variable(inst.target);
			goto send_common_iprimary_nosecondary;
		}

send_bin:
		{
			const InstAdd& inst = *(const InstAdd*)next_pc;
			set_local_variable(inst.stack_base, XTAL_VM_local_variable(inst.rhs));
			call_state.set(pc, pc+inst.ISIZE, inst.result, 1, inst.stack_base, 1, 0, 0);
			call_state.target = XTAL_VM_local_variable(inst.lhs);
		}

send_common_iprimary_nosecondary:
		XTAL_VM_LOCK{
			call_state.primary = fetch_defined_id(iprimary);
		}

send_common_nosecondary:
		call_state.secondary = undefined;
		call_state.self = XTAL_VM_ff().self;
		XTAL_VM_LOCK{			
			accessibility = 0;
			call_state.cls = call_state.target.get_class();
			call_state.member = environment_->member_cache_table_.cache(XTAL_detail_pvalue(call_state.cls), (IDPtr&)call_state.primary, accessibility);
		}
		common_flag = SEND;
		goto send_common4;

send_common:
		XTAL_VM_LOCK{
			call_state.cls = call_state.target.get_class();
		}
		common_flag = SEND;

send_common2:
		XTAL_VM_LOCK{			
			accessibility = 0;
			call_state.member = environment_->member_cache_table2_.cache(XTAL_detail_pvalue(call_state.cls), (IDPtr&)call_state.primary, ap(call_state.secondary), accessibility);
		}

send_common4:

		XTAL_VM_LOCK{
			
			if(accessibility){
				if(const inst_t* epc = check_accessibility(call_state, accessibility)){
					XTAL_VM_CONTINUE(epc);
				}
			}

			switch(common_flag){
				XTAL_NODEFAULT;

				XTAL_CASE(SEND){

				}

				XTAL_CASE(PROPERTY){
					if(XTAL_detail_type(call_state.member)==TYPE_IVAR_GETTER){
						InstanceVariableGetter* p = unchecked_cast<InstanceVariableGetter*>(ap(call_state.member));
						set_local_variable(call_state.result, call_state.target.instance_variables()->variable(p->number(), p->class_info()));
						XTAL_VM_CONTINUE(call_state.next_pc);
					}
				}

				XTAL_CASE(SET_PROPERTY){
					if(XTAL_detail_type(call_state.member)==TYPE_IVAR_SETTER){
						InstanceVariableSetter* p = unchecked_cast<InstanceVariableSetter*>(ap(call_state.member));
						call_state.target.instance_variables()->set_variable(p->number(), p->class_info(), XTAL_VM_local_variable(call_state.stack_base));
						XTAL_VM_CONTINUE(call_state.next_pc);
					}
				}

				XTAL_CASE(MEMBER){
					if(!XTAL_detail_is_undefined(call_state.member)){
						set_local_variable(call_state.result, ap(call_state.member));
						XTAL_VM_CONTINUE(call_state.next_pc);
					}
				}
			}
		}

		if(XTAL_detail_is_undefined(call_state.member)){
send_common3:
			XTAL_VM_LOCK{
				XTAL_VM_CHECK_EXCEPT;

				if(call_state.flags&MEMBER_FLAG_Q_BIT){
					for(int_t i=0; i<call_state.need_result_count; ++i){
						set_local_variable(call_state.result+i, undefined);
					}
					XTAL_VM_CONTINUE(call_state.next_pc);
				}

				XTAL_VM_CONTINUE(push_except(call_state.pc, unsupported_error(ap(call_state.cls), (IDPtr&)call_state.primary, ap(call_state.secondary))));
			}
		}

		call_state.self = call_state.target;
	}

	{
call_common:
		if(call_state.flags&CALL_FLAG_ARGS_BIT){
			XTAL_VM_LOCK{
				if(ArgumentsPtr args = ptr_cast<Arguments>(XTAL_VM_local_variable(call_state.stack_base+call_state.ordered+call_state.named*2))){
					push_args(args, call_state.stack_base, call_state.ordered, call_state.named);
					call_state.ordered += args->ordered_size();
					call_state.named += args->named_size();
				}
			}
		}

call_common2:
		XTAL_VM_LOCK{
			push_ff(call_state);
			
			switch(XTAL_detail_type(call_state.member)){
				case TYPE_FUN:
					set_arg_this(unchecked_cast<Fun*>(ap(call_state.member))->self());

					// スルー

				case TYPE_METHOD:{
					Method* p = unchecked_cast<Method*>(ap(call_state.member));
					if(call_state.ordered!=p->info()->max_param_count){
						if(!p->check_arg(to_smartptr(this))){
							break;
						}
					}
					carry_over(p);
					goto call_end;
				}

				XTAL_DEFAULT{
					ap(call_state.member)->rawcall(to_smartptr(this));
				}

				XTAL_CASE(TYPE_BASE){ 
					XTAL_detail_pvalue(ap(call_state.member))->rawcall(to_smartptr(this)); 
				}
				
				XTAL_CASE(TYPE_STATELESS_NATIVE_METHOD){ 
					unchecked_ptr_cast<StatelessNativeMethod>(ap(call_state.member))->on_rawcall(to_smartptr(this)); 
				}

				XTAL_CASE(TYPE_NATIVE_METHOD){ 
					unchecked_ptr_cast<NativeMethod>(ap(call_state.member))->on_rawcall(to_smartptr(this)); 
				}

				XTAL_CASE(TYPE_NATIVE_FUN){ 
					unchecked_ptr_cast<NativeFun>(ap(call_state.member))->on_rawcall(to_smartptr(this)); 
				}

				XTAL_CASE(TYPE_LAMBDA){ 
					unchecked_ptr_cast<Lambda>(ap(call_state.member))->on_rawcall(to_smartptr(this)); 
				}

				XTAL_CASE(TYPE_FIBER){ 
					unchecked_ptr_cast<Fiber>(ap(call_state.member))->on_rawcall(to_smartptr(this)); 
				}
			}

			if(XTAL_VM_ff().is_executed==0){
				if(except_[0]){ 
					XTAL_VM_CONTINUE(push_except(call_state.pc, except_[0]));
				}

				XTAL_VM_CONTINUE(push_except(call_state.pc, unsupported_error(ap(call_state.member)->get_class(), fetch_defined_id(DefinedID::id_op_call), undefined)));
			}
		}

call_end:
		if(except_[0]){
			const inst_t* next_pc;
			XTAL_VM_LOCK{
				next_pc = push_except(XTAL_VM_ff().next_pc);
			}
			XTAL_VM_CONTINUE(next_pc);
		}
		else{
			XTAL_VM_CONTINUE(XTAL_VM_ff().next_pc);
		}
	}

vmloopbegin:
	xop = *pc;
XTAL_VM_LOOP

//{OPS{{
	XTAL_VM_CASE_FIRST(InstLine){ // 3
		if(*hook_setting_bit_!=0){
			XTAL_VM_LOCK{
				check_breakpoint_hook(pc, BREAKPOINT_LINE_PROFILE);
				check_breakpoint_hook(pc, BREAKPOINT_LINE_LIGHT_WEIGHT);
				check_breakpoint_hook(pc, BREAKPOINT_INNER_LINE);
				check_breakpoint_hook(pc, BREAKPOINT_LINE);
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstLoadValue){ // 3
		AnyPtr& result = *(variables_top_ + inst.result);
		XTAL_VM_DEC(result);
		XTAL_detail_copy(result, ap(values[inst.value]));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstLoadConstant){ // 3
		set_local_variable(inst.result, XTAL_VM_ff().values[inst.value_number]); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstLoadInt1Byte){ // 3
		AnyPtr& result = *(variables_top_ + inst.result);
		XTAL_VM_DEC(result);
		result.value_.init_int(inst.value);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstLoadFloat1Byte){ // 3
		AnyPtr& result = *(variables_top_ + inst.result);
		XTAL_VM_DEC(result);
		result.value_.init_float((float_t)inst.value);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstLoadCallee){ // 3
		set_local_variable(inst.result, XTAL_VM_ff().fun);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstLoadThis){ // 3
		set_local_variable(inst.result, XTAL_VM_ff().self);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstCopy){ // 3
		set_local_variable(inst.result, XTAL_VM_local_variable(inst.target));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstInc){ // 27
		const AnyPtr& a = XTAL_VM_local_variable(inst.target); uint_t atype = XTAL_detail_urawtype(a)-TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);

		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			if(XTAL_LIKELY(atype==0)){
				result.value_.init_int(XTAL_detail_ivalue(a)+1);
			}
			else{
				result.value_.init_float(XTAL_detail_fvalue(a)+1);
			}
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = DefinedID::id_op_inc;
		next_pc = &inst;
		goto send_una;
	}

	XTAL_VM_CASE(InstDec){ // 27
		const AnyPtr& a = XTAL_VM_local_variable(inst.target); uint_t atype = XTAL_detail_urawtype(a)-TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);

		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			if(XTAL_LIKELY(atype==0)){
				result.value_.init_int(XTAL_detail_ivalue(a)-1);
			}
			else{
				result.value_.init_float(XTAL_detail_fvalue(a)-1);
			}
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = DefinedID::id_op_dec;
		next_pc = &inst;
		goto send_una;
	}

	XTAL_VM_CASE(InstPos){ // 27
		const AnyPtr& a = XTAL_VM_local_variable(inst.target); uint_t atype = XTAL_detail_urawtype(a)-TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);

		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			result.value_ = a.value_;
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = DefinedID::id_op_pos;
		next_pc = &inst;
		goto send_una;
	}

	XTAL_VM_CASE(InstNeg){ // 27
		const AnyPtr& a = XTAL_VM_local_variable(inst.target); uint_t atype = XTAL_detail_urawtype(a)-TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);

		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			if(XTAL_LIKELY(atype==0)){
				result.value_.init_int(-XTAL_detail_ivalue(a));
			}
			else{
				result.value_.init_float(-XTAL_detail_fvalue(a));
			}
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = DefinedID::id_op_neg;
		next_pc = &inst;
		goto send_una;
	}

	XTAL_VM_CASE(InstCom){ // 27
		const AnyPtr& a = XTAL_VM_local_variable(inst.target); uint_t atype = XTAL_detail_urawtype(a);
		AnyPtr& result = XTAL_VM_local_variable(inst.result);

		if(XTAL_LIKELY(atype==TYPE_INT)){
			XTAL_VM_DEC(result);
			result.value_.init_int(~XTAL_detail_ivalue(a));
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = DefinedID::id_op_com;
		next_pc = &inst;
		goto send_una;
	}

	XTAL_VM_CASE(InstAdd){ // 50
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype|btype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			switch((atype<<1) | (btype)){
				XTAL_NODEFAULT;
				XTAL_CASE((0<<1) | 0){ result.value_.init_int(XTAL_detail_ivalue(a) + XTAL_detail_ivalue(b)); } 
				XTAL_CASE((1<<1) | 0){ result.value_.init_float(XTAL_detail_fvalue(a) + XTAL_detail_ivalue(b)); } 
				XTAL_CASE((0<<1) | 1){ result.value_.init_float(XTAL_detail_ivalue(a) + XTAL_detail_fvalue(b)); } 
				XTAL_CASE((1<<1) | 1){ result.value_.init_float(XTAL_detail_fvalue(a) + XTAL_detail_fvalue(b)); } 
			}
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = inst.assign ? DefinedID::id_op_add_assign : DefinedID::id_op_add;
		next_pc = &inst;
		goto send_bin;
	}

	XTAL_VM_CASE(InstSub){ // 50
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype|btype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			switch((atype<<1) | (btype)){
				XTAL_NODEFAULT;
				XTAL_CASE((0<<1) | 0){ result.value_.init_int(XTAL_detail_ivalue(a) - XTAL_detail_ivalue(b)); } 
				XTAL_CASE((1<<1) | 0){ result.value_.init_float(XTAL_detail_fvalue(a) - XTAL_detail_ivalue(b)); } 
				XTAL_CASE((0<<1) | 1){ result.value_.init_float(XTAL_detail_ivalue(a) - XTAL_detail_fvalue(b)); } 
				XTAL_CASE((1<<1) | 1){ result.value_.init_float(XTAL_detail_fvalue(a) - XTAL_detail_fvalue(b)); } 
			}
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = inst.assign ? DefinedID::id_op_sub_assign : DefinedID::id_op_sub;
		next_pc = &inst;
		goto send_bin;
	}

	XTAL_VM_CASE(InstCat){ // 50		
		iprimary = inst.assign ? DefinedID::id_op_cat_assign : DefinedID::id_op_cat;
		next_pc = &inst;
		goto send_bin;
	}

	XTAL_VM_CASE(InstMul){ // 50
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype|btype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			switch((atype<<1) | (btype)){
				XTAL_NODEFAULT;
				XTAL_CASE((0<<1) | 0){ result.value_.init_int(XTAL_detail_ivalue(a) * XTAL_detail_ivalue(b)); } 
				XTAL_CASE((1<<1) | 0){ result.value_.init_float(XTAL_detail_fvalue(a) * XTAL_detail_ivalue(b)); } 
				XTAL_CASE((0<<1) | 1){ result.value_.init_float(XTAL_detail_ivalue(a) * XTAL_detail_fvalue(b)); } 
				XTAL_CASE((1<<1) | 1){ result.value_.init_float(XTAL_detail_fvalue(a) * XTAL_detail_fvalue(b)); } 
			}
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = inst.assign ? DefinedID::id_op_mul_assign : DefinedID::id_op_mul;
		next_pc = &inst;
		goto send_bin;
	}
	
	XTAL_VM_CASE(InstDiv){ // 50
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype|btype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			switch((atype<<1) | (btype)){
				XTAL_NODEFAULT;
				XTAL_CASE((0<<1) | 0){ if(XTAL_detail_ivalue(b)==0) goto zerodiv2; result.value_.init_int(XTAL_detail_ivalue(a) / XTAL_detail_ivalue(b)); } 
				XTAL_CASE((1<<1) | 0){ if(XTAL_detail_ivalue(b)==0) goto zerodiv2; result.value_.init_float(XTAL_detail_fvalue(a) / XTAL_detail_ivalue(b)); } 
				XTAL_CASE((0<<1) | 1){ if(XTAL_detail_fvalue(b)==0) goto zerodiv2; result.value_.init_float(XTAL_detail_ivalue(a) / XTAL_detail_fvalue(b)); } 
				XTAL_CASE((1<<1) | 1){ if(XTAL_detail_fvalue(b)==0) goto zerodiv2; result.value_.init_float(XTAL_detail_fvalue(a) / XTAL_detail_fvalue(b)); } 
			}
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
zerodiv2:
			result.value_.init_null();
			goto zerodiv;
		}

		iprimary = inst.assign ? DefinedID::id_op_div_assign : DefinedID::id_op_div;
		next_pc = &inst;
		goto send_bin;
	}

	XTAL_VM_CASE(InstMod){ // 50
		using namespace std;
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype|btype)&(~1U))==0)){
			XTAL_VM_DEC(result);			
			switch((atype<<1) | (btype)){
				XTAL_NODEFAULT;
				XTAL_CASE((0<<1) | 0){ if(XTAL_detail_ivalue(b)==0) goto zerodiv3; result.value_.init_int(XTAL_detail_ivalue(a) % XTAL_detail_ivalue(b)); } 
				XTAL_CASE((1<<1) | 0){ if(XTAL_detail_ivalue(b)==0) goto zerodiv3; result.value_.init_float(fmodf(XTAL_detail_fvalue(a), (float_t)XTAL_detail_ivalue(b))); } 
				XTAL_CASE((0<<1) | 1){ if(XTAL_detail_fvalue(b)==0) goto zerodiv3; result.value_.init_float(fmodf((float_t)XTAL_detail_ivalue(a), XTAL_detail_fvalue(b))); } 
				XTAL_CASE((1<<1) | 1){ if(XTAL_detail_fvalue(b)==0) goto zerodiv3; result.value_.init_float(fmodf(XTAL_detail_fvalue(a), XTAL_detail_fvalue(b))); } 
			}
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
zerodiv3:
			result.value_.init_null();
			goto zerodiv;
		}

		iprimary = inst.assign ? DefinedID::id_op_mod_assign : DefinedID::id_op_mod;
		next_pc = &inst;
		goto send_bin;
	}

	XTAL_VM_CASE(InstAnd){ // 50
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintであるか？
		if(XTAL_LIKELY((atype|btype)==0)){
			XTAL_VM_DEC(result);
			result.value_.init_int(XTAL_detail_ivalue(a) & XTAL_detail_ivalue(b));
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = inst.assign ? DefinedID::id_op_and_assign : DefinedID::id_op_and;
		next_pc = &inst;
		goto send_bin;
	}

	XTAL_VM_CASE(InstOr){ // 50
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintであるか？
		if(XTAL_LIKELY((atype|btype)==0)){
			XTAL_VM_DEC(result);
			result.value_.init_int(XTAL_detail_ivalue(a) | XTAL_detail_ivalue(b));
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = inst.assign ? DefinedID::id_op_or_assign : DefinedID::id_op_or;
		next_pc = &inst;
		goto send_bin;
	}

	XTAL_VM_CASE(InstXor){ // 50
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintであるか？
		if(XTAL_LIKELY((atype|btype)==0)){
			XTAL_VM_DEC(result);
			result.value_.init_int(XTAL_detail_ivalue(a) ^ XTAL_detail_ivalue(b));
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = inst.assign ? DefinedID::id_op_xor_assign : DefinedID::id_op_xor;
		next_pc = &inst;
		goto send_bin;
	}

	XTAL_VM_CASE(InstShl){ // 50
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintであるか？
		if(XTAL_LIKELY((atype|btype)==0)){
			XTAL_VM_DEC(result);
			result.value_.init_int(XTAL_detail_ivalue(a) << XTAL_detail_ivalue(b));
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = inst.assign ? DefinedID::id_op_shl_assign : DefinedID::id_op_shl;
		next_pc = &inst;
		goto send_bin;
	}

	XTAL_VM_CASE(InstShr){ // 50
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
	
		// 型がintであるか？
		if(XTAL_LIKELY((atype|btype)==0)){
			XTAL_VM_DEC(result);
			result.value_.init_int(XTAL_detail_ivalue(a) >> XTAL_detail_ivalue(b));
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = inst.assign ? DefinedID::id_op_shr_assign : DefinedID::id_op_shr;
		next_pc = &inst;
		goto send_bin;
	}

	XTAL_VM_CASE(InstUshr){ // 50
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintであるか？
		if(XTAL_LIKELY((atype|btype)==0)){
			XTAL_VM_DEC(result);
			result.value_.init_int((uint_t)XTAL_detail_ivalue(a) >> XTAL_detail_ivalue(b));
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		iprimary = inst.assign ? DefinedID::id_op_ushr_assign : DefinedID::id_op_ushr;
		next_pc = &inst;
		goto send_bin;
	}

	XTAL_VM_CASE(InstAt){ // 22
		AnyPtr& a = XTAL_VM_local_variable(inst.target); uint_t atype = XTAL_detail_type(a);
		AnyPtr& b = XTAL_VM_local_variable(inst.index); uint_t btype = XTAL_detail_urawtype(b);

		if(XTAL_LIKELY(((atype^TYPE_ARRAY) | (btype^TYPE_INT))==0)){
			int_t index = XTAL_detail_ivalue(b);
			const ArrayPtr& ary = unchecked_ptr_cast<Array>(a);

			XTAL_VM_LOCK{
				if((uint_t)index<ary->size()){
					set_local_variable(inst.result, ary->at(index));
				}
				else{
					set_local_variable(inst.result, ary->op_at(index));
					XTAL_VM_CHECK_EXCEPT;
				}
			}
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else if(XTAL_detail_raweq(a->get_class(), cpp_class<Map>())){
			XTAL_VM_LOCK{
				AnyPtr ret = unchecked_ptr_cast<Map>(a)->op_at(b);
				XTAL_VM_CHECK_EXCEPT;
				set_local_variable(inst.result, ret);
			}
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else{
			set_local_variable(inst.stack_base, b);
			call_state.set(pc, pc+inst.ISIZE, inst.result, 1, inst.stack_base, 1, 0, 0);
			call_state.target = a;
			iprimary = DefinedID::id_op_at;
			goto send_common_iprimary_nosecondary;
		}
	}

	XTAL_VM_CASE(InstSetAt){ // 23
		AnyPtr& a = XTAL_VM_local_variable(inst.target); uint_t atype = XTAL_detail_type(a);
		AnyPtr& b = XTAL_VM_local_variable(inst.index); uint_t btype = XTAL_detail_urawtype(b);
		AnyPtr& c = XTAL_VM_local_variable(inst.value);

		if(XTAL_LIKELY(((atype^TYPE_ARRAY) | (btype^TYPE_INT))==0)){
			int_t index = XTAL_detail_ivalue(b);
			const ArrayPtr& ary = unchecked_ptr_cast<Array>(a);

			XTAL_VM_LOCK{
				if((uint_t)index<ary->size()){
					ary->set_at(index, c);
				}
				else{
					ary->op_set_at(index, c);
					XTAL_VM_CHECK_EXCEPT;
				}
			}
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else if(XTAL_detail_raweq(a->get_class(), cpp_class<Map>())){
			XTAL_VM_LOCK{
				unchecked_ptr_cast<Map>(a)->op_set_at(b, c);
				XTAL_VM_CHECK_EXCEPT;
			}
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else{
			set_local_variable(inst.stack_base+0, b);
			set_local_variable(inst.stack_base+1, c);
			call_state.set(pc, pc + inst.ISIZE, inst.stack_base, 0, inst.stack_base, 2, 0, 0);
			call_state.target = a;
			iprimary = DefinedID::id_op_set_at;
			goto send_common_iprimary_nosecondary;
		}
	}

	XTAL_VM_CASE(InstGoto){ // 3
		XTAL_CHECK_YIELD;
		XTAL_VM_CONTINUE(pc + inst.address); 
	}

	XTAL_VM_CASE(InstNot){ // 3
		set_local_variable(inst.result, Bool(!XTAL_VM_local_variable(inst.target))); 
		XTAL_VM_CONTINUE(pc+inst.ISIZE); 
	}

	XTAL_VM_CASE(InstIf){ // 3
		XTAL_CHECK_YIELD;
		XTAL_VM_CONTINUE(pc + (XTAL_VM_local_variable(inst.target) ? inst.address_true : inst.address_false));
	}

	XTAL_VM_CASE(InstIfEq){ // 41
		XTAL_CHECK_YIELD;
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);
	
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a)-TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b)-TYPE_INT;

		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype|btype)&(~1U))==0)){
			int next = 0;
			switch((atype<<1) | (btype)){
				XTAL_NODEFAULT;
				XTAL_CASE((0<<1) | 0){ next = XTAL_detail_ivalue(a)==XTAL_detail_ivalue(b) ? inst2.address_true : inst2.address_false; } 
				XTAL_CASE((1<<1) | 0){ next = XTAL_detail_fvalue(a)==XTAL_detail_ivalue(b) ? inst2.address_true : inst2.address_false; } 
				XTAL_CASE((0<<1) | 1){ next = XTAL_detail_ivalue(a)==XTAL_detail_fvalue(b) ? inst2.address_true : inst2.address_false; } 
				XTAL_CASE((1<<1) | 1){ next = XTAL_detail_fvalue(a)==XTAL_detail_fvalue(b) ? inst2.address_true : inst2.address_false; } 
			}
			XTAL_VM_CONTINUE(next + pc + inst.ISIZE);
		}

		if(XTAL_detail_raweq(a, b)){
			XTAL_VM_CONTINUE(inst2.address_true + pc + inst.ISIZE);
		}		

		iprimary = DefinedID::id_op_eq;
		next_pc = &inst;
		goto send_comp;
	}

	XTAL_VM_CASE(InstIfLt){ // 41
		XTAL_CHECK_YIELD;
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);
	
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a)-TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b)-TYPE_INT;

		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype|btype)&(~1U))==0)){
			int next = 0;
			switch((atype<<1) | (btype)){
				XTAL_NODEFAULT;
				XTAL_CASE((0<<1) | 0){ next = XTAL_detail_ivalue(a)<XTAL_detail_ivalue(b) ? inst2.address_true : inst2.address_false; } 
				XTAL_CASE((1<<1) | 0){ next = XTAL_detail_fvalue(a)<XTAL_detail_ivalue(b) ? inst2.address_true : inst2.address_false; } 
				XTAL_CASE((0<<1) | 1){ next = XTAL_detail_ivalue(a)<XTAL_detail_fvalue(b) ? inst2.address_true : inst2.address_false; } 
				XTAL_CASE((1<<1) | 1){ next = XTAL_detail_fvalue(a)<XTAL_detail_fvalue(b) ? inst2.address_true : inst2.address_false; } 
			}
			XTAL_VM_CONTINUE(next + pc + inst.ISIZE);
		}

		iprimary = DefinedID::id_op_lt;
		next_pc = &inst;
		goto send_comp;
	}

	XTAL_VM_CASE(InstIfRawEq){ // 8
		XTAL_CHECK_YIELD;
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);
		pc = (XTAL_detail_raweq(XTAL_VM_local_variable(inst.lhs), XTAL_VM_local_variable(inst.rhs)) ? 
			(int)(inst2.address_true) : 
			(int)(inst2.address_false)) + pc + inst.ISIZE;
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(InstIfIs){ // 8
		XTAL_CHECK_YIELD;
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);
		pc = (XTAL_VM_local_variable(inst.lhs)->is(XTAL_VM_local_variable(inst.rhs)) ? 
			(int)(inst2.address_true) : 
			(int)(inst2.address_false)) + pc + inst.ISIZE;
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(InstIfIn){ // 41
		XTAL_CHECK_YIELD;
		typedef InstIf InstType2; 
		iprimary = DefinedID::id_op_in;
		next_pc = &inst;
		goto send_comp;
	}

	XTAL_VM_CASE(InstIfUndefined){ // 3
		XTAL_CHECK_YIELD;
		XTAL_VM_CONTINUE(pc + (XTAL_detail_is_undefined(XTAL_VM_local_variable(inst.target)) ? inst.address_true : inst.address_false));
	}

	XTAL_VM_CASE(InstIfDebug){ // 2
		XTAL_VM_CONTINUE(pc + (debug::is_enabled() ? (int)inst.ISIZE : (int)inst.address));
	}

	XTAL_VM_CASE(InstPush) XTAL_VM_LOCK{ // 3
		stack_.push(XTAL_VM_local_variable(inst.target));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstPop) XTAL_VM_LOCK{ // 3
		AnyPtr ret = stack_.top(); stack_.pop();
		set_local_variable(inst.result, ret);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstAdjustValues) XTAL_VM_LOCK{ // 3
		adjust_values2(inst.stack_base, inst.result_count, inst.need_result_count);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstLocalVariable) XTAL_VM_LOCK{ // 3
		set_local_variable(inst.result, local_variable_out_of_fun(inst.number, inst.depth)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstSetLocalVariable) XTAL_VM_LOCK{ // 3
		set_local_variable_out_of_fun(inst.number, inst.depth, XTAL_VM_local_variable(inst.target)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstInstanceVariable) XTAL_VM_LOCK{ // 3
		set_local_variable(inst.result, XTAL_VM_ff().self->instance_variables()->variable(inst.number, XTAL_VM_ff().code->class_info(inst.info_number)));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstSetInstanceVariable) XTAL_VM_LOCK{ // 3
		XTAL_VM_ff().self->instance_variables()->set_variable(inst.number, XTAL_VM_ff().code->class_info(inst.info_number), XTAL_VM_local_variable(inst.value));
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstInstanceVariableByName) XTAL_VM_LOCK{ // 3
		if(eval_n){
			const AnyPtr& ret = eval_instance_variable(XTAL_VM_ff().self, XTAL_VM_ff().identifiers[inst.identifier_number]);
			set_local_variable(inst.result, ret);
		}

		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstSetInstanceVariableByName) XTAL_VM_LOCK{ // 3
		if(eval_n){
			eval_set_instance_variable(XTAL_VM_ff().self, XTAL_VM_ff().identifiers[inst.identifier_number], XTAL_VM_local_variable(inst.value));
		}

		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstFilelocalVariable) XTAL_VM_LOCK{ // 3
		set_local_variable(inst.result, XTAL_VM_ff().code->member_direct(inst.value_number));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstSetFilelocalVariable) XTAL_VM_LOCK{ // 3
		XTAL_VM_ff().code->Frame::set_member_direct(inst.value_number, XTAL_VM_local_variable(inst.value));
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstFilelocalVariableByName) XTAL_VM_LOCK{ // 8
		if(eval_n) {
			const AnyPtr& ret = eval_local_variable(XTAL_VM_ff().identifiers[inst.identifier_number], eval_n + (fun_frames_.size()-eval_base_n));
			if(!XTAL_detail_is_undefined(ret)){
				set_local_variable(inst.result, ret);
				XTAL_VM_CONTINUE(pc + inst.ISIZE);
			}
		}

		const AnyPtr& ret = XTAL_VM_ff().code->member(XTAL_VM_ff().identifiers[inst.identifier_number]);
		if(!XTAL_detail_is_undefined(ret)){
			set_local_variable(inst.result, ret);
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
		else{
			XTAL_VM_THROW_EXCEPT(filelocal_unsupported_error(XTAL_VM_ff().code, XTAL_VM_ff().identifiers[inst.identifier_number]));
		}
	}

	XTAL_VM_CASE(InstSetFilelocalVariableByName) XTAL_VM_LOCK{ // 3
		if(eval_n){
			if(eval_set_local_variable(XTAL_VM_ff().identifiers[inst.identifier_number], XTAL_VM_local_variable(inst.value), eval_n + (fun_frames_.size()-eval_base_n))){
				XTAL_VM_CONTINUE(pc + inst.ISIZE);
			}
		}

		XTAL_VM_THROW_EXCEPT(filelocal_unsupported_error(XTAL_VM_ff().code, XTAL_VM_ff().identifiers[inst.identifier_number]));
	}

	XTAL_VM_CASE(InstMember){ // 11
		call_state.result = inst.result;
		call_state.need_result_count = 1;
		call_state.next_pc = pc + inst.ISIZE;
		call_state.flags = 0;
		call_state.primary = XTAL_VM_ff().identifiers[inst.primary];
		call_state.secondary = undefined;
		call_state.cls = XTAL_VM_local_variable(inst.target);
		call_state.self = XTAL_VM_ff().self;

		common_flag = MEMBER;
		if(!XTAL_detail_is_pvalue(call_state.cls)){
			goto send_common3;
		}
		goto send_common2;	
	}

	XTAL_VM_CASE(InstMemberEx){ // 11
		int_t flags = inst.flags;
		call_state.result = inst.result;
		call_state.need_result_count = 1;
		call_state.next_pc = pc + inst.ISIZE;
		call_state.flags = flags;
		call_state.primary = (flags&MEMBER_FLAG_P_BIT) ? unchecked_ptr_cast<ID>(XTAL_VM_local_variable(inst.primary)) : XTAL_VM_ff().identifiers[inst.primary];
		call_state.secondary = (flags&MEMBER_FLAG_S_BIT) ? XTAL_VM_local_variable(inst.secondary) : undefined;
		call_state.cls = XTAL_VM_local_variable(inst.target);
		call_state.self = XTAL_VM_ff().self;

		common_flag = MEMBER;
		if(!XTAL_detail_is_pvalue(call_state.cls)){
			goto send_common3;
		}
		goto send_common2;	
	}

	XTAL_VM_CASE(InstCall){ // 6
		call_state.set(pc, pc + inst.ISIZE, inst.result, inst.need_result, inst.stack_base, inst.ordered, 0, 0);
		call_state.member = XTAL_VM_local_variable(inst.target);
		call_state.self = XTAL_VM_ff().self;
		goto call_common2;
	}

	XTAL_VM_CASE(InstCallEx){ // 6
		int_t flags = inst.flags;
		call_state.set(pc, pc + inst.ISIZE, inst.result, inst.need_result, inst.stack_base, inst.ordered, inst.named, flags);
		call_state.member = XTAL_VM_local_variable(inst.target);
		call_state.self = (flags&CALL_FLAG_THIS) ? XTAL_VM_local_variable(inst.self) : XTAL_VM_ff().self;
		goto call_common;
	}

	XTAL_VM_CASE(InstSend){ // 8
		call_state.set(pc, pc + inst.ISIZE, inst.result, inst.need_result, inst.stack_base, inst.ordered, 0, 0);
		call_state.target = XTAL_VM_local_variable(inst.target);
		call_state.primary = XTAL_VM_ff().identifiers[inst.primary];
		goto send_common_nosecondary;
	}

	XTAL_VM_CASE(InstSendEx){ // 8
		int_t flags = inst.flags;
		call_state.set(pc, pc + inst.ISIZE, inst.result, inst.need_result, inst.stack_base, inst.ordered, inst.named, flags);
		call_state.target = XTAL_VM_local_variable(inst.target);
		call_state.primary = (flags&MEMBER_FLAG_P_BIT) ? unchecked_ptr_cast<ID>(XTAL_VM_local_variable(inst.primary)) : XTAL_VM_ff().identifiers[inst.primary];
		call_state.secondary = (flags&MEMBER_FLAG_S_BIT) ? XTAL_VM_local_variable(inst.secondary) : undefined;
		call_state.self = XTAL_VM_ff().self;
		goto send_common;
	}

	XTAL_VM_CASE(InstProperty){ // 7
		call_state.set(pc, pc + inst.ISIZE, inst.result, 1, inst.stack_base, 0, 0, 0);
		call_state.target = XTAL_VM_local_variable(inst.target);
		call_state.primary = XTAL_VM_ff().identifiers[inst.primary];
		call_state.secondary = undefined;
		call_state.self = XTAL_VM_ff().self;
		call_state.cls = call_state.target.get_class();
		common_flag = PROPERTY;
		goto send_common2;

	}

	XTAL_VM_CASE(InstSetProperty){ // 7
		call_state.set(pc, pc + inst.ISIZE, inst.stack_base, 0, inst.stack_base, 1, 0, 0);
		call_state.target = XTAL_VM_local_variable(inst.target);
		call_state.primary = XTAL_VM_ff().identifiers[inst.primary];
		call_state.secondary = undefined;
		call_state.self = XTAL_VM_ff().self;
		call_state.cls = call_state.target.get_class();
		common_flag = SET_PROPERTY;
		goto send_common2;
	}

	XTAL_VM_CASE(InstScopeBegin) XTAL_VM_LOCK{ // 3
		push_scope(XTAL_VM_ff().code->scope_info(inst.info_number));
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstScopeEnd) XTAL_VM_LOCK{ // 3
		pop_scope();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstReturn) XTAL_VM_LOCK{ // 7
		const MethodPtr& fun = XTAL_VM_ff().fun;

		pop_scope();
		const inst_t* next_pc = pop_ff(inst.base, inst.result_count);

		if(*hook_setting_bit_!=0){
			check_breakpoint_hook(pc, fun, BREAKPOINT_RETURN_PROFILE);
			check_breakpoint_hook(next_pc-1, fun, BREAKPOINT_RETURN_LIGHT_WEIGHT);
			check_breakpoint_hook(next_pc-1, fun, BREAKPOINT_INNER_RETURN);
			check_breakpoint_hook(pc, fun, BREAKPOINT_RETURN);
		}

		XTAL_VM_CONTINUE(next_pc);
	}

	XTAL_VM_CASE(InstYield){ // 7
		yield_base_ = inst.base;
		yield_result_count_ = inst.result_count;	
		yield_result_ = inst.result;
		yield_need_result_count_ = inst.need_result_count;

		if(XTAL_VM_ff().yieldable){
			resume_pc_ = pc + inst.ISIZE;
			XTAL_VM_RETURN;
		}
		else{
			XTAL_VM_LOCK{
				XTAL_VM_THROW_EXCEPT(cpp_class<YieldError>()->call(Xt("XRE1012")));
			}
		}
	}

	XTAL_VM_CASE(InstExit){ // 3
		resume_pc_ = 0; 
		XTAL_VM_RETURN;
	}

	XTAL_VM_CASE(InstRange){ // 11
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs);
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs);
		set_local_variable(inst.stack_base+0, b);
		set_local_variable(inst.stack_base+1, Int(inst.kind));
		call_state.set(pc, pc + inst.ISIZE, inst.result, 1, inst.stack_base, 2, 0, 0);
		call_state.target = a;
		iprimary = DefinedID::id_op_range;
		goto send_common_iprimary_nosecondary;
	}

	XTAL_VM_CASE(InstOnce) XTAL_VM_LOCK{ // 5
		const AnyPtr& ret = XTAL_VM_ff().code->once_value(inst.value_number);
		if(!XTAL_detail_is_undefined(ret)){
			set_local_variable(inst.result, ret);
			XTAL_VM_CONTINUE(pc + inst.address);
		}
		else{
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
	}

	XTAL_VM_CASE(InstSetOnce) XTAL_VM_LOCK{ // 3
		XTAL_VM_ff().code->set_once_value(inst.value_number, XTAL_VM_local_variable(inst.target)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstMakeArray) XTAL_VM_LOCK{ // 4
		SmartPtr<Array> value = xnew<Array>();
		set_local_variable(inst.result, value);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstArrayAppend) XTAL_VM_LOCK{ // 3
		ptr_cast<Array>(XTAL_VM_local_variable(inst.target))->push_back(XTAL_VM_local_variable(inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstMakeMap) XTAL_VM_LOCK{ // 4
		SmartPtr<Map> value = xnew<Map>();
		set_local_variable(inst.result, value);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstMapInsert) XTAL_VM_LOCK{ // 3
		ptr_cast<Map>(XTAL_VM_local_variable(inst.target))->set_at(XTAL_VM_local_variable(inst.key), XTAL_VM_local_variable(inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstMapSetDefault) XTAL_VM_LOCK{ // 3
		ptr_cast<Map>(XTAL_VM_local_variable(inst.target))->set_default_value(XTAL_VM_local_variable(inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstClassBegin) XTAL_VM_LOCK{ XTAL_VM_CONTINUE(FunInstClassBegin(pc)); /*
		XTAL_VM_FUN;
		ClassInfo* info = XTAL_VM_ff().code->class_info(inst.info_number);
		const FramePtr& outer = make_outer(info);
		ClassPtr cp = xnew<Class>(outer, XTAL_VM_ff().code, info);

		switch(info->kind){
			XTAL_NODEFAULT;

			XTAL_CASE(KIND_CLASS){

			}

			XTAL_CASE(KIND_SINGLETON){
				cp->set_singleton();
			}
		}
		
		int_t n = info->mixins;
		for(int_t i = 0; i<n; ++i){
			AnyPtr popped = XTAL_VM_local_variable(inst.mixin_base+i);
			if(const ClassPtr& cls = ptr_cast<Class>(popped)){
				cp->inherit_first(cls);
			}
			else{
				set_runtime_error(undefined);
			}
		}

		Scope& scope = scopes_.push();
		scope.frame = cp;
		scope.pos = 0;
		scope.size = 0;
		scope.flags = Scope::CLASS;
		cp->unset_orphan();

		CallState call_state;
		call_state.self = cp;
		call_state.next_pc = pc + inst.ISIZE;
		call_state.result = 0;
		call_state.need_result_count = 0;
		call_state.stack_base = 0;
		call_state.ordered = 0;
		call_state.named = 0;
		push_ff(call_state);

		XTAL_VM_ff().fun = XTAL_VM_prev_ff().fun;
		XTAL_VM_ff().outer = cp;
		XTAL_VM_ff().identifiers = XTAL_VM_prev_ff().identifiers;
		XTAL_VM_ff().code = XTAL_VM_prev_ff().code;
		XTAL_VM_ff().values = XTAL_VM_prev_ff().values;

		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(InstClassEnd) XTAL_VM_LOCK{ XTAL_VM_CONTINUE(FunInstClassEnd(pc)); /*
		XTAL_VM_FUN;
		Scope& scope = scopes_.top();
		if(XTAL_detail_raweq(scope.frame->get_class(), scope.frame)){
			Class* singleton = static_cast<Class*>(XTAL_detail_pvalue(scope.frame));
			singleton->init_singleton(to_smartptr(this));
		}

		set_local_variable(inst.result, scope.frame);
		scope.frame->set_orphan();
		scope.frame = null;
		scopes_.downsize(1);

		fun_frames_.downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(InstDefineClassMember) XTAL_VM_LOCK{ XTAL_VM_CONTINUE(FunInstDefineClassMember(pc)); /*
		XTAL_VM_FUN;
		if(const ClassPtr& p = ptr_cast<Class>(scopes_.top().frame)){
			p->set_member_direct(inst.number, identifier(inst.primary), XTAL_VM_local_variable(inst.value), XTAL_VM_local_variable(inst.secondary), inst.accessibility);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(InstDefineMember) XTAL_VM_LOCK{ XTAL_VM_CONTINUE(FunInstDefineMember(pc)); /*
		XTAL_VM_FUN;
		IDPtr primary = (inst.flags&MEMBER_FLAG_P_BIT) ? (const IDPtr&)XTAL_VM_local_variable(inst.primary) : identifier(inst.primary);
		AnyPtr secondary = (inst.flags&MEMBER_FLAG_S_BIT) ? XTAL_VM_local_variable(inst.secondary) : undefined;
		AnyPtr cls = XTAL_VM_local_variable(inst.target);
		AnyPtr value = XTAL_VM_local_variable(inst.value);
		cls->def(primary, ap(value), secondary, KIND_PUBLIC);
		XTAL_VM_CHECK_EXCEPT;
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}*/ }

	XTAL_VM_CASE(InstMakeFun) XTAL_VM_LOCK{ XTAL_VM_CONTINUE(FunInstMakeFun(pc)); /*
		XTAL_VM_FUN;
		int_t table_n = inst.info_number, end = inst.address;
		FunInfo* info = XTAL_VM_ff().code->fun_info(table_n);
		const FramePtr& outer = make_outer(info);
		switch(info->kind){
			XTAL_NODEFAULT;
			XTAL_CASE(KIND_FUN){ set_local_variable(inst.result, xnew<Fun>(outer, XTAL_VM_ff().self, XTAL_VM_ff().code, info)); }
			XTAL_CASE(KIND_LAMBDA){ set_local_variable(inst.result, xnew<Lambda>(outer, XTAL_VM_ff().self, XTAL_VM_ff().code, info)); }
			XTAL_CASE(KIND_METHOD){ set_local_variable(inst.result, xnew<Method>(outer, XTAL_VM_ff().code, info)); }
			XTAL_CASE(KIND_FIBER){ set_local_variable(inst.result, xnew<Fiber>(outer, XTAL_VM_ff().self, XTAL_VM_ff().code, info)); }
		}
		XTAL_VM_CONTINUE(pc + end);
	}*/ }

	XTAL_VM_CASE(InstMakeInstanceVariableAccessor) XTAL_VM_LOCK{ XTAL_VM_CONTINUE(FunInstMakeInstanceVariableAccessor(pc)); /*
		XTAL_VM_FUN;
		AnyPtr ret;
		switch(inst.type){
			XTAL_NODEFAULT;
			XTAL_CASE(0){ ret = XNew<InstanceVariableGetter>(inst.number, XTAL_VM_ff().code->class_info(inst.info_number)); }
			XTAL_CASE(1){ ret = XNew<InstanceVariableSetter>(inst.number, XTAL_VM_ff().code->class_info(inst.info_number)); }
		}
		set_local_variable(inst.result, ret);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}*/ }

	XTAL_VM_CASE(InstTryBegin){ XTAL_VM_CONTINUE(FunInstTryBegin(pc)); /*
		XTAL_VM_FUN;
		//FunFrame& f = XTAL_VM_ff();
		ExceptFrame& ef = except_frames_.push();
		ef.info = code()->except_info(inst.info_number);
		ef.fun_frame_size = fun_frames_.size();
		ef.stack_size = stack_.size();
		ef.scope_size = scopes_.size();
		ef.variables_top = variables_top();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(InstTryEnd){ // 2
		XTAL_VM_CONTINUE(except_frames_.pop().info->finally_pc + XTAL_VM_ff().code->bytecode_data()); 
	}

	XTAL_VM_CASE(InstPushGoto){ // 3
		stack_.push(Int((int_t)((pc+inst.address)-XTAL_VM_ff().code->bytecode_data()))); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstPopGoto){ // 2
		Any ret = stack_.top(); stack_.pop();
		XTAL_VM_CONTINUE(XTAL_VM_ff().code->bytecode_data()+XTAL_detail_ivalue(ret));
	}

	XTAL_VM_CASE(InstThrow) XTAL_VM_LOCK{ // 12
		AnyPtr except = stack_.top();
		stack_.pop();

		if(!except){
			except = except_[0];
		}

		if(pc!=&throw_code_){
			throw_pc_ = pc;
		}

		if(!except->is(cpp_class<Exception>())){
			except = cpp_class<RuntimeError>()->call(except);
		}

		except_[0] = except; 
		check_breakpoint_hook(pc==&throw_code_ ? throw_pc_ : pc, BREAKPOINT_THROW); 

		// 例外にバックトレースを追加する
		AnyPtr e = catch_except();
		set_except_x(append_backtrace(throw_pc_, e));

		// Xtalソース内でキャッチ等あるか調べる
		pc = catch_body(throw_pc_, cur);

		// Xtalソース内でキャッチ等されたなら最初に戻る
		if(pc){
			XTAL_VM_CONTINUE(pc);
		}
		
		// されてないならループ脱出
		XTAL_VM_RETURN;
	}

	XTAL_VM_CASE(InstAssert) XTAL_VM_LOCK{ XTAL_VM_CONTINUE(FunInstAssert(pc)); /*
		XTAL_VM_FUN;
		set_except_x(cpp_class<AssertionFailed>()->call(ptr_cast<String>(XTAL_VM_local_variable(inst.message))));
		breakpoint_hook(pc, fun(), BREAKPOINT_ASSERT);

		if(except_[0]){
			XTAL_VM_THROW_EXCEPT(except_[0]);
		}

		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(InstBreakPoint) XTAL_VM_LOCK{ // 5
		check_breakpoint_hook(pc, BREAKPOINT_LINE_LIGHT_WEIGHT);
		check_breakpoint_hook(pc, BREAKPOINT);
		//check_breakpoint_hook(pc, BREAKPOINT2);
		check_breakpoint_hook(pc, BREAKPOINT_LINE);

		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstMAX){ // 2
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

//}}OPS}

XTAL_VM_LOOP_END

}

#undef XTAL_VM_CONTINUE
#undef XTAL_VM_CONTINUE0
#undef XTAL_VM_THROW_EXCEPT
#undef XTAL_VM_CHECK_EXCEPT

#define XTAL_VM_CONTINUE(x) return (x)
#define XTAL_VM_CONTINUE0 return pc
#define XTAL_VM_THROW_EXCEPT(e) XTAL_VM_CONTINUE(push_except(pc, e))
#define XTAL_VM_CHECK_EXCEPT if(except_[0]){ XTAL_VM_CONTINUE(push_except(pc)); }

//{FUNS{{
const inst_t* VMachine::FunInstClassBegin(const inst_t* pc){
		XTAL_VM_DEF_INST(InstClassBegin);
		XTAL_VM_FUN;
		ClassInfo* info = XTAL_VM_ff().code->class_info(inst.info_number);
		const FramePtr& outer = make_outer(info);
		ClassPtr cp = xnew<Class>(outer, XTAL_VM_ff().code, info);

		switch(info->kind){
			XTAL_NODEFAULT;

			XTAL_CASE(KIND_CLASS){

			}

			XTAL_CASE(KIND_SINGLETON){
				cp->set_singleton();
			}
		}
		
		int_t n = info->mixins;
		for(int_t i = 0; i<n; ++i){
			AnyPtr popped = XTAL_VM_local_variable(inst.mixin_base+i);
			if(const ClassPtr& cls = ptr_cast<Class>(popped)){
				cp->inherit_first(cls);
			}
			else{
				XTAL_VM_THROW_EXCEPT(cpp_class<RuntimeError>()->call());
			}
		}

		Scope& scope = scopes_.push();
		scope.frame = cp;
		scope.pos = 0;
		scope.size = 0;
		scope.flags = Scope::CLASS;
		cp->unset_orphan();

		CallState call_state;
		call_state.self = cp;
		call_state.next_pc = pc + inst.ISIZE;
		call_state.result = 0;
		call_state.need_result_count = 0;
		call_state.stack_base = 0;
		call_state.ordered = 0;
		call_state.named = 0;
		push_ff(call_state);

		XTAL_VM_ff().fun = XTAL_VM_prev_ff().fun;
		XTAL_VM_ff().outer = cp;
		XTAL_VM_ff().identifiers = XTAL_VM_prev_ff().identifiers;
		XTAL_VM_ff().code = XTAL_VM_prev_ff().code;
		XTAL_VM_ff().values = XTAL_VM_prev_ff().values;

		XTAL_VM_CONTINUE(pc + inst.ISIZE);
}

const inst_t* VMachine::FunInstClassEnd(const inst_t* pc){
		XTAL_VM_DEF_INST(InstClassEnd);
		XTAL_VM_FUN;
		Scope& scope = scopes_.top();
		if(XTAL_detail_raweq(scope.frame->get_class(), (const AnyPtr&)scope.frame)){
			Class* singleton = static_cast<Class*>(XTAL_detail_pvalue((const AnyPtr&)scope.frame));
			singleton->init_singleton(to_smartptr(this));
		}

		set_local_variable(inst.result, scope.frame);
		scope.frame->set_orphan();
		scope.frame = null;
		scopes_.downsize(1);

		fun_frames_.downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
}

const inst_t* VMachine::FunInstDefineClassMember(const inst_t* pc){
		XTAL_VM_DEF_INST(InstDefineClassMember);
		XTAL_VM_FUN;
		if(const ClassPtr& p = ptr_cast<Class>(scopes_.top().frame)){
			p->set_member_direct(inst.number, XTAL_VM_ff().identifiers[inst.primary], XTAL_VM_local_variable(inst.value), XTAL_VM_local_variable(inst.secondary), inst.accessibility);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
}

const inst_t* VMachine::FunInstDefineMember(const inst_t* pc){
		XTAL_VM_DEF_INST(InstDefineMember);
		XTAL_VM_FUN;
		IDPtr primary = (inst.flags&MEMBER_FLAG_P_BIT) ? unchecked_ptr_cast<ID>(XTAL_VM_local_variable(inst.primary)) : XTAL_VM_ff().identifiers[inst.primary];
		AnyPtr secondary = (inst.flags&MEMBER_FLAG_S_BIT) ? XTAL_VM_local_variable(inst.secondary) : undefined;
		AnyPtr cls = XTAL_VM_local_variable(inst.target);
		AnyPtr value = XTAL_VM_local_variable(inst.value);
		cls->def(primary, value, secondary, KIND_PUBLIC);
		XTAL_VM_CHECK_EXCEPT;
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
}

const inst_t* VMachine::FunInstMakeFun(const inst_t* pc){
		XTAL_VM_DEF_INST(InstMakeFun);
		XTAL_VM_FUN;
		int_t table_n = inst.info_number, end = inst.address;
		FunInfo* info = XTAL_VM_ff().code->fun_info(table_n);
		const FramePtr& outer = make_outer(info);
		switch(info->kind){
			XTAL_NODEFAULT;
			XTAL_CASE(KIND_FUN){ set_local_variable(inst.result, xnew<Fun>(outer, XTAL_VM_ff().self, XTAL_VM_ff().code, info)); }
			XTAL_CASE(KIND_LAMBDA){ set_local_variable(inst.result, xnew<Lambda>(outer, XTAL_VM_ff().self, XTAL_VM_ff().code, info)); }
			XTAL_CASE(KIND_METHOD){ set_local_variable(inst.result, xnew<Method>(outer, XTAL_VM_ff().code, info)); }
			XTAL_CASE(KIND_FIBER){ set_local_variable(inst.result, xnew<Fiber>(outer, XTAL_VM_ff().self, XTAL_VM_ff().code, info)); }
		}
		XTAL_VM_CONTINUE(pc + end);
}

const inst_t* VMachine::FunInstMakeInstanceVariableAccessor(const inst_t* pc){
		XTAL_VM_DEF_INST(InstMakeInstanceVariableAccessor);
		XTAL_VM_FUN;
		AnyPtr ret;
		switch(inst.type){
			XTAL_NODEFAULT;
			XTAL_CASE(0){ ret = XNew<InstanceVariableGetter>(inst.number, XTAL_VM_ff().code->class_info(inst.info_number)); }
			XTAL_CASE(1){ ret = XNew<InstanceVariableSetter>(inst.number, XTAL_VM_ff().code->class_info(inst.info_number)); }
		}
		set_local_variable(inst.result, ret);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
}

const inst_t* VMachine::FunInstTryBegin(const inst_t* pc){
		XTAL_VM_DEF_INST(InstTryBegin);
		XTAL_VM_FUN;
		//FunFrame& f = XTAL_VM_ff();
		ExceptFrame& ef = except_frames_.push();
		ef.info = XTAL_VM_ff().code->except_info(inst.info_number);
		ef.fun_frame_size = fun_frames_.size();
		ef.stack_size = stack_.size();
		ef.scope_size = scopes_.size();
		ef.variables_top = XTAL_VM_variables_top();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
}

const inst_t* VMachine::FunInstAssert(const inst_t* pc){
		XTAL_VM_DEF_INST(InstAssert);
		XTAL_VM_FUN;
		set_except_x(cpp_class<AssertionFailed>()->call(ptr_cast<String>(XTAL_VM_local_variable(inst.message))));
		breakpoint_hook(pc, XTAL_VM_ff().fun, BREAKPOINT_ASSERT);

		if(except_[0]){
			XTAL_VM_THROW_EXCEPT(except_[0]);
		}

		XTAL_VM_CONTINUE(pc + inst.ISIZE);
}

//}}FUNS}

#undef XTAL_VM_NODEFAULT
#undef XTAL_VM_CASE_FIRST
#undef XTAL_VM_CASE
#undef XTAL_VM_SWITCH
#undef XTAL_VM_DEF_INST
#undef XTAL_VM_CONTINUE
#undef XTAL_VM_CONTINUE0
#undef XTAL_VM_RETURN

#undef XTAL_VM_EXCEPT
#undef XTAL_VM_THROW_EXCEPT
#undef XTAL_VM_CHECK_EXCEPT

#undef XTAL_CHECK_YIELD

//#undef inst

}//namespace

