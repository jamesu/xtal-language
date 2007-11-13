#include "xtal.h"
#include "xtal_macro.h"

#ifndef XTAL_NO_PARSER

namespace xtal{

#define c2(C1, C2) ((C2)<<8 | (C1))
#define c3(C1, C2, C3) ((C3)<<16 | (C2)<<8 | (C1))
#define c4(C1, C2, C3, C4) ((C4)<<24 | (C3)<<16 | (C2)<<8 | (C1))


void CompileError::init(const StringPtr& file_name){
	errors = xnew<Array>();
	source_file_name = file_name;
}

void CompileError::error(int_t lineno, const AnyPtr& message){
	if(errors->size()<10){
		errors->push_back(Xf("%(file)s:%(lineno)d:%(message)s")(
			Named("file", source_file_name),
			Named("lineno", lineno),
			Named("message", message)
		));
	}
}

Reader::Reader()
	:stream_(null){
	pos_ = 0;
	read_ = 0;
	recording_ = false;
}

int_t Reader::read(){
	int_t ret = peek();

	if(recording_ && ret!=-1){
		recorded_string_ += (char_t)ret;
	}

	++pos_;
	return ret;
}

int_t Reader::peek(int_t n){
	uint_t rpos = read_&BUF_MASK;
	uint_t ppos = pos_&BUF_MASK;

	while(pos_+n>=read_){
		uint_t now_read = 0;

		if(ppos>rpos){
			now_read = stream_->read(&buf_[rpos], ppos-rpos);
		}
		else{
			now_read = stream_->read(&buf_[rpos], BUF_SIZE-rpos);
		}

		if(now_read==0){
			return -1;
		}

		read_ += now_read;
		rpos = read_&BUF_MASK;
	}
	return buf_[(pos_+n) & BUF_MASK];
}

bool Reader::eat(int_t ch){
	if(peek()==ch){
		read();
		return true;
	}
	return false;
}

void Reader::begin_record(){
	recording_ = true;
	recorded_string_ = "";
}

StringPtr Reader::end_record(){
	recording_ = false;
	if(!recorded_string_.empty()){
		recorded_string_.resize(recorded_string_.size()-1);
	}
	return xnew<String>(recorded_string_.c_str(), recorded_string_.size());
}

Lexer::Lexer(){
	set_lineno(1);
	read_ = 0;
	pos_ = 0;

	keyword_map_ = xnew<Map>();
	keyword_map_->set_at(Xid(if), (int_t)Token::KEYWORD_IF);
	keyword_map_->set_at(Xid(for), (int_t)Token::KEYWORD_FOR);
	keyword_map_->set_at(Xid(else), (int_t)Token::KEYWORD_ELSE);
	keyword_map_->set_at(Xid(fun), (int_t)Token::KEYWORD_FUN);
	keyword_map_->set_at(Xid(method), (int_t)Token::KEYWORD_METHOD);
	keyword_map_->set_at(Xid(do), (int_t)Token::KEYWORD_DO);
	keyword_map_->set_at(Xid(while), (int_t)Token::KEYWORD_WHILE);
	keyword_map_->set_at(Xid(continue), (int_t)Token::KEYWORD_CONTINUE);
	keyword_map_->set_at(Xid(break), (int_t)Token::KEYWORD_BREAK);
	keyword_map_->set_at(Xid(fiber), (int_t)Token::KEYWORD_FIBER);
	keyword_map_->set_at(Xid(yield), (int_t)Token::KEYWORD_YIELD);
	keyword_map_->set_at(Xid(return), (int_t)Token::KEYWORD_RETURN);
	keyword_map_->set_at(Xid(once), (int_t)Token::KEYWORD_ONCE);
	keyword_map_->set_at(Xid(null), (int_t)Token::KEYWORD_NULL);
	keyword_map_->set_at(Xid(undefined), (int_t)Token::KEYWORD_UNDEFINED);
	keyword_map_->set_at(Xid(false), (int_t)Token::KEYWORD_FALSE);
	keyword_map_->set_at(Xid(true), (int_t)Token::KEYWORD_TRUE);
	keyword_map_->set_at(Xid(xtal), (int_t)Token::KEYWORD_XTAL);
	keyword_map_->set_at(Xid(try), (int_t)Token::KEYWORD_TRY);
	keyword_map_->set_at(Xid(catch), (int_t)Token::KEYWORD_CATCH);
	keyword_map_->set_at(Xid(finally), (int_t)Token::KEYWORD_FINALLY);
	keyword_map_->set_at(Xid(throw), (int_t)Token::KEYWORD_THROW);
	keyword_map_->set_at(Xid(class), (int_t)Token::KEYWORD_CLASS);
	keyword_map_->set_at(Xid(callee), (int_t)Token::KEYWORD_CALLEE);
	keyword_map_->set_at(Xid(this), (int_t)Token::KEYWORD_THIS);
	keyword_map_->set_at(Xid(current_context), (int_t)Token::KEYWORD_CURRENT_CONTEXT);
	keyword_map_->set_at(Xid(dofun), (int_t)Token::KEYWORD_DOFUN);
	keyword_map_->set_at(Xid(is), (int_t)Token::KEYWORD_IS);
	keyword_map_->set_at(Xid(in), (int_t)Token::KEYWORD_IN);
	keyword_map_->set_at(Xid(unittest), (int_t)Token::KEYWORD_UNITTEST);
	keyword_map_->set_at(Xid(assert), (int_t)Token::KEYWORD_ASSERT);
	keyword_map_->set_at(Xid(pure), (int_t)Token::KEYWORD_PURE);
	keyword_map_->set_at(Xid(nobreak), (int_t)Token::KEYWORD_NOBREAK);
	keyword_map_->set_at(Xid(switch), (int_t)Token::KEYWORD_SWITCH);
	keyword_map_->set_at(Xid(case), (int_t)Token::KEYWORD_CASE);
	keyword_map_->set_at(Xid(default), (int_t)Token::KEYWORD_DEFAULT);
	keyword_map_->set_at(Xid(singleton), (int_t)Token::KEYWORD_SINGLETON);
}	


void Lexer::init(const StreamPtr& stream, CompileError* error){
	reader_.set_stream(stream);
	error_ = error;
}

const Token& Lexer::read(){
	const Token& ret = peek();
	++pos_;
	return ret;
}

const Token& Lexer::peek(){
	if(pos_==read_){
		do_read();
		if(pos_==read_){
			static Token end(Token::TYPE_TOKEN, (int_t)-1, (int_t)0);
			return end;
		}
	}
	return buf_[pos_ & BUF_MASK];
}

void Lexer::push_token(int_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_TOKEN, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}
	
void Lexer::push_int_token(int_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_INT, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}

void Lexer::push_float_token(float_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_FLOAT, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}
	
void Lexer::push_keyword_token(const IDPtr& v, int_t num){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_KEYWORD, v, left_space_ | test_right_space(reader_.peek()), num);
	read_++;
}
	
void Lexer::push_identifier_token(const IDPtr& v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_IDENTIFIER, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}

void Lexer::putback(){
	pos_--;
}

void Lexer::putback(const Token& ch){
	pos_--;
	buf_[pos_ & BUF_MASK] = ch;
}

IDPtr Lexer::parse_identifier(){
	string_t buf;

	ChMaker chm;
	while(!chm.is_completed()){
		chm.add(reader_.read());
	}
	buf.append(chm.buf, chm.buf+chm.pos);

	while(test_ident_rest(reader_.peek())){
		chm.clear();
		while(!chm.is_completed()){
			chm.add(reader_.read());
		}
		buf.append(chm.buf, chm.buf+chm.pos);
	}
	
	return xnew<ID>(buf);
}

int_t Lexer::parse_integer(){
	int_t ret = 0;
	while(true){
		if(test_digit(reader_.peek())){
			ret *= 10;
			ret += reader_.read()-'0';
		}
		else if(reader_.peek()=='_'){
			reader_.read();
		}
		else{
			break;
		}
	}
	return ret;
}

int_t Lexer::parse_hex(){
	int_t ret = 0;
	while(true){
		if(test_digit(reader_.peek())){
			ret *= 16;
			ret += reader_.read()-'0';
		}
		else if(test_range(reader_.peek(), 'a', 'f')){
			ret *= 16;
			ret += reader_.read()-'a';
		}
		else if(test_range(reader_.peek(), 'A', 'F')){
			ret *= 16;
			ret += reader_.read()-'A';
		}
		else if(reader_.peek()=='_'){
			reader_.read();
		}
		else{
			break;
		}
	}

	if(test_ident_rest(reader_.peek())){
		error_->error(lineno(), Xt("Xtal Compile Error 1015")(Named("n", 16)));
	}

	return ret;		
}

int_t Lexer::parse_oct(){
	int_t ret = 0;
	while(true){
		if(test_range(reader_.peek(), '0', '7')){
			ret *= 8;
			ret += reader_.read()-'0';
		}
		else if(reader_.peek()=='_'){
			reader_.read();
		}
		else{
			break;
		}
	}

	if(test_ident_rest(reader_.peek()) || ('8'<=reader_.peek() && reader_.peek()<='9')){
		error_->error(lineno(), Xt("Xtal Compile Error 1015")(Named("n", 8)));
	}

	return ret;		
}

int_t Lexer::parse_bin(){
	int ret = 0;
	while(true){
		if(test_range(reader_.peek(), '0', '1')){
			ret *= 2;
			ret += reader_.read()-'0';
		}
		else if(reader_.peek()=='_'){
			reader_.read();
		}
		else{
			break;
		}
	}

	if(test_ident_rest(reader_.peek()) || ('2'<=reader_.peek() && reader_.peek()<='9')){
		error_->error(lineno(), Xt("Xtal Compile Error 1015")(Named("n", 2)));
	}

	return ret;
}

void Lexer::parse_number_suffix(int_t val){
	if(reader_.eat('f') || reader_.eat('F')){
		push_float_token((float_t)val);	
	}
	else{

		if(test_ident_rest(reader_.peek())){
			error_->error(lineno(), Xt("Xtal Compile Error 1010"));
		}

		push_int_token(val);
	}
}

void Lexer::parse_number_suffix(float_t val){
	if(reader_.eat('f') || reader_.eat('F')){
		push_float_token(val);	
	}
	else{
	
		if(test_ident_rest(reader_.peek())){
			error_->error(lineno(), Xt("Xtal Compile Error 1010"));
		}

		push_float_token(val);
	}
}

void Lexer::parse_number(){
	if(reader_.eat('0')){
		if(reader_.eat('x') || reader_.eat('X')){
			push_int_token(parse_hex());
			return;
		}
		else if(reader_.eat('o')){
			push_int_token(parse_oct());
			return;
		}
		else if(reader_.eat('b') || reader_.eat('B')){
			push_int_token(parse_bin());
			return;
		}
	}
	
	int_t ival = parse_integer();
	
	if(reader_.peek()=='.'){
		if(test_digit(reader_.peek(1))){
			reader_.read();

			float_t scale = 1;
			while(reader_.eat('0')){
				scale /= 10;
			}

			float_t fval = 0;
			int_t after_the_decimal_point = parse_integer();
			while(after_the_decimal_point!=0){
				fval+=after_the_decimal_point%10;
				fval/=10;
				after_the_decimal_point/=10;
			}

			fval *= scale;
			fval += ival;
			int_t e = 1;
			if(reader_.eat('e') || reader_.eat('E')){
				if(reader_.eat('-')){
					e=-1;
				}
				else{
					reader_.eat('+');
				}

				if(!test_digit(reader_.peek())){
					error_->error(lineno(), Xt("Xtal Compile Error 1014"));
				}

				e *= parse_integer();

				{
					using namespace std;
					fval *= (float_t)pow((float_t)10, (float_t)e);
				}
			}
			parse_number_suffix(fval);
		}
		else{
			push_int_token(ival);
		}
	}
	else{
		parse_number_suffix(ival);
	}
}
	
void Lexer::do_read(){
	left_space_ = 0;
	
	do{

		int_t ch = reader_.peek();
		
		switch(ch){

			XTAL_DEFAULT{
				if(ch!=-1 && test_ident_first(ch)){
					IDPtr identifier = parse_identifier();
					if(const AnyPtr& key = keyword_map_->at(identifier)){
						push_keyword_token(identifier, ivalue(key));
					}
					else{
						push_identifier_token(identifier);
					}
				}
				else if(test_digit(ch)){
					parse_number();
					return;
				}
				else{
					reader_.read();
					push_token(ch);
				}
			}
			
			XTAL_CASE('+'){ 
				reader_.read();
				if(reader_.eat('+')){
					push_token(c2('+', '+'));
				}
				else if(reader_.eat('=')){
					push_token(c2('+', '='));
				}
				else{
					push_token('+');
				}
			}
			
			XTAL_CASE('-'){
				reader_.read();
				if(reader_.eat('-')){
					push_token(c2('-', '-'));
				}
				else if(reader_.eat('=')){
					push_token(c2('-', '='));
				}
				else{
					push_token('-');
				}
			}
			
			XTAL_CASE('~'){
				reader_.read();
				if(reader_.eat('=')){
					push_token(c2('~', '='));
				}
				else{
					push_token('~');
				}
			}
			
			XTAL_CASE('*'){
				reader_.read();
				if(reader_.eat('=')){
					push_token(c2('*', '='));
				}
				else{
					push_token('*');
				}
			}
			
			XTAL_CASE('/'){
				reader_.read();
				if(reader_.eat('=')){
					push_token(c2('/', '='));
				}
				else if(reader_.eat('/')){
					while(true){
						int_t ch = reader_.read();
						if(ch=='\r'){
							reader_.eat('\n');
							set_lineno(lineno()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}
						else if(ch=='\n'){
							set_lineno(lineno()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}
						else if(ch==-1){
							break;
						}
					}
					continue;
				}
				else if(reader_.eat('*')){
					while(true){
						int_t ch = reader_.read();
						if(ch=='\r'){
							reader_.eat('\n');
							set_lineno(lineno()+1);
						}
						else if(ch=='\n'){
							set_lineno(lineno()+1);
						}
						else if(ch=='*'){
							if(reader_.eat('/')){
								left_space_ = Token::FLAG_LEFT_SPACE;
								break;
							}
						}
						else if(ch==-1){
							error_->error(lineno(), Xt("Xtal Compile Error 1021"));
							break;
						}
					}
					continue;
				}
				else{
					push_token('/');
				}
			}			
			
			XTAL_CASE('#'){
				reader_.read();
				if(reader_.eat('!')){
					while(true){
						int_t ch = reader_.read();
						if(ch=='\r'){
							reader_.eat('\n');
							set_lineno(lineno()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}
						else if(ch=='\n'){
							set_lineno(lineno()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}
						else if(ch==-1){
							break;
						}
					}
					continue;
				}
				else{
					push_token('#');
				}
			}			
			
			XTAL_CASE('%'){
				reader_.read();
				if(reader_.eat('=')){
					push_token(c2('%', '='));
				}
				else{
					push_token('%');
				}
			}
			
			XTAL_CASE('&'){
				reader_.read();
				if(reader_.eat('&')){
					push_token(c2('&', '&'));
				}
				else{
					push_token('&');
				}
			}
			
			XTAL_CASE('|'){
				reader_.read();
				if(reader_.eat('|')){
					push_token(c2('|', '|'));
				}
				else{
					push_token('|');
				}
			}
						
			XTAL_CASE('>'){
				reader_.read();
				if(reader_.eat('>')){
					if(reader_.eat('>')){
						if(reader_.eat('=')){
							push_token(c4('>','>','>','='));
						}
						else{
							push_token(c3('>','>','>'));
						}
					}
					else{
						push_token(c2('>','>'));
					}
				}
				else if(reader_.eat('=')){
					push_token(c2('>', '='));
				}
				else{
					push_token('>');
				}
			}
			
			XTAL_CASE('<'){
				reader_.read();
				if(reader_.eat('<')){
					if(reader_.eat('=')){
						push_token(c3('<','<','='));
					}
					else{
						push_token(c2('<','<'));
					}
				}
				else if(reader_.eat('=')){
					push_token(c2('<', '='));
				}
				else if(reader_.eat('.')){
					if(!reader_.eat('.')){
						error_->error(lineno(), Xt("Xtal Compile Error 1001"));					
					}

					if(reader_.eat('<')){
						push_token(c4('<', '.', '.', '<'));
					}
					else{
						push_token(c3('<', '.', '.'));
					}
				}
				else{
					push_token('<');
				}
			}
			
			XTAL_CASE('='){
				reader_.read();
				if(reader_.eat('=')){
					if(reader_.eat('=')){
						push_token(c3('=', '=', '='));
					}
					else{
						push_token(c2('=', '='));
					}
				}
				else{
					push_token('=');
				}
			}
			
			XTAL_CASE('!'){
				reader_.read();
				if(reader_.eat('=')){
					if(reader_.eat('=')){
						push_token(c3('!', '=', '='));
					}
					else{
						push_token(c2('!', '='));
					}
				}
				else if(reader_.peek()=='i'){
					if(reader_.peek(1)=='s'){
						if(!test_ident_rest(reader_.peek(2))){
							reader_.read();
							reader_.read();
							push_token(c3('!', 'i', 's'));
						}
						else{
							push_token('!');
						}
					}
					else if(reader_.peek(1)=='n'){
						if(!test_ident_rest(reader_.peek(2))){
							reader_.read();
							reader_.read();
							push_token(c3('!', 'i', 'n'));
						}
						else{
							push_token('!');
						}
					}
					else{
						push_token('!');
					}
				}
				else{
					push_token('!');
				}
			}
			
			XTAL_CASE('.'){
				reader_.read();
				if(reader_.eat('.')){
					if(reader_.eat('.')){
						push_token(c3('.', '.', '.'));
					}
					else if(reader_.eat('<')){
						push_token(c3('.', '.', '<'));
					}
					else{
						push_token(c2('.', '.'));
					}
				}
				else if(reader_.eat('?')){
					push_token(c2('.', '?'));
				}
				else{
					push_token('.');
				}
			}

			XTAL_CASE('"'){ //"
				reader_.read();
				push_token('"'); //"
			}
			
			XTAL_CASE(':'){
				reader_.read();
				if(reader_.eat(':')){
					if(reader_.eat('?')){
						push_token(c3(':', ':', '?'));
					}
					else{
						push_token(c2(':', ':'));
					}
				}
				else{
					push_token(':');
				}
			}

			XTAL_CASE('\''){
				reader_.read();
				push_identifier_token(read_string('\'', '\''));
			}

			XTAL_CASE(';'){ reader_.read(); push_token(';'); }
			XTAL_CASE('{'){ reader_.read(); push_token('{'); }
			XTAL_CASE('}'){ reader_.read(); push_token('}'); }
			XTAL_CASE('['){ reader_.read(); push_token('['); }
			XTAL_CASE(']'){ reader_.read(); push_token(']'); }
			XTAL_CASE('('){ reader_.read(); push_token('('); }
			XTAL_CASE(')'){ reader_.read(); push_token(')'); }

			XTAL_CASE4(' ', '\t', '\r', '\n'){
				deplete_space();
				left_space_ = Token::FLAG_LEFT_SPACE;
				continue;
			}

			XTAL_CASE(-1){
				push_token(-1);
			}
		}
			
		break;
	}while(true);

}

void Lexer::deplete_space(){
	while(true){
		int_t ch = reader_.peek();
		if(ch=='\r'){
			reader_.read();
			reader_.eat('\n');
			set_lineno(lineno()+1);
		}
		else if(ch=='\n'){
			reader_.read();
			set_lineno(lineno()+1);
		}
		else if(ch==' ' || ch=='\t'){
			reader_.read();
		}
		else{
			return;
		}
	}
}

int_t Lexer::test_right_space(int_t ch){
	if(test_space(ch)){
		return Token::FLAG_RIGHT_SPACE;
	}
	return 0;
}

int_t Lexer::read_direct(){
	return reader_.read();
}

StringPtr Lexer::read_string(int_t open, int_t close){
	string_t str;
	int_t depth = 1;
	while(true){
		int_t ch = reader_.read();
		if(ch==close){
			--depth;
			if(depth==0){
				break;
			}
		}
		if(ch==open){
			++depth;
		}
		if(ch==-1){
			error_->error(lineno(), Xt("Xtal Compile Error 1011"));
			break;
		}
		if(ch=='\\'){
			switch(reader_.peek()){
				XTAL_DEFAULT{ 
					str+='\\';
					str+=(char_t)reader_.peek(); 
				}
				
				XTAL_CASE('n'){ str+='\n'; }
				XTAL_CASE('r'){ str+='\r'; }
				XTAL_CASE('t'){ str+='\t'; }
				XTAL_CASE('f'){ str+='\f'; }
				XTAL_CASE('b'){ str+='\b'; }
				XTAL_CASE('\\'){ str+='\\'; }
				XTAL_CASE('"'){ str+='"'; } 
				
				XTAL_CASE('\r'){ 
					if(reader_.peek()=='\n'){
						reader_.read();
					}

					str+='\r';
					str+='\n';
					set_lineno(lineno()+1);
				}
				
				XTAL_CASE('\n'){ 
					str+='\n';
					set_lineno(lineno()+1);
				}
			}
			reader_.read();
		}
		else{
			if(ch=='\r'){
				if(reader_.peek()=='\n'){
					reader_.read();
				}

				str+='\r';
				str+='\n';
				set_lineno(lineno()+1);
			}
			else if(ch=='\n'){
				str+='\n';
				set_lineno(lineno()+1);
			}
			else{
				ChMaker chm;
				chm.add(ch);
				while(!chm.is_completed()){
					chm.add(reader_.read());
				}
				str.append(chm.buf, chm.buf+chm.pos);
			}
		}	
	}

	return xnew<String>(str.c_str(), str.size());
}


enum{//Expressions priority
	
	PRI_BEGIN_ = 0x1000,

	PRI_Q,

	PRI_OROR,
	PRI_ANDAND,

	PRI_OR,
	PRI_XOR,
	PRI_AND,

	PRI_EQ,
		PRI_NE = PRI_EQ,
		PRI_LT = PRI_EQ,
		PRI_GT = PRI_EQ,
		PRI_LE = PRI_EQ,
		PRI_GE = PRI_EQ,
		PRI_RAW_EQ = PRI_EQ,
		PRI_RAW_NE = PRI_EQ,
		PRI_IN = PRI_EQ,
		PRI_NIN = PRI_EQ,
		PRI_IS = PRI_EQ,
		PRI_NIS = PRI_EQ,

	PRI_SHL,
		PRI_SHR = PRI_SHL,
		PRI_USHR = PRI_SHL,

	PRI_ADD, 
		PRI_SUB = PRI_ADD, 
		PRI_CAT = PRI_ADD,
	
	PRI_MUL, 
		PRI_DIV = PRI_MUL, 
		PRI_MOD = PRI_MUL,

	PRI_NEG,
		PRI_POS = PRI_NEG,
		PRI_COM = PRI_NEG,
		PRI_NOT = PRI_NEG,

	PRI_POW,

	PRI_AT,
		PRI_SEND = PRI_AT,
		PRI_MEMBER = PRI_AT,
		PRI_CALL = PRI_AT,
		PRI_NS = PRI_AT,
		PRI_RANGE = PRI_AT,

	PRI_ONCE,
		PRI_STATIC = PRI_ONCE,

	PRI_END_,

	PRI_MAX = PRI_END_-PRI_BEGIN_
};




Parser::Parser(){
	expr_end_flag_ = false;
}

ExprPtr Parser::parse(const StreamPtr& stream, CompileError* error){
	error_ = error;
	lexer_.init(stream, error_);

	ExprPtr p = parse_toplevel();

	if(error_->errors->size()!=0){
		return null;
	}

	return p;
}

ExprPtr Parser::parse_stmt(const StreamPtr& stream, CompileError* error){
	error_ = error;
	lexer_.init(stream, error_);

	ExprPtr p = must_parse_stmt();

	if(!p){
		lexer_.read();
	}

	if(error_->errors->size()!=0){
		return null;
	}

	return p;
}

void Parser::expect(int_t ch){
	if(eat(ch)){
		return;
	}		
	const Token& tok = lexer_.read();
	error_->error(lineno(), Xt("Xtal Compile Error 1002")(Named("char", lexer_.peek().to_s())));
}

bool Parser::eat(int_t ch){
	const Token& n = lexer_.peek();
	if(n.type() == Token::TYPE_TOKEN){
		if(n.ivalue()==ch){
			lexer_.read();
			return true;
		}
	}
	return false;
}

bool Parser::eat(Token::Keyword kw){
	const Token& n = lexer_.peek();
	if(n.type() == Token::TYPE_KEYWORD){
		if(n.keyword_number()==kw){
			lexer_.read();
			return true;
		}
	}
	return false;
}
	
ExprPtr Parser::parse_term(){
	
	const Token& ch = lexer_.read();

	ExprPtr ret = null;
	int_t r_space = ch.right_space() ? PRI_MAX : 0;
	ExprMaker em(lineno());

	switch(ch.type()){
		XTAL_NODEFAULT;

		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){

				XTAL_DEFAULT{}

				XTAL_CASE('('){ 
					ret = parse_expr();
					if(ret){
						if(eat(',')){
							ArrayPtr mv = parse_exprs();
							mv->insert(0, ret);
							ret = em.multi_value(mv);
						}
						else{
							if(ret->tag()==EXPR_SEND){
								ret = em.bracket(ret);
							}	
						}
					}
					else{
						ret = em.undefined_();
					}
					expect(')'); 
					expr_end_flag_ = false; 
				}

				XTAL_CASE('['){ ret = parse_array();  expr_end_flag_ = false; }
				XTAL_CASE('|'){ ret = parse_fun(KIND_LAMBDA); }

				XTAL_CASE('{'){
					if(eat('|')){
						ret = parse_fun(KIND_LAMBDA, true);
					}
					else{
						ret = em.fun(KIND_LAMBDA, make_map(Xid(it), null), false, null); 
						ret->set_fun_body(parse_scope());
					}
				}

				XTAL_CASE(c2('|', '|')){
					ret = em.fun(KIND_LAMBDA, make_map(Xid(it), null), false, null); 
					if(eat('{')){
						ret->set_fun_body(parse_scope());
					}
					else{
						ret->set_fun_body(em.return_(make_array(must_parse_expr())));
					}
				}

				XTAL_CASE('_'){ ret = em.ivar(parse_identifier()); }

				XTAL_CASE('"'){ ret = em.string(KIND_STRING, lexer_.read_string('"', '"')); }
				
				XTAL_CASE('%'){
					int_t ch = lexer_.read_direct();
					int_t kind = KIND_STRING;

					if(ch=='t'){
						kind = KIND_TEXT;
						ch = lexer_.read_direct();
					}
					else if(ch=='f'){
						kind = KIND_FORMAT;
						ch = lexer_.read_direct();
					}

					int_t open = ch;
					int_t close = 0;

					switch(open){
						case '!': case '?': case '"': case '&': //"
						case '#': case '\'':case '|': case ':':
						case '^': case '+': case '-': case '*':
						case '/': case '@': case '$': case '.':
						case '=': case '~': case '`': case ';':
							close = open; break;

						case '(': close = ')'; break;
						case '<': close = '>'; break;
						case '{': close = '}'; break;
						case '[': close = ']'; break;

						default:
							close = open;
							error_->error(lineno(), Xt("Xtal Compile Error 1017"));
							break;
					}
				
					ret = em.string(kind, lexer_.read_string(open, close));
				}
				
				XTAL_CASE(c3('.','.','.')){ ret = em.args(); }

////////////////////////////////////////////////////////////////////////////////////////

				XTAL_CASE('+'){ if(ExprPtr rhs = parse_expr(PRI_POS - r_space)){ ret = em.una(EXPR_POS, rhs); } }
				XTAL_CASE('-'){ if(ExprPtr rhs = parse_expr(PRI_NEG - r_space)){ ret = em.una(EXPR_NEG, rhs); } }
				XTAL_CASE('~'){ if(ExprPtr rhs = parse_expr(PRI_COM - r_space)){ ret = em.una(EXPR_COM, rhs); } }
				XTAL_CASE('!'){ if(ExprPtr rhs = parse_expr(PRI_NOT - r_space)){ ret = em.una(EXPR_NOT, rhs); } }
			}
		}

		XTAL_CASE(Token::TYPE_KEYWORD){
			switch(ch.keyword_number()){

				XTAL_DEFAULT{}
				
				XTAL_CASE(Token::KEYWORD_ONCE){ ret = em.once(must_parse_expr(PRI_ONCE - r_space*2)); }
				XTAL_CASE(Token::KEYWORD_CLASS){ ret = parse_class(KIND_CLASS); }
				XTAL_CASE(Token::KEYWORD_SINGLETON){ ret = parse_class(KIND_SINGLETON); }
				XTAL_CASE(Token::KEYWORD_FUN){ ret = parse_fun(KIND_FUN); }
				XTAL_CASE(Token::KEYWORD_METHOD){ ret = parse_fun(KIND_METHOD); }
				XTAL_CASE(Token::KEYWORD_FIBER){ ret = parse_fun(KIND_FIBER); }
				XTAL_CASE(Token::KEYWORD_DOFUN){ ret = em.call(parse_fun(KIND_FUN), null, null, null); }
				XTAL_CASE(Token::KEYWORD_CALLEE){ ret = em.callee(); }
				XTAL_CASE(Token::KEYWORD_NULL){ ret = em.null_(); }
				XTAL_CASE(Token::KEYWORD_UNDEFINED){ ret = em.undefined_(); }
				XTAL_CASE(Token::KEYWORD_TRUE){ ret = em.true_(); }
				XTAL_CASE(Token::KEYWORD_FALSE){ ret = em.false_(); }
				XTAL_CASE(Token::KEYWORD_THIS){ ret = em.this_(); }
				XTAL_CASE(Token::KEYWORD_CURRENT_CONTEXT){ ret = em.current_context(); }
			}
		}
		
		XTAL_CASE(Token::TYPE_INT){ ret = em.int_(ch.ivalue()); }
		XTAL_CASE(Token::TYPE_FLOAT){ ret = em.float_(ch.fvalue()); }
		XTAL_CASE(Token::TYPE_IDENTIFIER){ ret = em.lvar(ch.identifier()); }
	}

	if(!ret){
		lexer_.putback(ch);
	}

	return ret;
}

ExprPtr Parser::parse_post(ExprPtr lhs, int_t pri){
	if(expr_end_flag_){
		const Token& ch = lexer_.peek();

		if(ch.type()==Token::TYPE_TOKEN && (ch.ivalue()=='.' || ch.ivalue()==c2('.','?'))){
			expr_end_flag_ = false;
		}
		else{
			return null;
		}
	}

	const Token& ch = lexer_.read();

	ExprPtr ret;

	ExprMaker em(lineno());
	int_t r_space = (ch.right_space()) ? PRI_MAX : 0;
	int_t l_space = (ch.left_space()) ? PRI_MAX : 0;

	switch(ch.type()){
	
		XTAL_DEFAULT{}
		
		XTAL_CASE(Token::TYPE_KEYWORD){
			switch(ch.keyword_number()){
				XTAL_DEFAULT{}
				
				XTAL_CASE(Token::KEYWORD_IS){ if(pri < PRI_IS - l_space){ ret = em.bin(EXPR_IS, lhs, parse_expr(PRI_IS - r_space)); } }
				XTAL_CASE(Token::KEYWORD_IN){ if(pri < PRI_IS - l_space){ ret = em.bin(EXPR_IN, lhs, parse_expr(PRI_IN - r_space)); } }
			}
		}
		
		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){

				XTAL_DEFAULT{ ret = null; }
			
				XTAL_CASE('+'){ if(pri < PRI_ADD - l_space){ ret = em.bin(EXPR_ADD, lhs, must_parse_expr(PRI_ADD - r_space)); } }
				XTAL_CASE('-'){ if(pri < PRI_SUB - l_space){ ret = em.bin(EXPR_SUB, lhs, must_parse_expr(PRI_SUB - r_space)); } }
				XTAL_CASE('~'){ if(pri < PRI_CAT - l_space){ ret = em.bin(EXPR_CAT, lhs, must_parse_expr(PRI_CAT - r_space)); } } 
				XTAL_CASE('*'){ if(pri < PRI_MUL - l_space){ ret = em.bin(EXPR_MUL, lhs, must_parse_expr(PRI_MUL - r_space)); } } 
				XTAL_CASE('/'){ if(pri < PRI_DIV - l_space){ ret = em.bin(EXPR_DIV, lhs, must_parse_expr(PRI_DIV - r_space)); } } 
				XTAL_CASE('%'){ if(pri < PRI_MOD - l_space){ ret = em.bin(EXPR_MOD, lhs, must_parse_expr(PRI_MOD - r_space)); } } 
				XTAL_CASE('^'){ if(pri < PRI_XOR - l_space){ ret = em.bin(EXPR_XOR, lhs, must_parse_expr(PRI_XOR - r_space)); } } 
				XTAL_CASE(c2('&','&')){ if(pri < PRI_ANDAND - l_space){ ret = em.bin(EXPR_ANDAND, lhs, must_parse_expr(PRI_ANDAND - r_space));} } 
				XTAL_CASE('&'){ if(pri < PRI_AND - l_space){ ret = em.bin(EXPR_AND, lhs, must_parse_expr(PRI_AND - r_space)); } } 
				XTAL_CASE(c2('|','|')){ if(pri < PRI_OROR - l_space){ ret = em.bin(EXPR_OROR, lhs, must_parse_expr(PRI_OROR - r_space));} } 
				XTAL_CASE('|'){ if(pri < PRI_OR - l_space){ ret = em.bin(EXPR_OR, lhs, must_parse_expr(PRI_OR - r_space)); } } 
				XTAL_CASE(c2('<','<')){ if(pri < PRI_SHL - l_space){ ret = em.bin(EXPR_SHL, lhs, must_parse_expr(PRI_SHL - r_space)); } } 
				XTAL_CASE(c2('>','>')){ if(pri < PRI_SHR - l_space){ ret = em.bin(EXPR_SHR, lhs, must_parse_expr(PRI_SHR - r_space)); } } 
				XTAL_CASE(c3('>','>','>')){ if(pri < PRI_USHR - l_space){ ret = em.bin(EXPR_USHR, lhs, must_parse_expr(PRI_USHR - r_space)); } } 
				XTAL_CASE(c2('<','=')){ if(pri < PRI_LE - l_space){ ret = em.bin(EXPR_LE, lhs, must_parse_expr(PRI_LE - r_space)); } } 
				XTAL_CASE('<'){ if(pri < PRI_LT - l_space){ ret = em.bin(EXPR_LT, lhs, must_parse_expr(PRI_LT - r_space)); } } 
				XTAL_CASE(c2('>','=')){ if(pri < PRI_GE - l_space){ ret = em.bin(EXPR_GE, lhs, must_parse_expr(PRI_GE - r_space)); } } 
				XTAL_CASE('>'){ if(pri < PRI_GT - l_space){ ret = em.bin(EXPR_GT, lhs, must_parse_expr(PRI_GT - r_space)); } } 
				XTAL_CASE(c2('=','=')){ if(pri < PRI_EQ - l_space){ ret = em.bin(EXPR_EQ, lhs, must_parse_expr(PRI_EQ - r_space)); } } 
				XTAL_CASE(c2('!','=')){ if(pri < PRI_NE - l_space){ ret = em.bin(EXPR_NE, lhs, must_parse_expr(PRI_NE - r_space)); } } 
				XTAL_CASE(c3('=','=','=')){ if(pri < PRI_RAW_EQ - l_space){ ret = em.bin(EXPR_RAWEQ, lhs, must_parse_expr(PRI_RAW_EQ - r_space)); } } 
				XTAL_CASE(c3('!','=','=')){ if(pri < PRI_RAW_NE - l_space){ ret = em.bin(EXPR_RAWNE, lhs, must_parse_expr(PRI_RAW_NE - r_space)); } } 
				XTAL_CASE(c3('!','i','s')){ if(pri < PRI_NIS - l_space){ ret = em.bin(EXPR_NIS, lhs, must_parse_expr(PRI_NIS - r_space)); } }
				XTAL_CASE(c3('!','i','n')){ if(pri < PRI_NIN - l_space){ ret = em.bin(EXPR_NIN, lhs, must_parse_expr(EXPR_NIN - r_space)); } }

				XTAL_CASE(c2(':',':')){
					if(pri < PRI_MEMBER - l_space){
						if(eat('(')){
							ret = em.member_e(lhs, must_parse_expr());
							expect(')');
						}
						else{
							ret = em.member(lhs, parse_identifier_or_keyword());
						}

						int_t r_space = (lexer_.peek().right_space() || lexer_.peek().left_space()) ? PRI_MAX : 0;
						if(eat('#')){
							ret->set_send_ns(must_parse_expr(PRI_NS - r_space));
						}
					}
				}

				XTAL_CASE(c3(':',':','?')){
					if(pri < PRI_MEMBER - l_space){
						if(eat('(')){
							ret = em.member_eq(lhs, must_parse_expr());
							expect(')');
						}
						else{
							ret = em.member_q(lhs, parse_identifier_or_keyword());
						}

						int_t r_space = (lexer_.peek().right_space() || lexer_.peek().left_space()) ? PRI_MAX : 0;
						if(eat('#')){
							ret->set_send_ns(must_parse_expr(PRI_NS - r_space));
						}
					}
				}

				XTAL_CASE('.'){
					if(pri < PRI_SEND - l_space){
						if(eat('(')){
							ret = em.send_e(lhs, must_parse_expr());
							expect(')');
						}
						else{
							ret = em.send(lhs, parse_identifier_or_keyword());
						}

						int_t r_space = (lexer_.peek().right_space() || lexer_.peek().left_space()) ? PRI_MAX : 0;
						if(eat('#')){
							ret->set_send_ns(must_parse_expr(PRI_NS - r_space));
						}
					}
				}

				XTAL_CASE(c2('.', '?')){ 
					if(pri < PRI_SEND - l_space){
						if(eat('(')){
							ret = em.send_eq(lhs, must_parse_expr());
							expect(')');
						}
						else{
							ret = em.send_q(lhs, parse_identifier_or_keyword());
						}

						int_t r_space = (lexer_.peek().right_space() || lexer_.peek().left_space()) ? PRI_MAX : 0;
						if(eat('#')){
							ret->set_send_ns(must_parse_expr(PRI_NS - r_space));
						}
					}
				}

				XTAL_CASE('?'){
					if(pri < PRI_Q - l_space){
						ret = em.q(lhs, null, null);
						ret->set_q_true(must_parse_expr());
						expect(':');
						ret->set_q_false(must_parse_expr());
					}
				}

				XTAL_CASE(c2('.', '.')){
					if(pri < PRI_RANGE - l_space){
						ret = em.range_(lhs, must_parse_expr(PRI_RANGE - r_space), RANGE_CLOSED);
					}
				}

				XTAL_CASE(c3('.', '.', '<')){
					if(pri < PRI_RANGE - l_space){
						ret = em.range_(lhs, must_parse_expr(PRI_RANGE - r_space), RANGE_LEFT_CLOSED_RIGHT_OPEN);
					}
				}

				XTAL_CASE(c3('<', '.', '.')){
					if(pri < PRI_RANGE - l_space){
						ret = em.range_(lhs, must_parse_expr(PRI_RANGE - r_space), RANGE_LEFT_OPEN_RIGHT_CLOSED);
					}
				}

				XTAL_CASE(c4('<', '.', '.', '<')){
					if(pri < PRI_RANGE - l_space){
						ret = em.range_(lhs, must_parse_expr(PRI_RANGE - r_space), RANGE_OPEN);
					}
				}

				XTAL_CASE('('){
					if(pri < PRI_CALL - l_space){
						ret = parse_call(lhs);
					}
				}

				XTAL_CASE('['){
					if(pri < PRI_AT - l_space){
						if(eat(':')){
							expect(']');
							ret = em.send(lhs, Xid(to_m));
						}
						else if(eat(']')){
							ret = em.send(lhs, Xid(to_a));
						}
						else{
							ret = em.bin(EXPR_AT, lhs, must_parse_expr());
							expect(']');
						}
					}
				}
			}
		}
	}

	if(!ret){
		lexer_.putback(ch);
	}

	return ret;
}

ExprPtr Parser::parse_each(const IDPtr& label, ExprPtr lhs){
	ExprMaker em(lexer_.lineno());	

	ArrayPtr params = xnew<Array>();
	params->push_back(em.lvar(Xid(iterator)));
	bool discard = false;
	if(eat('|')){ // ブロックパラメータ
		while(true){
			const Token& ch = lexer_.peek();
			if(ch.type()==ch.TYPE_IDENTIFIER){
				discard = false;
				lexer_.read();
				params->push_back(em.lvar(ch.identifier()));
				if(!eat(',')){
					expect('|');
					break;
				}
				else{
					discard = true;
				}
			}
			else{
				expect('|');
				break;
			}
		}
	}
	else{
		params->push_back(em.lvar(Xid(it)));
	}

	if(discard){
		params->push_back(em.lvar(Xid(_dummy_block_parameter_)));
	}

	ArrayPtr scope_stmts = xnew<Array>();
	scope_stmts->push_back(em.massign(params, make_array(em.send(lhs, Xid(block_first))), true));
			
	ExprPtr efor = em.for_(label, em.lvar(Xid(iterator)), em.massign(params, make_array(em.send(em.lvar(Xid(iterator)), Xid(block_next))), false), parse_scope(), null, null);
	if(eat(Token::KEYWORD_ELSE)){
		efor->set_for_else(must_parse_stmt());
	}
	else if(eat(Token::KEYWORD_NOBREAK)){
		efor->set_for_nobreak(must_parse_stmt());
	}

	ExprPtr catch_stmt = em.if_(em.una(EXPR_NOT, em.call(em.send_q(em.lvar(Xid(iterator)), Xid(block_catch)), make_array(em.lvar(Xid(e))), null, null)), em.una(EXPR_THROW, em.lvar(Xid(e))), null);
	scope_stmts->push_back(em.try_(efor, Xid(e), catch_stmt, em.send_q(em.lvar(Xid(iterator)), Xid(block_break))));

	return em.scope(scope_stmts);
}

ExprPtr Parser::parse_loop(){
	ExprMaker em(lexer_.lineno());	

	if(IDPtr identifier = parse_var()){
		const Token& ch = lexer_.read(); // :の次を読み取る
		if(ch.type()==Token::TYPE_KEYWORD){
			switch(ch.keyword_number()){
				XTAL_DEFAULT{}
				XTAL_CASE(Token::KEYWORD_FOR){ return parse_for(identifier); }
				XTAL_CASE(Token::KEYWORD_WHILE){ return parse_while(identifier); }
			}
		}

		lexer_.putback(ch);
		if(ExprPtr lhs = parse_expr()){
			if(!expr_end_flag_ && eat('{')){
				return parse_each(identifier, lhs);
			}
			else{
				return em.define(em.lvar(identifier), lhs);
			}
		}

		lexer_.putback();
		lexer_.putback();
	}
	return null;
}

ExprPtr Parser::parse_assign_stmt(){

	ExprMaker em(lineno());
	{
		const Token& ch = lexer_.read();

		if(ch.type()==Token::TYPE_TOKEN){
			switch(ch.ivalue()){
				XTAL_DEFAULT{}
				XTAL_CASE(c2('+','+')){ return em.una(EXPR_INC, must_parse_expr()); }
				XTAL_CASE(c2('-','-')){ return em.una(EXPR_DEC, must_parse_expr()); }
			}
		}
		lexer_.putback();
	}

	if(ExprPtr lhs = parse_expr()){
		if(expr_end_flag_)
			return lhs;
		
		const Token& ch = lexer_.read();

		switch(ch.type()){
			XTAL_DEFAULT{}
			
			XTAL_CASE(Token::TYPE_TOKEN){
				switch(ch.ivalue()){

					XTAL_DEFAULT{
						lexer_.putback();
						return lhs; 
					}

					XTAL_CASE(','){
						bool discard = true;
						ArrayPtr left_exprs = parse_exprs(&discard);
						left_exprs->push_front(lhs);
						if(discard){
							left_exprs->push_back(em.lvar(Xid(_dummy_lhs_parameter_)));
						}
						
						if(eat('=')){
							return em.massign(left_exprs, parse_exprs(), false);
						}
						else if(eat(':')){
							return em.massign(left_exprs, parse_exprs(), true);
						}
						else{
							error_->error(lineno(), Xt("Xtal Compile Error 1001"));
						}
						
						return null;
					}

					XTAL_CASE('='){  return em.assign(lhs, must_parse_expr()); }
					XTAL_CASE(':'){ 
						return em.define(lhs, must_parse_expr()); 
					}

					XTAL_CASE(c2('+','+')){ return em.una(EXPR_INC, lhs); }
					XTAL_CASE(c2('-','-')){ return em.una(EXPR_DEC, lhs); }
					
					XTAL_CASE(c2('+','=')){ return em.bin(EXPR_ADD_ASSIGN, lhs, must_parse_expr()); }
					XTAL_CASE(c2('-','=')){ return em.bin(EXPR_SUB_ASSIGN, lhs, must_parse_expr()); }
					XTAL_CASE(c2('~','=')){ return em.bin(EXPR_CAT_ASSIGN, lhs, must_parse_expr()); }
					XTAL_CASE(c2('*','=')){ return em.bin(EXPR_MUL_ASSIGN, lhs, must_parse_expr()); }
					XTAL_CASE(c2('/','=')){ return em.bin(EXPR_DIV_ASSIGN, lhs, must_parse_expr()); }
					XTAL_CASE(c2('%','=')){ return em.bin(EXPR_MOD_ASSIGN, lhs, must_parse_expr()); }
					XTAL_CASE(c2('^','=')){ return em.bin(EXPR_XOR_ASSIGN, lhs, must_parse_expr()); }
					XTAL_CASE(c2('|','=')){ return em.bin(EXPR_OR_ASSIGN, lhs, must_parse_expr()); }
					XTAL_CASE(c2('&','=')){ return em.bin(EXPR_AND_ASSIGN, lhs, must_parse_expr()); }
					XTAL_CASE(c3('<','<','=')){ return em.bin(EXPR_SHL_ASSIGN, lhs, must_parse_expr()); }
					XTAL_CASE(c3('>','>','=')){ return em.bin(EXPR_SHR_ASSIGN, lhs, must_parse_expr()); }
					XTAL_CASE(c4('>','>','>','=')){ return em.bin(EXPR_USHR_ASSIGN, lhs, must_parse_expr()); }

					XTAL_CASE('{'){
						return parse_each(null, lhs);
					}
				}
			}
		}

		return lhs;
	}

	return null;
}

ExprPtr Parser::parse_stmt(){

	ExprMaker em(lineno());
	ExprPtr ret;

	if(ExprPtr p = parse_loop()){
		ret = p;
		 eat(';'); 
	}
	else{

		const Token& ch = lexer_.read();

		switch(ch.type()){
			XTAL_DEFAULT{}

			XTAL_CASE(Token::TYPE_KEYWORD){
				switch(ch.keyword_number()){
			
					XTAL_CASE(Token::KEYWORD_WHILE){ ret = parse_while(); }
					XTAL_CASE(Token::KEYWORD_SWITCH){ ret = parse_switch(); }
					XTAL_CASE(Token::KEYWORD_IF){ ret = parse_if(); }
					XTAL_CASE(Token::KEYWORD_TRY){ ret = parse_try(); }

					XTAL_CASE(Token::KEYWORD_THROW){	
						ExprPtr temp = xnew<Expr>(EXPR_THROW);
						temp->set_throw_expr(must_parse_expr());
						ret = temp;
						eat(';'); 
					}	
					XTAL_CASE(Token::KEYWORD_RETURN){ ret = em.return_(parse_exprs());  eat(';'); }
					XTAL_CASE(Token::KEYWORD_CONTINUE){ ret = em.continue_(parse_identifier());  eat(';'); }
					XTAL_CASE(Token::KEYWORD_BREAK){ ret = em.break_(parse_identifier());  eat(';'); }
					XTAL_CASE(Token::KEYWORD_FOR){ ret = parse_for(); }
					XTAL_CASE(Token::KEYWORD_ASSERT){ ret = parse_assert();  eat(';'); }
					XTAL_CASE(Token::KEYWORD_YIELD){ ret = em.yield(parse_exprs()); eat(';'); }
				}
			}
			
			XTAL_CASE(Token::TYPE_TOKEN){
				switch(ch.ivalue()){
					XTAL_CASE('{'){ ret = parse_scope(); }
					XTAL_CASE(';'){ return em.null_(); }
				}
			}
		}
		
		if(!ret){
			lexer_.putback();
			ret = parse_assign_stmt();
			eat(';');
		}
	}

	return ret;
}
	
ExprPtr Parser::must_parse_stmt(){
	ExprPtr ret = parse_stmt();
	if(!ret){
		error_->error(lineno(), Xt("Xtal Compile Error 1001"));
	}
	return ret;
}

ExprPtr Parser::parse_assert(){
	ExprMaker em(lineno());
	ArrayPtr exprs = xnew<Array>();

	lexer_.begin_record();
	if(ExprPtr ep = parse_expr()){
		StringPtr ref_str = lexer_.end_record();
		exprs->push_back(ep);
		exprs->push_back(em.string(KIND_STRING, ref_str));
		if(eat(',')){
			if(ExprPtr ep = parse_expr()){
				exprs->push_back(ep);
			}
		}
	}
	else{
		lexer_.end_record();
	}

	return em.assert_(exprs);
}
	
ArrayPtr Parser::parse_exprs(bool* discard){
	ArrayPtr ret = xnew<Array>();
	while(true){
		if(ExprPtr p = parse_expr()){
			if(discard) *discard = false;
			ret->push_back(p);
			if(eat(',')){
				if(discard) *discard = true;
			}
			else{
				break;
			}
		}
		else{
			break;
		}
	}
	return ret;
}

ArrayPtr Parser::parse_stmts(){
	ArrayPtr ret = xnew<Array>();
	while(true){
		while(eat(';')){}
		if(ExprPtr p = parse_stmt()){
			ret->push_back(p);
		}
		else{
			break;
		}
	}
	return ret;
}

IDPtr Parser::parse_identifier(){
	if(lexer_.peek().type()==Token::TYPE_IDENTIFIER){
		return lexer_.read().identifier();
	}
	return null;
}

IDPtr Parser::parse_identifier_or_keyword(){
	if(lexer_.peek().type()==Token::TYPE_IDENTIFIER){
		return lexer_.read().identifier();
	}
	else if(lexer_.peek().type()==Token::TYPE_KEYWORD){
		return lexer_.read().identifier();
	}
	return null;
}

IDPtr Parser::parse_var(){
	if(IDPtr identifier = parse_identifier()){
		if(eat(':')){ 
			return identifier; 
		}
		else{
			lexer_.putback();
		}
	}
	return null;
}
	
ExprPtr Parser::parse_for(const IDPtr& label){
	ExprMaker em(lineno());

	ArrayPtr scope_stmts = xnew<Array>();
	expect('(');
	scope_stmts->push_back(parse_assign_stmt());
	expect(';');

	ExprPtr efor = em.for_(label, null, null, null, null, null);
	efor->set_for_cond(parse_expr());
	expect(';');

	efor->set_for_next(parse_assign_stmt());
	expect(')');

	efor->set_for_body(must_parse_stmt());

	if(eat(Token::KEYWORD_ELSE)){
		efor->set_for_else(must_parse_stmt());
	}
	else if(eat(Token::KEYWORD_NOBREAK)){
		efor->set_for_nobreak(must_parse_stmt());
	}

	scope_stmts->push_back(efor);

	return em.scope(scope_stmts);
}

ExprPtr Parser::parse_toplevel(){
	ExprMaker em(lineno());
	ExprPtr ret = em.scope(parse_stmts());
	expect(-1);
	return ret;
}

ExprPtr Parser::parse_scope(){
	ExprMaker em(lineno());
	ExprPtr ret = em.scope(parse_stmts());
	expect('}');
	expr_end_flag_ = true;
	return ret;
}

ExprPtr Parser::parse_class(int_t kind){
	ExprMaker em(lineno());
	ExprPtr eclass = em.class_(kind, null, xnew<Array>(), xnew<Map>());

	if(eat('(')){
		eclass->set_class_mixins(parse_exprs());
		expect(')');
	}

	expect('{');
	while(true){

		int_t accessibility = -1;
		
		if(eat('#')){// 可触性 protected 指定
			accessibility = KIND_PROTECTED;
		}
		else if(eat('-')){// 可触性 private 指定
			accessibility = KIND_PRIVATE;
		}
		else if(eat('+')){// 可触性 public 指定
			accessibility = KIND_PUBLIC;
		}

		if(IDPtr var = parse_identifier()){ // メンバ定義
			ExprMaker em(lineno());

			if(eat('#')){
				ExprPtr secondary_key = must_parse_expr(PRI_NS);
				expect(':');
				eclass->class_stmts()->push_back(em.cdefine(accessibility<0 ? KIND_PUBLIC : accessibility, var, secondary_key, must_parse_expr()));
			}
			else{
				expect(':');
				int_t ln = lineno();
				eclass->class_stmts()->push_back(em.cdefine(accessibility<0 ? KIND_PUBLIC : accessibility, var, em.null_(), must_parse_expr()));
			}
			eat(';');
			
		}
		else if(eat('_')){// インスタンス変数定義
			ExprMaker em(lineno());
			if(IDPtr var = parse_identifier()){
				if(eclass->class_ivars()->at(var)){
					error_->error(lineno(), Xt("Xtal Compile Error 1024")(Named("name", var)));
				}

				if(eat(':')){ // 初期値込み
					eclass->class_ivars()->set_at(var, must_parse_expr());
				}
				else{
					eclass->class_ivars()->set_at(var, null);
				}
				eat(';');

				if(accessibility!=-1){ // 可触性が付いているので、アクセッサを定義する
					eclass->class_stmts()->push_back(
						em.cdefine(accessibility, var, em.null_(), 
							em.fun(KIND_METHOD, null, false, 
								em.return_(make_array(em.ivar(var))))));
			
					IDPtr var2 = xnew<String>("set_")->cat(var);
					eclass->class_stmts()->push_back(
						em.cdefine(accessibility, var2, em.null_(), 
							em.fun(KIND_METHOD, make_map(Xid(value), null), false, 
								em.assign(em.ivar(var), em.lvar(Xid(value))))));
				}
			}
			else{
				error_->error(lineno(), Xt("Xtal Compile Error 1001"));
			}
		}
		else{
			break;
		}
	}

	expect('}');
	expr_end_flag_ = true;
	return eclass;
}

ExprPtr Parser::parse_try(){
	ExprMaker em(lineno());
	ExprPtr etry = em.try_(null, null, null, null);

	etry->set_try_body(must_parse_stmt());
	
	if(eat(Token::KEYWORD_CATCH)){
		expect('(');
		etry->set_try_catch_var(parse_identifier());
		expect(')');
		etry->set_try_catch(must_parse_stmt());
	}

	if(eat(Token::KEYWORD_FINALLY)){
		etry->set_try_finally(must_parse_stmt());
	}

	return etry;
}

ExprPtr Parser::parse_fun(int_t kind, bool body){
	ExprMaker em(lineno());
	bool lambda = kind==KIND_LAMBDA;
	
	MapPtr params = xnew<Map>();
	ExprPtr efun = em.fun(kind, params, false, null);

	int_t inst_assign_list_count = 0;
	IDPtr inst_assign_list[255];

	if(lambda || eat('(')){

		while(true){
			
			if(eat(lambda ? '|' : ')')){
				if(lambda && params->empty()){
					params->set_at(Xid(it), null);
				}
				break;
			}
				
			if(!lambda && eat(c3('.','.','.'))){
				efun->set_fun_extendable_param(true);
				expect(')');
				break;
			}
			
			if(eat('_')){
				if(IDPtr var = parse_identifier()){
					if(rawne(params->at(var), undefined)){
						error_->error(lineno(), Xt("Xtal Compile Error 1026")(Named("name", var)));
					}

					if(!lambda && eat(':')){
						params->set_at(var, must_parse_expr());
					}
					else{
						params->set_at(var, null);
					}

					if(inst_assign_list_count<255){
						inst_assign_list[inst_assign_list_count++] = var;
					}
				}
				else{
					error_->error(lineno(), Xt("Xtal Compile Error 1001"));
				}
			}
			else if(IDPtr var = parse_identifier()){
				if(rawne(params->at(var), undefined)){
					error_->error(lineno(), Xt("Xtal Compile Error 1026")(Named("name", var)));
				}

				if(!lambda && eat(':')){
					params->set_at(var, must_parse_expr());
				}
				else{
					params->set_at(var, null);
				}
			}
			
			if(eat(',')){
				if(lambda && eat('|')){
					params->set_at(Xid(_dummy_fun_parameter_), null);
					break;
				}
			}
			else if(eat(lambda ? '|' : ')')){
				break;
			}
			else{
				error_->error(lineno(), Xt("Xtal Compile Error 1010"));
				break;
			}
		}
	}

	if(inst_assign_list_count==0){
		ExprMaker em(lineno());
		if(body || eat('{')){
			efun->set_fun_body(parse_scope());
		}
		else{
			int_t ln = lineno();
			efun->set_fun_body(em.return_(parse_exprs()));
		}
	}
	else{
		ExprMaker em(lineno());

		ArrayPtr scope_stmts = xnew<Array>();
		for(int_t i=0; i<inst_assign_list_count; ++i){
			IDPtr var = inst_assign_list[i];
			scope_stmts->push_back(em.assign(em.ivar(var), em.lvar(var)));
		}

		if(body || eat('{')){
			ExprPtr scp = parse_scope();
			if(!scp->scope_stmts()->empty()){
				scope_stmts->push_back(scp);
			}
			efun->set_fun_body(em.scope(scope_stmts));
		}
		else{
			scope_stmts->push_back(em.return_(parse_exprs()));
			efun->set_fun_body(em.scope(scope_stmts));
		}
	}

	return efun;
}

ExprPtr Parser::parse_call(ExprPtr lhs){
	ExprMaker em(lineno());
	ArrayPtr ordered = xnew<Array>();
	MapPtr named = xnew<Map>();
	ExprPtr ecall = em.call(lhs, ordered, named, null);

	bool end = false;
	while(true){
		if(IDPtr var = parse_var()){
			named->set_at(var, must_parse_expr());
		}
		else{
			if(ExprPtr val = parse_expr()){
				if(val->tag()==EXPR_ARGS){
					if(ExprPtr val2 = parse_expr()){
						ecall->set_call_args(val2);
					}
					else{
						ecall->set_call_args(val);
					}

					end = true;
				}
				else{
					ordered->push_back(val);
				}
			}
			else{
				end = true;
			}
		}

		eat(',');
		if(end){
			expect(')');
			break;
		}
	}

	return ecall;
}

ExprPtr Parser::parse_expr(){
	expr_end_flag_ = false;
	ExprPtr ret = parse_expr(0);
	return ret;
}

ExprPtr Parser::parse_expr(int_t pri){
	
	ExprPtr ret = parse_term();
	if(!ret){
		return null;
	}
	
	while(true){
		if(ExprPtr ret2 = parse_post(ret, pri)){
			ret = ret2;
		}
		else{
			break;
		}
	}
	
	return ret;
}

ExprPtr Parser::must_parse_expr(int_t pri){
	ExprPtr ret = parse_expr(pri);
	if(!ret){
		error_->error(lineno(), Xt("Xtal Compile Error 1001"));
	}
	return ret;
}

ExprPtr Parser::must_parse_expr(){
	expr_end_flag_ = false;
	ExprPtr ret = must_parse_expr(0);
	return ret;

}

ExprPtr Parser::parse_if(){
	ExprMaker em(lineno());
	expect('(');
	ExprPtr eif = em.if_(null, null, null);
	if(IDPtr var = parse_var()){
		ExprPtr escope = em.scope(xnew<Array>());
		escope->scope_stmts()->push_back(em.define(em.lvar(var), must_parse_expr()));
		expect(')');
		eif->set_if_cond(em.lvar(var));
		eif->set_if_body(must_parse_stmt());
		if(eat(Token::KEYWORD_ELSE)){
			eif->set_if_else(must_parse_stmt());
		}
		escope->scope_stmts()->push_back(eif);
		return escope;
	}
	else{
		eif->set_if_cond(must_parse_expr());
		expect(')');
		eif->set_if_body(must_parse_stmt());
		if(eat(Token::KEYWORD_ELSE)){
			eif->set_if_else(must_parse_stmt());
		}
		return eif;		
	}
}

ExprPtr Parser::parse_while(const IDPtr& label){
	ExprMaker em(lineno());
	expect('(');
	ExprPtr efor = em.for_(label, null, null, null, null, null);
	/*if(IDPtr var = parse_var()){
		ExprPtr escope = em.scope(xnew<Array>());
		escope->scope_stmts()->push_back(em.define(em.lvar(var), must_parse_expr()));
		expect(')');
		efor->set_for_cond(em.lvar(var));
		efor->set_for_body(must_parse_stmt());
		if(eat(Token::KEYWORD_ELSE)){
			efor->set_for_else(must_parse_stmt());
		}
		else if(eat(Token::KEYWORD_NOBREAK)){
			efor->set_for_nobreak(must_parse_stmt());
		}		
		escope->scope_stmts()->push_back(efor);
		return escope;
	}
	else{*/
		efor->set_for_cond(must_parse_expr());
		expect(')');
		efor->set_for_body(must_parse_stmt());
		if(eat(Token::KEYWORD_ELSE)){
			efor->set_for_else(must_parse_stmt());
		}
		else if(eat(Token::KEYWORD_NOBREAK)){
			efor->set_for_nobreak(must_parse_stmt());
		}
		return efor;		
	/*}*/
}

ExprPtr Parser::parse_switch(){
	ExprMaker em(lineno());
	ArrayPtr scope_stmts = xnew<Array>();
	ExprPtr ret = em.scope(scope_stmts);
	expect('(');

	IDPtr var = parse_var();
	if(!var){
		var = Xid(_switch_);
	}
	
	scope_stmts->push_back(em.define(em.lvar(var), must_parse_expr()));

	expect(')');
	expect('{');
	
	ExprPtr switch_stmt = em.switch_(em.lvar(var), null, null);
	MapPtr cases = xnew<Map>();
	ExprPtr default_stmt;
	while(true){
		if(eat(Token::KEYWORD_CASE)){
			expect('(');
			ExprPtr when = must_parse_expr();
			expect(')');
			cases->set_at(when, parse_stmt());
		}
		else if(eat(Token::KEYWORD_DEFAULT)){
			if(default_stmt){
				error_->error(lineno(), Xt("Xtal Compile Error 1018")());					
			}
			default_stmt = parse_stmt();
		}
		else{
			expect('}');
			expr_end_flag_ = true;
			break;
		}
	}
	
	scope_stmts->push_back(em.switch_(em.lvar(var), cases, default_stmt));
	return ret;
}

ExprPtr Parser::parse_array(){
	ExprMaker em(lineno());
	
	if(eat(']')){//empty array
		return em.array(null);
	}
	
	if(eat(':')){//empty map
		expect(']');
		return em.map(null);
	}
		
	ExprPtr key = must_parse_expr();
	if(eat(':')){//map
		ExprPtr emap = em.map(xnew<Map>());
		emap->map_values()->set_at(key, must_parse_expr());	
		
		if(eat(',')){
			for(;;){
				key = parse_expr();
				if(key){
					expect(':');
					emap->map_values()->set_at(key, must_parse_expr());
					
					if(!eat(',')){
						break;
					}
				}
				else{
					break;
				}
			}
		}

		expect(']');
		return emap;
	}
	else{//array
		ExprPtr earray = em.array(xnew<Array>());
		earray->array_values()->push_back(key);
		if(eat(',')){
			while(true){
				key = parse_expr();
				if(key){
					earray->array_values()->push_back(key);
					if(!eat(',')){
						break;
					}
				}
				else{
					break;
				}
			}
		}
		expect(']');
		return earray;
	}
}

}

#endif
