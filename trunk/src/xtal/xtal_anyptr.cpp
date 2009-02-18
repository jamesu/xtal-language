#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

SmartPtr<Any>::SmartPtr(const char_t* str){
	*this = xnew<String>(str);
}

SmartPtr<Any>::SmartPtr(const avoid<char>::type* str){
	*this = xnew<String>(str);
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const Null&){
	dec_ref_count();
	set_null_force(*this);
	return *this;
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const Undefined&){
	dec_ref_count();
	set_undefined_force(*this);
	return *this;
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const SmartPtr<Any>& p){
	return ap_copy(*this, p);
}

SmartPtr<Any>::SmartPtr(RefCountingBase* p, int_t type, special_ctor_t)
	:Any(noinit_t()){
	set_p(type, p);
	core()->register_gc(p);
}

SmartPtr<Any>::SmartPtr(Base* p, const ClassPtr& c, special_ctor_t)
	:Any(p){
	p->set_class(c);
	core()->register_gc(p);
}

SmartPtr<Any>::SmartPtr(Singleton* p, const ClassPtr& c, special_ctor_t)
	:Any(p){
	core()->register_gc(p);
}

SmartPtr<Any>::SmartPtr(CppSingleton* p, const ClassPtr& c, special_ctor_t)
	:Any(p){
	core()->register_gc(p);
}

}