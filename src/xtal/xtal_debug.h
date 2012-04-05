/** \file src/xtal/xtal_debug.h
* \brief src/xtal/xtal_debug.h
*/

#ifndef XTAL_DEBUG_H_INCLUDE_GUARD
#define XTAL_DEBUG_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/**
* \xbind lib::builtin
* \brief デバッグ
*/
namespace debug{

class CallerInfo : public Base{
public:

	/**
	* \xbind
	* \brief 呼び出し場所の行数を返す
	*/
	int_t lineno(){ return line_; }

	const MethodPtr& fun(){ return fun_; }

	/**
	* \xbind
	* \brief 呼び出し場所のファイル名を返す
	*/
	StringPtr file_name();

	/**
	* \xbind
	* \brief 呼び出し場所の関数名を返す
	*/
	StringPtr fun_name();

	/**
	* \xbind
	* \brief 呼び出し場所の変数フレームオブジェクトを返す
	*/
	const FramePtr& variables_frame(){ return variables_frame_; }

	void set_line(int_t v){ line_ = v; }
	void set_fun(const MethodPtr& v){ fun_ = v; }
	void set_variables_frame(const FramePtr& v){ variables_frame_ = v; }

	void on_visit_members(Visitor& m);

private:
	int_t line_;
	MethodPtr fun_;
	FramePtr variables_frame_;
};

typedef SmartPtr<CallerInfo> CallerInfoPtr;

/**
* \xbind lib::builtin::debug
* \xinherit lib::builtin::Any
* \brief デバッグフック関数で渡されるオブジェクト
*/
class HookInfo : public Base{
public:
	
	/**
	* \xbind
	* \brief フックの種類を返す
	*/
	int_t kind(){ return kind_; } 

	/**
	* \xbind
	* \brief フックされた場所の行数を返す
	*/
	int_t lineno(){ return line_; }

	/**
	* \xbind
	* \brief フックされた場所のCodeオブジェクトを返す
	*/
	const CodePtr& code(){ return code_; }

	/**
	* \xbind
	* \brief フックされた場所のファイル名を返す
	*/
	const StringPtr& file_name(){ return file_name_; }

	/**
	* \xbind
	* \brief フックされた場所の関数名を返す
	*/
	const StringPtr& fun_name(){ return fun_name_; }

	/**
	* \xbind
	* \brief 例外オブジェクトを返す
	*/
	const AnyPtr& exception(){ return exception_; }

	/**
	* \xbind
	* \brief フックされた場所の変数フレームオブジェクトを返す
	*/
	const FramePtr& variables_frame(){ return variables_frame_; }

	/**
	* \brief 呼び出し元の情報を返す
	*/
	CallerInfoPtr caller(uint_t n);

	int_t call_stack_size();

	void set_kind(int_t v){ kind_ = v; }
	void set_line(int_t v){ line_ = v; }
	void set_code(const CodePtr& v){ code_ = v; }
	void set_file_name(const StringPtr& v){ file_name_ = v; }
	void set_fun_name(const StringPtr& v){ fun_name_ = v; }
	void set_exception(const AnyPtr& e){ exception_ = e; }
	void set_variables_frame(const FramePtr& v){ variables_frame_ = v; }

	/**
	* \xbind
	* \brief クローンを作る
	* HookInfoオブジェクトは使いまわされるため、情報を保持したい場合、クローンを作る必要がある。
	*/
	SmartPtr<HookInfo> clone();

	void set_vm(const VMachinePtr& v);

	const VMachinePtr& vm();

	void on_visit_members(Visitor& m);

private:

	int_t kind_;
	int_t line_;
	CodePtr code_;
	StringPtr file_name_;
	StringPtr fun_name_;
	AnyPtr exception_;
	FramePtr variables_frame_;
	VMachine* vm_;

	friend class VMachine;
};

typedef SmartPtr<HookInfo> HookInfoPtr;

class Debug{};

/**
* \xbind lib::builtin::debug
* \brief デバッグ機能を有効にする
* デバッグ機能はデフォルトでは無効になっている。
*/
void enable();

/**
* \xbind lib::builtin::debug
* \brief デバッグ機能を無効にする
*/
void disable();

/**
* \xbind lib::builtin::debug
* \brief デバッグ機能を強制的に有効にする
* デバッグ機能はデフォルトでは無効になっている。
*/
void enable_force(uint_t count);

/**
* \xbind lib::builtin::debug
* \brief デバッグ機能を強制的に無効にする
*/
uint_t disable_force();

/**
* \xbind lib::builtin::debug
* \brief デバッグ機能が有効かどうか
*/
bool is_enabled();

/**
* \xbind lib::builtin::debug
* \brief デバッグコンパイルが有効かどうか
*/
bool is_debug_compile_enabled();

/**
* \xbind lib::builtin::debug
* \brief デバッグコンパイルを有効にする
*/
void enable_debug_compile();

/**
* \xbind lib::builtin::debug
* \brief デバッグコンパイルを無効にする
*/
void disable_debug_compile();

uint_t hook_setting_bit();

uint_t* hook_setting_bit_ptr();

void set_hook(int_t hooktype, const AnyPtr& hook);

const AnyPtr& hook(int_t hooktype);

/**
* \xbind lib::builtin::debug
* \brief ブレークポイントに達するたびに呼び出されるフック関数を登録する
* \param hook 登録するフック関数
*/
void set_breakpoint_hook(const AnyPtr& hook);

/**
* \xbind lib::builtin::debug
* \brief 関数呼び出しされる度に呼び出されるフック関数を登録する
* \param hook 登録するフック関数
*/
void set_call_hook(const AnyPtr& hook);

/**
* \xbind lib::builtin::debug
* \brief 関数からreturnされる度に呼び出されるフック関数を登録する
* \param hook 登録するフック関数
*/
void set_return_hook(const AnyPtr& hook);

/**
* \xbind lib::builtin::debug
* \brief 例外かthrowされる度に呼び出されるフック関数を登録する
* \param hook 登録するフック関数
*/
void set_throw_hook(const AnyPtr& hook);

/**
* \xbind lib::builtin::debug
* \brief アサートが失敗される度に呼び出されるフック関数を登録する
* \param hook 登録するフック関数
*/
void set_assert_hook(const AnyPtr& hook);

/**
* \xbind lib::builtin::debug
* \brief set_breakpoint_hook関数で登録した関数を取得する
*/
const AnyPtr& breakpoint_hook();

/**
* \xbind lib::builtin::debug
* \brief set_call_hook関数で登録した関数を取得する
*/
const AnyPtr& call_hook();

/**
* \xbind lib::builtin::debug
* \brief set_return_hook関数で登録した関数を取得する
*/
const AnyPtr& return_hook();

/**
* \xbind lib::builtin::debug
* \brief set_throw_hook関数で登録した関数を取得する
*/
const AnyPtr& throw_hook();

/**
* \xbind lib::builtin::debug
* \brief set_assert_hook関数で登録した関数を取得する
*/
const AnyPtr& assert_hook();

/**
* \xbind lib::builtin::debug
* \brief 再定義が有効かどうか
*/
bool is_redefine_enabled();

/**
* \xbind lib::builtin::debug
* \brief 再定義を有効にする
*/
void enable_redefine();

/**
* \xbind lib::builtin::debug
* \brief 再定義を無効にする
*/
void disable_redefine();

//////////////////////////////

enum{
	RUN,
	STEP_OVER,
	STEP_INTO,
	STEP_OUT,
	REDO
};

void call_breakpoint_hook(int_t kind, const HookInfoPtr& info);


#if 0

/**
* \biref デバッガとやりとりするためのストリームクラス
*/
class DebugStream : public Stream{
public:

	DebugStream(){
		socket_ = -1;
	}

	DebugStream(int s){
		socket_ = s;
	}

	DebugStream(const StringPtr& path, const StringPtr& port){
		socket_ = -1;
		open(path, port);
	}

	virtual ~DebugStream(){
		close();
	}

	void open(const StringPtr& host, const StringPtr& port){
		close();
		socket_ = socket(AF_INET, SOCK_STREAM, 0);

		if(is_open()){
			struct addrinfo hints = {0};
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = 0;

			struct addrinfo* res = 0;
			int err = getaddrinfo(host->c_str(), port->c_str(), &hints, &res);
			if(err!=0){
				return;
			}

			for(struct addrinfo* ai= res; ai; ai=ai->ai_next){
				socket_ = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
				if(socket_<0){
					break;
				}

				if(connect(socket_, ai->ai_addr, ai->ai_addrlen)<0){
					close();
					continue;
				}
				else{
					break;
				}
			}

			freeaddrinfo(res);
		}
	}

	bool is_open(){
		return socket_>=0;
	}

	virtual void close(){
		if(socket_>=0){
			::close(socket_);
			socket_ = -1;
		}
	}

	virtual uint_t read(void* dest, uint_t size){
		int read = recv(socket_, (char*)dest, size, 0);
		if(read<0){
			close();
			return 0;
		}
		return read;
	}

	virtual uint_t write(const void* src, uint_t size){
		int temp = ::send(socket_, (char*)src, size, 0);
		if(temp<0){
			close();
			return 0;
		}
		return temp;
	}

private:
	void* debug_stream_;
};

#endif

////////////////////////////////////////////////////////////////////////////////////

/**
* \brief デバッガの受信側
*/
class CommandReciver : public Base{
public:

	/**
	* \brief デバッガをスタートする
	* \param stream デバッガと通信するためのストリーム
	*/
	bool start(const StreamPtr& stream);

	/**
	* デバッガを更新する
	* 
	*/
	void update();

private:

	ArrayPtr recv_command();

	CodePtr require_source_hook(const StringPtr& name);

	void exec_command(const ArrayPtr& cmd);

	ArrayPtr make_debug_object(const AnyPtr& v, int depth = 3);

	ArrayPtr make_call_stack_info(const debug::HookInfoPtr& info);

	MapPtr make_eval_expr_info(const debug::HookInfoPtr& info, int level);

	void send_break(debug::HookInfoPtr info, int level);

	int linehook(debug::HookInfoPtr info);

private:
	StreamPtr stream_;
	MapPtr eval_exprs_;
	MapPtr code_map_;
};

////////////////////////////////////////////////////////////////////////////////////

/**
* \brief デバッガの送信側
*/
class CommandSender : public Base{
public:

	CommandSender();

	/**
	* \brief デバッガをスタートする
	* \param stream デバッガと通信するためのストリーム
	*/
	void start(const StreamPtr& stream);

	/**
	* デバッガを更新する
	* 
	*/
	void update();

public:
	// 評価式に関する設定と取得

	void add_eval_expr(const StringPtr& expr);

	void remove_eval_expr(const StringPtr& expr);

	ArrayPtr eval_expr_result(const StringPtr& expr);

public:
	// コールスタックに関する設定と取得

	int call_stack_size();

	StringPtr call_stack_fun_name(int n);
	StringPtr call_stack_fun_name();

	StringPtr call_stack_file_name(int n);
	StringPtr call_stack_file_name();

	int call_stack_lineno(int n);
	int call_stack_lineno();

	void move_call_stack(int n);

public:
	// 要求されたスクリプトファイルに関する設定と取得

	StringPtr required_file();

	void required_source(const CodePtr& code);

	int level(){ return level_;	}

public:
	// ブレークポイントに関する設定と取得

	void add_breakpoint(const StringPtr& path, int n, const StringPtr& cond);
	void remove_breakpoint(const StringPtr& path, int n);

public:
	void nostep();
	void start();

	void run();
	void step_over();
	void step_into();
	void step_out();
	void redo();

protected:

	// ブレークした際のシグナル
	virtual void on_breaked(){}

	// ブレークした際のシグナル
	virtual void on_required(){}

	// コンパイルエラーが起こった際のシグナル
	virtual void on_compile_error(){}

	// キャッチされない例外が起こった際のシグナル
	virtual void on_uncatched_exception(){}

protected:
	void send_command(const IDPtr& id);

private:
	StreamPtr stream_;

	struct CallInfo{
		StringPtr fun_name;
		StringPtr file_name;
		int lineno;
	};

	TArray<CallInfo> call_stack_;

	struct ExprValue{
		ExprValue(){
			count = 0;
		}

		int count;
		CodePtr code;
		ArrayPtr result;
	};

	MapPtr exprs_;
	StringPtr required_file_;
	int level_;

	IDPtr prev_command_;
};

}

}

#endif // XTAL_DEBUG_H_INCLUDE_GUARD
