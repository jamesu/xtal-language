
#pragma once

namespace xtal{

class DebugInfo : public Base{
public:

	int_t kind(){ return kind_; } 
	int_t line(){ return line_; }
	const StringPtr& file_name(){ return file_name_; }
	const StringPtr& fun_name(){ return fun_name_; }
	const StringPtr& message(){ return message_; }
	const FramePtr& local_variables(){ return local_variables_; }

	void set_kind(int_t v){ kind_ = v; }
	void set_line(int_t v){ line_ = v; }
	void set_file_name(const StringPtr& v){ file_name_ = v; }
	void set_fun_name(const StringPtr& v){ fun_name_ = v; }
	void set_message(const StringPtr& v){ message_ = v; }
	void set_local_variables(const FramePtr& v){ local_variables_ = v; }

	SmartPtr<DebugInfo> clone(){
		return xnew<DebugInfo>(*this);
	}

private:

	void visit_members(Visitor& m);

	int_t kind_;
	int_t line_;
	StringPtr file_name_;
	StringPtr fun_name_;
	StringPtr message_;
	FramePtr local_variables_;
};

typedef SmartPtr<DebugInfo> DebugInfoPtr;

class Debug : public CppSingleton{
public:

	Debug(){
		enable_count_ = 0;
	}

	/**
	* @brief �f�o�b�O�@�\��L���ɂ���
	* �f�o�b�O�@�\�̓f�t�H���g�ł͖����ɂȂ��Ă���B
	*/
	void enable();

	/**
	* @brief �f�o�b�O�@�\�𖳌��ɂ���
	*/
	void disable();

	/**
	* @brief �f�o�b�O�@�\���L�����ǂ���
	*/
	bool is_enabled();

	/**
	* @brief �u���[�N�|�C���g������x�ɌĂяo�����t�b�N�֐���o�^����
	*/
	void set_break_point_hook(const AnyPtr& hook);

	/**
	* @brief �֐��Ăяo�������x�ɌĂяo�����t�b�N�֐���o�^����
	*/
	void set_call_hook(const AnyPtr& hook);

	/**
	* @brief �֐�����return�����x�ɌĂяo�����t�b�N�֐���o�^����
	*/
	void set_return_hook(const AnyPtr& hook);

	/**
	* @brief ��O��throw�����x�ɌĂяo�����t�b�N�֐���o�^����
	*/
	void set_throw_hook(const AnyPtr& hook);

	/**
	* @brief �A�T�[�g�����s�����x�ɌĂяo�����t�b�N�֐���o�^����
	*/
	void set_assert_hook(const AnyPtr& hook);

	/**
	* @brief set_break_point_hook�֐��œo�^�����֐����擾����
	*/
	const AnyPtr& break_point_hook();

	/**
	* @brief set_call_hook�֐��œo�^�����֐����擾����
	*/
	const AnyPtr& call_hook();

	/**
	* @brief set_return_hook�֐��œo�^�����֐����擾����
	*/
	const AnyPtr& return_hook();

	/**
	* @brief set_throw_hook�֐��œo�^�����֐����擾����
	*/
	const AnyPtr& throw_hook();

	/**
	* @brief set_assert_hook�֐��œo�^�����֐����擾����
	*/
	const AnyPtr& assert_hook();

private:

	int_t enable_count_;
	AnyPtr break_point_hook_;
	AnyPtr call_hook_;
	AnyPtr return_hook_;
	AnyPtr throw_hook_;
	AnyPtr assert_hook_;

public:
	void visit_members(Visitor& m){
		CppSingleton::visit_members(m);
		m & break_point_hook_ & call_hook_ & return_hook_ & throw_hook_ & assert_hook_;
	}
};

const SmartPtr<Debug>& debug();

void enable_debug();

void disable_debug();

}