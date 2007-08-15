
#include "xtal.h"
#include "xtal_macro.h"

using namespace xtal;

static void print_usage(){
	fprintf(stderr,
		"usage: xtal [options] [script [args]].\n"
		"Available options are:\n"
		"  -v       show version information\n"
	);
}

static void handle_argv(char** argv){
	int i;
	for(i=1; argv[i]!=0; i++){
		if(argv[i][0]!='-')
			break;

		switch(argv[i][1]){
		case 'v':
			fprintf(stderr, "xtal %d.%d.%d.%d\n", VERSION1, VERSION2, VERSION3, VERSION4);
			break;

		default:
			print_usage();
			return;
		}
	
	}

	if(argv[i]!=0){
		ArrayPtr arg_list(xnew<Array>());
		const char *filename = argv[i++];
		for(i=1; argv[i]!=0; i++){
			arg_list->push_back(argv[i]);
		}
		builtin()->def("argv", arg_list);
		load(filename);
	}
}


#include "xtal_parser.h"

using namespace xtal;
typedef SmartPtr<Reader> ReaderPtr;


namespace grammer{

class Parser;

class ParserPtr : public SmartPtr<Parser>{
public:

	ParserPtr(const SmartPtr<Parser>& p)
		:SmartPtr<Parser>(p){}

};


class Parser : public Base{
public:
	
	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out) = 0;
	
	bool try_parse(const ReaderPtr& r, const ArrayPtr& out){
		if(out){
			int_t len = out->length();
			int_t pos = r->position();
			if(parse(r, out)){
				return true;
			}
			r->set_position(pos);
			out->resize(len);
		}else{
			int_t pos = r->position();
			if(parse(r, out)){
				return true;
			}
			r->set_position(pos);
		}
		return false;
	}
};

class ChParser : public Parser{
	int ch_;
public:
	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(r->read()==ch_){
			if(out){
				out->push_back(ch_);
			}
			return true;
		}
		return false;
	}
};

class StringParser : public Parser{
	StringPtr str_;
public:
	StringParser(const StringPtr& str)
		:str_(str){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		const char_t* str = str_->c_str();
		for(int_t i=0; i<str_->size(); ++i){
			if(r->read()!=str[i]){
				return false;
			}
		}
		if(out){
			out->push_back(str_);
		}
		return true;
	}
};

class AnyChParser : public Parser{
public:
	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		int_t ret = r->read();
		if(ret<0)
			return false;
		char buf[2] = {ret, 0};
		if(out) out->push_back(xnew<String>(buf));
		return true;
	}
};

class ManyParser : public Parser{
	ParserPtr p_;
public:
	ManyParser(const ParserPtr& p)
		:p_(p){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		while(p_->try_parse(r, out)){}
		return true;
	}
};

class Many1Parser : public Parser{
	ParserPtr p_;
public:
	Many1Parser(const ParserPtr& p)
		:p_(p){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(p_->parse(r, out)){
			while(p_->try_parse(r, out)){}
			return true;
		}
		return false;
	}
};

class SkipParser : public Parser{
	ParserPtr p_;
public:
	SkipParser(const ParserPtr& p)
		:p_(p){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		return p_->parse(r, null);
	}
};

class SubParser : public Parser{
	ParserPtr lhs_, rhs_;
public:
	SubParser(const ParserPtr& l, const ParserPtr& r)
		:lhs_(l), rhs_(r){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		int_t pos = r->position();
		if(rhs_->try_parse(r, null)){
			r->set_position(pos);
			return false;
		}

		return lhs_->parse(r, out);
	}
};

class OrParser : public Parser{
	ParserPtr lhs_, rhs_;
public:
	OrParser(const ParserPtr& l, const ParserPtr& r)
		:lhs_(l), rhs_(r){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		return lhs_->try_parse(r, out) || rhs_->parse(r, out);
	}
};

class AndParser : public Parser{
	ParserPtr lhs_, rhs_;
public:	
	AndParser(const ParserPtr& l, const ParserPtr& r)
		:lhs_(l), rhs_(r){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		return lhs_->parse(r, out) && rhs_->parse(r, out);
	}
};

class InvertParser : public Parser{
	ParserPtr p_;
public:
	InvertParser(const ParserPtr& p)
		:p_(p){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		int_t len = out->length();
		int_t pos = r->position();
		bool ret = !parse(r, out);
		r->set_position(pos);
		out->resize(len);
		return ret;
	}
};

class EmptyParser : public Parser{
public:
	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		return true;
	}
};

class JoinParser : public Parser{
	ParserPtr p_;
	StringPtr sep_;
public:
	JoinParser(const ParserPtr& p, const StringPtr& sep = "")
		:p_(p), sep_(sep){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		ArrayPtr ret = xnew<Array>();
		if(p_->parse(r, ret)){
			out->push_back(ret->join(sep_));
			return true;
		}
		return false;
	}
};

class ArrayParser : public Parser{
	ParserPtr p_;
public:
	ArrayParser(const ParserPtr& p)
		:p_(p){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(out){
			int_t pos = out->size();
			if(p_->parse(r, out)){
				out->push_back(out->splice(pos, out->size()-pos));
				return true;
			}
			return false;
		}

		return p_->parse(r, out);
	}
};

class InsertValParser : public Parser{
	AnyPtr val_;
	int_t pos_;
public:
	InsertValParser(const AnyPtr& val, int_t pos)
		:val_(val), pos_(pos){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(out) out->insert(out->size() - pos_, val_);
		return true;
	}
};

class SpliceParser : public Parser{
	int_t n_;
public:
	SpliceParser(int_t n)
		:n_(n){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(out) out->push_back(out->splice(out->size() - n_, n_));
		return true;
	}
};


inline ParserPtr operator *(const ParserPtr& p){
	return ParserPtr(xnew<ManyParser>(p));
}

inline ParserPtr operator +(const ParserPtr& p){
	return ParserPtr(xnew<Many1Parser>(p));
}

inline ParserPtr operator -(const ParserPtr& p){
	return ParserPtr(xnew<SkipParser>(p));
}

inline ParserPtr operator |(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<OrParser>(l, r));
}

inline ParserPtr operator >>(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<AndParser>(l, r));
}

inline ParserPtr operator ~(const ParserPtr& p){
	return ParserPtr(xnew<InvertParser>(p));
}

inline ParserPtr operator -(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<SubParser>(l, r));
}

/*
inline ParserPtr operator !(const ParserPtr& p){
	return p | e;
}
*/

inline ParserPtr join(const ParserPtr& p, const StringPtr& sep = ""){
	return ParserPtr(xnew<JoinParser>(p, sep));
}

inline ParserPtr str(const StringPtr& p){
	return ParserPtr(xnew<StringParser>(p));
}

inline ParserPtr array(const ParserPtr& p){
	return ParserPtr(xnew<ArrayParser>(p));
}

inline ParserPtr insert_val(const AnyPtr& p, int_t pos = 0){
	return ParserPtr(xnew<InsertValParser>(p, pos));
}

inline ParserPtr splice(int_t n){
	return ParserPtr(xnew<SpliceParser>(n));
}

class IntParser : public Parser{
public:
	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(!test_digit(r->peek())){
			return false;
		}
		int_t ret = 0;
		while(1){
			if(test_digit(r->peek())){
				ret *= 10;
				ret += r->read()-'0';
			}else if(r->peek()=='_'){
				r->read();
			}else{
				break;
			}
		}
		out->push_back(ret);
		return true;
	}
};
			
}


void debug_line(const SmartPtr<debug::Info>& info){
	std::cout << Xf("kind=%d, line=%s, file=%s, fun=%s\n")(info->kind(), info->line(), info->file_name(), info->fun_name());

	/*if(info->local_variables()){
		Xfor2(key, value, info->local_variables()->each_member()){
			std::cout << Xf("key=%s, value=%s\n")(key, value);
		}
	}*/
}

//#include <crtdbg.h>

int main(int argc, char** argv){

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | /*_CRTDBG_CHECK_ALWAYS_DF |*/ _CRTDBG_DELAY_FREE_MEM_DF);

	try{

		initialize();

		//debug::enable();
		//debug::set_line_hook(fun(&debug_line));
		//debug::set_call_hook(fun(&debug_line));
		//debug::set_return_hook(fun(&debug_line));
		
		
		{
			using namespace grammer;

			const char* source = "189+256*100+55";
			StreamPtr stream = xnew<MemoryStream>(source, strlen(source));
			ReaderPtr reader = xnew<Reader>();
			reader->set_stream(stream);

			ParserPtr anych = xnew<AnyChParser>();
			ParserPtr integer = xnew<IntParser>();
			
			ParserPtr term = integer >> insert_val("INT", 1) >> splice(2);

			ParserPtr expr_mul = term >> *((
				(-str("*") >> insert_val("MUL", 1) >> splice(2)) | 
				(-str("/") >> insert_val("DIV", 1) >> splice(2))
				) >> term);

			ParserPtr expr_add = expr_mul >> *((
				(-str("+") >> insert_val("ADD", 1) >> splice(2)) | 
				(-str("-") >> insert_val("SUB", 1) >> splice(2))
				) >> expr_mul);

			ArrayPtr ret = xnew<Array>();

			if(expr_add->parse(reader, ret)){
				ret[0]->p();
			}

		}


		{
			StringPtr path(argv[0]);


#ifdef WIN32
			StringPtr sep("\\");
#else
			StringPtr sep("/");
#endif

			ArrayPtr temp = cast<ArrayPtr>(path->split(sep)->send("to_a"));
			temp->pop_back();
#ifdef WIN32
			temp->push_back("message.xtal");
#else
			temp->push_back("message_en.xtal");
#endif
			path = temp->join(sep)->to_s();
			add_get_text_map(cast<MapPtr>(load(path)));
		}

		AnyPtr cd = Xsrc((
			(|| it.p)();
		))();

		int c;
		c = clock();
		handle_argv(argv);
		printf("%g\n", (clock()-c)/1000.0f);
		
		/*		
		c = clock();
		load("../bench/vec.xtal");
		printf("%g\n", (clock()-c)/1000.0f);		
		
		c = clock();
		load("../bench/inst.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		full_gc();
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/loop.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/nested_loops.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/fib.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/loop_iter.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/array_for.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/array_each.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		//*/

		//*

		load("../test/test_empty.xtal");
		load("../test/test_array.xtal");
		load("../test/test_float.xtal");
		load("../test/test_class.xtal");
		load("../test/test_multiple_values.xtal");
		load("../test/test_except.xtal");
		load("../test/test_fiber.xtal");
		load("../test/test_fun.xtal");
		load("../test/test_iter.xtal");
		load("../test/test_fib.xtal");
		load("../test/test_calc.xtal");
		load("../test/test_for.xtal");
		load("../test/test_if.xtal");
		load("../test/test_nested_loops.xtal");
		load("../test/test_assign.xtal");
		load("../test/test_op_assign.xtal");
		load("../test/test_inc.xtal");
		load("../test/test_toplevel.xtal");
		load("../test/test_serialize.xtal");
		
		//*/

	}catch(AnyPtr e){
		fprintf(stderr, "%s\n", e->to_s()->c_str());
	}

	vmachine().get()->print_info();

	uninitialize();

	return 0;
}
