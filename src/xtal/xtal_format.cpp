#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

namespace{
	MapPtr user_text_map_;

	void uninitialize_format(){
		user_text_map_ = null;
	}
}

void initialize_format(){
	register_uninitializer(&uninitialize_format);
	user_text_map_ = xnew<Map>();

	{
		ClassPtr p = new_cpp_class<Format>("Format");
		p->method("to_s", &Format::to_s);
		p->method("instance_serial_save", &Format::instance_serial_save);
		p->method("instance_serial_load", &Format::instance_serial_load);
		p->def("serial_new", ctor<Format>());
	}

	{
		ClassPtr p = new_cpp_class<Text>("Text");
		p->method("to_s", &Text::to_s);
		p->method("instance_serial_save", &Text::instance_serial_save);
		p->method("instance_serial_load", &Text::instance_serial_load);
		p->def("serial_new", ctor<Text>());
	}

	builtin()->def("Format", get_cpp_class<Format>());
	builtin()->def("Text", get_cpp_class<Text>());
}


void set_text_map(const MapPtr& map){
	user_text_map_ = map;
}

void add_text_map(const MapPtr& map){
	user_text_map_ = user_text_map_->cat(map);
}

MapPtr get_text_map(){
	return user_text_map_;
}

TextPtr text(const StringPtr& text){
	return xnew<Text>(text);
}

FormatPtr format(const StringPtr& text){
	return xnew<Format>(text);
}


char_t FormatSpecifier::change_int_type(){
	switch(type_){
	case 'i': case 'd': case 'x': case 'X':
		return type_;
		break;
	
	default:
		return 'i';
		break;
	}
}

char_t FormatSpecifier::change_float_type(){
	switch(type_){
	case 'e': case 'E': case 'g':
	case 'G': case 'f':
		return type_;
		break;
	
	default:
		return 'g';
		break;
	}
}
	
int_t FormatSpecifier::type(){
	return type_;
}

int_t FormatSpecifier::max_buffer_size(){
	return width_ + precision_;
}

void FormatSpecifier::make_format_specifier(char_t* dest, char_t type){
	*dest++ = '%';

	{ // 書式指定子を埋め込む
		const char_t* src = buf_;
		while(*dest++ = *src++){}
		--dest;
	}

	{ // int_t が64-bitの時に特別な書式指定子を埋め込む
		const char* src = XTAL_INT_FMT;
		while(*dest++ = *src++){}
		--dest;
	}

	*dest++ = type;
	*dest++ = 0;
}

const char_t* FormatSpecifier::parse_format(const char_t* str){
	width_ = 0;
	precision_ = 0;
	pos_ = 0;		
	str = parse_format_inner(str);
	if(*str) type_ = *str++;
	buf_[pos_] = '\0';
	return str;
}

const char_t* FormatSpecifier::parse_format_inner(const char_t* str){

	while(str[0]){
		switch(str[0]){
		case '-': case '+': case '0': case ' ': case '#':
			buf_[pos_++] = *str++;
			if(pos_ == BUF_MAX){
				return str;
			}
			continue;
		}
		break;
	}
	
	width_ = 0;
	while(str[0]){
		if(str[0]>='0' && str[0]<= '9'){
			width_ *= 10;
			width_ += str[0]-'0';
			buf_[pos_++] = *str++; 
			if(pos_ == BUF_MAX){
				return str;
			}
		}else{
			break;
		}
	}

	if(str[0]=='.'){	
		buf_[pos_++] = *str++; 
		if(pos_ == BUF_MAX){
			return str;
		}
		
		precision_ = 0;
		while(str[0]){
			if(str[0]>='0' && str[0]<= '9'){
				precision_ *= 10;
				precision_ += str[0]-'0';
				buf_[pos_++] = *str++; 
				if(pos_ == BUF_MAX){
					return str;
				}
			}else{
				break;
			}
		}
	}

	return str;
}

Format::Format(const StringPtr& str){
	set(str->c_str());
}

void Format::set(const char* str){

	values_ = xnew<Map>();
	original_ = xnew<String>(str);

	const char* begin = str;
	char buf[256];
	int_t bufpos = 0, n = 0;
	while(true){
		if(str[0]=='%'){
			values_->set_at((int_t)values_->size(), xnew<String>(begin, str));
			str++;
			
			if(str[0]=='%'){
				begin = str++;
			}else{
				if(str[0]=='('){
					str++;
					bufpos = 0;
					while(str[0]!=0 && str[0]!=')' && bufpos!=255){
						buf[bufpos++] = str[0];
						str++;					
					}

					if(str[0]==')'){
						str++;
					}

					buf[bufpos++] = 0;
				}else{
					bufpos = XTAL_SPRINTF(buf, 256-bufpos, "%d", (u32)n++);
				}

				SmartPtr<FormatSpecifier> ret = xnew<FormatSpecifier>();
				str = ret->parse_format(str);
				values_->set_at(buf, ret);
				begin = str;
			}
		}else if(str[0]=='\0'){
			values_->set_at((int_t)values_->size(), xnew<String>(begin, str));
			break;
		}else{
			str++;
		}
	}
}

void Format::call(const VMachinePtr& vm){
	string_t buf;
	char_t cbuf[256];
	char_t spec[FormatSpecifier::FORMAT_SPECIFIER_MAX];
	char_t* pcbuf;
	
	Xfor2(k, v, values_){
		if(type(k)==TYPE_INT){
			buf += v->to_s()->c_str();
		}else{
			AnyPtr a = vm->arg(k->to_s()->intern());
			if(!a){
				a = vm->arg(k->to_i());
			}
			
			SmartPtr<FormatSpecifier> fs = xnew<FormatSpecifier>(*ptr_cast<FormatSpecifier>(v));
			size_t malloc_size = 0;
			if(fs->max_buffer_size()>=256){
				malloc_size = fs->max_buffer_size() + 1;
				pcbuf = (char_t*)user_malloc(malloc_size);
			}else{
				pcbuf = cbuf;
			}
			
			switch(type(a)){
				XTAL_DEFAULT{
					StringPtr str = a->to_s();
					
					if(str->buffer_size()>=256){
						if(str->buffer_size()>malloc_size){
							if(malloc_size!=0){
								user_free(pcbuf);
							}
							
							malloc_size = str->buffer_size() + fs->max_buffer_size() + 1;
							pcbuf = (char*)user_malloc(malloc_size);
						}
					}
					fs->make_format_specifier(spec, 's');
					XTAL_SPRINTF(pcbuf, malloc_size ? malloc_size : 255, spec, str->c_str());
					buf += pcbuf;
				}

				XTAL_CASE(TYPE_NULL){
					buf += "<null>";
				}

				XTAL_CASE(TYPE_INT){
					fs->make_format_specifier(spec, fs->change_int_type());
					XTAL_SPRINTF(pcbuf, malloc_size ? malloc_size : 255, spec, ivalue(a));
					buf += pcbuf;
				}
				
				XTAL_CASE(TYPE_FLOAT){
					fs->make_format_specifier(spec, fs->change_float_type());
					XTAL_SPRINTF(pcbuf, malloc_size ? malloc_size : 255, spec, fvalue(a));
					buf += pcbuf;
				}
			}
			
			if(malloc_size!=0){
				user_free(pcbuf);
			}
		}
	}

	vm->return_result(xnew<String>(buf));
}

void Format::to_s(const VMachinePtr& vm){
	call(vm);
}

AnyPtr Format::instance_serial_save(){
	return original_;
}

void Format::instance_serial_load(const StringPtr& v){
	set(v->c_str());
}

Text::Text(const StringPtr& key)
	:key_(key){}

void Text::call(const VMachinePtr& vm){
	if(user_text_map_){
		if(const AnyPtr& value=user_text_map_->at(key_)){
			xnew<Format>(value->to_s())->call(vm);
			return;
		}
	}
}

void Text::to_s(const VMachinePtr& vm){
	call(vm);
}

AnyPtr Text::instance_serial_save(){
	return key_;
}

void Text::instance_serial_load(const StringPtr& v){
	key_ = v;
}


/*
return [
	"Xtal Compile Error 1001":"構文エラーです。",
	"Xtal Compile Error 1002":"予期せぬ文字 '%(char)s' が検出されました。",
	"Xtal Compile Error 1003":"';' がありません。",

	"Xtal Compile Error 1006":"不正なbreak文、またはcontinue文です。",

	"Xtal Compile Error 1008":"不正な多重代入文です。",
	"Xtal Compile Error 1009":"定義されていない変数 '%(name)s' に代入しようとしました 。",
	"Xtal Compile Error 1010":"不正な数字リテラルのサフィックスです。",
	"Xtal Compile Error 1011":"文字列リテラルの途中でファイルが終わりました。",
	"Xtal Compile Error 1012":"不正な代入文の左辺です。",
	"Xtal Compile Error 1013":"比較演算式の結果を演算しようとしています。",
	"Xtal Compile Error 1014":"不正な浮動小数点数リテラルです。",
	"Xtal Compile Error 1015":"不正な16進数値リテラルのサフィックスです。",
	"Xtal Compile Error 1016":"assert文の引数の数が不正です。",
	"Xtal Compile Error 1017":"不正な%%記法リテラルです。",
	"Xtal Compile Error 1018":"default節が重複定義されました。",
	
	"Xtal Compile Error 1019":"'%(name)s'は代入不可能です。", ////

	"Xtal Compile Error 1020":"不正な2進数値リテラルのサフィックスです。",
	"Xtal Compile Error 1021":"コメントの途中でファイルが終わりました。",
	"Xtal Compile Error 1022":"関数から返せる多値の最大は255個です。",
	"Xtal Compile Error 1023":"定義されていないインスタンス変数名 '%(name)s' を参照しています。",
	"Xtal Compile Error 1024":"同名のインスタンス変数名 '%(name)s' が既に定義されています。",
	"Xtal Compile Error 1025":"比較演算式の結果を最比較しようとしています。",
	"Xtal Compile Error 1026":"同じスコープ内で、同じ変数名 '%(name)s' が既に定義されています。",
	"Xtal Compile Error 1027":"プログラムが大きすぎて、バイトコードの生成に失敗しました。",
	
	"Xtal Runtime Error 1001":"'%(n)d'番目の引数%(param_name)sの型エラーです。 '%(required)s'型を要求していますが、'%(type)s'型の値が渡されました。",
	"Xtal Runtime Error 1002":"evalに渡されたソースのコンパイル中、コンパイルエラーが発生しました。",
	"Xtal Runtime Error 1003":"不正なインスタンス変数の参照です。",
	"Xtal Runtime Error 1004":"型エラーです。 '%(required)s'型を要求していますが、'%(type)s'型の値が渡されました。",
	"Xtal Runtime Error 1005":"'%(name)s'関数呼び出しの引数の数が不正です。%(min)s以上の引数を受け取る関数に、%(value)s個の引数を渡しました。",
	"Xtal Runtime Error 1006":"'%(name)s'関数呼び出しの引数の数が不正です。%(min)s以上、%(max)s以下の引数を受け取る関数に、%(value)s個の引数を渡しました。",
	"Xtal Runtime Error 1007":"'%(name)s'関数呼び出しの引数の数が不正です。引数を取らない関数に、%(value)s個の引数を渡しました。",
	"Xtal Runtime Error 1008":"'%(name)s'はシリアライズできません。",
	"Xtal Runtime Error 1009":"不正なコンパイル済みXtalファイルです。",
	"Xtal Runtime Error 1010":"コンパイルエラーが発生しました。",
	"Xtal Runtime Error 1011":"%(object)s :: '%(name)s' は既に定義されています。",
	"Xtal Runtime Error 1012":"yieldがfiberの非実行中に実行されました。",
	"Xtal Runtime Error 1013":"%(object)s :: new 関数が登録されていないため、インスタンスを生成できません。",
	"Xtal Runtime Error 1014":"ファイル '%(name)s' を開けません。",
	"Xtal Runtime Error 1015":"%(object)s :: '%(name)s' は定義されていません。",
	"Xtal Runtime Error 1016":"ファイル '%(name)s' のコンパイル中、コンパイルエラーが発生しました。",
	"Xtal Runtime Error 1017":"%(object)s :: '%(name)s' は %(accessibility)s です。",
	"Xtal Runtime Error 1018":"既に閉じられたストリームです。",
	"Xtal Runtime Error 1019":"C++で定義されたクラスの多重継承は出来ません。",
	"Xtal Runtime Error 1020":"配列の範囲外アクセスです。",
];
*/

}
