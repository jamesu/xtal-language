
#pragma once

namespace xtal{

// 引数オブジェクト
class Arguments : public Base{
public:

	Arguments(const AnyPtr& ordered = undefined, const AnyPtr& named = undefined);

	const AnyPtr& op_at_int(int_t index){
		return ordered_->at(index);
	}

	const AnyPtr& op_at_string(const StringPtr& key){
		return named_->at(key); 
	}

	int_t length(){
		return ordered_->length();
	}
	
	AnyPtr ordered_arguments(){
		return ordered_->each();
	}
	
	AnyPtr named_arguments(){
		return named_->each();
	}

public:

	ArrayPtr ordered_;
	MapPtr named_;

	virtual void visit_members(Visitor& m);
};

class InstanceVariableGetter : public HaveName{
public:

	InstanceVariableGetter(int_t number, ClassInfo* info);

	virtual void rawcall(const VMachinePtr& vm);

private:
	int_t number_;
	ClassInfo* info_;
};

class InstanceVariableSetter : public HaveName{
public:

	InstanceVariableSetter(int_t number, ClassInfo* info);

	virtual void rawcall(const VMachinePtr& vm);

private:
	int_t number_;
	ClassInfo* info_;
};

class Method : public HaveName{
public:

	Method(const FramePtr& outer, const CodePtr& code, FunInfo* core);

	const FramePtr& outer(){ return outer_; }
	const CodePtr& code(){ return code_; }
	int_t pc(){ return info_->pc; }
	const inst_t* source(){ return code_->data()+info_->pc; }
	const IDPtr& param_name_at(size_t i){ return code_->identifier(i+info_->variable_identifier_offset); }
	int_t param_size(){ return info_->variable_size; }	
	bool extendable_param(){ return (info_->flags&FunInfo::FLAG_EXTENDABLE_PARAM)!=0; }
	FunInfo* info(){ return info_; }
	void set_info(FunInfo* fc){ info_ = fc; }
	bool check_arg(const VMachinePtr& vm);
	virtual StringPtr object_name(int_t depth = -1);

public:
		
	virtual void rawcall(const VMachinePtr& vm);
	
protected:

	FramePtr outer_;
	CodePtr code_;
	FunInfo* info_;
	
	virtual void visit_members(Visitor& m){
		HaveName::visit_members(m);
		m & outer_ & code_;
	}

};

class Fun : public Method{
public:

	Fun(const FramePtr& outer, const AnyPtr& athis, const CodePtr& code, FunInfo* core)
		:Method(outer, code, core), this_(athis){
	}


public:
	
	virtual void rawcall(const VMachinePtr& vm);

protected:

	virtual void visit_members(Visitor& m){
		Method::visit_members(m);
		m & this_;
	}

	AnyPtr this_;
};

class Lambda : public Fun{
public:

	Lambda(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunInfo* core)
		:Fun(outer, th, code, core){
	}

public:
	
	virtual void rawcall(const VMachinePtr& vm);
};

class Fiber : public Fun{
public:

	Fiber(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunInfo* core);

	virtual void finalize();
			
public:

	void block_next(const VMachinePtr& vm){
		call_helper(vm, true);
	}

	void halt();

	void rawcall(const VMachinePtr& vm){
		call_helper(vm, false);
	}

	void call_helper(const VMachinePtr& vm, bool add_succ_or_fail_result);

	bool is_alive(){
		return alive_;
	}

	AnyPtr reset();

private:

	VMachinePtr vm_;
	const inst_t* resume_pc_;
	bool alive_;

	void visit_members(Visitor& m){
		Fun::visit_members(m);
		m & vm_;
	}
};

}

