
#include "xtal.h"

#ifndef XTAL_NO_PARSER

#include "xtal_codebuilder.h"
#include "xtal_expr.h"
#include "xtal_parser.h"
#include "xtal_constant.h"
#include "xtal_funimpl.h"
#include "xtal_codeimpl.h"
#include "xtal_streamimpl.h"
#include "xtal_fun.h"
#include "xtal_vmachineimpl.h"


namespace xtal{

CodeBuilder::CodeBuilder(){

}

CodeBuilder::~CodeBuilder(){

}

Fun CodeBuilder::compile(const Stream& stream, const String& source_file_name){

	result_ = Code();
	
	p_ = result_.impl();
	p_->source_file_name_ = source_file_name;

	lines_.push(1);
	fun_frame_begin(true, 0, 0, 0, false);
	Stmt* ep = parser_.parse(stream, source_file_name);
	com_ = parser_.common();
	p_->symbol_table_ = com_->ident_table;
	p_->value_table_ = com_->value_table;
	
	if(ep){
		compile(ep);
	}

	parser_.release();
	Code ret = result_;
	result_ = null;

	if(com_->errors.size()==0){
		Fun p(null, null, ret, &ret.impl()->xfun_core_table_[0]);
		p.set_object_name("<TopLevel>", 1, null);
		return p;
	}else{
		return null;
	}
}

void CodeBuilder::interactive_compile(){
	result_ = Code();
	p_ = result_.impl();
	p_->source_file_name_ = "<ix>";

	lines_.push(1);
	fun_frame_begin(true, 0, 0, 0, false);
	Fun fun(null, null, result_, &p_->xfun_core_table_[0]);
	fun.set_object_name("<TopLevel>", 1, null);

	Stream stream;
	new(stream) InteractiveStreamImpl();
	parser_.begin_interactive_parsing(stream);

	int_t pc_pos = 0;
	
	while(true){
		Stmt* ep = parser_.interactive_parse();
		((InteractiveStreamImpl*)stream.impl())->set_continue_stmt(false);
		com_ = parser_.common();

		p_->symbol_table_ = com_->ident_table;
		p_->value_table_ = com_->value_table;
		com_->register_ident("toplevel");
		
		if(ep && com_->errors.empty()){
			compile(ep);
		}else{
			if(com_->errors.empty()){
				break;
			}
			com_->error(1, Xt("Xtal Compile Error 1001"));
		}
	
		if(com_->errors.size()==0){
			process_labels();
			put_code_u8(CODE_RETURN_0);
			put_code_u8(CODE_THROW);

			fun.set_core(&p_->xfun_core_table_[0]);
			XTAL_TRY{
				vmachine().impl()->execute(fun, &p_->code_[pc_pos]);
			}XTAL_CATCH(e){
				printf("%s\n", e.to_s().c_str());
			}
			
			p_->code_.pop_back();
			pc_pos = p_->code_.size();

		}else{
			printf("Error: %s\n", errors().at(0).to_s().c_str());
			com_->errors.clear();
		}
	}
}


Array CodeBuilder::errors(){
	return com_->errors;
}

ID CodeBuilder::to_id(int_t ident){
	return (ID&)com_->ident_table[ident];
}

int_t CodeBuilder::lookup_variable(int_t key){
	for(size_t i = 0, last = variables_.size(); i<last; ++i){
		if(variables_[i]==key){
			return i;
		}
	}
	return -1;
}

bool CodeBuilder::variable_on_heap(int_t pos){
	if(debug::is_enabled()){
		return true;
	}

	for(size_t i = 0, last = scopes_.size(); i<last; ++i){
		if(pos<scopes_[i].variable_size){
			return scopes_[i].on_heap;
		}
		pos -= scopes_[i].variable_size;
	}
	return false;
}

bool CodeBuilder::put_set_local_code(int_t var){
	int_t id = lookup_variable(var);
	if(id>=0){
		bool on_heap = variable_on_heap(id);
 
		if(on_heap){
			if(id<=0xff){
				put_code_u8(CODE_SET_LOCAL);
				put_code_u8(id);
			}else{
				put_code_u8(CODE_SET_LOCAL_W);
				put_code_u16(id);
			}		
		}else if(id>=4){
			if(id<=0xff){
				put_code_u8(CODE_SET_LOCAL_NOT_ON_HEAP);
				put_code_u8(id);
			}else{
				put_code_u8(CODE_SET_LOCAL_W);
				put_code_u16(id);
			}
		}
		else if(id == 0){ put_code_u8(CODE_SET_LOCAL_0); }
		else if(id == 1){ put_code_u8(CODE_SET_LOCAL_1); }
		else if(id == 2){ put_code_u8(CODE_SET_LOCAL_2); }
		else if(id == 3){ put_code_u8(CODE_SET_LOCAL_3); }
		return true;
	}else{
		//com_->error(line(), Xt("定義されていない変数%sに代入しようとしました")(to_id(var)));
		put_code_u8(CODE_SET_GLOBAL);
		put_code_u16(var);
		return false;
	}
}

void CodeBuilder::put_define_local_code(int_t var){
	int_t id = lookup_variable(var);
	if(id>=0){
		put_set_local_code(var);
	}else{
		put_code_u8(CODE_DEFINE_GLOBAL);
		put_code_u16(var);
	}
}

bool CodeBuilder::put_local_code(int_t var){
	int_t id = lookup_variable(var);
	if(id>=0){
		bool on_heap = variable_on_heap(id);

		if(on_heap){
			if(id<=0xff){
				put_code_u8(CODE_LOCAL);
				put_code_u8(id);
			}else{
				put_code_u8(CODE_LOCAL_W);
				put_code_u16(id);
			}		
		}else if(id>=4){
			if(id<=0xff){
				put_code_u8(CODE_LOCAL_NOT_ON_HEAP);
				put_code_u8(id);
			}else{
				put_code_u8(CODE_LOCAL_W);
				put_code_u16(id);
			}
		}
		else if(id == 0){ put_code_u8(CODE_LOCAL_0); }
		else if(id == 1){ put_code_u8(CODE_LOCAL_1); }
		else if(id == 2){ put_code_u8(CODE_LOCAL_2); }
		else if(id == 3){ put_code_u8(CODE_LOCAL_3); }
		return true;
	}else{
		put_code_u8(CODE_GLOBAL);
		put_code_u16(var);
		return false;
	}
}

void CodeBuilder::put_send_code(int_t var, Expr* pvar, int_t need_result_count, bool discard, bool tail, bool if_defined){
	if(pvar){
		compile(pvar);
	}	
	
	if(if_defined){
		put_code_u8(CODE_SEND_IF_DEFINED);
	}else{
		put_code_u8(CODE_SEND);
	}
	if(pvar){
		put_code_u16(0);
	}else{
		put_code_u16(var);
	}
	put_code_u8(0);
	put_code_u8(0);
	put_code_u8(need_result_count);
	put_code_u8(0 | (tail ? (1<<1) : 0) | (discard ? RESULT_DISCARD : 0));
}

void CodeBuilder::put_set_send_code(int_t var, Expr* pvar, bool if_defined){
	if(pvar){
		ExprBuilder& e = *parser_.expr_builder();
		compile(e.bin(CODE_CAT, e.string(com_->register_value("set_")), pvar));
	}	
	
	if(if_defined){
		put_code_u8(CODE_SEND_IF_DEFINED);
	}else{
		put_code_u8(CODE_SEND);
	}

	if(pvar){
		put_code_u16(0);
	}else{
		put_code_u16(com_->register_ident(String("set_", 4, to_id(var).c_str(), to_id(var).size())));
	}
	put_code_u8(1);
	put_code_u8(0);
	put_code_u8(0);
	put_code_u8(0 | 0);
}

void CodeBuilder::put_member_code(int_t var, Expr* pvar, bool if_defined){
	if(pvar){
		compile(pvar);
	}
	
	if(if_defined){
		put_code_u8(CODE_MEMBER_IF_DEFINED);
	}else{
		put_code_u8(CODE_MEMBER);
	}

	if(pvar){
		put_code_u16(0);
	}else{
		put_code_u16(var);
	}
}

void CodeBuilder::put_define_member_code(int_t var, Expr* pvar){
	if(pvar){
		compile(pvar);
	}

	put_code_u8(CODE_DEFINE_MEMBER);

	if(pvar){
		put_code_u16(0);
	}else{
		put_code_u16(var);
	}
}

int_t CodeBuilder::lookup_instance_variable(int_t key){
	if(!class_scopes_.empty()){
		int_t i = 0;
		for(TPairList<int_t, Expr*>::Node* p=class_scopes_.top()->inst_vars.head; p; p=p->next){
			if(p->key==key){
				return i; 
			}
			i++;
		}
	}
	com_->error(line(), Xt("Xtal Compile Error 1023")(Named("name", String("_").cat(to_id(key)))));
	return 0;
}

void CodeBuilder::put_set_instance_variable_code(int_t var){
	put_code_u8(CODE_SET_INSTANCE_VARIABLE);
	put_code_u8(lookup_instance_variable(var));
	put_code_u16(class_scopes_.empty() ? 0 : class_scopes_.top()->frame_number);
}

void CodeBuilder::put_instance_variable_code(int_t var){
	put_code_u8(CODE_INSTANCE_VARIABLE);
	put_code_u8(lookup_instance_variable(var));
	put_code_u16(class_scopes_.empty() ? 0 : class_scopes_.top()->frame_number);
}

int_t CodeBuilder::reserve_label(){
	fun_frames_.top().labels.resize(fun_frames_.top().labels.size()+1);
	return fun_frames_.top().labels.size()-1;
}

void CodeBuilder::set_label(int_t labelno){
	fun_frames_.top().labels[labelno].pos = code_size();
}

void CodeBuilder::put_jump_code_nocode(int_t oppos, int_t labelno){
	FunFrame::Label::From f;
	f.line = lines_.top();
	f.set_pos = oppos;
	f.pos = code_size();
	put_code_i16(0x7fff);
	fun_frames_.top().labels[labelno].froms.push_back(f);
}

void CodeBuilder::put_jump_code(int_t code, int_t labelno){
	int_t oppos = code_size();
	put_code_u8(code);
	put_jump_code_nocode(oppos, labelno);
}

void CodeBuilder::process_labels(){
	for(size_t i = 0; i<fun_frames_.top().labels.size(); ++i){
		FunFrame::Label &l = fun_frames_.top().labels[i];
		for(size_t j = 0; j<l.froms.size(); ++j){
			FunFrame::Label::From &f = l.froms[j];
			set_code_i16(f.pos, l.pos-f.set_pos);
		}
	}
	fun_frames_.top().labels.clear();
}

void CodeBuilder::break_off(int_t n){
	for(uint_t scope_count = scopes_.size(); scope_count!=(uint_t)n; scope_count--){
		for(uint_t k = 0; k<fun_frame().finallys.size(); ++k){
			if((uint_t)fun_frame().finallys[k].frame_count==scope_count){
				int_t label = reserve_label();
				put_jump_code(CODE_PUSH_GOTO, label);
				put_code_u8(CODE_TRY_END);
				set_label(label);
			}
		}
		if(scopes_[scopes_.size()-scope_count].type!=SCOPE)
			put_code_u8(CODE_BLOCK_END);
	}
}

void CodeBuilder::put_if_code(Expr* e, int_t label_if, int_t label_if2){
	BinCompExpr* e2 = expr_cast<BinCompExpr>(e);
	if(e2 && CODE_EQ<=e2->code && e2->code<=CODE_GE){
		if(expr_cast<BinCompExpr>(e2->lhs)){
			com_->error(line(), Xt("Xtal Compile Error 1025"));
		}
		if(expr_cast<BinCompExpr>(e2->rhs)){
			com_->error(line(), Xt("Xtal Compile Error 1025"));
		}
		compile(e2->lhs);
		compile(e2->rhs);
		put_jump_code(CODE_EQ_IF+e2->code-CODE_EQ, label_if);
		if(e2->code==CODE_NE || e2->code==CODE_LE || e2->code==CODE_GE){
			put_code_u8(CODE_NOT);
		}
		put_jump_code(CODE_IF, label_if2);
	}else{
		compile(e);
		put_jump_code(CODE_IF, label_if);
	}
}

void CodeBuilder::push_loop(int break_labelno, int continue_labelno, int_t name, bool have_label){
	FunFrame::Loop loop;
	loop.break_label = break_labelno;
	loop.continue_label = continue_labelno;
	loop.name = name;
	loop.frame_count = scopes_.size();
	loop.have_label = have_label;
	fun_frames_.top().loops.push(loop);
}

void CodeBuilder::pop_loop(){
	fun_frames_.top().loops.pop();
}

void CodeBuilder::set_on_heap_flag(){
	for(int_t i = 0; i<(int_t)scopes_.size(); ++i){
		scopes_[i].on_heap = true;	
	}
}

void CodeBuilder::block_begin(int_t type, int_t kind, TList<int_t>& vars, bool on_heap, int_t mixins){
	Scope s;
	s.variable_size = 0;
	s.type = type;
	s.kind = kind;
	s.on_heap = on_heap;
	s.mixins = mixins;
	s.frame_core_num = p_->frame_core_table_.size();

	// もしヒープに乗るスコープが来たなら、今まで積んだ全てのスコープをヒープに乗せるフラグを立てる
	if(on_heap || debug::is_enabled()){
		set_on_heap_flag();
	}
	scopes_.push(s);

	p_->frame_core_table_.push_back(FrameCore());
	p_->frame_core_table_.back().kind = scopes_.top().kind;
	p_->frame_core_table_.back().variable_symbol_offset = p_->symbol_table_.size();
	p_->frame_core_table_.back().line_number = lines_.top();

	for(TList<int_t>::Node* p = vars.head; p; p = p->next){
		variables_.push(p->value);
		scopes_.top().variable_size++;
		p_->frame_core_table_.back().variable_size++;
		p_->symbol_table_.push_back(to_id(p->value));
	}

	if(scopes_.top().type==FRAME){
		put_code_u8(CODE_CLASS_BEGIN);
		put_code_u16(s.frame_core_num);
		if(scopes_.top().kind==KIND_CLASS){
			put_code_u8(scopes_.top().mixins);
		}
	}else if(scopes_.top().type==FUN){
		
	}else{
		if(scopes_.top().variable_size){
			put_code_u8(CODE_BLOCK_BEGIN);
			put_code_u16(s.frame_core_num);
		}else{
			scopes_.top().type = SCOPE;
		}
	}
}

void CodeBuilder::block_end(){
	if(scopes_.top().type==FRAME){
		variables_.downsize(scopes_.top().variable_size);
		put_code_u8(CODE_CLASS_END);
	}else if(scopes_.top().type==FUN){
		variables_.downsize(scopes_.top().variable_size);
	}else{
		if(scopes_.top().variable_size){
			variables_.downsize(scopes_.top().variable_size);
			if(scopes_.top().on_heap || debug::is_enabled()){
				put_code_u8(CODE_BLOCK_END);
			}else{
				put_code_u8(CODE_BLOCK_END_NOT_ON_HEAP);
			}
		}
	}
	scopes_.pop();
}

void CodeBuilder::put_code_u8(int_t val){
	p_->code_.push_back(val);
}

void CodeBuilder::put_code_i8(int_t val){
	p_->code_.push_back(val);
}
		
void CodeBuilder::put_code_u16(int_t val){
	p_->code_.push_back((val>>8)&0xff);
	p_->code_.push_back((val>>0)&0xff);
}

void CodeBuilder::put_code_i16(int_t val){
	p_->code_.push_back((val>>8)&0xff);
	p_->code_.push_back((val>>0)&0xff);
}

void CodeBuilder::set_code_u8(int_t i, int_t val){
	XTAL_ASSERT((uint_t)i<p_->code_.size());
	p_->code_[i]=val;
}

void CodeBuilder::set_code_i8(int_t i, int_t val){
	XTAL_ASSERT((uint_t)i<p_->code_.size());
	p_->code_[i]=val;
}

void CodeBuilder::set_code_u16(int_t i, int_t val){
	XTAL_ASSERT((uint_t)i<p_->code_.size());
	p_->code_[i]=(val>>8)&0xff;
	p_->code_[i+1]=(val>>0)&0xff;
}

void CodeBuilder::set_code_i16(int_t i, int_t val){
	XTAL_ASSERT((uint_t)i<p_->code_.size());
	p_->code_[i]=(val>>8)&0xff;
	p_->code_[i+1]=(val>>0)&0xff;
}

int_t CodeBuilder::code_size(){
	return p_->code_.size();
}

int_t CodeBuilder::fun_frame_begin(bool have_args, int_t offset, unsigned char min_param_count, unsigned char max_param_count, bool extra_comma){
	FunFrame& f = fun_frames_.push();	
	f.used_args_object = false;
	f.labels.clear();
	f.loops.clear();
	f.finallys.clear();
	f.frame_count = scopes_.size();

	p_->xfun_core_table_.push_back(FunCore());
	p_->xfun_core_table_.back().variable_symbol_offset = p_->symbol_table_.size();
	p_->xfun_core_table_.back().pc = code_size()+offset;
	p_->xfun_core_table_.back().line_number = lines_.top();
	p_->xfun_core_table_.back().min_param_count = min_param_count;
	p_->xfun_core_table_.back().max_param_count = max_param_count;
	p_->xfun_core_table_.back().used_args_object = have_args;
	p_->xfun_core_table_.back().extra_comma = extra_comma;
	fun_frame().used_args_object = have_args;
		
	if(debug::is_enabled()){
		set_on_heap_flag();
	}

	return p_->xfun_core_table_.size()-1;
}

void CodeBuilder::register_param(int_t name){
	p_->xfun_core_table_.back().variable_size++;
	p_->symbol_table_.push_back(to_id(name));
}

void CodeBuilder::fun_frame_end(){
	process_labels();
	fun_frames_.downsize(1);
}

CodeBuilder::FunFrame &CodeBuilder::fun_frame(){
	return fun_frames_.top();
}

#define XTAL_EXPR_CASE(KEY) break; case KEY::TYPE: if(KEY* e = (KEY*)ex)if(e)

void CodeBuilder::compile(Expr* ex, int_t need_result_count, bool discard){

	int_t result_count = 1;

	if(!ex){
		put_code_u8(CODE_ADJUST_RESULT);
		put_code_u8(0);
		put_code_u8(need_result_count);
		put_code_u8(discard ? RESULT_DISCARD : 0);
		return;
	}

	/*
	if(debug::is_enabled() && lines_.top()!=ex->line){
		put_code_u8(CODE_BREAKPOINT);
		put_code_u8(BREAKPOINT_LINE);
	}
	*/
	
	lines_.push(ex->line);
	p_->set_line_number_info(ex->line);

	switch(ex->type){

		XTAL_NODEFAULT;

		XTAL_EXPR_CASE(Expr){
			(void)e;
		}

		XTAL_EXPR_CASE(PseudoVariableExpr){
			put_code_u8(e->code);
		}

		XTAL_EXPR_CASE(CalleeExpr){
			(void)e;
			put_code_u8(CODE_PUSH_CALLEE);
		}

		XTAL_EXPR_CASE(ArgsExpr){
			(void)e;
			put_local_code(com_->register_ident(Xid(__ARGS__)));
		}

		XTAL_EXPR_CASE(IntExpr){
			if(e->value==0){
				put_code_u8(CODE_PUSH_INT_0);
			}else if(e->value==1){
				put_code_u8(CODE_PUSH_INT_1);
			}else if(e->value==2){
				put_code_u8(CODE_PUSH_INT_2);
			}else if(e->value==3){
				put_code_u8(CODE_PUSH_INT_3);
			}else if(e->value==4){
				put_code_u8(CODE_PUSH_INT_4);
			}else if(e->value==5){
				put_code_u8(CODE_PUSH_INT_5);
			}else if(e->value==(i8)e->value){
				put_code_u8(CODE_PUSH_INT_1BYTE);
				put_code_u8(e->value);
			}else if(e->value==(i16)e->value){
				put_code_u8(CODE_PUSH_INT_2BYTE);
				put_code_u16(e->value);
			}else{
				put_code_u8(CODE_GET_VALUE);
				put_code_u16(com_->register_value(e->value));
			}
		}

		XTAL_EXPR_CASE(FloatExpr){
			if(e->value==0){
				put_code_u8(CODE_PUSH_FLOAT_0);
			}else if(e->value==0.25f){
				put_code_u8(CODE_PUSH_FLOAT_0_25);
			}else if(e->value==0.5f){
				put_code_u8(CODE_PUSH_FLOAT_0_5);
			}else if(e->value==1){
				put_code_u8(CODE_PUSH_FLOAT_1);
			}else if(e->value==2){
				put_code_u8(CODE_PUSH_FLOAT_2);
			}else if(e->value==3){
				put_code_u8(CODE_PUSH_FLOAT_3);
			}else{
				put_code_u8(CODE_GET_VALUE);
				put_code_u16(com_->register_value(e->value));
			}
		}

		XTAL_EXPR_CASE(StringExpr){
			if(e->kind==KIND_TEXT){
				put_code_u8(CODE_GET_VALUE);
				put_code_u16(com_->register_value(get_text(cast<String>(com_->value_table[e->value]).c_str())));
			}else if(e->kind==KIND_FORMAT){
				put_code_u8(CODE_GET_VALUE);
				put_code_u16(com_->register_value(format(cast<String>(com_->value_table[e->value]).c_str())));
			}else{
				put_code_u8(CODE_GET_VALUE);
				put_code_u16(com_->register_value(com_->value_table[e->value]));
			}
		}

		XTAL_EXPR_CASE(ArrayExpr){
			int_t count = 0;
			TList<Expr*>::Node* p;
			for(p = e->values.head; p; p = p->next){
				compile(p->value);
				count++;
				if(count>32){
					break;
				}
			}
			put_code_u8(CODE_PUSH_ARRAY);
			put_code_u8(count);
			for(; p; p = p->next){
				compile(p->value);
				put_code_u8(CODE_ARRAY_APPEND);				
			}
		}

		XTAL_EXPR_CASE(MapExpr){
			int_t count = 0;
			TPairList<Expr*, Expr*>::Node* p;
			for(p = e->values.head; p; p = p->next){
				compile(p->key);
				compile(p->value);
				count++;
				if(count>16){
					break;
				}
			}
			put_code_u8(CODE_PUSH_MAP);	
			put_code_u8(count);
			for(; p; p = p->next){
				compile(p->key);
				compile(p->value);
				put_code_u8(CODE_MAP_APPEND);				
			}
		}

		XTAL_EXPR_CASE(BinExpr){
			if(expr_cast<BinCompExpr>(e->lhs)){
				com_->error(line(), Xt("Xtal Compile Error 1013"));
			}
			if(expr_cast<BinCompExpr>(e->rhs)){
				com_->error(line(), Xt("Xtal Compile Error 1013"));
			}
			
			compile(e->lhs);
			compile(e->rhs);
			put_code_u8(e->code);
		}

		XTAL_EXPR_CASE(BinCompExpr){
			if(expr_cast<BinCompExpr>(e->lhs)){
				com_->error(line(), Xt("Xtal Compile Error 1025"));
			}
			if(expr_cast<BinCompExpr>(e->rhs)){
				com_->error(line(), Xt("Xtal Compile Error 1025"));
			}

			compile(e->lhs);
			compile(e->rhs);
			put_code_u8(e->code);
			if(e->code==CODE_GE || e->code==CODE_LE || e->code==CODE_NE){
				put_code_u8(CODE_NOT);
			}
		}

		XTAL_EXPR_CASE(PopExpr){
			
		}

		XTAL_EXPR_CASE(TerExpr){
			int_t label_if = reserve_label();
			int_t label_end = reserve_label();
			compile(e->first);
			put_jump_code(CODE_IF, label_if);
			compile(e->second);
			put_jump_code(CODE_GOTO, label_end);
			set_label(label_if);
			compile(e->third);
			set_label(label_end);
		}
		
		XTAL_EXPR_CASE(AtExpr){
			compile(e->lhs);
			compile(e->index);
			put_code_u8(CODE_AT);
		}

		XTAL_EXPR_CASE(AndAndExpr){
			int_t label_if = reserve_label();
			compile(e->lhs);
			put_code_u8(CODE_DUP);
			put_jump_code(CODE_IF, label_if);
			put_code_u8(CODE_POP);
			compile(e->rhs);
			set_label(label_if);
		}

		XTAL_EXPR_CASE(OrOrExpr){
			int_t label_if = reserve_label();
			compile(e->lhs);
			put_code_u8(CODE_DUP);
			put_jump_code(CODE_UNLESS, label_if);
			put_code_u8(CODE_POP);
			compile(e->rhs);
			set_label(label_if);
		}

		XTAL_EXPR_CASE(UnaExpr){
			compile(e->expr);
			put_code_u8(e->code);
		}

		XTAL_EXPR_CASE(OnceExpr){
			int_t label_end = reserve_label();
			
			put_jump_code(CODE_ONCE, label_end);
			
			int_t num = com_->append_value(nop);
			put_code_u16(num);
			
			compile(e->expr);
			put_code_u8(CODE_DUP);
			
			put_code_u8(CODE_SET_VALUE);
			put_code_u16(num);
			
			set_label(label_end);	
		}

		XTAL_EXPR_CASE(SendExpr){
			compile(e->lhs);
			put_send_code(e->var, e->pvar, need_result_count, e->discard, e->tail, e->if_defined);
			result_count = need_result_count;
		}

		XTAL_EXPR_CASE(CallExpr){
			
			for(TList<Expr*>::Node* p = e->ordered.head; p; p = p->next){ 
				if(p->next){
					compile(p->value);
				}else{
					if(!expr_cast<ArgsExpr>(p->value)){
						compile(p->value);
					}
				}
			}

			for(TPairList<int_t, Expr*>::Node* p = e->named.head; p; p = p->next){ 
				put_code_u8(CODE_GET_VALUE);
				put_code_u16(com_->register_value(to_id(p->key)));
				compile(p->value);
			}

			if(SendExpr* e2 = expr_cast<SendExpr>(e->expr)){ // a.b(); メッセージ送信式
				compile(e2->lhs);

				if(e2->pvar){
					compile(e2->pvar);
				}

				if(e2->if_defined){
					put_code_u8(CODE_SEND_IF_DEFINED);
				}else{
					put_code_u8(CODE_SEND);
				}

				if(e2->pvar){
					put_code_u16(0);
				}else{
					put_code_u16(e2->var);
				}
			}else if(expr_cast<CalleeExpr>(e->expr)){ //recall(); 再帰呼び出しだ
				put_code_u8(CODE_CALLEE);
			}else{
				compile(e->expr);
				put_code_u8(CODE_CALL);
			}

			if(expr_cast<ArgsExpr>(e->ordered.tail ? (Expr*)e->ordered.tail->value : (Expr*)0)){
				put_code_u8(e->ordered.size-1);
				put_code_u8(e->named.size);
				put_code_u8(need_result_count);
				put_code_u8(1 | (e->tail ? (1<<1) : 0) | (e->discard ? RESULT_DISCARD : 0));
			}else{
				put_code_u8(e->ordered.size);
				put_code_u8(e->named.size);
				put_code_u8(need_result_count);
				put_code_u8(0 | (e->tail ? (1<<1) : 0) | (e->discard ? RESULT_DISCARD : 0));
			}			
			result_count = need_result_count;
		}

		XTAL_EXPR_CASE(FunExpr){

			int_t minv = -1, maxv = 0;
			for(TPairList<int_t, Expr*>::Node* p = e->params.head; p; p = p->next){
				if(p->value){
					if(minv!=-1){
						
					}else{
						minv = maxv;
					}
				}else{
					if(minv!=-1){
						com_->error(line(), Xt("Xtal Compile Error 1001"));
					}
				}
				maxv++;
			}
			if(minv==-1){
				minv = maxv;
			}

			int_t n = fun_frame_begin(e->have_args, 6, minv, maxv, e->extra_comma);
			
			for(TPairList<int_t, Expr*>::Node* p = e->params.head; p; p = p->next){ 
				register_param(p->key);
			}

			int_t fun_end_label = reserve_label();

			int_t oppos = code_size();
			put_code_u8(CODE_PUSH_FUN);
			put_code_u8(e->kind);
			put_code_u16(n);
			put_jump_code_nocode(oppos, fun_end_label);
			
			if(debug::is_enabled()){
				put_code_u8(CODE_BREAKPOINT);
				put_code_u8(BREAKPOINT_CALL);
			}

			block_begin(FUN, 0, e->vars, e->on_heap);{
				for(TPairList<int_t, Expr*>::Node* p = e->params.head; p; p = p->next){
					// デフォルト値を持つ
					if(p->value){
						
						int_t id = lookup_variable(p->key);
						int_t label = reserve_label();
						
						put_jump_code(CODE_IF_ARG_IS_NULL, label);
						put_code_u8(id);						
						compile(p->value);
						put_set_local_code(p->key);
						
						set_label(label);
					}
				}
				compile(e->stmt);
				break_off(fun_frame().frame_count+1);
				if(debug::is_enabled()){
					put_code_u8(CODE_BREAKPOINT);
					put_code_u8(BREAKPOINT_RETURN);
				}
				put_code_u8(CODE_RETURN_0);
			}block_end();

			set_label(fun_end_label);
			fun_frame_end();
		}

		XTAL_EXPR_CASE(LocalExpr){
			put_local_code(e->var);
		}

		XTAL_EXPR_CASE(InstanceVariableExpr){
			put_instance_variable_code(e->var);
		}

		XTAL_EXPR_CASE(MemberExpr){
			compile(e->lhs);
			put_member_code(e->var, e->pvar, e->if_defined);
		}

		XTAL_EXPR_CASE(FrameExpr){
			block_begin(FRAME, e->kind, e->vars, e->on_heap);{
				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile(p->value);
				}
			}block_end();
		}	

		XTAL_EXPR_CASE(ClassExpr){

			for(TList<Expr*>::Node* p = e->mixins.head; p; p = p->next){
				compile(p->value);
			}

			block_begin(FRAME, e->kind, e->vars, e->on_heap, e->mixins.size);{
				class_scopes_.push(e);
				p_->frame_core_table_.back().instance_variable_symbol_offset = 0;
				p_->frame_core_table_.back().instance_variable_size = e->inst_vars.size;
				class_scopes_.top()->frame_number = p_->frame_core_table_.size()-1;

				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile(p->value);
				}
				class_scopes_.downsize(1);
			}block_end();
		}
	}
	
	p_->set_line_number_info(ex->line);
	lines_.pop();

	if(need_result_count!=result_count){
		put_code_u8(CODE_ADJUST_RESULT);
		put_code_u8(result_count);
		put_code_u8(need_result_count);
		put_code_u8(discard ? RESULT_DISCARD : 0);
	}
}

void CodeBuilder::compile(Stmt* ex){

	if(!ex)
		return;

	if(debug::is_enabled() && lines_.top()!=ex->line){
		put_code_u8(CODE_BREAKPOINT);
		put_code_u8(BREAKPOINT_LINE);
	}

	lines_.push(ex->line);
	p_->set_line_number_info(ex->line);

	switch(ex->type){

		XTAL_NODEFAULT;

		XTAL_EXPR_CASE(ExprStmt){
			compile(e->expr, 0);
		}

		XTAL_EXPR_CASE(PushStmt){
			compile(e->expr);
		}
		
		XTAL_EXPR_CASE(DefineStmt){
			if(LocalExpr* p = expr_cast<LocalExpr>(e->lhs)){
				compile(e->rhs);
				
				if(expr_cast<FunExpr>(e->rhs) || expr_cast<ClassExpr>(e->rhs)){
					put_code_u8(CODE_SET_NAME);
					put_code_u16(p->var);
				}

				put_define_local_code(p->var);
			}else if(MemberExpr* p = expr_cast<MemberExpr>(e->lhs)){
				compile(p->lhs);
				compile(e->rhs);

				if(p->var!=0 && (expr_cast<FunExpr>(e->rhs) || expr_cast<ClassExpr>(e->rhs))){
					put_code_u8(CODE_SET_NAME);
					put_code_u16(p->var);
				}

				put_define_member_code(p->var, p->pvar);
			}else{
				com_->error(line(), Xt("Xtal Compile Error 1012"));
			}
		}
		
		XTAL_EXPR_CASE(AssignStmt){
			if(LocalExpr* p = expr_cast<LocalExpr>(e->lhs)){
				compile(e->rhs);
				put_set_local_code(p->var);
			}else if(InstanceVariableExpr* p = expr_cast<InstanceVariableExpr>(e->lhs)){
				compile(e->rhs);
				put_set_instance_variable_code(p->var);
			}else if(SendExpr* p = expr_cast<SendExpr>(e->lhs)){
				compile(e->rhs);
				compile(p->lhs);
				put_set_send_code(p->var, p->pvar, p->if_defined);
			}else if(AtExpr* p = expr_cast<AtExpr>(e->lhs)){
				compile(e->rhs);
				compile(p->lhs);
				compile(p->index);
				put_code_u8(CODE_SET_AT);	
			}else{
				com_->error(line(), Xt("Xtal Compile Error 1012"));
			}
		}

		XTAL_EXPR_CASE(OpAssignStmt){
			if(LocalExpr* p = expr_cast<LocalExpr>(e->lhs)){
				put_local_code(p->var);
				compile(e->rhs);
				put_code_u8(e->code);
				put_set_local_code(p->var);
			}else if(InstanceVariableExpr* p = expr_cast<InstanceVariableExpr>(e->lhs)){
				put_instance_variable_code(p->var);
				compile(e->rhs);
				put_code_u8(e->code);
				put_set_instance_variable_code(p->var);
			}else if(SendExpr* p = expr_cast<SendExpr>(e->lhs)){
				compile(p->lhs);
				put_code_u8(CODE_DUP);
				put_send_code(p->var, p->pvar, 1, p->discard, p->tail, p->if_defined);
				compile(e->rhs);
				put_code_u8(e->code);
				put_code_u8(CODE_INSERT_1);
				put_set_send_code(p->var, p->pvar, p->if_defined);
			}else if(AtExpr* p = expr_cast<AtExpr>(e->lhs)){
				compile(p->lhs);
				put_code_u8(CODE_DUP);
				compile(p->index);
				put_code_u8(CODE_DUP);
				put_code_u8(CODE_INSERT_2);
				put_code_u8(CODE_AT);
				compile(e->rhs);
				put_code_u8(e->code);
				put_code_u8(CODE_INSERT_2);
				put_code_u8(CODE_SET_AT);	
			}
		}

		XTAL_EXPR_CASE(IncStmt){
			if(LocalExpr* p = expr_cast<LocalExpr>(e->lhs)){
				int_t id = lookup_variable(p->var);
				if(id>=0){
					bool on_heap = variable_on_heap(id);
					if(on_heap){
						if(id<=0xff){
							put_code_u8(CODE_LOCAL);
							put_code_u8(id);
						}else{
							put_code_u8(CODE_LOCAL_W);
							put_code_u16(id);
						}
						put_code_u8(e->code);
						put_set_local_code(p->var);
					}else{
						if(id<=0xff){
							if(e->code == CODE_INC){
								put_code_u8(CODE_LOCAL_NOT_ON_HEAP_INC);
							}else{
								put_code_u8(CODE_LOCAL_NOT_ON_HEAP_DEC);
							}
							put_code_u8(id);
							put_code_u8(CODE_SET_LOCAL_NOT_ON_HEAP);
							put_code_u8(id);
						}else{
							put_code_u8(CODE_LOCAL_W);
							put_code_u16(id);
							put_code_u8(e->code);
							put_set_local_code(p->var);
						}
					}
				}else{
					put_code_u8(CODE_GLOBAL);
					put_code_u16(p->var);
					put_code_u8(e->code);
					put_set_local_code(p->var);
				}

			}else if(InstanceVariableExpr* p = expr_cast<InstanceVariableExpr>(e->lhs)){
				put_instance_variable_code(p->var);
				put_code_u8(e->code);
				put_set_instance_variable_code(p->var);
			}else if(SendExpr* p = expr_cast<SendExpr>(e->lhs)){
				compile(p->lhs);
				put_code_u8(CODE_DUP);
				put_send_code(p->var, p->pvar, 1, p->discard, p->tail, p->if_defined);
				put_code_u8(e->code);
				put_code_u8(CODE_INSERT_1);
				put_set_send_code(p->var, p->pvar, p->if_defined);
			}else if(AtExpr* p = expr_cast<AtExpr>(e->lhs)){
				compile(p->lhs);
				put_code_u8(CODE_DUP);
				compile(p->index);
				put_code_u8(CODE_DUP);
				put_code_u8(CODE_INSERT_2);
				put_code_u8(CODE_AT);
				put_code_u8(e->code);
				put_code_u8(CODE_INSERT_2);
				put_code_u8(CODE_SET_AT);		
			}
		}

		XTAL_EXPR_CASE(UnaStmt){
			compile(e->expr);
			put_code_u8(e->code);
		}
		
		XTAL_EXPR_CASE(YieldStmt){
			for(TList<Expr*>::Node* p = e->exprs.head; p; p = p->next){
				compile(p->value);
			}
			put_code_u8(CODE_YIELD);
			put_code_u8(e->exprs.size);
		}

		XTAL_EXPR_CASE(ReturnStmt){

			bool have_finally = false;
			for(uint_t scope_count = scopes_.size(); scope_count!=(uint_t)fun_frame().frame_count+1; scope_count--){
				for(uint_t k = 0; k<(uint_t)fun_frame().finallys.size(); ++k){
					if((uint_t)fun_frame().finallys[k].frame_count==scope_count){
						have_finally = true;
					}
				}
			}

			if(!have_finally && e->exprs.size==1){
				if(CallExpr* ce = expr_cast<CallExpr>(e->exprs.head->value)){
					ce->tail = true;
					compile(ce);
					break;
				}else if(SendExpr* se = expr_cast<SendExpr>(e->exprs.head->value)){
					se->tail = true;
					compile(se);
					break;
				}
			}

			for(TList<Expr*>::Node* p = e->exprs.head; p; p = p->next){
				compile(p->value);
			}
			
			{
				
				break_off(fun_frame().frame_count+1);

				if(debug::is_enabled()){
					put_code_u8(CODE_BREAKPOINT);
					put_code_u8(BREAKPOINT_RETURN);
				}

				if(e->exprs.size==0){
					put_code_u8(CODE_RETURN_0);
				}else if(e->exprs.size==1){
					put_code_u8(CODE_RETURN_1);
				}else if(e->exprs.size==2){
					put_code_u8(CODE_RETURN_2);
				}else{
					put_code_u8(CODE_RETURN_N);
					put_code_u8(e->exprs.size);
					if(e->exprs.size>=256){
						com_->error(line(), Xt("Xtal Compile Error 1022"));
					}
				}	
			}
		}

		XTAL_EXPR_CASE(AssertStmt){
							
			if(e->exprs.size==1){
				compile(e->exprs.head->value);
				put_code_u8(CODE_GET_VALUE);
				put_code_u16(0);		
				put_code_u8(CODE_GET_VALUE);
				put_code_u16(0);		
			}else if(e->exprs.size==2){
				compile(e->exprs.head->value);
				compile(e->exprs.head->next->value);
				put_code_u8(CODE_GET_VALUE);
				put_code_u16(0);		
			}else if(e->exprs.size==3){
				compile(e->exprs.head->value);
				compile(e->exprs.head->next->value);
				compile(e->exprs.head->next->next->value);
			}else{
				com_->error(line(), Xt("Xtal Compile Error 1016"));
			}
			
			put_code_u8(CODE_ASSERT);
		}

		XTAL_EXPR_CASE(TryStmt){
			int_t end_label = reserve_label();
			int_t catch_label = reserve_label();
			int_t finally_label = reserve_label();
			
			int_t oppos = code_size();
			put_code_u8(CODE_TRY_BEGIN);
			if(e->catch_stmt){
				put_jump_code_nocode(oppos, catch_label);
			}else{
				put_code_u16(0);
			}
			put_jump_code_nocode(oppos, finally_label);
			put_jump_code_nocode(oppos, end_label);

			CodeBuilder::FunFrame::Finally exc;
			exc.frame_count = scopes_.size();
			exc.finally_label = finally_label;
			fun_frame().finallys.push(exc);

			compile(e->try_stmt);
			
			put_jump_code(CODE_PUSH_GOTO, end_label);
			put_code_u8(CODE_TRY_END);

			set_label(catch_label);
			// catch節のコードを埋め込む
			if(e->catch_stmt){
				
				// catch節の中での例外に備え、例外フレームを構築。
				int_t oppos2 = code_size();
				put_code_u8(CODE_TRY_BEGIN);
				put_code_u16(0);
				put_jump_code_nocode(oppos2, finally_label);
				put_jump_code_nocode(oppos2, finally_label);

				CodeBuilder::FunFrame::Finally exc;
				exc.frame_count = scopes_.size();
				exc.finally_label = finally_label;
				fun_frame().finallys.push(exc);

				block_begin(BLOCK, 0, e->catch_vars, e->on_heap);{
					if(e->catch_vars.head){
						put_set_local_code(e->catch_vars.head->value);
					}
					compile(e->catch_stmt);
				}block_end();

				put_code_u8(CODE_TRY_END);
				fun_frame().finallys.pop();
			}
			
			set_label(finally_label);
			// finally節のコードを埋め込む
			compile(e->finally_stmt);
			
			fun_frame().finallys.pop();

			put_code_u8(CODE_POP_GOTO);

			set_label(end_label);
		}
		
		XTAL_EXPR_CASE(IfStmt){
			int_t label_if = reserve_label();
			int_t label_if2 = reserve_label();
			int_t label_end = reserve_label();

			put_if_code(e->cond_expr, label_if, label_if2);

			compile(e->body_stmt);
			
			if(e->else_stmt){
				put_jump_code(CODE_GOTO, label_end);
			}
			
			set_label(label_if);
			set_label(label_if2);
			compile(e->else_stmt);
		
			set_label(label_end);
		}

		XTAL_EXPR_CASE(WhileStmt){
			int_t label_cond = reserve_label();
			int_t label_cond_end = reserve_label();
			int_t label_if = reserve_label();
			int_t label_if2 = reserve_label();
			int_t label_end = reserve_label();
			
			if(e->cond_expr){
				put_if_code(e->cond_expr, label_if, label_if2);
			}
			put_jump_code(CODE_GOTO, label_cond_end);

			if(e->next_stmt){
				set_label(label_cond);
				compile(e->next_stmt);
			}else{
				set_label(label_cond);
			}

			if(e->cond_expr){
				if(e->else_stmt){
					put_if_code(e->cond_expr, label_end, label_end);
				}else{
					put_if_code(e->cond_expr, label_if, label_if2);
				}
			}
			
			set_label(label_cond_end);
			push_loop(label_end, label_cond, e->label);
			compile(e->body_stmt);
			pop_loop(); 
			
			put_jump_code(CODE_GOTO, label_cond);

			set_label(label_if);
			set_label(label_if2);
			if(e->nobreak_stmt){
				compile(e->nobreak_stmt);
			}else{
				compile(e->else_stmt);
			}

			set_label(label_end);
		}

		XTAL_EXPR_CASE(MultipleAssignStmt){
			int_t pushed_count = 0;
			for(TList<Expr*>::Node* rhs=e->rhs.head; rhs; rhs=rhs->next){	
				if(!rhs->next){
					if(expr_cast<CallExpr>(rhs->value) || expr_cast<SendExpr>(rhs->value)){
						int_t rrc;
						if(pushed_count<e->lhs.size){
							rrc = e->lhs.size - pushed_count;
						}else{
							rrc = 1;
						}

						if(CallExpr* ce = expr_cast<CallExpr>(rhs->value)){
							ce->discard = e->discard;
						}

						if(SendExpr* ce = expr_cast<SendExpr>(rhs->value)){
							ce->discard = e->discard;
						}

						compile(rhs->value, rrc);
						pushed_count += rrc;

						break;
					}else{
						int_t rrc;
						if(pushed_count<e->lhs.size){
							rrc = e->lhs.size - pushed_count;
						}else{
							rrc = 1;
						}
						compile(rhs->value, rrc, e->discard);
						pushed_count += rrc;
						break;
					}
				}else{
					compile(rhs->value);
					pushed_count++;
				}
			}

			if(e->lhs.size!=pushed_count){
				put_code_u8(CODE_ADJUST_RESULT);
				put_code_u8(pushed_count);
				put_code_u8(e->lhs.size);
				put_code_u8(e->discard ? RESULT_DISCARD : 0);
			}

			if(e->define){
				for(TList<Expr*>::Node* lhs=e->lhs.tail; lhs; lhs=lhs->prev){	
					if(LocalExpr* e2 = expr_cast<LocalExpr>(lhs->value)){
						put_define_local_code(e2->var);
					}else if(MemberExpr* e2 = expr_cast<MemberExpr>(lhs->value)){
						compile(e2->lhs);
						put_define_member_code(e2->var, e2->pvar);
					}else{
						com_->error(line(), Xt("Xtal Compile Error 1008"));
					}
				}
			}else{
				for(TList<Expr*>::Node* lhs=e->lhs.tail; lhs; lhs=lhs->prev){	
					if(LocalExpr* e2 = expr_cast<LocalExpr>(lhs->value)){
						put_set_local_code(e2->var);
					}else if(SendExpr* e2 = expr_cast<SendExpr>(lhs->value)){
						compile(e2->lhs);
						put_set_send_code(e2->var, e2->pvar, e2->if_defined);
					}else if(InstanceVariableExpr* e2 = expr_cast<InstanceVariableExpr>(lhs->value)){
						put_set_instance_variable_code(e2->var);					
					}else if(AtExpr* e2 = expr_cast<AtExpr>(lhs->value)){
						compile(e2->lhs);
						compile(e2->index);
						put_code_u8(CODE_SET_AT);
					}else{
						com_->error(line(), Xt("Xtal Compile Error 1008"));
					}
				}
			}
		}

		XTAL_EXPR_CASE(BreakStmt){
			if(fun_frame().loops.empty()){
				com_->error(line(), Xt("Xtal Compile Error 1007"));
			}else{
				if(e->var){
					bool found = false;
					int_t name = e->var;
					for(int_t i = 0, last = fun_frame().loops.size(); i<last; ++i){
						if(fun_frame().loops[i].name==name){
							break_off(fun_frame().loops[i].frame_count);
							put_jump_code(CODE_GOTO, fun_frame().loops[i].break_label);
							found = true;
							break;
						}
					}

					if(!found){
						com_->error(line(), Xt("Xtal Compile Error 1005"));
					}
				}else{
					bool found = false;
					for(int_t i = 0, last = fun_frame().loops.size(); i<last; ++i){
						if(!fun_frame().loops[i].have_label){
							break_off(fun_frame().loops[i].frame_count);
							put_jump_code(CODE_GOTO, fun_frame().loops[i].break_label);
							found = true;
							break;
						}
					}

					if(!found){
						com_->error(line(), Xt("Xtal Compile Error 1005"));
					}
				}
			}
		}	

		XTAL_EXPR_CASE(ContinueStmt){
			if(fun_frame().loops.empty()){
				com_->error(line(), Xt("Xtal Compile Error 1006"));
			}else{
				if(e->var){
					bool found = false;
					int_t name = e->var;
					for(int_t i = 0, last = fun_frame().loops.size(); i<last; ++i){
						if(fun_frame().loops[i].name==name){
							break_off(fun_frame().loops[i].frame_count);
							put_jump_code(CODE_GOTO, fun_frame().loops[i].continue_label);
							found = true;
							break;
						}
					}

					if(!found){
						com_->error(line(), Xt("Xtal Compile Error 1004"));
					}
				}else{
					bool found = false;
					for(size_t i = 0, last = fun_frame().loops.size(); i<last; ++i){
						if(!fun_frame().loops[i].have_label){
							break_off(fun_frame().loops[i].frame_count);
							put_jump_code(CODE_GOTO, fun_frame().loops[i].continue_label);		
							found = true;
							break;
						}
					}

					if(!found){
						com_->error(line(), Xt("Xtal Compile Error 1004"));
					}
				}
			}
		}	
		
		XTAL_EXPR_CASE(BlockStmt){
			block_begin(BLOCK, 0, e->vars, e->on_heap);{
				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile(p->value);
				}
			}block_end();
		}

		XTAL_EXPR_CASE(SetAccessibilityStmt){
			put_code_u8(CODE_SET_ACCESSIBILITY);
			put_code_u16(e->var);
			put_code_u8(e->kind);
		}
		
		XTAL_EXPR_CASE(TopLevelStmt){
			block_begin(BLOCK, 0, e->vars, e->on_heap);{
				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile(p->value);
				}
				
				break_off(1);
				if(e->export_expr){
					compile(e->export_expr);
					put_code_u8(CODE_RETURN_1);
				}else{
					put_code_u8(CODE_RETURN_0);
				}
			}block_end();
			
			process_labels();
			put_code_u8(CODE_THROW);
		}	
	}

	p_->set_line_number_info(ex->line);
	lines_.pop();
}

}

#endif

