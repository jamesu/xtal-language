
#pragma once

#include "fwd.h"
#include "vmachine.h"

namespace xtal{

/** @addtogroup lib */
/*@{*/

/**
* @brief file_name�t�@�C�����R���p�C������B
*
* @param file_name Xtal�X�N���v�g���L�q���ꂽ�t�@�C���̖��O
* @return ���s�ł���֐��I�u�W�F�N�g
* ���̖߂�l��object_dump����ƁA�o�C�g�R�[�h�`���ŕۑ������B
*/
Any compile_file(const String& file_name);

/**
* @brief source��������R���p�C������B
*
* @param source Xtal�X�N���v�g���L�q���ꂽ������
* @return ���s�ł���֐��I�u�W�F�N�g
* ���̖߂�l��object_dump����ƁA�o�C�g�R�[�h�`���ŕۑ������B
*/
Any compile(const String& source);

/**
* @brief file_name�t�@�C�����R���p�C�����Ď��s����B
*
* @param file_name Xtal�X�N���v�g���L�q���ꂽ�t�@�C���̖��O
* @return �X�N���v�g����export���ꂽ�l
*/
Any load(const String& file_name);

/**
* @brief file_name�t�@�C�����R���p�C�����ăR���p�C���ς݃\�[�X��ۑ����A���s����B
*
* @param file_name Xtal�X�N���v�g���L�q���ꂽ�t�@�C���̖��O
* @return �X�N���v�g����export���ꂽ�l
*/
Any load_and_save(const String& file_name);

/**
* @brief interactive xtal�̎��s
*/
void ix();

/**
* @brief �K�[�x�W�R���N�V���������s����
*
* ���قǎ��Ԃ͂�����Ȃ����A���S�ɃS�~������ł��Ȃ��K�[�x�W�R���N�g�֐�
*/
void gc();

/**
* @brief �z�I�u�W�F�N�g���������t���K�[�x�W�R���N�V���������s����
*
* ���Ԃ͂����邪�A���݃S�~�ƂȂ��Ă�����̂͂Ȃ�ׂ��S�ĉ������K�[�x�W�R���N�g�֐�
*/
void full_gc();

/**
* @brief �K�[�x�W�R���N�V������L��������
*
* enable_gc���Ă΂ꂽ�񐔂Ɠ��������Ăяo���ƃK�[�x�W�R���N�V�������L���ɂȂ�
*/
void enable_gc();

/**
* @brief �K�[�x�W�R���N�V�����𖳌�������
*
* �����ł��ꂪ����Ăяo���ꂽ���L������Ă���A�Ăяo������enable_gc���ĂтȂ��ƃK�[�x�W�R���N�V�����͗L���ɂȂ�Ȃ�
*/
void disable_gc();

/**
* @brief �I�u�W�F�N�g�𒼗񉻂��ĕۑ�����
*
* @param obj ���񉻂��ĕۑ��������I�u�W�F�N�g
* @param out ���񉻐�̃X�g���[��
*/
void object_dump(const Any& obj, const Stream& out);

/**
* @brief �I�u�W�F�N�g�𒼗񉻂��ēǂݍ���
*
* @param in ���񉻂��ꂽ�I�u�W�F�N�g��ǂݍ��ރX�g���[��
* @return �������ꂽ�I�u�W�F�N�g
*/
Any object_load(const Stream& in);

/**
* @brief �I�u�W�F�N�g���X�N���v�g�����ĕۑ�����
*
* @param obj �X�N���v�g�����ĕۑ��������I�u�W�F�N�g
* @param out ���񉻐�̃X�g���[��
*/
void object_to_script(const Any& obj, const Stream& out);

/**
* @brief �I�u�W�F�N�g��C++�ɖ��ߍ��߂�`�ɂ��ĕۑ�����
*
* @param obj C++���������I�u�W�F�N�g
* @param out ���񉻐�̃X�g���[��
*/
void object_to_cpp(const Any& obj, const Stream& out);

/**
* @brief ���C��VMachine�I�u�W�F�N�g��Ԃ�
*/
const VMachine& vmachine();

/**
* @brief Iterator�N���X��Ԃ�
*/
const Class& Iterator();

/**
* @brief Enumerable�N���X��Ԃ�
*/
const Class& Enumerable();

/**
* @brief builtin�N���X��Ԃ�
*/
const Class& builtin();

/**
* @brief lib�I�u�W�F�N�g��Ԃ�
*/
const Any& lib();

/**
* @brief nop�I�u�W�F�N�g��Ԃ�
*/
const Any& nop();

Any get_text(const char* text);

Any format(const char* text);

void set_get_text_map(const Any& map);

Any get_get_text_map();

Any source(const char* src, int_t size, const char* file);

void print(const VMachine& vm);

void println(const VMachine& vm);

void InitFormat();
void InitInt();
void InitFloat();

void iter_next(Any& target, Any& value, bool first);
void iter_next(Any& target, Any& value1, Any& value2, bool first);
void iter_next(Any& target, Any& value1, Any& value2, Any& value3, bool first);

struct IterBreaker{
	Any target;
	IterBreaker(const Any& tar=null):target(tar){}
	~IterBreaker();
	operator Any&(){ return target; }
	operator bool(){ return target; }
};

/*@}*/


}
