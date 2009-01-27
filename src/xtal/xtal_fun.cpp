#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Arguments::Arguments(const AnyPtr& ordered, const AnyPtr& named){
	if(ordered){ ordered_ = ptr_cast<Array>(ordered); }
	else{ ordered_ = xnew<Array>(); }
	if(named){ named_ = ptr_cast<Map>(named); }
	else{ named_ = xnew<Map>(); }
}

void Arguments::visit_members(Visitor& m){
	Base::visit_members(m);
	m & ordered_ & named_;
}

InstanceVariableGetter::InstanceVariableGetter(int_t number, ClassCore* core)
	:number_(number), core_(core){
}

void InstanceVariableGetter::rawcall(const VMachinePtr& vm){
	const AnyPtr& self = vm->get_arg_this();
	InstanceVariables* p;
	if(type(self)==TYPE_BASE){
		p = pvalue(self)->instance_variables();
	}
	else{
		p = &empty_instance_variables;
	}
	vm->return_result(p->variable(number_, core_));
}

InstanceVariableSetter::InstanceVariableSetter(int_t number, ClassCore* core)
	:number_(number), core_(core){
}

void InstanceVariableSetter::rawcall(const VMachinePtr& vm){
	const AnyPtr& self = vm->get_arg_this();
	InstanceVariables* p;
	if(type(self)==TYPE_BASE){
		p = pvalue(self)->instance_variables();
	}
	else{
		p = &empty_instance_variables;
	}
	p->set_variable(number_, core_, vm->arg(0));
	vm->return_result();
}


Fun::Fun(const FramePtr& outer, const AnyPtr& athis, const CodePtr& code, FunCore* core)
	:outer_(outer), this_(athis), code_(code), core_(core){
}

StringPtr Fun::object_name(int_t depth){
	if(!name_){
		set_object_name(ptr_cast<String>(Xf("<(%s):%s:%d>")->call(get_class()->object_name(depth), code_->source_file_name(), code_->compliant_lineno(code_->data()+core_->pc))), 1, parent_);
	}

	return HaveName::object_name(depth);
}

void Fun::check_arg(const VMachinePtr& vm){
	int_t n = vm->ordered_arg_count();
	if(n<core_->min_param_count || (!(core_->flags&FunCore::FLAG_EXTENDABLE_PARAM) && n>core_->max_param_count)){
		if(core_->min_param_count==0 && core_->max_param_count==0){
			XTAL_THROW(builtin()->member(Xid(ArgumentError))->call(
				Xt("Xtal Runtime Error 1007")->call(
					Named(Xid(object), object_name()),
					Named(Xid(value), n)
				)
			), return);
		}
		else{
			if(core_->flags&FunCore::FLAG_EXTENDABLE_PARAM){
				XTAL_THROW(builtin()->member(Xid(ArgumentError))->call(
					Xt("Xtal Runtime Error 1005")->call(
						Named(Xid(object), object_name()),
						Named(Xid(min), core_->min_param_count),
						Named(Xid(value), n)
					)
				), return);
			}
			else{
				XTAL_THROW(builtin()->member(Xid(ArgumentError))->call(
					Xt("Xtal Runtime Error 1006")->call(
						Named(Xid(object), object_name()),
						Named(Xid(min), core_->min_param_count),
						Named(Xid(max), core_->max_param_count),
						Named(Xid(value), n)
					)
				), return);
			}
		}
	}
}

void Fun::rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=core_->max_param_count)
		check_arg(vm);
	vm->set_arg_this(this_);
	vm->carry_over(this);
}

void Lambda::rawcall(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	vm->mv_carry_over(this);
}


void Method::rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=core_->max_param_count)
		check_arg(vm);
	vm->carry_over(this);
}


Fiber::Fiber(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunCore* core)
	:Fun(outer, th, code, core), vm_(null), resume_pc_(0), alive_(true){
}


void Fiber::halt(){
	if(resume_pc_!=0){
		vm_->exit_fiber();
		resume_pc_ = 0;
		environment()->vm_take_back(vm_);
		vm_ = null;
		alive_ = false;
	}
}

void Fiber::call_helper(const VMachinePtr& vm, bool add_succ_or_fail_result){
	if(alive_){
		vm->set_arg_this(this_);
		if(resume_pc_==0){
			if(!vm_){ vm_ = environment()->vm_take_over(); }
			resume_pc_ = vm_->start_fiber(this, vm.get(), add_succ_or_fail_result);
		}
		else{ 
			resume_pc_ = vm_->resume_fiber(this, resume_pc_, vm.get(), add_succ_or_fail_result);
		}
		if(resume_pc_==0){
			environment()->vm_take_back(vm_);
			vm_ = null;
			alive_ = false;
		}
	}
	else{
		vm->return_result();
	}
}

AnyPtr Fiber::reset(){
	halt();
	alive_ = true;
	return from_this(this);
}


void initialize_fun(){
	{
		ClassPtr p = new_cpp_class<Fun>(Xid(Fun));
	}

	{
		ClassPtr p = new_cpp_class<Method>(Xid(Method));
		p->inherit(get_cpp_class<Fun>());
	}

	{
		ClassPtr p = new_cpp_class<Fiber>(Xid(Fiber));
		p->inherit(get_cpp_class<Fun>());
		p->inherit(Iterator());
		p->method(Xid(reset), &Fiber::reset);
		p->method(Xid(block_first), &Fiber::block_next);
		p->method(Xid(block_next), &Fiber::block_next);
		p->method(Xid(halt), &Fiber::halt);
		p->method(Xid(is_alive), &Fiber::is_alive);
	}

	{
		ClassPtr p = new_cpp_class<Lambda>(Xid(Lambda));
		p->inherit(get_cpp_class<Fun>());
	}

	{
		ClassPtr p = new_cpp_class<Arguments>(Xid(Arguments));
		p->def(Xid(new), ctor<Arguments, const AnyPtr&, const AnyPtr&>()->param(Named(Xid(ordered), null), Named(Xid(named), null)));
		p->method(Xid(size), &Arguments::length);
		p->method(Xid(length), &Arguments::length);
		p->method(Xid(ordered_arguments), &Arguments::ordered_arguments);
		p->method(Xid(named_arguments), &Arguments::named_arguments);
		
		p->def(Xid(op_at), method(&Arguments::op_at_int), get_cpp_class<Int>());
		p->def(Xid(op_at), method(&Arguments::op_at_string), get_cpp_class<String>());
	}

	builtin()->def(Xid(Arguments), get_cpp_class<Arguments>());
	builtin()->def(Xid(Fun), get_cpp_class<Fun>());
	builtin()->def(Xid(Fiber), get_cpp_class<Fiber>());
}

}