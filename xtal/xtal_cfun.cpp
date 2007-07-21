
#include "xtal_cfun.h"
#include "xtal.h"
#include "xtal_macro.h"
#include "xtal_vmachineimpl.h"

namespace xtal{

class CFunImpl : public HaveNameImpl{
public:
	typedef void (*fun_t)(const VMachine&, const ParamInfo&, void* data);

	CFunImpl(fun_t f, const void* val, int_t val_size, int_t param_n){
		fun_ = f;
		
		data_size_ = val_size;
		data_ = user_malloc(val_size);
		memcpy(data_, val, val_size);
		
		if(param_n){
			pi_.params = (Named*)user_malloc(sizeof(Named)*param_n);
		}else{
			pi_.params = 0;
		}
		param_n_ = pi_.min_param_count = pi_.max_param_count = param_n;
		for(int_t i=0; i<param_n; ++i){
			new(&pi_.params[i]) Named();
		}
		pi_.fun = this;
	}
	
	virtual ~CFunImpl(){
		for(int_t i=0; i<param_n_; ++i){
			pi_.params[i].~Named();
		}
		user_free(data_, data_size_);
		user_free(pi_.params, sizeof(Named)*param_n_);		
	}

	void param(
		const Named2& value0, 
		const Named2& value1,
		const Named2& value2,
		const Named2& value3,
		const Named2& value4
	){
		if(param_n_>0)pi_.params[0] = Named(value0.name, value0.value);
		if(param_n_>1)pi_.params[1] = Named(value1.name, value1.value);
		if(param_n_>2)pi_.params[2] = Named(value2.name, value2.value);
		if(param_n_>3)pi_.params[3] = Named(value3.name, value3.value);
		if(param_n_>4)pi_.params[4] = Named(value4.name, value4.value);

		pi_.min_param_count = pi_.max_param_count = param_n_;
		for(int_t i=0; i<param_n_; ++i){
			if(pi_.params[i].value){
				pi_.min_param_count--;
			}
		}
	}

	virtual void visit_members(Visitor& m){
		HaveNameImpl::visit_members(m);
		std::for_each(pi_.params, pi_.params+param_n_, m);
	}

	void check_arg(const VMachine& vm);

	virtual void call(const VMachine& vm){
		if(vm.impl()->ordered_arg_count()==pi_.min_param_count){
			check_arg(vm);
		}
		fun_(vm, pi_, data_);
	}

protected:
	void* data_;
	int_t data_size_;
	fun_t fun_;
	ParamInfo pi_;
	int_t param_n_;
};

void CFunImpl::check_arg(const VMachine& vm){
	int_t n = vm.impl()->ordered_arg_count();
	if(n<pi_.min_param_count || n>pi_.max_param_count){
		if(pi_.min_param_count==0 && pi_.max_param_count==0){
			XTAL_THROW(builtin().member("ArgumentError")(
				Xt("Xtal Runtime Error 1007")(
					Named("name", pi_.fun.cref().object_name()),
					Named("value", n)
				)
			));
		}else{
			XTAL_THROW(builtin().member("ArgumentError")(
				Xt("Xtal Runtime Error 1006")(
					Named("name", pi_.fun.cref().object_name()),
					Named("min", pi_.min_param_count),
					Named("max", pi_.max_param_count),
					Named("value", n)
				)
			));
		}
	}
}

class CFunArgsImpl : public CFunImpl{
public:

	CFunArgsImpl(fun_t f, const void* val, int_t val_size, int_t param_n)
		:CFunImpl(f, val, val_size, param_n){}

	virtual void call(const VMachine& vm){
		fun_(vm, pi_, data_);
	}
};

CFun::CFun(CFunImpl::fun_t fun, const void* val, int_t val_size, int_t param_n, int_t is_args)
	:Any(null){
	if(is_args){
		new(*this) CFunArgsImpl(fun, val, val_size, param_n);
	}else{
		new(*this) CFunImpl(fun, val, val_size, param_n);
	}
}

const CFun& CFun::param(
	const Named2& value0, 
	const Named2& value1,
	const Named2& value2,
	const Named2& value3,
	const Named2& value4
) const{
	impl()->param(value0, value1, value2, value3, value4);
	return *this;
}


}
