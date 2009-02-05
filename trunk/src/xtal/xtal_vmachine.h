
#pragma once

namespace xtal{

// XTAL仮想マシン
class VMachine : public GCObserver{
public:

	VMachine();

public:

	// 関数呼び出し側が使うための関数群

	/**
	* @brief 引数を1個積む。
	*
	*/
	void push_arg(const AnyPtr& value);
	
	/**
	* @brief 名前付き引数を1個積む。
	*
	*/
	void push_arg(const IDPtr& name, const AnyPtr& value);
	
	/**
	* @brief 名前付き引数を1個積む。
	*
	*/
	void push_arg(const Named& p){ push_arg(p.name, p.value); }

	/**
	* @brief 引数を配列の要素数積む。
	*
	*/
	void push_ordered_args(const ArrayPtr& p);

	/**
	* @brief 名前つき引数を連想配列の要素数積む。
	*
	*/
	void push_named_args(const MapPtr& p);

	/**
	* @brief pos番目の戻り値を得る。
	*
	*/
	const AnyPtr& result(int_t pos = 0);

	/**
	* @brief 呼び出しの後始末をする。
	*
	*/
	void cleanup_call();	

	/**
	* @brief pos番目の戻り値を返し、呼び出しの後始末をする。
	*
	*/
	AnyPtr result_and_cleanup_call(int_t pos = 0);
		
	/**
	* @brief thisを差し替える。
	*
	*/	
	void set_arg_this(const AnyPtr& self){ 
		ff().self(self);
	}

	/**
	* @brief ヒントの設定
	*
	* 例外が起きたときのエラーメッセージのために、
	* 現在呼び出している関数を登録する。
	*/
	void set_hint(const AnyPtr& object){ 
		ff().hint(object);
	}
	
	void send_error(){
		ff().set_null();
		fun_frames_.push(fun_frames_.top());
	}

// 

	/// @brief 関数を呼び出す用意をし、同時に引数を`i`個積む
	void setup_call(int_t need_result_count = 1){
		push_ff(need_result_count);	
	}

//{REPEAT{{
/*
	/// @brief 関数を呼び出す用意をし、同時に引数を`n+1`個積む
	template<class A0 #COMMA_REPEAT#class A`i+1`#>
	void setup_call(int_t need_result_count, const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#){
		push_ff(need_result_count);
		push_arg(a0);
		#REPEAT#push_arg(a`i+1`);# 
	}
*/

	/// @brief 関数を呼び出す用意をし、同時に引数を1個積む
	template<class A0 >
	void setup_call(int_t need_result_count, const A0& a0 ){
		push_ff(need_result_count);
		push_arg(a0);
		 
	}

	/// @brief 関数を呼び出す用意をし、同時に引数を2個積む
	template<class A0 , class A1>
	void setup_call(int_t need_result_count, const A0& a0 , const A1& a1){
		push_ff(need_result_count);
		push_arg(a0);
		push_arg(a1); 
	}

	/// @brief 関数を呼び出す用意をし、同時に引数を3個積む
	template<class A0 , class A1, class A2>
	void setup_call(int_t need_result_count, const A0& a0 , const A1& a1, const A2& a2){
		push_ff(need_result_count);
		push_arg(a0);
		push_arg(a1);push_arg(a2); 
	}

	/// @brief 関数を呼び出す用意をし、同時に引数を4個積む
	template<class A0 , class A1, class A2, class A3>
	void setup_call(int_t need_result_count, const A0& a0 , const A1& a1, const A2& a2, const A3& a3){
		push_ff(need_result_count);
		push_arg(a0);
		push_arg(a1);push_arg(a2);push_arg(a3); 
	}

	/// @brief 関数を呼び出す用意をし、同時に引数を5個積む
	template<class A0 , class A1, class A2, class A3, class A4>
	void setup_call(int_t need_result_count, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4){
		push_ff(need_result_count);
		push_arg(a0);
		push_arg(a1);push_arg(a2);push_arg(a3);push_arg(a4); 
	}

	/// @brief 関数を呼び出す用意をし、同時に引数を6個積む
	template<class A0 , class A1, class A2, class A3, class A4, class A5>
	void setup_call(int_t need_result_count, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5){
		push_ff(need_result_count);
		push_arg(a0);
		push_arg(a1);push_arg(a2);push_arg(a3);push_arg(a4);push_arg(a5); 
	}

//}}REPEAT}

public:

	// 関数呼び出され側が使うための関数群

	/**
	* @brief pos番目の引数を得る。
	*
	*/
	const AnyPtr& arg(int_t pos);

	/**
	* @brief nameに対応する引数を得る。
	*
	*/
	const AnyPtr& arg(const IDPtr& name);

	/**
	* @brief pos番目の引数を得る。もしpos番目の引数がなければnameに対応する引数を得る。
	*
	*/
	const AnyPtr& arg(int_t pos, const IDPtr& name);
	
	const AnyPtr& arg(int_t pos, Fun* names);

	/**
	* @brief pos番目の引数を得る。
	*
	* もしpos番目の引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(int_t pos, const AnyPtr& def);

	/**
	* @brief nameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(const IDPtr& name, const AnyPtr& def);

	/**
	* @brief pos番目の引数を得る。もしpos番目の引数がなければnameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(int_t pos, const IDPtr& name, const AnyPtr& def);
	
	/**
	* @brief nameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(const Named& name_and_def);

	/**
	* @brief pos番目の引数を得る。もしpos番目の引数がなければnameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(int_t pos, const Named& name_and_def);

	/**
	* @brief pos番目の名前指定引数の名前を取得。
	*
	* @param pos 0縲從amed_arg_count()-1まで
	*/
	const IDPtr& arg_name(int_t pos){
		return unchecked_ptr_cast<ID>(get(named_arg_count()*2-1-(pos*2+0)));
	}

	/**
	* @brief thisを取得。
	*
	*/
	const AnyPtr& get_arg_this(){ 
		return ff().self(); 
	}

	/**
	* @brief 位置指定引数の数を取得。
	*
	*/
	int_t ordered_arg_count(){ 
		return ff().ordered_arg_count; 
	}
	
	/**
	* @brief 名前指定引数の数を取得
	*
	*/
	int_t named_arg_count(){ 
		return ff().named_arg_count; 
	}
	
	/**
	* @brief 引数の数を調整する。
	*
	*/
	void adjust_arg(int_t n);

	/**
	* @brief 引数の多値を平らにする
	*
	*/
	void flatten_arg();

	/**
	* @brief  引数の多値を平らにする
	*
	*/
	void flatten_all_arg();
	
	/**
	* @brief 呼び出し元が必要としている戻り値の数。
	*
	*/
	int_t need_result_count(){ 
		return ff().need_result_count; 
	}
	
	/**
	* @brief 呼び出し元が戻り値を必要としているかどうか。
	*
	*/
	bool need_result(){ 
		return ff().need_result_count!=0; 
	}
	
	/*
	* Argumentsオブジェクトを生成する。
	* return_result()を呼んだ後は正常な値は得られない。
	*/
	ArgumentsPtr make_arguments();
	
	/**
	* @brief 呼び出し元に引数の数だけの戻り値を返す。
	*
	* need_result_countの方が数が多い場合はその分nullが返される。
	* need_result_countの方が数が少ない場合は返した値は捨てられる。
	*/
	void return_result();
	void return_result(const AnyPtr& value1);
	void return_result(const AnyPtr& value1, const AnyPtr& value2);
	void return_result(const AnyPtr& value1, const AnyPtr& value2, const AnyPtr& value3);
	void return_result(const AnyPtr& value1, const AnyPtr& value2, const AnyPtr& value3, const AnyPtr& value4);
	void return_result_array(const ArrayPtr& values);

	/**
	* @brief return_resultやcarry_overを既に呼び出したならtrueを、そうでないならfalseを返す。
	*
	*/
	bool processed(){ 
		return *ff().called_pc!=InstThrowUnsupportedError::NUMBER; 
	}
	
	void replace_result(int_t pos, const AnyPtr& v);

	void recycle_call();

	void recycle_call(const AnyPtr& a1);

public:

	const AnyPtr& catch_except(){
		except_[2] = except();
		except_[0] = null;
		except_[1] = null;
		return ap(except_[2]);
	}

	const AnyPtr& except(){
		if(ap(except_[0])) return ap(except_[0]);
		return ap(except_[1]);
	}

	void set_except(const AnyPtr& e){
		except_[0] = e;
		except_[1] = null;
	}

	void set_except_0(const Any& e);

	void set_unsuported_error_info(const Any& target, const Any& primary_key, const Any& secondary_key){
		ff().target_ = target;
		ff().primary_key_ = primary_key;
		ff().secondary_key_ = secondary_key;
	}

	void execute_inner(const inst_t* start);

	void execute(Fun* fun, const inst_t* start_pc);

	void carry_over(Fun* fun);

	void mv_carry_over(Fun* fun);

	const inst_t* resume_pc(){
		return resume_pc_;
	}

	void present_for_vm(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result);

	const inst_t* start_fiber(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result);

	const inst_t* resume_fiber(Fiber* fun, const inst_t* pc, VMachine* vm, bool add_succ_or_fail_result);
	
	void exit_fiber();
	
	void reset();

public:

	void adjust_result(int_t n){		
		adjust_result(n, ff().need_result_count);
	}

	void adjust_result(int_t n, int_t need_result_count);

public:

	// スタックのi番目の値を取得する。
	const AnyPtr& get(int_t i){ return ap(stack_[i]); }

	// スタックの0番目の値を取得する。
	const AnyPtr& get(){ return ap(stack_.top()); }

	// スタックのi番目の値を設定する。
	void set(int_t i, const Any& v){ stack_[i]=v; }

	// スタックの0番目の値を設定する。
	void set(const Any& v){ stack_.top()=v; }

	// スタックをn拡大する。
	void upsize(int_t n){ stack_.upsize_unchecked(n); }

	// スタックをn縮小する
	void downsize(int_t n){ stack_.downsize(n); }

	// スタックをn個にする。
	void resize(int_t n){ stack_.resize(n); }

	// スタックに値vをプッシュする。
	void push(const Any& v){ stack_.push_unchecked(v); }

	// スタックに値vをプッシュする。
	void push(const Named& v){ push(v.name); push(v.value); }

	// スタックから値をポップする。
	const AnyPtr& pop(){ return ap(stack_.pop()); }

	// 先頭の値をプッシュする。
	void dup(){ push(get()); }

	// i番目の値をプッシュする。
	void dup(int_t i){ push(get(i)); }

	// スタックの大きさを返す。
	int_t stack_size(){ return (int_t)stack_.size(); }
	
	// srcのスタックの内容をsize個プッシュする。
	void push(VMachine* src, int_t size){ stack_.push(src->stack_, size); }
	
	// srcのスタックの内容をsize個プッシュする。
	void push(VMachine* src, int_t src_offset, int_t size){ stack_.push(src->stack_, src_offset, size); }

	// srcのスタックの内容をsize個取り除いて、プッシュする。
	void move(VMachine* src, int_t size){ stack_.move(src->stack_, size); }
	
public:

	struct FunFrame{

		// pop_ffしたときはこのpcから実行する
		const inst_t* poped_pc;

		// callしたときはこのpcから実行する
		const inst_t* called_pc;

		// 関数が呼ばれたときの順番指定引数の数
		int_t ordered_arg_count;
		
		// 関数が呼ばれたときの名前指定引数の数
		int_t named_arg_count;

		// 関数呼び出し側が必要とする戻り値の数
		int_t need_result_count;

		// yieldが可能かフラグ。このフラグは呼び出しを跨いで伝播する。
		int_t yieldable;

		// thisが持つインスタンス変数へのポインタ
		InstanceVariables* instance_variables;

		// スコープがオブジェクト化されてない時のローカル変数領域
		Stack<Any> variables_;

		// 呼び出された関数オブジェクト
		Any fun_; 

		// スコープの外側のフレームオブジェクト
		Any outer_;

		// 関数が呼ばれたときのthisオブジェクト
		Any self_;

		// オブジェクト化した引数。
		Any arguments_;
		
		// デバッグメッセージ出力用のヒント
		Any hint_;

		// UnsuportedErrorのためにtargetをおくところ
		Any target_;

		// UnsuportedErrorのためにprimary_keyをおくところ
		Any primary_key_;

		// UnsuportedErrorのためにsecondary_keyをおくところ
		Any secondary_key_;

		void set_null(){
			set_null_force(fun_); 
			set_null_force(outer_);
			set_null_force(arguments_);
			set_null_force(hint_);
			set_null_force(target_);
			set_null_force(primary_key_);
			set_null_force(secondary_key_);
		}

		const FunPtr& fun() const{ return unchecked_ptr_cast<Fun>(ap(fun_)); }
		const FramePtr& outer() const{ return unchecked_ptr_cast<Frame>(ap(outer_)); }
		const AnyPtr& variable(int_t i) const{ return ap(variables_[i]); }
		const AnyPtr& self() const{ return ap(self_); }
		const ArgumentsPtr& arguments() const{ return unchecked_ptr_cast<Arguments>(ap(arguments_)); }
		const AnyPtr& hint() const{ return ap(hint_); }

		int_t args_stack_size(){
			return ordered_arg_count+(named_arg_count<<1);
		}

		void fun(const Any& v){ fun_ = v; }
		void outer(const Any& v){ outer_ = v; }
		void variable(int_t i, const Any& v){ variables_[i] = v; }
		void self(const Any& v){ self_ = v; }
		void arguments(const Any& v){ arguments_ = v; }
		void hint(const Any& v){ hint_ = v; }

		void inc_ref();
		void dec_ref();
	};

	friend void visit_members(Visitor& m, const FunFrame& v);

	// 例外を処理するためのフレーム
	struct ExceptFrame{
		ExceptInfo* core;
		int_t stack_count;
		int_t fun_frame_count;
		int_t variable_size;
		Any outer;
	};

	void push_ff(int_t need_result_count);
	void push_ff(const inst_t* pc, int_t need_result_count, int_t ordered_arg_count, int_t named_arg_count, const AnyPtr& self);
	void push_ff(const inst_t* pc, const InstCall& inst, const AnyPtr& self);
	void recycle_ff(const inst_t* pc, int_t ordered_arg_count, int_t named_arg_count, const AnyPtr& self);
	const inst_t* pop_ff(){ return fun_frames_.pop().poped_pc; }

	void push_args(const ArgumentsPtr& args, int_t named_arg_count);

	FunFrame& ff(){ return fun_frames_.top(); }
	FunFrame& prev_ff(){ return fun_frames_[1]; }

	const FunPtr& fun(){ return ff().fun(); }
	const FunPtr& prev_fun(){ return prev_ff().fun(); }

	const FramePtr& outer(){ return ff().outer(); }
	const FramePtr& prev_outer(){ return prev_ff().outer(); }

	const CodePtr& code();
	const CodePtr& prev_code();

	const IDPtr& identifier(int_t n);
	const IDPtr& prev_identifier(int_t n);
	const IDPtr& identifier_or_pop(int_t n);

	void return_result_instance_variable(int_t number, ClassInfo* core);

	ArgumentsPtr make_args(Fun* fun);

	AnyPtr append_backtrace(const inst_t* pc, const AnyPtr& ep);
	
	const VMachinePtr& myself(){ return *(const VMachinePtr*)&myself_; }

private:
	const inst_t* inner_send(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, const Any& secondary_key, const Any& target);
	const inst_t* inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget);

	const inst_t* inner_send(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, const Any& secondary_key, const Any& target, const Any& a0);
	const inst_t* inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0);

	const inst_t* inner_send(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, const Any& secondary_key, const Any& target, const Any& a0, const Any& a1);
	const inst_t* inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0, int_t na1);

	const inst_t* inner_send_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, const Any& secondary_key, const Any& target, const Any& a0);
	const inst_t* inner_send_from_stack_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0);

	const inst_t* inner_send_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, const Any& secondary_key, const Any& target, const Any& a0, const Any& a1);
	const inst_t* inner_send_from_stack_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0, int_t na1);

	const inst_t* push_except();

	void set_local_variable(int_t pos, const Any&);
	const AnyPtr& local_variable(int_t pos);

	const inst_t* catch_body(const inst_t* pc, int_t stack_size, int_t fun_frames_size);

	void debug_hook(const inst_t* pc, int_t kind);

public:

//{DECLS{{
	const inst_t* FunNop(const inst_t* pc);
	const inst_t* FunPushNull(const inst_t* pc);
	const inst_t* FunPushUndefined(const inst_t* pc);
	const inst_t* FunPushTrue(const inst_t* pc);
	const inst_t* FunPushFalse(const inst_t* pc);
	const inst_t* FunPushInt1Byte(const inst_t* pc);
	const inst_t* FunPushInt2Byte(const inst_t* pc);
	const inst_t* FunPushFloat1Byte(const inst_t* pc);
	const inst_t* FunPushFloat2Byte(const inst_t* pc);
	const inst_t* FunPushCallee(const inst_t* pc);
	const inst_t* FunPushArgs(const inst_t* pc);
	const inst_t* FunPushThis(const inst_t* pc);
	const inst_t* FunPushCurrentContext(const inst_t* pc);
	const inst_t* FunPop(const inst_t* pc);
	const inst_t* FunDup(const inst_t* pc);
	const inst_t* FunInsert1(const inst_t* pc);
	const inst_t* FunInsert2(const inst_t* pc);
	const inst_t* FunInsert3(const inst_t* pc);
	const inst_t* FunAdjustResult(const inst_t* pc);
	const inst_t* FunIf(const inst_t* pc);
	const inst_t* FunUnless(const inst_t* pc);
	const inst_t* FunGoto(const inst_t* pc);
	const inst_t* FunLocalVariableInc(const inst_t* pc);
	const inst_t* FunLocalVariableIncDirect(const inst_t* pc);
	const inst_t* FunLocalVariableDec(const inst_t* pc);
	const inst_t* FunLocalVariableDecDirect(const inst_t* pc);
	const inst_t* FunLocalVariableInc2Byte(const inst_t* pc);
	const inst_t* FunLocalVariableDec2Byte(const inst_t* pc);
	const inst_t* FunLocalVariable1Byte(const inst_t* pc);
	const inst_t* FunLocalVariable1ByteDirect(const inst_t* pc);
	const inst_t* FunLocalVariable2Byte(const inst_t* pc);
	const inst_t* FunSetLocalVariable1Byte(const inst_t* pc);
	const inst_t* FunSetLocalVariable1ByteDirect(const inst_t* pc);
	const inst_t* FunSetLocalVariable2Byte(const inst_t* pc);
	const inst_t* FunInstanceVariable(const inst_t* pc);
	const inst_t* FunSetInstanceVariable(const inst_t* pc);
	const inst_t* FunCleanupCall(const inst_t* pc);
	const inst_t* FunReturn(const inst_t* pc);
	const inst_t* FunYield(const inst_t* pc);
	const inst_t* FunExit(const inst_t* pc);
	const inst_t* FunValue(const inst_t* pc);
	const inst_t* FunCheckUnsupported(const inst_t* pc);
	const inst_t* FunProperty(const inst_t* pc);
	const inst_t* FunSetProperty(const inst_t* pc);
	const inst_t* FunCall(const inst_t* pc);
	const inst_t* FunSend(const inst_t* pc);
	const inst_t* FunMember(const inst_t* pc);
	const inst_t* FunDefineMember(const inst_t* pc);
	const inst_t* FunGlobalVariable(const inst_t* pc);
	const inst_t* FunSetGlobalVariable(const inst_t* pc);
	const inst_t* FunDefineGlobalVariable(const inst_t* pc);
	const inst_t* FunOnce(const inst_t* pc);
	const inst_t* FunSetOnce(const inst_t* pc);
	const inst_t* FunBlockBegin(const inst_t* pc);
	const inst_t* FunBlockBeginDirect(const inst_t* pc);
	const inst_t* FunBlockEnd(const inst_t* pc);
	const inst_t* FunBlockEndDirect(const inst_t* pc);
	const inst_t* FunTryBegin(const inst_t* pc);
	const inst_t* FunTryEnd(const inst_t* pc);
	const inst_t* FunPushGoto(const inst_t* pc);
	const inst_t* FunPopGoto(const inst_t* pc);
	const inst_t* FunIfEq(const inst_t* pc);
	const inst_t* FunIfNe(const inst_t* pc);
	const inst_t* FunIfLt(const inst_t* pc);
	const inst_t* FunIfLe(const inst_t* pc);
	const inst_t* FunIfGt(const inst_t* pc);
	const inst_t* FunIfGe(const inst_t* pc);
	const inst_t* FunIfRawEq(const inst_t* pc);
	const inst_t* FunIfRawNe(const inst_t* pc);
	const inst_t* FunIfIn(const inst_t* pc);
	const inst_t* FunIfNin(const inst_t* pc);
	const inst_t* FunIfIs(const inst_t* pc);
	const inst_t* FunIfNis(const inst_t* pc);
	const inst_t* FunIfArgIsUndefined(const inst_t* pc);
	const inst_t* FunIfArgIsUndefinedDirect(const inst_t* pc);
	const inst_t* FunPos(const inst_t* pc);
	const inst_t* FunNeg(const inst_t* pc);
	const inst_t* FunCom(const inst_t* pc);
	const inst_t* FunNot(const inst_t* pc);
	const inst_t* FunAt(const inst_t* pc);
	const inst_t* FunSetAt(const inst_t* pc);
	const inst_t* FunAdd(const inst_t* pc);
	const inst_t* FunSub(const inst_t* pc);
	const inst_t* FunCat(const inst_t* pc);
	const inst_t* FunMul(const inst_t* pc);
	const inst_t* FunDiv(const inst_t* pc);
	const inst_t* FunMod(const inst_t* pc);
	const inst_t* FunAnd(const inst_t* pc);
	const inst_t* FunOr(const inst_t* pc);
	const inst_t* FunXor(const inst_t* pc);
	const inst_t* FunShl(const inst_t* pc);
	const inst_t* FunShr(const inst_t* pc);
	const inst_t* FunUshr(const inst_t* pc);
	const inst_t* FunEq(const inst_t* pc);
	const inst_t* FunNe(const inst_t* pc);
	const inst_t* FunLt(const inst_t* pc);
	const inst_t* FunLe(const inst_t* pc);
	const inst_t* FunGt(const inst_t* pc);
	const inst_t* FunGe(const inst_t* pc);
	const inst_t* FunRawEq(const inst_t* pc);
	const inst_t* FunRawNe(const inst_t* pc);
	const inst_t* FunIn(const inst_t* pc);
	const inst_t* FunNin(const inst_t* pc);
	const inst_t* FunIs(const inst_t* pc);
	const inst_t* FunNis(const inst_t* pc);
	const inst_t* FunInc(const inst_t* pc);
	const inst_t* FunDec(const inst_t* pc);
	const inst_t* FunAddAssign(const inst_t* pc);
	const inst_t* FunSubAssign(const inst_t* pc);
	const inst_t* FunCatAssign(const inst_t* pc);
	const inst_t* FunMulAssign(const inst_t* pc);
	const inst_t* FunDivAssign(const inst_t* pc);
	const inst_t* FunModAssign(const inst_t* pc);
	const inst_t* FunAndAssign(const inst_t* pc);
	const inst_t* FunOrAssign(const inst_t* pc);
	const inst_t* FunXorAssign(const inst_t* pc);
	const inst_t* FunShlAssign(const inst_t* pc);
	const inst_t* FunShrAssign(const inst_t* pc);
	const inst_t* FunUshrAssign(const inst_t* pc);
	const inst_t* FunRange(const inst_t* pc);
	const inst_t* FunClassBegin(const inst_t* pc);
	const inst_t* FunClassEnd(const inst_t* pc);
	const inst_t* FunDefineClassMember(const inst_t* pc);
	const inst_t* FunMakeArray(const inst_t* pc);
	const inst_t* FunArrayAppend(const inst_t* pc);
	const inst_t* FunMakeMap(const inst_t* pc);
	const inst_t* FunMapInsert(const inst_t* pc);
	const inst_t* FunMapSetDefault(const inst_t* pc);
	const inst_t* FunMakeFun(const inst_t* pc);
	const inst_t* FunMakeInstanceVariableAccessor(const inst_t* pc);
	const inst_t* FunThrow(const inst_t* pc);
	const inst_t* FunThrowUnsupportedError(const inst_t* pc);
	const inst_t* FunThrowUndefined(const inst_t* pc);
	const inst_t* FunIfDebug(const inst_t* pc);
	const inst_t* FunAssert(const inst_t* pc);
	const inst_t* FunBreakPoint(const inst_t* pc);
	const inst_t* FunMAX(const inst_t* pc);
//}}DECLS}
	
	const inst_t* OpAdd(const inst_t* pc, int_t op);
	const inst_t* OpSub(const inst_t* pc, int_t op);
	const inst_t* OpMul(const inst_t* pc, int_t op);
	const inst_t* OpDiv(const inst_t* pc, int_t op);
	const inst_t* OpMod(const inst_t* pc, int_t op);
	const inst_t* OpAnd(const inst_t* pc, int_t op);
	const inst_t* OpOr(const inst_t* pc, int_t op);
	const inst_t* OpXor(const inst_t* pc, int_t op);
	const inst_t* OpShl(const inst_t* pc, int_t op);
	const inst_t* OpShr(const inst_t* pc, int_t op);
	const inst_t* OpUshr(const inst_t* pc, int_t op);
	const inst_t* OpInc(const inst_t* pc, int_t op);
	const inst_t* OpDec(const inst_t* pc, int_t op);

private:

	VMachine(const VMachine&);
	VMachine& operator=(const VMachine&);

private:

	inst_t end_code_;
	inst_t throw_code_;
	inst_t throw_unsupported_error_code_;
	inst_t throw_undefined_code_;
	inst_t check_unsupported_code_;
	inst_t cleanup_call_code_;

	const inst_t* resume_pc_;
	int_t yield_result_count_;

	Any myself_;

	const IDPtr* id_;

	// 計算用スタック
	Stack<Any> stack_;

	// 関数呼び出しの度に積まれるフレーム
	Stack<FunFrame> fun_frames_;

	// tryの度に積まれるフレーム。
	PODStack<ExceptFrame> except_frames_;
	
	Any except_[3];

	SmartPtr<DebugInfo> debug_info_;

	SmartPtr<Debug> debug_;

	int_t debug_enable_;

protected:

	virtual void visit_members(Visitor& m);

	virtual void before_gc();
	virtual void after_gc();

public:

	void print_info();

};

}
