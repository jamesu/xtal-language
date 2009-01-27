#include "xtal.h"
#include "xtal_macro.h"

#include <string.h>

namespace xtal{

static uint_t make_string_hashcode(const char_t* str, uint_t size){
	uint_t hash = 2166136261U;
	for(uint_t i=0; i<size; ++i){
		hash = hash*137 ^ str[i];
	}
	return hash;
}

static void make_string_hashcode_and_length(const char_t* str, uint_t size, uint_t& hash, uint_t& length){
	hash = 2166136261U;
	length = 0;

	ChMaker chm;

	uint_t i=0;
	while(i<size){
		chm.clear();
		while(!chm.is_completed()){
			if(i<size){ chm.add(str[i++]); } 
			else{ break; }
		}
	
		for(int_t j=0; j<chm.pos; ++j){
			hash = hash*137 ^ chm.buf[j];
		}

		length += 1;
	}
}

static void make_string_size_and_hashcode_and_length(const char_t* str, uint_t& size, uint_t& hash, uint_t& length){
	hash = 2166136261U;
	length = 0;
	size = 0;

	ChMaker chm;

	uint_t i=0;
	while(str[i]){
		chm.clear();
		while(!chm.is_completed()){
			if(str[i]){ chm.add(str[i++]); } 
			else{ break; }
		}
	
		for(int_t j=0; j<chm.pos; ++j){
			hash = hash*137 ^ chm.buf[j];
		}

		length += 1;
		size += chm.pos;
	}
}

static void make_small_string_size_and_hashcode_and_length(const char_t* str, uint_t& size, uint_t& hash, uint_t& length){
	hash = 2166136261U;
	length = 0;
	size = 0;

	ChMaker chm;

	uint_t i=0;
	while(str[i] && i<Innocence::SMALL_STRING_MAX){
		chm.clear();
		while(!chm.is_completed()){
			if(str[i] && i<Innocence::SMALL_STRING_MAX){ chm.add(str[i++]); } 
			else{ break; }
		}
	
		for(int_t j=0; j<chm.pos; ++j){
			hash = hash*137 ^ chm.buf[j];
		}

		length += 1;
		size += chm.pos;
	}
}

static uint_t count_string_length(const char_t* str){
	ChMaker chm;
	uint_t length = 0;
	uint_t i=0;
	while(str[i]){
		chm.clear();
		while(!chm.is_completed()){
			if(str[i]){ chm.add(str[i++]); } 
			else{ break; }
		}
		length += 1;
	}
	return length;
}

static uint_t string_size_z(const char_t* str){
	uint_t ret = 0;
	while(*str++){
		ret++;
	}
	return ret;
}

static int_t string_compare(const char_t* a, const char_t* b){
	while(*a!=*b){
		if(!*a){
			return 0;
		}
		a++;
		b++;
	}
	return (int_t)*a - (int_t)*b;
}

class StringEachIter : public Base{
	StringStreamPtr ss_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & ss_;
	}

public:

	StringEachIter(const StringPtr& str)
		:ss_(xnew<StringStream>(str)){
	}

	void block_next(const VMachinePtr& vm){
		if(ss_->eos()){
			vm->return_result(null, null);
			return;
		}

		vm->return_result(from_this(this), ss_->get_s(1));
	}
};

class ChRangeIter : public Base{
public:

	ChRangeIter(const ChRangePtr& range)
		:it_(range->left()), end_(range->right()){}

	void block_next(const VMachinePtr& vm){
		if(ch_cmp(it_->data(), it_->data_size(), end_->data(), end_->data_size())>0){
			vm->return_result(null, null);
			return;
		}

		StringPtr temp = it_;
		it_ = ch_inc(it_->data(), it_->data_size());
		vm->return_result(from_this(this), temp);
	}

private:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & it_ & end_;
	}

	StringPtr it_, end_;
};

AnyPtr ChRange::each(){
	return xnew<ChRangeIter>(from_this(this));
}

////////////////////////////////////////////////////////////////

class StringMgr : public GCObserver{
public:

	struct Key{
		const char_t* str;
		uint_t size;
		uint_t hashcode;
	};

	struct Fun{
		static uint_t hash(const Key& key){
			return key.hashcode;
		}

		static bool eq(const Key& a, const Key& b){
			return a.hashcode==b.hashcode && a.size==b.size && memcmp(a.str, b.str, a.size*sizeof(char_t))==0;
		}
	};

	typedef Hashtable<Key, IDPtr, Fun> table_t; 
	table_t table_;

	struct Fun2{
		static uint_t hash(const void* key){
			return ((uint_t)key)>>3;
		}

		static bool eq(const void* a, const void* b){
			return a==b;
		}
	};

	typedef Hashtable<const void*, IDPtr, Fun2> table2_t; 
	table2_t table2_;

	StringMgr(){
		guard_ = 0;
	}

protected:

	int_t guard_;

	struct Guard{
		int_t& count;
		Guard(int_t& c):count(c){ count++; }
		~Guard(){ count--; }
	private:
		void operator=(const Guard&);
	};

	virtual void visit_members(Visitor& m){
		GCObserver::visit_members(m);
		for(table_t::iterator it = table_.begin(); it!=table_.end(); ++it){
			m & it->second;
		}		
	}

public:

	const IDPtr& insert(const char_t* str, uint_t size){
		uint_t hashcode, length;
		make_string_hashcode_and_length(str, size, hashcode, length);
		return insert(str, size, hashcode, length);
	}

	const IDPtr& insert(const char_t* str){
		uint_t hashcode, length, size;
		make_string_size_and_hashcode_and_length(str, size, hashcode, length);
		return insert(str, size, hashcode, length);
	}

	const IDPtr& insert(const char_t* str, uint_t size, uint_t hash, uint_t length);

	const IDPtr& insert_literal_string(const char_t* str){
		IDPtr& ret = table2_[str];
		if(!ret){
			uint_t hashcode, length, size;
			make_string_size_and_hashcode_and_length(str, size, hashcode, length);
			ret = insert(str, size, hashcode, length);
		}
		return ret;
	}

	virtual void before_gc();
};

const IDPtr& StringMgr::insert(const char_t* str, uint_t size, uint_t hashcode, uint_t length){
	Guard guard(guard_);

	Key key = {str, size, hashcode};
	table_t::iterator it = table_.find(key, hashcode);
	if(it!=table_.end()){
		return it->second;
	}

	it = table_.insert(key, xnew<ID>(str, size, hashcode, length), hashcode).first;
	it->first.str = it->second->data();
	return it->second;
}

void StringMgr::before_gc(){
	return;

	if(guard_){
		return;
	}
}


namespace{

SmartPtr<StringMgr> str_mgr_;

void uninitialize_string(){
	str_mgr_ = null;
}

}

class InternedStringIter : public Base{
	StringMgr::table_t::iterator iter_, last_;
public:

	InternedStringIter()
		:iter_(str_mgr_->table_.begin()), last_(str_mgr_->table_.end()){
	}
			
	void block_next(const VMachinePtr& vm){
		if(iter_!=last_){
			vm->return_result(from_this(this), iter_->second);
			++iter_;
		}
		else{
			vm->return_result(null, null);
		}
	}
};

AnyPtr interned_strings(){
	return xnew<InternedStringIter>();
}

int_t edit_distance(const StringPtr& str1, const StringPtr& str2){
	return edit_distance(str1->data(), str1->data_size()*sizeof(char_t), str2->data(), str2->data_size()*sizeof(char_t));
}

const IDPtr& string_literal_to_id(const char_t* str){
	return str_mgr_->insert_literal_string(str);
}


////////////////////////////////////////////////////////////////

void String::init_string(const char_t* str, uint_t sz){
	if(sz<SMALL_STRING_MAX){
		set_small_string();
		std::memcpy(svalue_, str, sz*sizeof(char_t));
	}
	else{
		uint_t hash, length;
		make_string_hashcode_and_length(str, sz, hash, length);
		if(length<=1){
			set_p(pvalue(str_mgr_->insert(str, sz, hash, length)));
			pvalue(*this)->inc_ref_count();
		}
		else{
			set_p(new LargeString(str, sz, hash, length));
			pvalue(*this)->set_class(new_cpp_class<String>());
			register_gc(pvalue(*this));
		}
	}
}


String::String(const char_t* str):Any(noinit_t()){
	init_string(str, string_size_z(str));
}

String::String(const avoid<char>::type* str):Any(noinit_t()){
	uint_t n = strlen((char*)str);
	UserMallocGuard umg(n*sizeof(char_t));
	char_t* buf = (char_t*)umg.get();
	for(uint_t i=0; i<n; ++i){
		buf[i] = (char_t)str[i];
	}
	init_string(buf, n);
}

String::String(const string_t& str):Any(noinit_t()){
	init_string(str.c_str(), str.size());
}

String::String(const char_t* str, uint_t size):Any(noinit_t()){
	init_string(str, size);
}

String::String(const char_t* begin, const char_t* last):Any(noinit_t()){
	init_string(begin, last-begin);
}

String::String(const char_t* str1, uint_t size1, const char_t* str2, uint_t size2):Any(noinit_t()){
	if(size1==0){
		init_string(str2, size2);
		return;
	}
	else if(size2==0){
		init_string(str1, size1);
		return;
	}

	uint_t sz = size1 + size2;
	if(sz<SMALL_STRING_MAX){
		set_small_string();
		std::memcpy(svalue_, str1, size1*sizeof(char_t));
		std::memcpy(&svalue_[size1], str2, size2*sizeof(char_t));
	}
	else{
		set_p(new LargeString(str1, size1, str2, size2));
		pvalue(*this)->set_class(new_cpp_class<String>());
		register_gc(pvalue(*this));
	}
}

String::String(char_t a)
	:Any(noinit_t()){
	if(1<SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a;
	}
	else{
		init_string(&a, 1);
	}
}

String::String(char_t a, char_t b)
	:Any(noinit_t()){
	if(2<SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b;
	}
	else{
		char_t buf[2] = {a, b};
		init_string(buf, 2);
	}
}

String::String(char_t a, char_t b, char_t c)
	:Any(noinit_t()){
	if(3<SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b; svalue_[2] = c;
	}
	else{
		char_t buf[3] = {a, b, c};
		init_string(buf, 3);
	}
}

String::String(const char_t* str, uint_t size, uint_t hashcode, uint_t length, bool intern_flag):Any(noinit_t()){
	uint_t sz = size;
	if(sz<SMALL_STRING_MAX){
		set_small_string();
		std::memcpy(svalue_, str, sz*sizeof(char_t));
	}
	else{
		if(!intern_flag && length<=1){
			set_p(pvalue(str_mgr_->insert(str, sz, hashcode, length)));
			pvalue(*this)->inc_ref_count();
		}
		else{
			set_p(new LargeString(str, sz, hashcode, length, intern_flag));
			pvalue(*this)->set_class(new_cpp_class<String>());
			register_gc(pvalue(*this));
		}
	}
}

String::String(LargeString* left, LargeString* right):Any(noinit_t()){
	if(left->data_size()==0){
		init_string(right->c_str(), right->data_size());
		return;
	}
	else if(right->data_size()==0){
		init_string(left->c_str(), left->data_size());
		return;
	}

	set_p(new LargeString(left, right));
	pvalue(*this)->set_class(new_cpp_class<String>());
	register_gc(pvalue(*this));
}

String::String(const String& s)
	:Any(s){
	inc_ref_count_force(s);
}

const char_t* String::c_str(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->c_str();
	}
	else{
		uint_t size, hash, length;
		make_string_size_and_hashcode_and_length(svalue_, size, hash, length);
		return str_mgr_->insert(svalue_, size, hash, length)->data();
	}
}

const char_t* String::data(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->c_str();
	}
	else{
		return svalue_;
	}
}

uint_t String::data_size(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->data_size();
	}
	else{
		for(uint_t i=0; i<SMALL_STRING_MAX; ++i){
			if(svalue_[i]=='\0'){
				return i;
			}
		}
		XTAL_ASSERT(false);
		return 0;
	}
}

uint_t String::length(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->length();
	}
	else{
		return count_string_length(svalue_);
	}
}

uint_t String::size(){
	return length();
}

StringPtr String::clone(){
	return from_this(this);
}

const IDPtr& String::intern(){
	if(type(*this)==TYPE_BASE){
		LargeString* p = ((LargeString*)pvalue(*this));
		if(p->is_interned()) return static_ptr_cast<ID>(ap(*this));
		return static_ptr_cast<ID>(str_mgr_->insert(p->c_str(), p->data_size(), p->hashcode(), p->length()));
	}
	else{
		return static_ptr_cast<ID>(ap(*this));
	}
}

bool String::is_interned(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->is_interned();
	}
	else{
		return true;
	}
}

StringPtr String::to_s(){
	return from_this(this);
}

int_t String::to_i(){
	const char_t* str = data();
	int_t ret = 0;
	int_t start = 0, sign = 1;
	
	if(str[0]=='-'){
		sign = -1;
		start = 1;
	}
	else if(str[0]=='+'){
		start = 1;
	}

	for(uint_t i=start, sz=data_size(); i<sz; ++i){
		ret *= 10;
		ret += str[i] - '0';
	}
	return ret*sign; 
}

float_t String::to_f(){
	const char_t* str = data();
	float_t ret = 0;
	float_t scale = 10;
	int_t start = 0, sign = 1;
	
	if(str[0]=='-'){
		sign = -1;
		start = 1;
	}
	else if(str[0]=='+'){
		start = 1;
	}

	for(uint_t i=0, sz=data_size(); i<sz; ++i){
		if(str[i]=='.'){
			scale = 0.1f;
			continue;
		}

		ret *= scale;
		ret += str[i];
	} 
	return ret*sign; 
}

AnyPtr String::each(){
	return xnew<StringEachIter>(from_this(this));
}

ChRangePtr String::op_range(const StringPtr& right, int_t kind){
	if(kind!=RANGE_CLOSED){
		XTAL_THROW(RuntimeError()->call(Xt("Xtal Runtime Error 1025")), return xnew<ChRange>(empty_id, empty_id));		
	}

	if(length()==1 && right->length()==1){
		return xnew<ChRange>(from_this(this), right);
	}
	else{
		XTAL_THROW(RuntimeError()->call(Xt("Xtal Runtime Error 1023")), return xnew<ChRange>(empty_id, empty_id));		
	}
}

StringPtr String::op_cat(const StringPtr& v){
	uint_t mysize = data_size();
	uint_t vsize = v->data_size();

	if(mysize+vsize <= 16 || mysize<SMALL_STRING_MAX || vsize<SMALL_STRING_MAX)
		return xnew<String>(data(), mysize, v->data(), vsize);
	return xnew<String>((LargeString*)pvalue(*this), (LargeString*)pvalue(v));
}

bool String::op_eq(const StringPtr& v){ 
	return data_size()==v->data_size() && memcmp(data(), v->data(), data_size()*sizeof(char_t))==0; 
}

bool String::op_lt(const StringPtr& v){
	return string_compare(c_str(), v->c_str()) < 0;
}

StringPtr String::cat(const StringPtr& v){
	return op_cat(v);
}

uint_t String::hashcode(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->hashcode();
	}
	else{
		return make_string_hashcode(svalue_, data_size());
	}
}
	
AnyPtr String::split(const AnyPtr& pattern){
	using namespace xpeg;
	ArrayPtr ret = xnew<Array>();
	ExecutorPtr exec(xnew<Executor>(from_this(this)));
	if(exec->match(pattern)){
		ret->push_back(exec->prefix());
		while(exec->match(pattern)){
			ret->push_back(exec->prefix());
		}
		ret->push_back(exec->suffix());
	}
	else{
		ret->push_back(from_this(this));
	}
	return ret;
}

int_t String::calc_offset(int_t i){
	uint_t sz = data_size();
	if(i<0){
		i = sz + i;
		if(i<0){
			throw_index_error();
			return 0;
		}
	}
	else{
		if((uint_t)i >= sz){
			throw_index_error();
			return 0;
		}
	}
	return i;
}

void String::throw_index_error(){
	XTAL_THROW(RuntimeError()->call(Xt("Xtal Runtime Error 1020")), return);
}

////////////////////////////////////////////////////////////////


void LargeString::visit_members(Visitor& m){
	Base::visit_members(m);
	if((flags_ & ROPE)!=0){
		m & rope_.left & rope_.right;
	}
}

void visit_members(Visitor& m, const Named& p){
	m & p.name & p.value;
}

struct StringKey{
	const char_t* str;
	int_t size;

	StringKey(const char_t* str, int_t size)
		:str(str), size(size){}

	friend bool operator <(const StringKey& a, const StringKey& b){
		if(a.size<b.size)
			return true;
		if(a.size>b.size)
			return false;
		return memcmp(a.str, b.str, a.size)<0;
	}
};

void LargeString::common_init(uint_t size){
	XTAL_ASSERT(size>=Innocence::SMALL_STRING_MAX);

	data_size_ = size;
	str_.p = static_cast<char_t*>(user_malloc(sizeof(char_t)*(data_size_+1)));
	str_.p[data_size_] = 0;
	flags_ = 0;
}

LargeString::LargeString(const char_t* str1, uint_t size1, const char_t* str2, uint_t size2){
	common_init(size1 + size2);
	std::memcpy(str_.p, str1, size1*sizeof(char_t));
	std::memcpy(str_.p+size1, str2, size2*sizeof(char_t));
	make_string_hashcode_and_length(str_.p, data_size_, str_.hashcode, length_);
}

LargeString::LargeString(const char_t* str, uint_t size, uint_t hashcode, uint_t length, bool intern_flag){
	common_init(size);
	std::memcpy(str_.p, str, data_size_*sizeof(char_t));
	str_.hashcode = hashcode;
	flags_ = intern_flag ? INTERNED : 0;
	length_ = length;
}

LargeString::LargeString(LargeString* left, LargeString* right){
	left->inc_ref_count();
	right->inc_ref_count();
	rope_.left = left;
	rope_.right = right;
	data_size_ = left->data_size() + right->data_size();
	flags_ = ROPE;
	length_ = left->length() + right->length();
}

LargeString::~LargeString(){
	if((flags_ & ROPE)==0){
		user_free(str_.p);
	}
	else{
		rope_.left->dec_ref_count();
		rope_.right->dec_ref_count();
	}
}

const char_t* LargeString::c_str(){ 
	if((flags_ & ROPE)!=0)
		became_unified();
	return str_.p; 
}

uint_t LargeString::hashcode(){ 
	if((flags_ & ROPE)!=0)
		became_unified();
	return str_.hashcode; 
}

void LargeString::became_unified(){
	uint_t pos = 0;
	char_t* memory = (char_t*)user_malloc(sizeof(char_t)*(data_size_+1));
	write_to_memory(this, memory, pos);
	memory[pos] = 0;
	rope_.left->dec_ref_count();
	rope_.right->dec_ref_count();
	str_.p = memory;
	flags_ = 0;
	make_string_hashcode_and_length(str_.p, data_size_, str_.hashcode, length_);
}

void LargeString::write_to_memory(LargeString* p, char_t* memory, uint_t& pos){
	PStack<LargeString*> stack;
	for(;;){
		if((p->flags_ & ROPE)==0){
			std::memcpy(&memory[pos], p->str_.p, p->data_size_*sizeof(char_t));
			pos += p->data_size_;
			if(stack.empty())
				return;
			p = stack.pop();
		}
		else{
			stack.push(p->rope_.right);
			p = p->rope_.left;
		}
	}
}



ID::ID(const char_t* str)
	:String(*str_mgr_->insert(str)){}

ID::ID(const avoid<char>::type* str)	
	:String(noinit_t()){
	uint_t n = strlen((char*)str);
	UserMallocGuard umg(n*sizeof(char_t));
	char_t* buf = (char_t*)umg.get();
	for(uint_t i=0; i<n; ++i){
		buf[i] = str[i];
	}
		
	*this = ID(buf, n);
}

ID::ID(const string_t& str)
	:String(*str_mgr_->insert(str.c_str(), str.size())){}

ID::ID(const char_t* str, uint_t size)
	:String(*str_mgr_->insert(str, size)){}

ID::ID(const char_t* begin, const char_t* last)
	:String(*str_mgr_->insert(begin, last-begin)){}

ID::ID(char_t a)
	:String(noinit_t()){
	if(1<SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a;
	}
	else{
		*this = ID(&a, 1);
	}
}

ID::ID(char_t a, char_t b)
	:String(noinit_t()){
	if(2<SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b;
	}
	else{
		char_t buf[2] = {a, b};
		*this = ID(buf, 2);
	}
}

ID::ID(char_t a, char_t b, char_t c)
	:String(noinit_t()){
	if(3<SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b; svalue_[2] = c;
	}
	else{
		char_t buf[3] = {a, b, c};
		*this = ID(buf, 3);
	}
}

ID::ID(const char_t* str, uint_t len, uint_t hashcode, uint_t length)
	:String(str, len, hashcode, length, true){}


ID::ID(const StringPtr& name)
	:String(*(name ? name->intern() : (const IDPtr&)name)){}


AnyPtr SmartPtrCtor1<String>::call(type v){
	return xnew<String>(v);
}

AnyPtr SmartPtrCtor2<String>::call(type v){
	return xnew<String>(v);
}

AnyPtr SmartPtrCtor1<ID>::call(type v){
	return str_mgr_->insert(v);
}

AnyPtr SmartPtrCtor2<ID>::call(type v){
	if(v) return v->intern();
	return v;
}

AnyPtr SmartPtrCtor3<ID>::call(type v){
	return xnew<ID>(v);
}

void initialize_string(){
	register_uninitializer(&uninitialize_string);
	str_mgr_ = xnew<StringMgr>();
		
	{
		ClassPtr p = new_cpp_class<StringEachIter>(Xid(StringEachIter));
		p->inherit(Iterator());
		p->method(Xid(block_next), &StringEachIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<ChRangeIter>(Xid(ChRangeIter));
		p->inherit(Iterator());
		p->method(Xid(block_next), &ChRangeIter::block_next);
	}

	{
		new_cpp_class<Range>(Xid(Range));

		ClassPtr p = new_cpp_class<ChRange>(Xid(ChRange));
		p->inherit(get_cpp_class<Range>());
		p->def(Xid(new), ctor<ChRange, const StringPtr&, const StringPtr&>()->param(Named(Xid(left), null), Named(Xid(right), null)));
		p->method(Xid(each), &ChRange::each);
	}

	{
		ClassPtr p = new_cpp_class<String>(Xid(String));
		p->inherit(Iterable());

		p->def(Xid(new), ctor<String>());
		p->method(Xid(to_i), &String::to_i);
		p->method(Xid(to_f), &String::to_f);
		p->method(Xid(to_s), &String::to_s);
		p->method(Xid(clone), &String::clone);

		p->method(Xid(length), &String::length);
		p->method(Xid(size), &String::size);
		p->method(Xid(intern), &String::intern);

		p->method(Xid(each), &String::each);

		p->method(Xid(op_range), &String::op_range, get_cpp_class<String>());
		p->method(Xid(op_cat), &String::op_cat, get_cpp_class<String>());
		p->method(Xid(op_cat_assign), &String::op_cat, get_cpp_class<String>());
		p->method(Xid(op_eq), &String::op_eq, get_cpp_class<String>());
		p->method(Xid(op_lt), &String::op_lt, get_cpp_class<String>());
	}

	{
		ClassPtr p = new_cpp_class<InternedStringIter>(Xid(InternedStringIter));
		p->inherit(Iterator());
		p->method(Xid(block_next), &InternedStringIter::block_next);
	}

	set_cpp_class<ID>(get_cpp_class<String>());
}


}