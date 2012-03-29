#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_objectspace.h"

//#define XTAL_DEBUG_PRINT

#ifdef XTAL_DEBUG_PRINT
#include <map>
#include <string>
#endif

namespace xtal{

enum{
	OBJECTS_ALLOCATE_SHIFT = 9,
	OBJECTS_ALLOCATE_SIZE = 1 << OBJECTS_ALLOCATE_SHIFT,
	OBJECTS_ALLOCATE_MASK = OBJECTS_ALLOCATE_SIZE-1
};

struct ScopeCounter{
	uint_t* p;
	ScopeCounter(uint_t* p):p(p){ *p+=1; }
	~ScopeCounter(){ *p-=1; }
};

struct ConnectedPointer{
	uint_t pos;
	RefCountingBase**** bp;

	typedef std::random_access_iterator_tag iterator_category;
	typedef RefCountingBase* value_type;
	typedef int_t difference_type;
	typedef int_t distance_type;	// retained
	typedef value_type* pointer;
	typedef value_type& reference;

	ConnectedPointer()
		:pos(0), bp(0){}

	ConnectedPointer(int_t p, RefCountingBase***& pp)
		:pos(p), bp(&pp){}

public:

	pointer segment_begin(){
		return &(*bp)[pos>>OBJECTS_ALLOCATE_SHIFT][0];
	}

	pointer segment_end(){
		return &(*bp)[pos>>OBJECTS_ALLOCATE_SHIFT][OBJECTS_ALLOCATE_SIZE];
	}

	pointer segment_rbegin(){
		return segment_end()-1;
	}

	pointer segment_rend(){
		return segment_begin()-1;
	}

	pointer segment_get(){
		return &(*bp)[pos>>OBJECTS_ALLOCATE_SHIFT][pos&OBJECTS_ALLOCATE_MASK];
	}

	uint_t segment_pos(){
		return pos&OBJECTS_ALLOCATE_MASK;
	}

	uint_t segment_number(){
		return pos>>OBJECTS_ALLOCATE_SHIFT;
	}

	void move_next_segment(){
		pos = (pos+OBJECTS_ALLOCATE_SIZE) & ~OBJECTS_ALLOCATE_MASK;
	}

	void move_back_segment(){
		pos = (pos & ~OBJECTS_ALLOCATE_MASK) - 1;
	}

public:

	reference operator *(){
		return (*bp)[pos>>OBJECTS_ALLOCATE_SHIFT][pos&OBJECTS_ALLOCATE_MASK];
	}

	reference operator [](int_t index){
		int_t pos2 = pos+index;
		return (*bp)[pos2>>OBJECTS_ALLOCATE_SHIFT][pos2&OBJECTS_ALLOCATE_MASK];
	}

	ConnectedPointer& operator ++(){
		++pos;
		return *this;
	}

	ConnectedPointer operator ++(int){
		ConnectedPointer temp(pos, *bp);
		++pos;
		return temp; 
	}

	ConnectedPointer& operator --(){
		--pos;
		return *this;
	}

	ConnectedPointer operator --(int){
		ConnectedPointer temp(pos, *bp);
		--pos;
		return temp; 
	}

	friend bool operator==(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos==b.pos;
	}

	friend bool operator!=(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos!=b.pos;
	}

	friend bool operator<(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos<b.pos;
	}

	friend bool operator>(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos>b.pos;
	}

	friend bool operator<=(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos<=b.pos;
	}

	friend bool operator>=(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos>=b.pos;
	}

	friend int_t operator-(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos - b.pos;
	}

	friend ConnectedPointer operator-(const ConnectedPointer& a, int_t b){
		return ConnectedPointer(a.pos - b, *a.bp);
	}

	friend ConnectedPointer operator-(int_t a, const ConnectedPointer& b){
		return ConnectedPointer(a - b.pos, *b.bp);
	}

	friend ConnectedPointer operator+(const ConnectedPointer& a, int_t b){
		return ConnectedPointer(a.pos + b, *a.bp);
	}

	friend ConnectedPointer operator+(int_t a, const ConnectedPointer& b){
		return ConnectedPointer(a + b.pos, *b.bp);
	}
};

struct ConnectedPointerEnumerator{
	ConnectedPointer begin_;
	ConnectedPointer end_;

	ConnectedPointerEnumerator(const ConnectedPointer& b, const ConnectedPointer& e){
		begin_ = b;
		end_ = e;
	}

	RefCountingBase** begin(){
		return begin_.segment_get();
	}

	RefCountingBase** end(){
		if(begin_.segment_number()==end_.segment_number()){
			return end_.segment_get();
		}
		else{
			return begin_.segment_end();
		}
	}

	bool move(){
		if(begin_.segment_number()==end_.segment_number()){
			return false;
		}

		begin_.move_next_segment();
		return true;
	}

//	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
//	}while(e.move());
};

struct ConnectedPointerReverseEnumerator{
	ConnectedPointer begin_;
	ConnectedPointer end_;

	ConnectedPointerReverseEnumerator(const ConnectedPointer& b, const ConnectedPointer& e){
		begin_ = e - 1;
		end_ = b - 1;
	}

	RefCountingBase** begin(){
		return begin_.segment_get();
	}

	RefCountingBase** end(){
		if(begin_.segment_number()==end_.segment_number()){
			return end_.segment_get();
		}
		else{
			return begin_.segment_rend();
		}
	}

	bool move(){
		if(begin_.segment_number()==end_.segment_number()){
			return false;
		}

		begin_.move_back_segment();
		return true;
	}


//	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; --pp){
//	}while(e.move());
};

void ObjectSpace::initialize(){
	objects_list_begin_ = 0;
	objects_list_current_ = 0;
	objects_list_end_ = 0;
	objects_count_ = 0;
	objects_max_ = 0;
	processed_line_ = 0;
	cycle_count_ = 0;
	objects_generation_line_ = 0;
	objects_destroyed_count_ = 0;

	disable_finalizer_ = false;

	disable_gc();

	value_map_.expand(5);

	expand_objects_list();

	static CppClassSymbolData* symbols[] = { 
		&CppClassSymbol<Class>::value,
		&CppClassSymbol<Any>::value,
		&CppClassSymbol<Array>::value,
		&CppClassSymbol<String>::value,
	};

	Class* classes[sizeof(symbols)/sizeof(symbols[0])];
	uint_t nsize = sizeof(symbols)/sizeof(symbols[0]);

	for(uint_t i=0; i<nsize; ++i){
		Class* p = object_xmalloc<Class>();
		p->special_initialize();
		classes[i] = p;
		value_map_.insert(symbols[i]->key(), p);
	}

	for(uint_t i=0; i<nsize; ++i){
		Class* p = classes[i];
		new(p) Class(Class::cpp_class_t());
		p->inc_ref_count();
	}
	
	for(uint_t i=0; i<nsize; ++i){
		Class* p = classes[i];
		p->special_initialize(&VirtualMembersT<Class>::value);
	}

	for(uint_t i=0; i<nsize; ++i){
		Class* p = classes[i];
		p->set_symbol_data(symbols[i]);
		register_gc(p);
	}

	SmartPtr<Lib> lib = XNew<Lib>();
	lib->inherit(cpp_class(&CppClassSymbol<AutoLoader>::value));
	lib->append_load_path(XTAL_STRING("."));
	lib->set_cpp_singleton();

	SmartPtr<Global> global = XNew<Global>();
	global->inherit(cpp_class(&CppClassSymbol<Class>::value));
	global->set_cpp_singleton();

	for(CppClassSymbolData* p=CppClassSymbolData::head; p; p=p->next){
		cpp_class(p)->prebind();
	}
}

void ObjectSpace::uninitialize(){
	value_map_.destroy();

	clear_cache();

	disable_finalizer_ = true;
	full_gc();

	if(objects_count_ != 0){
		if(!ignore_memory_assert()){
			//fprintf(stderr, "finished gc\n");
			//fprintf(stderr, "exists cycled objects %d\n", objects_count_);
			//print_alive_objects();

			print_all_objects();

#ifndef XTAL_CHECK_REF_COUNT

			// このassertでとまる場合、オブジェクトをすべて開放できていない。
			// グローバル変数などでオブジェクトを握っていないか、循環参照はないか調べること。
			XTAL_ASSERT(false);

#endif
		}

		// 強制的に開放してしまおう
		{
			ConnectedPointer current(objects_count_, objects_list_begin_);
			ConnectedPointer begin(0, objects_list_begin_);

			destroy_objects(begin, current);
			free_objects(begin, current);
		}
	}

	for(RefCountingBase*** it=objects_list_current_; it!=objects_list_end_; ++it){
		RefCountingBase** begin = *it;
		RefCountingBase** current = *it;
		RefCountingBase** end = *it+OBJECTS_ALLOCATE_SIZE;
		fit_simple_dynamic_pointer_array(&begin, &end, &current);
	}

	objects_list_current_ = objects_list_begin_;
	fit_simple_dynamic_pointer_array(&objects_list_begin_, &objects_list_end_, &objects_list_current_);
}
	
void ObjectSpace::shrink_to_fit(){
	if(cycle_count_>0){
		return;
	}

	ScopeCounter cc(&cycle_count_);

	if(objects_list_current_!=objects_list_end_){
		++objects_list_current_;

		for(RefCountingBase*** it=objects_list_current_; it!=objects_list_end_; ++it){
			RefCountingBase** begin = *it;
			RefCountingBase** current = *it;
			RefCountingBase** end = *it+OBJECTS_ALLOCATE_SIZE;
			fit_simple_dynamic_pointer_array(&begin, &end, &current);
		}

		fit_simple_dynamic_pointer_array(&objects_list_begin_, &objects_list_end_, &objects_list_current_);

		objects_max_ = (objects_list_current_-objects_list_begin_)*OBJECTS_ALLOCATE_SIZE;
		
		--objects_list_current_;
	}

	ConnectedPointer current(objects_count_, objects_list_begin_);
	ConnectedPointer begin(0, objects_list_begin_);

	ConnectedPointerEnumerator e(begin, current);
	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
		(*pp)->shrink_to_fit();
	}while(e.move());
}

void ObjectSpace::print_all_objects(){
#ifdef XTAL_DEBUG_PRINT
			ConnectedPointer current(objects_count_, objects_list_begin_);
			ConnectedPointer begin(0, objects_list_begin_);

			std::map<std::string, int> table;
			std::string classpre = "CLASS: ";
			std::string stringpre = "STRING: ";
			for(ConnectedPointer it = begin; it!=current; ++it){
				switch(XTAL_detail_type(**it)){
					XTAL_DEFAULT{
						RefCountingBase* p = *it;
						int_t rc = p->ref_count();

						//table["Base"]++; 
						if(Class* cp=dynamic_cast<Class*>(p)){
							table["Class"]++;
							const char_t* name = cp->object_temporary_name()->c_str();
							int refcount = cp->ref_count();
							table[classpre + cp->object_temporary_name()->c_str()]++;
						}
						else{
							table[typeid(*p).name()]++;
						}
					}

					XTAL_CASE(TYPE_STRING){ 
						unchecked_ptr_cast<String>(ap(**it))->is_interned() ? table["iString"]++ : table["String"]++; 
						const char_t* str = unchecked_ptr_cast<String>(ap(**it))->c_str();
						table[stringpre+str]++;
					}

				}
			}

			std::map<std::string, int>::iterator it=table.begin(), last=table.end();
			for(; it!=last; ++it){
				if(it->second>3){
					printf("alive %s %d\n", it->first.c_str(), it->second);
				}
			}
			int m = (objects_list_end_ - objects_list_begin_)*OBJECTS_ALLOCATE_SIZE;
			printf("object_list_size=%d, objects_count=%d\n", m, objects_count_);
			//printf("used_memory %d\n", used_memory);	
#endif
}

void ObjectSpace::enable_gc(){
	cycle_count_--;
}

void ObjectSpace::disable_gc(){
	cycle_count_++;
}
	
void ObjectSpace::expand_objects_list(){
	if(objects_count_==objects_max_){
		expand_simple_dynamic_pointer_array(&objects_list_begin_, &objects_list_end_, &objects_list_current_, 1);

		RefCountingBase** begin = 0;
		RefCountingBase** current = 0;
		RefCountingBase** end = 0;
		expand_simple_dynamic_pointer_array(&begin, &end, &current, OBJECTS_ALLOCATE_SIZE);
		objects_list_begin_[objects_max_>>OBJECTS_ALLOCATE_SHIFT] = begin;
		objects_max_ += OBJECTS_ALLOCATE_SIZE;
	}
}

uint_t ObjectSpace::alive_object_count(){
	return objects_count_;
}

RefCountingBase* ObjectSpace::alive_object(uint_t i){
	ConnectedPointer current(i, objects_list_begin_);
	if((*current)->ref_count()){
		return *current;
	}
	else{
		return 0;
	}
}

ConnectedPointer ObjectSpace::sweep_dead_objects(ConnectedPointer first, ConnectedPointer last, ConnectedPointer end){
	if(first==last){
		return end;
	}

	ConnectedPointerReverseEnumerator cpre1(first+1, last);
	ConnectedPointerReverseEnumerator cpre2(first+1, end);

	RefCountingBase** endpp = cpre2.begin();
	RefCountingBase** endppend = cpre2.end();

	do for(RefCountingBase** pp=cpre1.begin(), **ppend=cpre1.end(); pp!=ppend; --pp){
		RefCountingBase* p = *pp;

		if(!p->object_destroyed()){
			if(!p->alive_ref_count()){
				p->object_destroy();
			}
			else{
				continue;
			}
		}

		p->object_free();
		objects_destroyed_count_--;

		if(endpp==endppend){
			cpre2.move();
			endpp = cpre2.begin();
			endppend = cpre2.end();
		}

		--end;

		*pp = *endpp;
		--endpp;
	}while(cpre1.move());

	if(objects_destroyed_count_<0){
		objects_destroyed_count_ = 0;
	}

	return end;
}

void ObjectSpace::destroy_objects(ConnectedPointer it, ConnectedPointer end){
	ConnectedPointerEnumerator e(it, end);
	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
		(*pp)->unset_finalizer_flag();
		(*pp)->object_destroy();
	}while(e.move());
}

void ObjectSpace::free_objects(ConnectedPointer it, ConnectedPointer end){
	ConnectedPointerEnumerator e(it, end);
	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
		(*pp)->object_free();
		objects_destroyed_count_--;
	}while(e.move());

	if(objects_destroyed_count_<0){
		objects_destroyed_count_ = 0;
	}
}

void ObjectSpace::adjust_objects_list(ConnectedPointer it){
	objects_count_ = it.pos;
	objects_list_current_ = objects_list_begin_ + (objects_count_>>OBJECTS_ALLOCATE_SHIFT);
	expand_objects_list();
	XTAL_ASSERT(objects_list_current_<=objects_list_end_);
}

void ObjectSpace::add_ref_count_objects(ConnectedPointer it, ConnectedPointer end, int_t v){
	Visitor m(v);
	ConnectedPointerEnumerator e(it, end);
	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
		(*pp)->visit_members(m);
	}while(e.move());
}

void ObjectSpace::gc(){
	if(cycle_count_!=0){ return; }

	ScopeCounter cc(&cycle_count_);

	ConnectedPointer first(objects_generation_line_, objects_list_begin_);
	ConnectedPointer last(objects_count_, objects_list_begin_);
	ConnectedPointer end(objects_count_, objects_list_begin_);
	
	end = sweep_dead_objects(first, last, end);

	adjust_objects_list(end);
}

ConnectedPointer ObjectSpace::find_alive_objects(ConnectedPointer alive, ConnectedPointer current){ 
	// 死者の中から復活した者を見つける
	Visitor m(1);
	bool end = false;
	while(!end){
		end = true;

		ConnectedPointerEnumerator e(alive, current);
		do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
			if((*pp)->alive_ref_count()){
				end = false;
				(*pp)->visit_members(m); // 生存確定オブジェクトは、参照カウンタを元に戻す
				std::swap(*pp, *alive++);
			}
		}while(e.move());
	}

	return alive;
}

void ObjectSpace::full_gc(){
	if(cycle_count_!=0){ 
		return; 
	}

	{
		ConnectedPointer first(0, objects_list_begin_);
		ConnectedPointer last(objects_count_, objects_list_begin_);

		ConnectedPointer end(objects_count_, objects_list_begin_);
		end = sweep_dead_objects(first, last, end);
		adjust_objects_list(end);
	}

	ScopeCounter cc(&cycle_count_);
	
	while(true){			
		ConnectedPointer current(objects_count_, objects_list_begin_);
		ConnectedPointer begin(0, objects_list_begin_);
		if(current==begin){
			break;
		}

		// 参照カウンタを減らす
		// これにより、ルートから示されている以外のオブジェクトは参照カウンタが0となる
		add_ref_count_objects(begin, current, -1);
	
		ConnectedPointer alive = begin;

		alive = find_alive_objects(alive, current);

		// begin ～ aliveまでのオブジェクトは生存確定
		// alive ～ currentまでのオブジェクトは死亡予定

		// 死者も、参照カウンタを元に戻す
		add_ref_count_objects(alive, current, 1);

		if(!disable_finalizer_){
			bool exists_have_finalizer = false;
			
			// 死者となる予定のオブジェクトのfinalizerを走らせる
			for(ConnectedPointer it=alive; it!=current; ++it){
				RefCountingBase* p = *it;
				if(p->have_finalizer()){
					exists_have_finalizer = true;
					VMachinePtr oldvm = set_vmachine(vmachine_take_over());
					((Base*)p)->finalize();
					vmachine_take_back(set_vmachine(oldvm));
				}
			}

			if(exists_have_finalizer){
				// finalizerでオブジェクトが作られたかもしれないので、currentを反映する
				current = ConnectedPointer(objects_count_, objects_list_begin_);
				begin = ConnectedPointer(0, objects_list_begin_);

				// 死者が生き返ったかも知れないのでチェックする

				// 参照カウンタを減らす
				add_ref_count_objects(alive, current, -1);
				
				alive = find_alive_objects(alive, current);

				// begin ～ aliveまでのオブジェクトは生存確定
				// alive ～ currentまでのオブジェクトは死亡確定

				// 死者も、参照カウンタを元に戻す
				add_ref_count_objects(alive, current, 1);
			}
		}

		destroy_objects(alive, current);
		free_objects(alive, current);
		
		adjust_objects_list(alive);

		if(current==alive){
			break;
		}

		current = alive;
	}

	objects_generation_line_ = objects_count_;
	objects_destroyed_count_ = 0;
}

void ObjectSpace::set_cpp_class(CppClassSymbolData* key, const ClassPtr& cls){
	if(map_t::Node* it = value_map_.find(key->key())){
		it->value() = cls;
	}
	else{
		value_map_.insert(key->key(), cls);
	}
}

Class* ObjectSpace::make_cpp_class(CppClassSymbolData* key){
	ClassPtr cls = xnew<Class>(Class::cpp_class_t());
	cls->set_symbol_data(key);
	value_map_.insert(key->key(), cls);
	return cls.get();
}

RefCountingBase* ObjectSpace::make_cpp_value(CppValueSymbolData* key){
	AnyPtr any = key->maker();
	value_map_.insert(key->key(), XTAL_detail_rcpvalue(any));
	return XTAL_detail_rcpvalue(any);
}

void ObjectSpace::register_gc(RefCountingBase* p){
	if(!p->virtual_members()->is_container){
		return;
	}

	if(objects_count_==objects_max_){
		if((objects_count_>>2) < (uint_t)objects_destroyed_count_){
			gc();
		}

		ScopeCounter cc(&cycle_count_);
		expand_objects_list();
	}

	*ConnectedPointer(objects_count_, objects_list_begin_) = p;
	objects_count_++;
}

}
