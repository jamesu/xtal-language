#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

NativeFunPtr new_native_fun(const param_types_holder_n& pth, const void* val){
	return xnew<NativeMethod>(pth, val);
}

NativeFunPtr new_native_fun(const param_types_holder_n& pth, const void* val, const AnyPtr& this_){
	return xnew<NativeFun>(pth, val, this_);
}

StatelessNativeMethod::StatelessNativeMethod(const param_types_holder_n& pth){
	value_.init_stateless_native_method(&pth);
}

void StatelessNativeMethod::on_rawcall(const VMachinePtr& vm) const{
	const param_types_holder_n& pth = *XTAL_detail_pthvalue(*this);
	int_t param_n = pth.param_n;

	if(vm->ordered_arg_count()!=param_n){
		if(!pth.vm){
			if(!pth.extendable || vm->ordered_arg_count()<param_n){
				set_argument_num_error(Xid(NativeFunction), vm->ordered_arg_count(), param_n, param_n, vm);
				return;
			}
		}
	}

	FunctorParam fp;
	fp.vm = &*vm;
	fp.fun = 0;
	fp.result = undefined;

	{ // check arg type

		const CppClassSymbolData* anycls = &CppClassSymbol<Any>::value;
		const CppClassSymbolData* intcls = &CppClassSymbol<Int>::value;
		const CppClassSymbolData* floatcls = &CppClassSymbol<Float>::value;

		int_t num = 1;
		fp.args[0] = (Any&)vm->arg_this();

		if(!pth.vm){
			if(pth.extendable){
				vm->set_local_variable(param_n, vm->make_arguments(param_n));
				fp.args[param_n+1] = (Any&)vm->arg_unchecked(param_n);
			}

			for(int_t i=0; i<param_n; ++i){
				fp.args[i+1] = (Any&)vm->arg_unchecked(i);
			}

			num = param_n+1;
		}
			
		for(int_t i=0; i<num; ++i){
			if(pth.param_types[i]==anycls){
				continue;
			}
			else{
				const AnyPtr& arg = ap(fp.args[i]);
				const ClassPtr& cls = cpp_class(pth.param_types[i]);
				if(!arg->is(cls)){ 
					set_argument_type_error(Xid(NativeFunction), i, cls, arg->get_class(), vm);
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

NativeMethod::NativeMethod(const param_types_holder_n& pth, const void* val){
	pth_ = &pth;

	val_size_ = pth.size==1 ? 0 : pth.size;
	min_param_count_ = pth.vm ? 0 : pth.param_n;
	max_param_count_ = (pth.vm || pth.extendable) ? 255 : pth.param_n;

	uint_t data_size = val_size_ + pth.param_n*sizeof(NamedParam);
	data_ = data_size==0 ? 0 : xmalloc(data_size);

	// 関数をコピー
	std::memcpy(data_, val, val_size_);

	if(pth.param_n){
		NamedParam* params = (NamedParam*)((u8*)data_ + val_size_);
		for(int_t i=0; i<pth.param_n; ++i){
			new(&params[i]) NamedParam();
		}
	}
}

NativeMethod::~NativeMethod(){
	const param_types_holder_n& pth = *pth_;
	NamedParam* params = (NamedParam*)((u8*)data_ + val_size_);

	for(int_t i=0; i<pth.param_n; ++i){
		params[i].~NamedParam();
	}

	uint_t data_size = val_size_ + pth.param_n*sizeof(NamedParam);
	xfree(data_, data_size);
}

const NativeFunPtr& NativeMethod::param(int_t i, const IDPtr& key, const AnyPtr& value){
	// iは1始まり
	XTAL_ASSERT(i!=0);

	// 今のところ、可変長引数をとる関数は、paramは設定でない制限あり
	XTAL_ASSERT(pth_->vm==0 && pth_->extendable==0);

	// iが引数の数より大きすぎる
	XTAL_ASSERT(i<=pth_->param_n);

	i--;

	NamedParam* params = (NamedParam*)((u8*)data_ + val_size_);

	// 既に設定済み
	XTAL_ASSERT(XTAL_detail_raweq(params[i].name, null) && XTAL_detail_raweq(params[i].value, undefined));

	params[i].name = key;
	params[i].value = value;

	if(min_param_count_>i){
		min_param_count_ = (u8)i;
	}

	return to_smartptr(this);
}

const NativeFunPtr& NativeMethod::add_param(const IDPtr& key, const AnyPtr& value){
	NamedParam* params = (NamedParam*)((u8*)data_ + val_size_);

	// 今のところ、可変長引数をとる関数は、paramは設定でない制限あり
	XTAL_ASSERT(pth_->vm==0 && pth_->extendable==0);

	// 引数の数よりparamを多く設定している
	XTAL_ASSERT(min_param_count_>0);

	min_param_count_--;
	for(int_t i=min_param_count_; i<max_param_count_-1; ++i){
		params[i] = params[i+1];
	}

	params[max_param_count_-1].name = key;
	params[max_param_count_-1].value = value;
	
	return to_smartptr(this);
}


void NativeMethod::on_visit_members(Visitor& m){
	RefCountingBase::on_visit_members(m);
	NamedParam* params = (NamedParam*)((u8*)data_ + val_size_);

	const param_types_holder_n& pth = *pth_;
	for(uint_t i=0; i<pth.param_n; ++i){
		m & params[i].name & params[i].value;
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
					vm->adjust_args(params, pth.param_n);
				}
			}

			for(int_t i=0; i<param_n; ++i){
				fp.args[i+1] = (Any&)vm->arg_unchecked(i);
			}
			num = pth.param_n+1;
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


NativeFun::NativeFun(const param_types_holder_n& pth, const void* val, const AnyPtr& this_)
:NativeMethod(pth, val), this_(this_){
}

void NativeFun::on_visit_members(Visitor& m){
	NativeMethod::on_visit_members(m);
	m & this_;
}

void NativeFun::on_rawcall(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	NativeMethod::on_rawcall(vm);
}

DoubleDispatchMethod::DoubleDispatchMethod(const IDPtr& primary_key)
	:primary_key_(primary_key){}

void DoubleDispatchMethod::on_rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()>0){
		vm->arg_this()->rawsend(vm, primary_key_, vm->arg(0)->get_class());
	}
}

DoubleDispatchFun::DoubleDispatchFun(const ClassPtr& klass, const IDPtr& primary_key)
	:klass_(klass), primary_key_(primary_key){}

void DoubleDispatchFun::on_rawcall(const VMachinePtr& vm){
	klass_->member(primary_key_, vm->arg(0)->get_class())->rawcall(vm);
}

void DoubleDispatchFun::on_visit_members(Visitor& m){
	Base::on_visit_members(m);
	m & klass_;
}

DoubleDispatchMethodPtr double_dispatch_method(const IDPtr& primary_key){
	return xnew<DoubleDispatchMethod>(primary_key);
}

DoubleDispatchFunPtr double_dispatch_fun(const ClassPtr& klass, const IDPtr& primary_key){
	return xnew<DoubleDispatchFun>(klass, primary_key);
}

}
