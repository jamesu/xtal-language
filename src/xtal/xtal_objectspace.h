/** \file src/xtal/xtal_objectspace.h
* \brief src/xtal/xtal_objectspace.h
*/

#ifndef XTAL_OBJECTSPACE_H_INCLUDE_GUARD
#define XTAL_OBJECTSPACE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

struct ConnectedPointer;

class ObjectSpace{
public:

	ObjectSpace()
		:value_map_(map_t::no_use_memory_t()){}

	void initialize();

	void uninitialize();

	void print_all_objects();

	void enable_gc();

	void disable_gc();

	void gc();

	void full_gc();

	void register_gc(RefCountingBase* p);

public:
	void set_cpp_class(CppClassSymbolData* key, const ClassPtr& cls);

	Class* make_cpp_class(CppClassSymbolData* key);

	const ClassPtr& cpp_class(CppClassSymbolData* key){
		if(map_t::Node* it = value_map_.find(key->key())){
			return to_smartptr((Class*)it->value().get());
		}
		return to_smartptr(make_cpp_class(key));
	}

	const ClassPtr& cpp_class(uint_t key){
		return to_smartptr((Class*)value_map_.find(key)->value().get());
	}

	RefCountingBase* make_cpp_value(CppValueSymbolData* key);

	const AnyPtr& cpp_value(CppValueSymbolData* key){
		if(map_t::Node* it = value_map_.find(key->key())){
			return to_smartptr(it->value().get());
		}
		return to_smartptr(make_cpp_value(key));
	}

public:
	uint_t alive_object_count();

	RefCountingBase* alive_object(uint_t i);

public:

	void shrink_to_fit();

	void inc_objects_destroyed_count(){
		objects_destroyed_count_++;
	}

private:
	
	ConnectedPointer sweep_dead_objects(ConnectedPointer first, ConnectedPointer last, ConnectedPointer end);

	void unset_finalize_objects(ConnectedPointer it, ConnectedPointer end);

	void destroy_objects(ConnectedPointer it, ConnectedPointer current);

	void free_objects(ConnectedPointer it, ConnectedPointer current);

	void adjust_objects_list(ConnectedPointer it);

	ConnectedPointer find_alive_objects(ConnectedPointer alive, ConnectedPointer current);

	void add_ref_count_objects(ConnectedPointer it, ConnectedPointer current, int_t v);

	void expand_objects_list();

private:
	RefCountingBase*** objects_list_begin_;
	RefCountingBase*** objects_list_current_;
	RefCountingBase*** objects_list_end_;

	uint_t objects_generation_line_;
	uint_t objects_count_;
	uint_t objects_max_;
	uint_t processed_line_;
	int_t objects_destroyed_count_;

	bool disable_finalizer_;

	uint_t cycle_count_;

public:
	struct Fun{
		static uint_t hash(uint_t key){
			return key;
		}

		static bool eq(uint_t a, uint_t b){
			return a==b;
		}
	};

	typedef Hashtable<uint_t, BasePtr<RefCountingBase>, Fun> map_t; 

private:
	map_t value_map_;
};

}

#endif // XTAL_OBJECTSPACE_H_INCLUDE_GUARD
