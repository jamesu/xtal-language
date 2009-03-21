#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Lib::Lib(bool most_top_level){
	set_object_name("lib");
	set_object_force(1000);
	load_path_list_ = xnew<Array>();
	path_ = xnew<Array>();
	most_top_level_ = most_top_level;
}

Lib::Lib(const ArrayPtr& path)
	:path_(path){
	load_path_list_ = xnew<Array>();
	most_top_level_ = false;
}

const AnyPtr& Lib::do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){	
	const AnyPtr& ret = Class::do_member(primary_key, secondary_key, inherited_too, accessibility, nocache);
	if(rawne(ret, undefined)){
		return ret;
	}
	else{

#ifndef XTAL_NO_PARSER
		Xfor(var, load_path_list_){
			StringPtr file_name = Xf("%s%s%s%s")->call(var, join_path("/"), primary_key, ".xtal")->to_s();
			return rawdef(primary_key, load(file_name), secondary_key);
		}
#endif
		return undefined;

		/* 指定した名前をフォルダーとみなす
		ArrayPtr next = path_.clone();
		next.push_back(name);
		AnyPtr lib = xnew<Lib>(next);
		return rawdef(name, lib, secondary_key);
		*/
	}
}

void Lib::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	value->set_object_parent(from_this(this));
	rawdef(primary_key, value, secondary_key);
}

const AnyPtr& Lib::rawdef(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key){
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		Value val = {members_.size(), KIND_PUBLIC};
		map_members_->insert(key, val);
		members_.push_back(value);
		invalidate_cache_member();
		value->set_object_parent(from_this(this));
		return members_.back();
	}
	else{
		XTAL_SET_EXCEPT(cpp_class<RedefinedError>()->call(Xt("Xtal Runtime Error 1011")->call(Named(Xid(object), this->object_name()))));
		return undefined;
	}
}

StringPtr Lib::join_path(const StringPtr& sep){
	if(path_->empty()){
		return sep;
	}
	else{
		return sep->cat(path_->join(sep))->cat(sep);
	}
}

}
