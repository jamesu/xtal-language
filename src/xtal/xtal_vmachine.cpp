#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_stringspace.h"
#include "xtal_details.h"
#include "xtal_cache.h"

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
#	define XTAL_VM_CASE(key) } case key::NUMBER: { XTAL_VM_DEF_INST(key);
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
	XTAL_TYPE_SWITCH(*this, 27){
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

void Any::rawcall(const VMachinePtr& vm) const{
	XTAL_TYPE_SWITCH(*this, 27){
		XTAL_NODEFAULT;

		XTAL_CASE8(TYPE_NULL,TYPE_UNDEFINED,TYPE_FALSE,TYPE_TRUE,TYPE_INT,TYPE_FLOAT,TYPE_IMMEDIATE_VALUE,TYPE_POINTER){ 
			rawsend(vm, XTAL_DEFINED_ID(op_call));
		}

		XTAL_CASE(TYPE_STATELESS_NATIVE_METHOD){ unchecked_ptr_cast<StatelessNativeMethod>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_IVAR_GETTER){ unchecked_ptr_cast<InstanceVariableGetter>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_IVAR_SETTER){ unchecked_ptr_cast<InstanceVariableSetter>(ap(*this))->on_rawcall(vm); }

		XTAL_CASE5(TYPE_SMALL_STRING,TYPE_LONG_LIVED_STRING,TYPE_INTERNED_STRING,TYPE_PADDING_0,TYPE_PADDING_1){ 
			rawsend(vm, XTAL_DEFINED_ID(op_call));
		}

		XTAL_CASE(TYPE_BASE){ XTAL_detail_pvalue(*this)->rawcall(vm); }

		XTAL_CASE4(TYPE_STRING,TYPE_ARRAY,TYPE_VALUES,TYPE_TREE_NODE){ 
			rawsend(vm, XTAL_DEFINED_ID(op_call));
		}

		XTAL_CASE(TYPE_NATIVE_METHOD){ unchecked_ptr_cast<NativeMethod>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_NATIVE_FUN){ unchecked_ptr_cast<NativeFun>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_METHOD){ unchecked_ptr_cast<Method>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_FUN){ unchecked_ptr_cast<Fun>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_LAMBDA){ unchecked_ptr_cast<Lambda>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_FIBER){ unchecked_ptr_cast<Fiber>(ap(*this))->on_rawcall(vm); }
	}

	if(vm->is_executed()==0){
		if(vm->except()){ 
			return;
		}

		set_unsupported_error(get_class(), XTAL_DEFINED_ID(op_call), undefined, vm);
		return;
	}
}

bool Any::is(const AnyPtr& klass) const{
	const ClassPtr& my_class = get_class();
	if(XTAL_detail_raweq(my_class, klass)) return true;
	return environment_->is_cache_table_.cache(my_class, klass);
}

bool Any::is(CppClassSymbolData* key) const{
	return is(cpp_class(key));
}

void StatelessNativeMethod::on_rawcall(const VMachinePtr& vm) const{
	const param_types_holder_n& pth = *XTAL_detail_pthvalue(*this);

	FunctorParam fp;
	fp.vm = &*vm;
	fp.fun = 0;
	fp.result = undefined;

	if(pth.vm){ // 第一引数がVMachine
		fp.args[0] = (Any&)vm->arg_this();
		fp.args[1] = (Any&)vm->arg_unchecked(0);
	}
	else{
		int_t param_n = pth.param_n;

		if(vm->ordered_arg_count()!=param_n){
			if(!pth.extendable || vm->ordered_arg_count()<param_n){
				set_argument_num_error(Xid(NativeFunction), vm->ordered_arg_count(), param_n, param_n, vm);
				return;
			}
		}

		{ // check arg type

			const CppClassSymbolData* anycls = &CppClassSymbol<Any>::value;
			const CppClassSymbolData* intcls = &CppClassSymbol<Int>::value;
			const CppClassSymbolData* floatcls = &CppClassSymbol<Float>::value;

			int_t num = 1;
			fp.args[0] = (Any&)vm->arg_this();

			if(pth.extendable){
				vm->set_local_variable(param_n, vm->make_arguments(param_n));
				fp.args[param_n+1] = (Any&)vm->arg_unchecked(param_n);
			}

			for(int_t i=0; i<param_n; ++i){
				fp.args[i+1] = (Any&)vm->arg_unchecked(i);
			}

			num = param_n+1;
				
			// 型チエック
			for(int_t i=0; i<num; ++i){
				if(pth.param_types[i]==anycls){
					continue;
				}
				else{
					const AnyPtr& arg = ap(fp.args[i]);
					const ClassPtr& cls = cpp_class(pth.param_types[i]);
					if(!arg->is(cls)){ 
						set_argument_type_error(object_name(), i, cls, arg->get_class(), vm);
						return;
					}

					if(pth.param_types[i]==intcls){
						fp.args[i].value_.init_int(arg->to_i());
					}
					else if(pth.param_types[i]==floatcls){
						fp.args[i].value_.init_float(arg->to_f());
					}
				}
			}
		}
	}

	pth.fun(fp);

	if(!vm->is_executed()){
		vm->return_result(ap(fp.result));
	}
}

void NativeMethod::on_rawcall(const VMachinePtr& vm){
	const param_types_holder_n& pth = *pth_;
	int_t param_n = pth.param_n;

	if(vm->ordered_arg_count()!=min_param_count_){
		int_t n = vm->ordered_arg_count();
		if(n<min_param_count_ || n>max_param_count_){
			set_argument_num_error(Xid(NativeFunction), n, min_param_count_, max_param_count_, vm);
			return;
		}
	}

	FunctorParam fp;
	fp.vm = &*vm;
	fp.fun = data_;
	fp.result = undefined;
	
	{ // check arg type
		NamedParam* params = (NamedParam*)((u8*)data_ + val_size_);

		const CppClassSymbolData* anycls = &CppClassSymbol<Any>::value;
		const CppClassSymbolData* intcls = &CppClassSymbol<Int>::value;
		const CppClassSymbolData* floatcls = &CppClassSymbol<Float>::value;

		int_t num = 1;
		fp.args[0] = (Any&)vm->arg_this();

		if(!pth.vm){
			if(pth.extendable){
				vm->set_local_variable(param_n, vm->inner_make_arguments(params, param_n));
				fp.args[param_n+1] = (Any&)vm->arg_unchecked(param_n);
			}
			else{
				if(vm->ordered_arg_count()!=pth.param_n){
					if(!vm->adjust_args(params, pth.param_n)){
						return;
					}
				}
			}

			for(int_t i=0; i<param_n; ++i){
				fp.args[i+1] = (Any&)vm->arg_unchecked(i);
			}
			num = pth.param_n+1;
		}

		if(num>FunctorParam::MAX_ARGS){
			num = FunctorParam::MAX_ARGS;
		}
			
		for(int_t i=0; i<num; ++i){
			if(pth.param_types[i]==anycls){
				continue;
			}
			else{
				const AnyPtr& arg = ap(fp.args[i]);
				const ClassPtr& cls = cpp_class(pth.param_types[i]);
				if(!arg->is(cls)){ 
					set_argument_type_error(object_name(), i, cls, arg->get_class(), vm);
					return;
				}

				if(pth.param_types[i]==intcls){
					fp.args[i].value_.init_int(arg->to_i());
				}
				else if(pth.param_types[i]==floatcls){
					fp.args[i].value_.init_float(arg->to_f());
				}
			}
		}
	}

	pth.fun(fp);

	if(!vm->is_executed()){
		vm->return_result(ap(fp.result));
	}
}

void Method::on_rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=info_->max_param_count){
		if(!check_arg(vm)){
			return;
		}
	}

	vm->carry_over(this);
}

void Fun::on_rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=info()->max_param_count){
		if(!check_arg(vm)){
			return;
		}
	}

	vm->set_arg_this(this_);
	vm->carry_over(this);
}

void Lambda::on_rawcall(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	vm->carry_over(this, true);
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const SmartPtr<Any>& p){
	XTAL_detail_inc_ref_count(p);
	XTAL_detail_dec_ref_count(*this);
	XTAL_detail_copy(*this, p);
	return *this;
}

const AnyPtr& InstanceVariables::variable(uint_t index, ClassInfo* class_info){
	char* buf = (char*)(this + 1);
	if(class_info==info_){
		return ((AnyPtr*)buf)[index];
	}

	if(!info_){
		int_t install_count = *(int_t*)buf; buf += sizeof(int_t);
		AnyPtr* values = (AnyPtr*)buf;
		for(int_t i=0; i<install_count; ++i){
			if(class_info==XTAL_detail_rawvalue(values[i]).immediate_second_vpvalue()){
				int_t pos = XTAL_detail_rawvalue(values[i]).immediate_first_value();
				return values[install_count+pos+index];
			}
		}
	}

	return undefined;
}

void InstanceVariables::set_variable(uint_t index, ClassInfo* class_info, const AnyPtr& value){
	char* buf = (char*)(this + 1);
	if(class_info==info_){
		((AnyPtr*)buf)[index] = value;
		return;
	}

	if(!info_){
		int_t install_count = *(int_t*)buf; buf += sizeof(int_t);
		AnyPtr* values = (AnyPtr*)buf;
		for(int_t i=0; i<install_count; ++i){
			if(class_info==XTAL_detail_rawvalue(values[i]).immediate_second_vpvalue()){
				int_t pos = XTAL_detail_rawvalue(values[i]).immediate_first_value();
				values[install_count+pos+index] = value;
				return;
			}
		}
	}
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
				if(!adjust_args(fun)){
					return;
				}
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

const AnyPtr& MemberCacheTable::cache(const AnyPtr& target_class, const IDPtr& primary_key, int_t& accessibility){
	uint_t itarget_class = XTAL_detail_rawhash(target_class);
	uint_t iprimary_key = XTAL_detail_uvalue(primary_key);

	uint_t hash = itarget_class ^ (iprimary_key ^ (iprimary_key>>24));
	Unit& unit = table_[hash % CACHE_MASK];

	if(((member_mutate_count_ ^ unit.mutate_count) | 
		XTAL_detail_rawbitxor(primary_key, unit.primary_key) | 
		XTAL_detail_rawbitxor(target_class, unit.target_class))==0){
		hit_++;
		accessibility = unit.accessibility;
		return unit.member;
	}
	else{
		if(!XTAL_detail_is_pvalue(target_class)){
			accessibility = -1;
			return undefined;
		}

		miss_++;

		bool nocache = false;
		accessibility = 0;
		const AnyPtr& ret = XTAL_detail_pvalue(target_class)->rawmember(primary_key, undefined, true, accessibility, nocache);

		if(XTAL_detail_is_undefined(ret)){
			accessibility = -1;
			return undefined;
		}

		if(!nocache){
			unit.member = ret;
			unit.target_class = target_class;
			unit.primary_key = primary_key;
			unit.accessibility = accessibility;
			unit.mutate_count = member_mutate_count_;
		}
		return ret;
	}
}

const AnyPtr& MemberCacheTable2::cache(const AnyPtr& target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility){
	uint_t itarget_class = XTAL_detail_rawhash(target_class);
	uint_t iprimary_key = XTAL_detail_uvalue(primary_key);
	uint_t isecondary_key = XTAL_detail_uvalue(secondary_key);

	uint_t hash = itarget_class ^ (iprimary_key ^ (iprimary_key>>24)) ^ isecondary_key;
	Unit& unit = table_[hash % CACHE_MASK];

	if(((member_mutate_count_ ^ unit.mutate_count) | 
		XTAL_detail_rawbitxor(primary_key, unit.primary_key) | 
		XTAL_detail_rawbitxor(target_class, unit.target_class) | 
		XTAL_detail_rawbitxor(secondary_key, unit.secondary_key))==0){

		hit_++;
		accessibility = unit.accessibility;
		return unit.member;
	}
	else{
		if(!XTAL_detail_is_pvalue(target_class)){
			accessibility = -1;
			return undefined;
		}

		miss_++;

		bool nocache = false;
		accessibility = 0;
		const AnyPtr& ret = XTAL_detail_pvalue(target_class)->rawmember(primary_key, secondary_key, true, accessibility, nocache);

		if(XTAL_detail_is_undefined(ret)){
			accessibility = -1;
			return undefined;
		}

		if(!nocache){
			unit.member = ret;
			unit.target_class = target_class;
			unit.primary_key = primary_key;
			unit.secondary_key = secondary_key;
			unit.accessibility = accessibility;
			unit.mutate_count = member_mutate_count_;
		}
		return ret;
	}
}

void VMachine::push_ff(CallState& call_state){
	FunFrame* fp = fun_frames_.push();
	if(XTAL_UNLIKELY(!fp)){ 
		fun_frames_.top() = fp = new_object_xmalloc<FunFrame>();
	}

	FunFrame& f = *fp;
	f.need_result_count = call_state.need_result_count;
	f.ordered_arg_count = call_state.ordered_arg_count;
	f.named_arg_count = call_state.named_arg_count;
	f.poped_pc = call_state.poped_pc;
	f.result = call_state.result;

	f.is_executed = 0;
	f.next_pc = 0;
	f.prev_stack_base = XTAL_VM_variables_top();
	f.scope_lower = scopes_.size();

	XTAL_VM_set(f.self, ap(call_state.aself));
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

	XTAL_VM_LOCK{
		// 戻り値の値を調節する
		if(f.need_result_count!=result_count){
			adjust_values3(variables_.data() + src_base, result_count, f.need_result_count);
		}

		// 戻り値をコピーする
		for(uint_t i=0, sz=f.need_result_count; i<sz; ++i){
			set_local_variable(result_base_ + i, variables_.at(src_base + i));
		}

		//f.fun = null;
		//f.outer = null;
		//f.self = null;
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
	if(XTAL_UNLIKELY(!scope.frame)){
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
		//scope.frame->detach();
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
			if(force || (scope.frame->info()->flags&ScopeInfo::FLAG_SCOPE_CHAIN)){
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
	if(debug::is_enabled() || (scope->flags&ScopeInfo::FLAG_SCOPE_CHAIN)){
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

void VMachine::execute_inner2(const inst_t* start, int_t XTAL_VM_eval_n, ExceptFrame& cur){
	static Any XTAL_VM_values[4] = { null, undefined, Bool(false), Bool(true) };

	register const inst_t* pc = start;
	int_t eval_base_n = fun_frames_.size();
	inst_t xop = 0;

	XTAL_ASSERT(cur.stack_size>=0);

/*		
	union i2f{
		float_t f;
		int_t i;
	};
#define XTAL_VM_bin_to_f2 \
	i2f va; va.i = XTAL_detail_ivalue(a)&(~0+(1-atype));\
	float_t fa = (XTAL_detail_ivalue(a)&(~0+atype)) + va.f;\
	i2f vb; vb.i = XTAL_detail_ivalue(b)&(~0+(1-btype));\
	float_t fb = (XTAL_detail_ivalue(b)&(~0+btype)) + vb.f

#define XTAL_VM_bin_to_f \
	float_t fa = atype?XTAL_detail_fvalue(a):XTAL_detail_ivalue(a);\
	float_t fb = btype?XTAL_detail_fvalue(b):XTAL_detail_ivalue(b)
*/

		/*
		if((atype|btype)==0){
			XTAL_VM_DEC(result);
			result.value_.init_int(XTAL_detail_ivalue(a) + XTAL_detail_ivalue(b));
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
		else if((atype|btype)==1){
			XTAL_VM_bin_to_f;
			XTAL_VM_DEC(result);
			result.value_.init_float(fa + fb);
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
		*/


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

vmloopbegin:
	xop = *pc;
XTAL_VM_LOOP

//{OPS{{
	XTAL_VM_CASE_FIRST(InstLine){ // 3
		XTAL_VM_FUN;
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
		XTAL_detail_copy(result, XTAL_VM_values[inst.value]);
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
			if(atype==0){
				result.value_.init_int(XTAL_detail_ivalue(a)+1);
				XTAL_VM_CONTINUE(pc + inst.ISIZE);
			}
			else{
				result.value_.init_float(XTAL_detail_fvalue(a)+1);
				XTAL_VM_CONTINUE(pc + inst.ISIZE);
			}
		}

		XTAL_VM_CONTINUE(execute_send_una(pc, &inst, DefinedID::id_op_inc));
	}

	XTAL_VM_CASE(InstDec){ // 27
		const AnyPtr& a = XTAL_VM_local_variable(inst.target); uint_t atype = XTAL_detail_urawtype(a)-TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);

		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			if(atype==0){
				result.value_.init_int(XTAL_detail_ivalue(a)-1);
				XTAL_VM_CONTINUE(pc + inst.ISIZE);
			}
			else{
				result.value_.init_float(XTAL_detail_fvalue(a)-1);
				XTAL_VM_CONTINUE(pc + inst.ISIZE);
			}
		}

		XTAL_VM_CONTINUE(execute_send_una(pc, &inst, DefinedID::id_op_dec));
	}

	XTAL_VM_CASE(InstPos){ // 27
		const AnyPtr& a = XTAL_VM_local_variable(inst.target); uint_t atype = XTAL_detail_urawtype(a)-TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);

		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			XTAL_detail_copy(result, a);
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		XTAL_VM_CONTINUE(execute_send_una(pc, &inst, DefinedID::id_op_pos));
	}

	XTAL_VM_CASE(InstNeg){ // 27
		const AnyPtr& a = XTAL_VM_local_variable(inst.target); uint_t atype = XTAL_detail_urawtype(a)-TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);

		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			if(atype==0){
				result.value_.init_int(-XTAL_detail_ivalue(a));
				XTAL_VM_CONTINUE(pc + inst.ISIZE);
			}
			else{
				result.value_.init_float(-XTAL_detail_fvalue(a));
				XTAL_VM_CONTINUE(pc + inst.ISIZE);
			}
		}

		XTAL_VM_CONTINUE(execute_send_una(pc, &inst, DefinedID::id_op_neg));
	}

	XTAL_VM_CASE(InstCom){ // 27
		const AnyPtr& a = XTAL_VM_local_variable(inst.target); uint_t atype = XTAL_detail_urawtype(a);
		AnyPtr& result = XTAL_VM_local_variable(inst.result);

		if(XTAL_LIKELY(atype==TYPE_INT)){
			XTAL_VM_DEC(result);
			result.value_.init_int(~XTAL_detail_ivalue(a));
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		XTAL_VM_CONTINUE(execute_send_una(pc, &inst, DefinedID::id_op_com));
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
				XTAL_CASE((0<<1) | 0){ result.value_.init_int(XTAL_detail_ivalue(a) + XTAL_detail_ivalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
				XTAL_CASE((1<<1) | 0){ result.value_.init_float(XTAL_detail_fvalue(a) + XTAL_detail_ivalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
				XTAL_CASE((0<<1) | 1){ result.value_.init_float(XTAL_detail_ivalue(a) + XTAL_detail_fvalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
				XTAL_CASE((1<<1) | 1){ result.value_.init_float(XTAL_detail_fvalue(a) + XTAL_detail_fvalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
			}
		}

		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_add_assign : DefinedID::id_op_add));
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
				XTAL_CASE((0<<1) | 0){ result.value_.init_int(XTAL_detail_ivalue(a) - XTAL_detail_ivalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
				XTAL_CASE((1<<1) | 0){ result.value_.init_float(XTAL_detail_fvalue(a) - XTAL_detail_ivalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
				XTAL_CASE((0<<1) | 1){ result.value_.init_float(XTAL_detail_ivalue(a) - XTAL_detail_fvalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
				XTAL_CASE((1<<1) | 1){ result.value_.init_float(XTAL_detail_fvalue(a) - XTAL_detail_fvalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
			}
		}

		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_sub_assign : DefinedID::id_op_sub));
	}

	XTAL_VM_CASE(InstCat){ // 50		
		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_cat_assign : DefinedID::id_op_cat));
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
				XTAL_CASE((0<<1) | 0){ result.value_.init_int(XTAL_detail_ivalue(a) * XTAL_detail_ivalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
				XTAL_CASE((1<<1) | 0){ result.value_.init_float(XTAL_detail_fvalue(a) * XTAL_detail_ivalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
				XTAL_CASE((0<<1) | 1){ result.value_.init_float(XTAL_detail_ivalue(a) * XTAL_detail_fvalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
				XTAL_CASE((1<<1) | 1){ result.value_.init_float(XTAL_detail_fvalue(a) * XTAL_detail_fvalue(b)); XTAL_VM_CONTINUE(pc + inst.ISIZE); } 
			}
		}

		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_mul_assign : DefinedID::id_op_mul));

	}
	
	XTAL_VM_CASE(InstDiv){ // 50
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype|btype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			if(btype==0){
				if(XTAL_detail_ivalue(b)==0){
					result.value_.init_null();
					XTAL_VM_CONTINUE(execute_divzero());
				}

				if(atype==0){
					result.value_.init_int(XTAL_detail_ivalue(a) / XTAL_detail_ivalue(b));
					XTAL_VM_CONTINUE(pc + inst.ISIZE);
				}
				else{
					result.value_.init_float(XTAL_detail_fvalue(a) / (float_t)XTAL_detail_ivalue(b));
					XTAL_VM_CONTINUE(pc + inst.ISIZE);
				}
			}
			else{
				if(XTAL_detail_fvalue(b)==0){
					result.value_.init_null();
					XTAL_VM_CONTINUE(execute_divzero());
				}

				if(atype==0){
					result.value_.init_float((float_t)XTAL_detail_ivalue(a) / XTAL_detail_fvalue(b));
					XTAL_VM_CONTINUE(pc + inst.ISIZE);
				}
				else{
					result.value_.init_float(XTAL_detail_fvalue(a) / XTAL_detail_fvalue(b));
					XTAL_VM_CONTINUE(pc + inst.ISIZE);
				}
			}
		}

		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_div_assign : DefinedID::id_op_div));
	}

	XTAL_VM_CASE(InstMod){ // 50
		using namespace std;
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a) - TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b) - TYPE_INT;
		AnyPtr& result = XTAL_VM_local_variable(inst.result);
		
		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype|btype)&(~1U))==0)){
			XTAL_VM_DEC(result);
			if(btype==0){
				if(XTAL_detail_ivalue(b)==0){
					result.value_.init_null();
					XTAL_VM_CONTINUE(execute_divzero());
				}

				if(atype==0){
					result.value_.init_int(XTAL_detail_ivalue(a) % XTAL_detail_ivalue(b));
					XTAL_VM_CONTINUE(pc + inst.ISIZE);
				}
				else{
					result.value_.init_float(fmodf(XTAL_detail_fvalue(a), (float_t)XTAL_detail_ivalue(b)));
					XTAL_VM_CONTINUE(pc + inst.ISIZE);
				}
			}
			else{
				if(XTAL_detail_fvalue(b)==0){
					result.value_.init_null();
					XTAL_VM_CONTINUE(execute_divzero());
				}

				if(atype==0){
					result.value_.init_float(fmodf((float_t)XTAL_detail_ivalue(a), XTAL_detail_fvalue(b)));
					XTAL_VM_CONTINUE(pc + inst.ISIZE);
				}
				else{
					result.value_.init_float(fmodf(XTAL_detail_fvalue(a), XTAL_detail_fvalue(b)));
					XTAL_VM_CONTINUE(pc + inst.ISIZE);
				}
			}
		}

		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_mod_assign : DefinedID::id_op_mod));
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

		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_and_assign : DefinedID::id_op_and));
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

		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_or_assign : DefinedID::id_op_or));
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

		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_xor_assign : DefinedID::id_op_xor));
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

		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_shl_assign : DefinedID::id_op_shl));
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

		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_shr_assign : DefinedID::id_op_shr));
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

		XTAL_VM_CONTINUE(execute_send_bin(pc, &inst, inst.assign ? DefinedID::id_op_ushr_assign : DefinedID::id_op_ushr));
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
			CallState call_state;
			call_state.set(pc, pc+inst.ISIZE, inst.result, 1, inst.stack_base, 1, 0, 0);
			call_state.atarget = a;
			XTAL_VM_CONTINUE(execute_send_iprimary_nosecondary(pc, DefinedID::id_op_at, call_state));
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
			CallState call_state;
			call_state.set(pc, pc + inst.ISIZE, inst.stack_base, 0, inst.stack_base, 2, 0, 0);
			call_state.atarget = a;
			XTAL_VM_CONTINUE(execute_send_iprimary_nosecondary(pc, DefinedID::id_op_set_at, call_state));
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
			switch((atype<<1) | (btype)){
				XTAL_NODEFAULT;
				XTAL_CASE((0<<1) | 0){ XTAL_VM_CONTINUE(pc + inst.ISIZE + (XTAL_detail_ivalue(a)==XTAL_detail_ivalue(b) ? inst2.address_true : inst2.address_false)); } 
				XTAL_CASE((1<<1) | 0){ XTAL_VM_CONTINUE(pc + inst.ISIZE + (XTAL_detail_fvalue(a)==XTAL_detail_ivalue(b) ? inst2.address_true : inst2.address_false)); } 
				XTAL_CASE((0<<1) | 1){ XTAL_VM_CONTINUE(pc + inst.ISIZE + (XTAL_detail_ivalue(a)==XTAL_detail_fvalue(b) ? inst2.address_true : inst2.address_false)); } 
				XTAL_CASE((1<<1) | 1){ XTAL_VM_CONTINUE(pc + inst.ISIZE + (XTAL_detail_fvalue(a)==XTAL_detail_fvalue(b) ? inst2.address_true : inst2.address_false)); } 
			}
		}

		if(XTAL_detail_raweq(a, b)){
			XTAL_VM_CONTINUE(inst2.address_true + pc + inst.ISIZE);
		}		

		XTAL_VM_CONTINUE(execute_send_comp(pc, &inst, DefinedID::id_op_eq));
	}

	XTAL_VM_CASE(InstIfLt){ // 41
		XTAL_CHECK_YIELD;
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);
	
		AnyPtr& a = XTAL_VM_local_variable(inst.lhs); uint_t atype = XTAL_detail_urawtype(a)-TYPE_INT;
		AnyPtr& b = XTAL_VM_local_variable(inst.rhs); uint_t btype = XTAL_detail_urawtype(b)-TYPE_INT;

		// 型がintかfloatであるか？
		if(XTAL_LIKELY(((atype|btype)&(~1U))==0)){
			switch((atype<<1) | (btype)){
				XTAL_NODEFAULT;
				XTAL_CASE((0<<1) | 0){ XTAL_VM_CONTINUE(pc + inst.ISIZE + (XTAL_detail_ivalue(a)<XTAL_detail_ivalue(b) ? inst2.address_true : inst2.address_false)); } 
				XTAL_CASE((1<<1) | 0){ XTAL_VM_CONTINUE(pc + inst.ISIZE + (XTAL_detail_fvalue(a)<XTAL_detail_ivalue(b) ? inst2.address_true : inst2.address_false)); } 
				XTAL_CASE((0<<1) | 1){ XTAL_VM_CONTINUE(pc + inst.ISIZE + (XTAL_detail_ivalue(a)<XTAL_detail_fvalue(b) ? inst2.address_true : inst2.address_false)); } 
				XTAL_CASE((1<<1) | 1){ XTAL_VM_CONTINUE(pc + inst.ISIZE + (XTAL_detail_fvalue(a)<XTAL_detail_fvalue(b) ? inst2.address_true : inst2.address_false)); } 
			}
		}

		XTAL_VM_CONTINUE(execute_send_comp(pc, &inst, DefinedID::id_op_lt));
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
		XTAL_VM_CONTINUE(execute_send_comp(pc, &inst, DefinedID::id_op_in));
	}

	XTAL_VM_CASE(InstIfUndefined){ // 3
		XTAL_CHECK_YIELD;
		XTAL_VM_CONTINUE(pc + (XTAL_detail_is_undefined(XTAL_VM_local_variable(inst.target)) ? inst.address_true : inst.address_false));
	}

	XTAL_VM_CASE(InstIfDebug){ // 2
		XTAL_VM_CONTINUE(pc + (debug::is_enabled() ? (int)inst.ISIZE : (int)inst.address));
	}

	XTAL_VM_CASE(InstPush){ // 3
		stack_.push(XTAL_VM_local_variable(inst.target));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstPop){ // 3
		AnyPtr ret = stack_.top(); stack_.pop();
		set_local_variable(inst.result, ret);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstAdjustValues){ // 3
		adjust_values2(inst.stack_base, inst.result_count, inst.need_result_count);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstLocalVariable){ // 3
		set_local_variable(inst.result, local_variable_out_of_fun(inst.number, inst.depth)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstSetLocalVariable){ // 3
		set_local_variable_out_of_fun(inst.number, inst.depth, XTAL_VM_local_variable(inst.target)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstInstanceVariable){ // 3
		set_local_variable(inst.result, XTAL_VM_ff().self->instance_variables()->variable(inst.number, XTAL_VM_ff().code->class_info(inst.info_number)));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstSetInstanceVariable){ // 3
		XTAL_VM_ff().self->instance_variables()->set_variable(inst.number, XTAL_VM_ff().code->class_info(inst.info_number), XTAL_VM_local_variable(inst.value));
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstInstanceVariableByName){ // 3
		if(XTAL_VM_eval_n){
			const AnyPtr& ret = eval_instance_variable(XTAL_VM_ff().self, XTAL_VM_ff().identifiers[inst.identifier_number]);
			set_local_variable(inst.result, ret);
		}

		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstSetInstanceVariableByName){ // 3
		if(XTAL_VM_eval_n){
			eval_set_instance_variable(XTAL_VM_ff().self, XTAL_VM_ff().identifiers[inst.identifier_number], XTAL_VM_local_variable(inst.value));
		}

		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstFilelocalVariable){ // 3
		set_local_variable(inst.result, XTAL_VM_ff().code->member_direct(inst.value_number));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstSetFilelocalVariable){ // 3
		XTAL_VM_ff().code->Frame::set_member_direct(inst.value_number, XTAL_VM_local_variable(inst.value));
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstFilelocalVariableByName){ // 8
		if(XTAL_VM_eval_n) {
			const AnyPtr& ret = eval_local_variable(XTAL_VM_ff().identifiers[inst.identifier_number], XTAL_VM_eval_n + (fun_frames_.size()-eval_base_n));
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

	XTAL_VM_CASE(InstSetFilelocalVariableByName){ // 3
		if(XTAL_VM_eval_n){
			if(eval_set_local_variable(XTAL_VM_ff().identifiers[inst.identifier_number], XTAL_VM_local_variable(inst.value), XTAL_VM_eval_n + (fun_frames_.size()-eval_base_n))){
				XTAL_VM_CONTINUE(pc + inst.ISIZE);
			}
		}

		XTAL_VM_THROW_EXCEPT(filelocal_unsupported_error(XTAL_VM_ff().code, XTAL_VM_ff().identifiers[inst.identifier_number]));
	}

	XTAL_VM_CASE(InstMember){ // 11
		CallState call_state;
		call_state.result = inst.result;
		call_state.need_result_count = 1;
		call_state.poped_pc = pc + inst.ISIZE;
		call_state.flags = 0;

		call_state.aprimary = XTAL_VM_ff().identifiers[inst.primary];
		call_state.asecondary = undefined;
		call_state.acls = XTAL_VM_local_variable(inst.target);
		call_state.aself = XTAL_VM_ff().self;

		XTAL_VM_LOCK{
			int_t accessibility;
			call_state.amember = environment_->member_cache_table_.cache(ap(call_state.acls), (IDPtr&)call_state.aprimary, accessibility);

			if(accessibility){
				if(accessibility<0){
					XTAL_VM_CONTINUE(execute_member2q(pc, call_state));
				}
				else if(const inst_t* epc = check_accessibility(call_state, accessibility)){
					XTAL_VM_CONTINUE(epc);
				}
			}

			set_local_variable(call_state.result, ap(call_state.amember));
			XTAL_VM_CONTINUE(call_state.poped_pc);
		}
	}

	XTAL_VM_CASE(InstMemberEx){ // 11
		CallState call_state;
		int_t flags = inst.flags;
		call_state.poped_pc = pc + inst.ISIZE;
		call_state.result = inst.result;
		call_state.need_result_count = 1;
		call_state.flags = flags;

		call_state.aprimary = (flags&MEMBER_FLAG_P_BIT) ? unchecked_ptr_cast<ID>(XTAL_VM_local_variable(inst.primary)) : XTAL_VM_ff().identifiers[inst.primary];
		call_state.asecondary = (flags&MEMBER_FLAG_S_BIT) ? XTAL_VM_local_variable(inst.secondary) : undefined;
		call_state.acls = XTAL_VM_local_variable(inst.target);
		call_state.aself = XTAL_VM_ff().self;

		XTAL_VM_LOCK{
			int_t accessibility;
			call_state.amember = environment_->member_cache_table2_.cache(ap(call_state.acls), (IDPtr&)call_state.aprimary, ap(call_state.asecondary), accessibility);

			if(accessibility){
				if(accessibility<0){
					XTAL_VM_CONTINUE(execute_member2q(pc, call_state));
				}
				else if(const inst_t* epc = check_accessibility(call_state, accessibility)){
					XTAL_VM_CONTINUE(epc);
				}
			}

			set_local_variable(call_state.result, ap(call_state.amember));
			XTAL_VM_CONTINUE(call_state.poped_pc);
		}
	}

	XTAL_VM_CASE(InstCall){ // 6
		CallState call_state;
		call_state.set(pc, pc + inst.ISIZE, inst.result, inst.need_result, inst.stack_base, inst.ordered, 0, 0);
		call_state.amember = XTAL_VM_local_variable(inst.target);
		call_state.aself = XTAL_VM_ff().self;
		XTAL_VM_CONTINUE(execute_call(pc, call_state));
	}

	XTAL_VM_CASE(InstCallEx){ // 6
		CallState call_state;
		int_t flags = inst.flags;
		call_state.set(pc, pc + inst.ISIZE, inst.result, inst.need_result, inst.stack_base, inst.ordered, inst.named, flags);

		call_state.amember = XTAL_VM_local_variable(inst.target);
		call_state.aself = (flags&CALL_FLAG_THIS) ? XTAL_VM_local_variable(inst.self) : XTAL_VM_ff().self;
		
		XTAL_VM_CONTINUE(execute_callex(pc, call_state));
	}

	XTAL_VM_CASE(InstSend){ // 8
		CallState call_state;
		call_state.set(pc, pc + inst.ISIZE, inst.result, inst.need_result, inst.stack_base, inst.ordered, 0, 0);

		call_state.atarget = XTAL_VM_local_variable(inst.target);
		call_state.aprimary = XTAL_VM_ff().identifiers[inst.primary];
		call_state.asecondary = undefined;
		call_state.aself = XTAL_VM_ff().self;
		call_state.acls = call_state.atarget.get_class();

		XTAL_VM_CONTINUE(execute_send(pc, call_state));
	}

	XTAL_VM_CASE(InstSendEx){ // 8
		CallState call_state;
		int_t flags = inst.flags;
		call_state.set(pc, pc + inst.ISIZE, inst.result, inst.need_result, inst.stack_base, inst.ordered, inst.named, flags);
		call_state.atarget = XTAL_VM_local_variable(inst.target);
		call_state.aprimary = (flags&MEMBER_FLAG_P_BIT) ? unchecked_ptr_cast<ID>(XTAL_VM_local_variable(inst.primary)) : XTAL_VM_ff().identifiers[inst.primary];
		call_state.asecondary = (flags&MEMBER_FLAG_S_BIT) ? XTAL_VM_local_variable(inst.secondary) : undefined;
		call_state.aself = XTAL_VM_ff().self;
		call_state.acls = call_state.atarget.get_class();

		XTAL_VM_CONTINUE(execute_sendex(pc, call_state));
	}

	XTAL_VM_CASE(InstProperty){ // 7
		CallState call_state;
		call_state.set(pc, pc + inst.ISIZE, inst.result, 1, inst.stack_base, 0, 0, 0);
		call_state.atarget = XTAL_VM_local_variable(inst.target);
		call_state.aprimary = XTAL_VM_ff().identifiers[inst.primary];
		call_state.asecondary = undefined;
		call_state.aself = XTAL_VM_ff().self;
		call_state.acls = call_state.atarget.get_class();

		XTAL_VM_LOCK{
			int_t accessibility;
			call_state.amember = environment_->member_cache_table_.cache(ap(call_state.acls), (IDPtr&)call_state.aprimary, accessibility);

			if(accessibility){
				if(accessibility<0){
					XTAL_VM_CONTINUE(execute_member2q(pc, call_state));
				}				
				else if(const inst_t* epc = check_accessibility(call_state, accessibility)){
					XTAL_VM_CONTINUE(epc);
				}
			}

			if(XTAL_detail_type(call_state.amember)==TYPE_IVAR_GETTER){
				InstanceVariableGetter* p = unchecked_cast<InstanceVariableGetter*>(ap(call_state.amember));
				set_local_variable(call_state.result, call_state.atarget.instance_variables()->variable(p->number(), p->class_info()));
				XTAL_VM_CONTINUE(call_state.poped_pc);
			}

			call_state.aself = call_state.atarget;
		}

		XTAL_VM_CONTINUE(execute_callex(pc, call_state));
	}

	XTAL_VM_CASE(InstSetProperty){ // 7
		CallState call_state;
		call_state.set(pc, pc + inst.ISIZE, inst.stack_base, 0, inst.stack_base, 1, 0, 0);
		call_state.atarget = XTAL_VM_local_variable(inst.target);
		call_state.aprimary = XTAL_VM_ff().identifiers[inst.primary];
		call_state.asecondary = undefined;
		call_state.aself = XTAL_VM_ff().self;
		call_state.acls = call_state.atarget.get_class();

		XTAL_VM_LOCK{
			int_t accessibility;
			call_state.amember = environment_->member_cache_table_.cache(ap(call_state.acls), (IDPtr&)call_state.aprimary, accessibility);

			if(accessibility){
				if(accessibility<0){
					XTAL_VM_CONTINUE(execute_member2q(pc, call_state));
				}
				else if(const inst_t* epc = check_accessibility(call_state, accessibility)){
					XTAL_VM_CONTINUE(epc);
				}
			}

			if(XTAL_detail_type(call_state.amember)==TYPE_IVAR_SETTER){
				InstanceVariableSetter* p = unchecked_cast<InstanceVariableSetter*>(ap(call_state.amember));
				call_state.atarget.instance_variables()->set_variable(p->number(), p->class_info(), XTAL_VM_local_variable(call_state.stack_base));
				XTAL_VM_CONTINUE(call_state.poped_pc);
			}

			call_state.aself = call_state.atarget;
		}

		XTAL_VM_CONTINUE(execute_callex(pc, call_state));
	}

	XTAL_VM_CASE(InstScopeBegin){ // 3
		push_scope(XTAL_VM_ff().code->scope_info(inst.info_number));
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstScopeEnd){ // 3
		pop_scope();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstReturn){ // 7
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
		CallState call_state;
		call_state.set(pc, pc + inst.ISIZE, inst.result, 1, inst.stack_base, 2, 0, 0);
		call_state.atarget = a;
		XTAL_VM_CONTINUE(execute_send_iprimary_nosecondary(pc, DefinedID::id_op_range, call_state));
	}

	XTAL_VM_CASE(InstOnce){ // 5
		const AnyPtr& ret = XTAL_VM_ff().code->once_value(inst.value_number);
		if(!XTAL_detail_is_undefined(ret)){
			set_local_variable(inst.result, ret);
			XTAL_VM_CONTINUE(pc + inst.address);
		}
		else{
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
	}

	XTAL_VM_CASE(InstSetOnce){ // 3
		XTAL_VM_ff().code->set_once_value(inst.value_number, XTAL_VM_local_variable(inst.target)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstMakeArray){ // 4
		SmartPtr<Array> value = xnew<Array>();
		set_local_variable(inst.result, value);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstArrayAppend){ // 3
		ptr_cast<Array>(XTAL_VM_local_variable(inst.target))->push_back(XTAL_VM_local_variable(inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstMakeMap){ // 4
		SmartPtr<Map> value = xnew<Map>();
		set_local_variable(inst.result, value);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstMapInsert){ // 3
		ptr_cast<Map>(XTAL_VM_local_variable(inst.target))->set_at(XTAL_VM_local_variable(inst.key), XTAL_VM_local_variable(inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstMapSetDefault){ // 3
		ptr_cast<Map>(XTAL_VM_local_variable(inst.target))->set_default_value(XTAL_VM_local_variable(inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(InstClassBegin){ XTAL_VM_CONTINUE(FunInstClassBegin(pc)); /*
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
		call_state.clear();
		call_state.aself = cp;
		call_state.poped_pc = pc + inst.ISIZE;
		call_state.result = 0;
		call_state.need_result_count = 0;
		call_state.stack_base = 0;
		call_state.ordered_arg_count = 0;
		call_state.named_arg_count = 0;
		push_ff(call_state);

		XTAL_VM_ff().fun = XTAL_VM_prev_ff().fun;
		XTAL_VM_ff().outer = cp;
		XTAL_VM_ff().identifiers = XTAL_VM_prev_ff().identifiers;
		XTAL_VM_ff().code = XTAL_VM_prev_ff().code;
		XTAL_VM_ff().values = XTAL_VM_prev_ff().values;

		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(InstClassEnd){ XTAL_VM_CONTINUE(FunInstClassEnd(pc)); /*
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
	}*/ }

	XTAL_VM_CASE(InstDefineClassMember){ XTAL_VM_CONTINUE(FunInstDefineClassMember(pc)); /*
		XTAL_VM_FUN;
		if(const ClassPtr& p = ptr_cast<Class>(scopes_.top().frame)){
			p->set_member_direct(inst.number, XTAL_VM_ff().identifiers[inst.primary], XTAL_VM_local_variable(inst.value), XTAL_VM_local_variable(inst.secondary), inst.accessibility);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(InstDefineMember){ XTAL_VM_CONTINUE(FunInstDefineMember(pc)); /*
		XTAL_VM_FUN;
		IDPtr primary = (inst.flags&MEMBER_FLAG_P_BIT) ? unchecked_ptr_cast<ID>(XTAL_VM_local_variable(inst.primary)) : XTAL_VM_ff().identifiers[inst.primary];
		AnyPtr secondary = (inst.flags&MEMBER_FLAG_S_BIT) ? XTAL_VM_local_variable(inst.secondary) : undefined;
		AnyPtr cls = XTAL_VM_local_variable(inst.target);
		AnyPtr value = XTAL_VM_local_variable(inst.value);
		cls->def(primary, value, secondary, KIND_PUBLIC);
		XTAL_VM_CHECK_EXCEPT;
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}*/ }

	XTAL_VM_CASE(InstMakeFun){ XTAL_VM_CONTINUE(FunInstMakeFun(pc)); /*
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

	XTAL_VM_CASE(InstMakeInstanceVariableAccessor){ XTAL_VM_CONTINUE(FunInstMakeInstanceVariableAccessor(pc)); /*
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
		ExceptFrame& ef = except_frames_.push();
		ef.info = XTAL_VM_ff().code->except_info(inst.info_number);
		ef.fun_frame_size = fun_frames_.size();
		ef.stack_size = stack_.size();
		ef.scope_size = scopes_.size();
		ef.variables_top = XTAL_VM_variables_top();
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

	XTAL_VM_CASE(InstThrow){ // 12
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

	XTAL_VM_CASE(InstAssert){ XTAL_VM_CONTINUE(FunInstAssert(pc)); /*
		XTAL_VM_FUN;
		set_except_x(cpp_class<AssertionFailed>()->call(ptr_cast<String>(XTAL_VM_local_variable(inst.message))));
		breakpoint_hook(pc, XTAL_VM_ff().fun, BREAKPOINT_ASSERT);

		if(except_[0]){
			XTAL_VM_THROW_EXCEPT(except_[0]);
		}

		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(InstBreakPoint){ // 5
		XTAL_VM_FUN;
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

////////////////////////////////////////////////////////////////

const inst_t* VMachine::execute_send(const inst_t* pc, CallState& call_state){
	XTAL_VM_LOCK{
		int_t accessibility;
		call_state.amember = environment_->member_cache_table_.cache(ap(call_state.acls), (IDPtr&)call_state.aprimary, accessibility);

		if(accessibility){
			if(accessibility<0){
				XTAL_VM_CONTINUE(execute_member2q(pc, call_state));
			}
			else if(const inst_t* epc = check_accessibility(call_state, accessibility)){
				XTAL_VM_CONTINUE(epc);
			}
		}

		call_state.aself = call_state.atarget;
	}

	XTAL_VM_CONTINUE(execute_call(pc, call_state));
}

const inst_t* VMachine::execute_sendex(const inst_t* pc, CallState& call_state){
	XTAL_VM_LOCK{
		int_t accessibility;
		call_state.amember = environment_->member_cache_table2_.cache(ap(call_state.acls), (IDPtr&)call_state.aprimary, ap(call_state.asecondary), accessibility);

		if(accessibility){
			if(accessibility<0){
				XTAL_VM_CONTINUE(execute_member2q(pc, call_state));
			}
			else if(const inst_t* epc = check_accessibility(call_state, accessibility)){
				XTAL_VM_CONTINUE(epc);
			}
		}

		call_state.aself = call_state.atarget;
	}

	XTAL_VM_CONTINUE(execute_callex(pc, call_state));
}

const inst_t* VMachine::execute_callex(const inst_t* pc, CallState& call_state){
	if(call_state.flags&CALL_FLAG_ARGS_BIT){
		XTAL_VM_LOCK{
			if(ArgumentsPtr args = ptr_cast<Arguments>(XTAL_VM_local_variable(call_state.stack_base+call_state.ordered_arg_count+call_state.named_arg_count*2))){
				push_args(args, call_state.stack_base, call_state.ordered_arg_count, call_state.named_arg_count);
				call_state.ordered_arg_count += args->ordered_size();
				call_state.named_arg_count += args->named_size();
			}
		}
	}

	XTAL_VM_CONTINUE(execute_call(pc, call_state));
}

const inst_t* VMachine::execute_call(const inst_t* pc, CallState& call_state){
	XTAL_VM_LOCK{
		push_ff(call_state);
		call_state.amember.rawcall(to_smartptr(this));

		if(except_[0]){
			XTAL_VM_CONTINUE(push_except(XTAL_VM_ff().next_pc));
		}
		else{
			XTAL_VM_CONTINUE(XTAL_VM_ff().next_pc);
		}
	}
}

const inst_t* VMachine::execute_member2q(const inst_t* pc, CallState& call_state){
	XTAL_VM_LOCK{
		XTAL_VM_CHECK_EXCEPT;

		if(call_state.flags&MEMBER_FLAG_Q_BIT){
			for(int_t i=0; i<call_state.need_result_count; ++i){
				set_local_variable(call_state.result+i, undefined);
			}
			XTAL_VM_CONTINUE(call_state.poped_pc);
		}

		XTAL_VM_CONTINUE(push_except(call_state.pc, unsupported_error(ap(call_state.acls), (IDPtr&)call_state.aprimary, ap(call_state.asecondary))));
	}
}

const inst_t* VMachine::execute_send_iprimary_nosecondary(const inst_t* pc, int_t iprimary, CallState& call_state){
	XTAL_VM_LOCK{
		call_state.aprimary = fetch_defined_id(iprimary);
		call_state.asecondary = undefined;
		call_state.aself = XTAL_VM_ff().self;
		call_state.acls = call_state.atarget.get_class();

		XTAL_VM_CONTINUE(execute_send(pc, call_state));
	}
}

const inst_t* VMachine::execute_send_comp(const inst_t* pc, const void* pinst, int_t iprimary){
	const InstIfEq& inst = *(const InstIfEq*)pinst;
	typedef InstIf InstType2; 
	InstType2& inst2 = *(InstType2*)((u8*)pinst+inst.ISIZE);
	set_local_variable(inst.stack_base, XTAL_VM_local_variable(inst.rhs));
	CallState call_state;
	call_state.set(pc, pc+inst.ISIZE, inst2.target, 1, inst.stack_base, 1, 0, MEMBER_FLAG_Q_BIT);
	call_state.atarget = XTAL_VM_local_variable(inst.lhs);

	XTAL_VM_CONTINUE(execute_send_iprimary_nosecondary(pc, iprimary, call_state));
}

const inst_t* VMachine::execute_send_bin(const inst_t* pc, const void* pinst, int_t iprimary){
	const InstAdd& inst = *(const InstAdd*)pinst;
	set_local_variable(inst.stack_base, XTAL_VM_local_variable(inst.rhs));
	CallState call_state;
	call_state.set(pc, pc+inst.ISIZE, inst.result, 1, inst.stack_base, 1, 0, 0);
	call_state.atarget = XTAL_VM_local_variable(inst.lhs);

	XTAL_VM_CONTINUE(execute_send_iprimary_nosecondary(pc, iprimary, call_state));
}

const inst_t* VMachine::execute_send_una(const inst_t* pc, const void* pinst, int_t iprimary){
	const InstInc& inst = *(const InstInc*)pinst;
	CallState call_state;
	call_state.set(pc, pc+inst.ISIZE, inst.result, 1, inst.stack_base, 0, 0, 0);
	call_state.atarget = XTAL_VM_local_variable(inst.target);

	XTAL_VM_CONTINUE(execute_send_iprimary_nosecondary(pc, iprimary, call_state));
}

const inst_t* VMachine::execute_divzero(){
	XTAL_VM_LOCK{
		set_runtime_error(Xt("XRE1024"), to_smartptr(this));
	}

	XTAL_VM_CONTINUE(&throw_code_);
}

////////////////////////////////////////////////////////////////

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
		call_state.clear();
		call_state.aself = cp;
		call_state.poped_pc = pc + inst.ISIZE;
		call_state.result = 0;
		call_state.need_result_count = 0;
		call_state.stack_base = 0;
		call_state.ordered_arg_count = 0;
		call_state.named_arg_count = 0;
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

