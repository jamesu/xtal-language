
#pragma once

namespace xtal{

struct CoreSetting{
	ThreadLib* thread_lib;
	StreamLib* stream_lib;
	FilesystemLib* filesystem_lib;
	AllocatorLib* allocator_lib;
	ChCodeLib* chcode_lib;

	CoreSetting();
};

class Core{
public:

	Core(){}

	~Core(){
		uninitialize();
	}

	void initialize(const CoreSetting& setting);

	void uninitialize();

	void debug_print();

public:

	void* user_malloc(size_t size);

	void* user_malloc_nothrow(size_t size);

	void user_free(void* p);

	void* so_malloc(size_t size);

	void so_free(void* p, size_t size);

	void print_alive_objects();

	const SmartPtr<Filesystem>& filesystem(){
		return filesystem_;
	}

	ChCodeLib* chcode_lib(){
		return setting_.chcode_lib;
	}

public:

	void expand_objects_list();

	void enable_gc();

	void disable_gc();

	void gc();

	void full_gc();

	void register_gc(RefCountingBase* p);

	void register_gc_observer(GCObserver* p);

	void unregister_gc_observer(GCObserver* p);

public:

	int_t register_cpp_class(CppClassSymbolData* key);

	const ClassPtr& new_cpp_class(const StringPtr& name, CppClassSymbolData* key);

	bool exists_cpp_class(CppClassSymbolData* key){
		return key->value>=0 && key->value<(int_t)class_table_.size() && class_table_[key->value];
	}

	const ClassPtr& get_cpp_class(CppClassSymbolData* key){
		XTAL_ASSERT(exists_cpp_class(key));
		return from_this(class_table_[key->value]);
	}

	void set_cpp_class(const ClassPtr& cls, CppClassSymbolData* key){
		int_t index = register_cpp_class(key);
		if(class_table_[index]){
			class_table_[index]->dec_ref_count();
		}
		class_table_[index] = cls.get();
		class_table_[index]->inc_ref_count();
	}

	template<class T>
	const ClassPtr& new_cpp_class(const StringPtr& name = empty_string){
		return new_cpp_class(name, &CppClassSymbol<T>::value);
	}

	template<class T>
	const SmartPtr<T>& new_cpp_singleton(){
		int_t index = register_cpp_class(&CppClassSymbol<T>::value);

		if(Class* p = class_table_[index]){
			return unchecked_ptr_cast<T>(from_this(p));
		}

		T* p = (T*)so_malloc(sizeof(T));
		new(p) T();
		register_gc(p);
		class_table_[index] = p;
		return unchecked_ptr_cast<T>(from_this(class_table_[index]));
	}

	template<class T>
	bool exists_cpp_class(){
		return exists_cpp_class(&CppClassSymbol<T>::value);
	}

	template<class T>
	const ClassPtr& get_cpp_class(){
		return get_cpp_class(&CppClassSymbol<T>::value);
	}

	template<class T>
	void set_cpp_class(const ClassPtr& cls){
		set_cpp_class(cls, &CppClassSymbol<T>::value);
	}

	const ClassPtr& Iterator(){
		return Iterator_;
	}

	const ClassPtr& Iterable(){
		return Iterable_;
	}

	const ClassPtr& builtin(){
		return builtin_;
	}

	const LibPtr& lib(){
		return lib_;
	}

	VMachinePtr vm_take_over();

	void vm_take_back(const VMachinePtr& vm);

	const AnyPtr& cache_member(const AnyPtr& target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility){
		return member_cache_table_.cache(target_class, primary_key, secondary_key, accessibility);
	}

	bool cache_is(const AnyPtr& target_class, const AnyPtr& klass){
		return is_cache_table_.cache(target_class, klass);
	}

	void inc_mutate_count_cache_member(){
		member_cache_table_.inc_mutate_count();
	}

	void inc_mutate_count_cache_is(){
		is_cache_table_.inc_mutate_count();
		member_cache_table_.inc_mutate_count();
	}

	const SmartPtr<StringMgr>& string_mgr(){
		return string_mgr_;
	}

	const SmartPtr<ThreadMgr>& thread_mgr(){
		return thread_mgr_;
	}

public:

	enum{
		id_op_call,
		id_op_pos,
		id_op_neg,
		id_op_com,
		id_op_at,
		id_op_set_at,
		id_op_range,
		id_op_add,
		id_op_cat,
		id_op_sub,
		id_op_mul,
		id_op_div,
		id_op_mod,
		id_op_and,
		id_op_or,
		id_op_xor,
		id_op_shl,
		id_op_shr,
		id_op_ushr,
		id_op_eq,
		id_op_lt,
		id_op_in,
		id_op_inc,
		id_op_dec,
		id_op_add_assign,
		id_op_sub_assign,
		id_op_cat_assign,
		id_op_mul_assign,
		id_op_div_assign,
		id_op_mod_assign,
		id_op_and_assign,
		id_op_or_assign,
		id_op_xor_assign,
		id_op_shl_assign,
		id_op_shr_assign,
		id_op_ushr_assign,

		id_op_MAX
	};

	const IDPtr* id_op_list(){
		return id_op_list_;
	}

private:

	void bind();
	void exec_script();
	
private:

	SmallObjectAllocator so_alloc_;
	CoreSetting setting_;

	RefCountingBase** objects_begin_ ;
	RefCountingBase** objects_current_;
	RefCountingBase** objects_end_;

	RefCountingBase*** objects_list_begin_;
	RefCountingBase*** objects_list_current_;
	RefCountingBase*** objects_list_end_;

	GCObserver** gcobservers_begin_;
	GCObserver** gcobservers_current_;
	GCObserver** gcobservers_end_;

	uint_t objects_count_;
	uint_t prev_objects_count_;

	uint_t cycle_count_;

	IDPtr id_op_list_[id_op_MAX];

	PODArrayList<Class*> class_table_;

	SmartPtr<Filesystem> filesystem_;

	ClassPtr Iterator_;
	ClassPtr Iterable_;
	ClassPtr builtin_;
	LibPtr lib_;

	ArrayPtr vm_list_;

	struct MemberCacheTable{
		struct Unit{
			uint_t mutate_count;
			uint_t accessibility;
			AnyPtr target_class;
			AnyPtr primary_key;
			AnyPtr secondary_key;
			AnyPtr member;
		};

		enum{ CACHE_MAX = 256, CACHE_MASK = CACHE_MAX-1 };

		static uint_t calc_index(uint_t hash){
			return hash & CACHE_MASK;
		}

		Unit table_[CACHE_MAX];

		int_t hit_;
		int_t miss_;
		int_t collided_;
		uint_t mutate_count_;

		MemberCacheTable(){
			hit_ = 0;
			miss_ = 0;
			collided_ = 0;
			mutate_count_ = 0;
		}

		int_t hit_count(){
			return hit_;
		}

		int_t miss_count(){
			return miss_;
		}

		int_t collided_count(){
			return collided_;
		}

		void inc_mutate_count(){
			mutate_count_++;
		}

		const AnyPtr& cache(const AnyPtr& target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility);

		void clear(){
			for(int_t i=0; i<CACHE_MAX; ++i){
				Unit& unit = table_[i];
				unit.target_class = null;
				unit.primary_key = null;
				unit.secondary_key = null;
				unit.member = null;	
			}
			mutate_count_++;
		}
	};

	struct IsCacheTable{
		struct Unit{
			uint_t mutate_count;
			AnyPtr target_class;
			AnyPtr klass;
			bool result;
		};

		enum{ CACHE_MAX = 64, CACHE_MASK = CACHE_MAX-1 };

		static uint_t calc_index(uint_t hash){
			return hash & CACHE_MASK;
		}

		Unit table_[CACHE_MAX];

		int_t hit_;
		int_t miss_;
		int_t collided_;
		uint_t mutate_count_;

		IsCacheTable(){
			hit_ = 0;
			miss_ = 0;
			collided_ = 0;
			mutate_count_ = 0;
		}

		int_t hit_count(){
			return hit_;
		}

		int_t miss_count(){
			return miss_;
		}
		
		int_t collided_count(){
			return collided_;
		}

		void inc_mutate_count(){
			mutate_count_++;
		}

		bool cache(const AnyPtr& target_class, const AnyPtr& klass);

		void clear(){
			for(int_t i=0; i<CACHE_MAX; ++i){
				Unit& unit = table_[i];
				unit.target_class = null;
				unit.klass = null;
			}
			mutate_count_++;
		}
	};

	MemberCacheTable member_cache_table_;
	IsCacheTable is_cache_table_;

	SmartPtr<StringMgr> string_mgr_;
	SmartPtr<ThreadMgr> thread_mgr_;
};

Core* core();

void set_core(Core* e);


/**
* @brief ���[�U�[���o�^�����������A���P�[�g�֐����g���ă������m�ۂ���B
*
* �������m�ێ��s�͗�O�ŕԂ����B
*/
inline void* user_malloc(size_t size){
	return core()->user_malloc(size);
}

/**
* @brief ���[�U�[���o�^�����������A���P�[�g�֐����g���ă������m�ۂ���B
*
* �������m�ێ��s��NULL�l�ŕԂ����B
*/
inline void* user_malloc_nothrow(size_t size){
	return core()->user_malloc_nothrow(size);
}

/**
* @brief ���[�U�[���o�^�����������f�A���P�[�g�֐����g���ă������������B
*
*/
inline void user_free(void* p){
	return core()->user_free(p);
}

/**
* @brief �������I�u�W�F�N�g�p�Ƀ��������A���P�[�g����B
*/
inline void* so_malloc(size_t size){
	return core()->so_malloc(size);
}

/**
* @brief �������I�u�W�F�N�g�p�̃��������������B
*/
inline void so_free(void* p, size_t size){
	return core()->so_free(p, size);
}

/**
* @brief �K�[�x�W�R���N�V���������s����
*
* ���قǎ��Ԃ͂�����Ȃ����A���S�ɃS�~������ł��Ȃ��K�[�x�W�R���N�g�֐�
* �Ⴆ�Ώz�Q�Ƃ͂���ł͌��m�ł��Ȃ��B
* �Q�[���Ŏg�p����ꍇ�A���t���[��������ĂԂ��Ƃ𐄏�����B
*/
inline void gc(){
	return core()->gc();
}

/**
* @brief �z�I�u�W�F�N�g���������t���K�[�x�W�R���N�V���������s����
*
* ���Ԃ͂����邪�A���݃S�~�ƂȂ��Ă�����̂͂Ȃ�ׂ��S�ĉ������K�[�x�W�R���N�g�֐�
* �z�Q�Ƃ����m�ł���B
* �Q�[���Ŏg�p����ꍇ�A�V�[���̐؂�ւ����ȂǁA�ߖڐߖڂɌĂԂ��Ƃ𐄏�����B
*/
inline void full_gc(){
	return core()->full_gc();
}

/**
* @brief �K�[�x�W�R���N�V�����𖳌�������
*
* gc��full_gc�̌Ăяo���𖳌�������֐��B
* �����ł��ꂪ����Ăяo���ꂽ���L������Ă���A�Ăяo������enable_gc���ĂтȂ��ƃK�[�x�W�R���N�V�����͗L���ɂȂ�Ȃ�
*/
inline void disable_gc(){
	return core()->disable_gc();
}

/**
* @brief �K�[�x�W�R���N�V������L��������
*
* disable_gc���Ă΂ꂽ�񐔂Ɠ��������Ăяo���ƃK�[�x�W�R���N�V�������L���ɂȂ�
* 
*/
inline void enable_gc(){
	return core()->enable_gc();
}

/**
* @brief T�`��xtal�ň�����N���X�𐶐����A�o�^����B
* ���ɐ�������Ă���ꍇ�A�����ς݂̃N���X��Ԃ��B
*/
template<class T>
const ClassPtr& new_cpp_class(const StringPtr& name){
	return core()->new_cpp_class<T>(name);
}

/**
* @brief T�`��xtal�ň�����N���X�𐶐����A�o�^����B
* ���ɐ�������Ă���ꍇ�A�����ς݂̃N���X��Ԃ��B
*/
template<class T>
const SmartPtr<T>& new_cpp_singleton(){
	return core()->new_cpp_singleton<T>();
}

/**
* @brief ����new_cpp_class<T>()�Ő���������Ă��邩���ׂ�B
*/
template<class T>
bool exists_cpp_class(){
	return core()->exists_cpp_class<T>();
}

/**
* @brief new_cpp_class<T>()�Ő������ꂽ�N���X���擾����B
*/
template<class T>
const ClassPtr& get_cpp_class(){
	return core()->get_cpp_class<T>();
}

/**
* @brief get_cpp_class<T>�ȂǂŕԂ����N���X��ݒ肷��B
*/
template<class T>
void set_cpp_class(const ClassPtr& cls){
	return core()->set_cpp_class<T>(cls);
}

/**
* @brief Iterator�N���X��Ԃ�
*/
inline const ClassPtr& Iterator(){
	return core()->Iterator();
}

/**
* @brief Iterable�N���X��Ԃ�
*/
inline const ClassPtr& Iterable(){
	return core()->Iterable();
}

/**
* @brief builtin�V���O���g���N���X��Ԃ�
*/
inline const ClassPtr& builtin(){
	return core()->builtin();
}

/**
* @brief lib�N���X��Ԃ�
*/
inline const LibPtr& lib(){
	return core()->lib();
}

const StreamPtr& stdin_stream();

const StreamPtr& stdout_stream();

const StreamPtr& stderr_stream();

const ClassPtr& RuntimeError();

const ClassPtr& CompileError();

const ClassPtr& UnsupportedError();

const ClassPtr& ArgumentError();

inline const IDPtr& intern_literal(const char_t* str){
	return core()->string_mgr()->insert_literal(str);
}

inline const IDPtr& intern(const char_t* str){
	return core()->string_mgr()->insert(str);
}

inline const IDPtr& intern(const char_t* str, uint_t data_size){
	return core()->string_mgr()->insert(str, data_size);
}

inline const IDPtr& intern(const char_t* str, uint_t data_size, uint_t hash){
	return core()->string_mgr()->insert(str, data_size, hash);
}

inline AnyPtr interned_strings(){
	return core()->string_mgr()->interned_strings();
}

inline bool thread_enabled(){
	return core()->thread_mgr()->thread_enabled();
}

inline void yield_thread(){
	return core()->thread_mgr()->yield_thread();
}

inline void sleep_thread(float_t sec){
	return core()->thread_mgr()->sleep_thread(sec);
}

inline ThreadPtr new_thread(const AnyPtr& callback_fun){
	return core()->thread_mgr()->new_thread(callback_fun);
}

inline MutexPtr new_mutex(){
	return core()->thread_mgr()->new_mutex();
}

inline void lock_mutex(const MutexPtr& p){
	return core()->thread_mgr()->lock_mutex(p);
}

/**
* @brief VMachinePtr�I�u�W�F�N�g��Ԃ�
*
* �O���[�o����VMachinePtr�I�u�W�F�N�g��Ԃ��B
* �X���b�h���ɂ��̃O���[�o��VMachinePtr�I�u�W�F�N�g�͑��݂���B
*/
inline const VMachinePtr& vmachine(){
	return core()->thread_mgr()->vmachine();
}

#ifndef XTAL_NO_PARSER

/**
* @brief file_name�t�@�C�����R���p�C������B
*
* @param file_name Xtal�X�N���v�g���L�q���ꂽ�t�@�C���̖��O
* @return ���s�ł���֐��I�u�W�F�N�g
* ���̖߂�l��serialize����ƁA�o�C�g�R�[�h�`���ŕۑ������B
*/
CodePtr compile_file(const StringPtr& file_name);

/**
* @brief source��������R���p�C������B
*
* @param source Xtal�X�N���v�g���L�q���ꂽ������
* @return ���s�ł���֐��I�u�W�F�N�g
* ���̖߂�l��serialize����ƁA�o�C�g�R�[�h�`���ŕۑ������B
*/
CodePtr compile(const StringPtr& source);

/**
* @brief file_name�t�@�C�����R���p�C�����Ď��s����B
*
* @param file_name Xtal�X�N���v�g���L�q���ꂽ�t�@�C���̖��O
* @return �X�N���v�g����return���ꂽ�l
*/
AnyPtr load(const StringPtr& file_name);

/**
* @brief file_name�t�@�C�����R���p�C�����ăR���p�C���ς݃\�[�X��ۑ����A���s����B
*
* @param file_name Xtal�X�N���v�g���L�q���ꂽ�t�@�C���̖��O
* @return �X�N���v�g����return���ꂽ�l
*/
AnyPtr load_and_save(const StringPtr& file_name);

CodePtr source(const char_t* src, int_t size, const char* file);

#endif

CodePtr compiled_source(const void* src, int_t size, const char* file);

/**
* @brief �擪�o�C�g�����āA���̃}���`�o�C�g���������������𒲂ׂ�B
*
* �}�C�i�X�̒l���Ԃ��ꂽ�ꍇ�A�Œᕶ������Ԃ��B
* -2�̏ꍇ�A�Œ�2�����ȏ�̕����ŁA�{���̒�����2�����ڂ�ǂ܂Ȃ��Ɣ��f�ł��Ȃ��A�Ƃ����Ӗ��ƂȂ�B
*/
int_t ch_len(char_t lead);

/**
* @brief �}���`�o�C�g���������������𒲂ׂ�B
*
* int_t ch_len(char_t lead)���Ă΂ꂽ��A�}�C�i�X�̒l��Ԃ����ꍇ�ɑ����ČĂԁB
* ch_len��-2�̒l��Ԃ�����́Astr�̐�ɂ͍Œ�2�o�C�g���̃f�[�^���i�[���邱�ƁB
*/
int_t ch_len2(const char_t* str);


/**
* @brief ���̕�����Ԃ�
*
* �Ⴆ�� a ��n�����ꍇ�Ab ���Ԃ�
*/
StringPtr ch_inc(const char_t* data, int_t data_size);

/**
* @brief �����̑召����
*
* ���̒l a �̕����̕���b�̕�����菬����
* 0�̒l ������
* ���̒l b�̕����̕���a�̕�����菬����
*/
int_t ch_cmp(const char_t* a, uint_t asize, const char_t* b, uint_t bsize);

}