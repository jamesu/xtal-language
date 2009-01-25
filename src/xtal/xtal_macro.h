
#pragma once

/** @addtogroup xmacro */
/*@{*/

/**
* @brief foreachを簡単に記述するためのマクロ
*
* @code
* Xfor(value, array){
*   // use value
* }
* @endcode
*/
#define Xfor(var, tar) \
	if(bool not_end = true)\
	for(::xtal::BlockValueHolder1 block_value_holder(tar, not_end); not_end; not_end=false)\
	for(const ::xtal::AnyPtr &var = block_value_holder.values[0]; not_end; not_end=false)\
	for(not_end=::xtal::block_next(block_value_holder, true); not_end; not_end=::xtal::block_next(block_value_holder, false))

/**
* @brief foreachを簡単に記述するためのマクロ
*
* @code
* Xfor2(key, value, map){
*   // use key and value
* }
* @endcode
*/
#define Xfor2(var1, var2, tar) \
	if(bool not_end = true)\
	for(::xtal::BlockValueHolder2 block_value_holder(tar, not_end); not_end; not_end=false)\
	for(const ::xtal::AnyPtr &var1 = block_value_holder.values[0], &var2 = block_value_holder.values[1]; not_end; not_end=false)\
	for(not_end=::xtal::block_next(block_value_holder, true); not_end; not_end=::xtal::block_next(block_value_holder, false))

/**
* @brief foreachを簡単に記述するためのマクロ
*
* @code
* Xfor3(v1, v2, v3, hoge.send("each3")){
*   // use v1, v2 and v3
* }
* @endcode
*/
#define Xfor3(var1, var2, var3, tar) \
	if(bool not_end = true)\
	for(::xtal::BlockValueHolder3 block_value_holder(tar, not_end); not_end; not_end=false)\
	for(const ::xtal::AnyPtr &var1 = block_value_holder.values[0], &var2 = block_value_holder.values[1], &var3 = block_value_holder.values[2]; not_end; not_end=false)\
	for(not_end=::xtal::block_next(block_value_holder, true); not_end; not_end=::xtal::block_next(block_value_holder, false))


/**
* @brief foreachを簡単に記述するためのマクロ
*
* 各要素を受け取る変数に型をつけることが出来る。
* @code
* Xfor_cast(AnyPtr& value, array){
*   // use value
* }
* @endcode
*/
#define Xfor_cast(var, tar) \
	if(bool not_end = true)\
	for(::xtal::BlockValueHolder1 block_value_holder(tar, not_end); not_end; not_end=false)\
	for(not_end=::xtal::block_next(block_value_holder, true); not_end; not_end=::xtal::block_next(block_value_holder, false))\
	if(var = ::xtal::tricky_cast(block_value_holder.values[0], (void (*)(var##e))0))

/**
* @brief foreachを簡単に記述するためのマクロ
*
* 各要素を受け取る変数に型をつけることが出来る。
* @code
* Xfor2_cast(AnyPtr& key, AnyPtr& value, map){
*   // use key and value
* }
* @endcode
*/
#define Xfor2_cast(var1, var2, tar) \
	if(bool not_end = true)\
	for(::xtal::BlockValueHolder2 block_value_holder(tar, not_end); not_end; not_end=false)\
	for(not_end=::xtal::block_next(block_value_holder, true); not_end; not_end=::xtal::block_next(block_value_holder, false))\
	if(var1 = ::xtal::tricky_cast(block_value_holder.values[0], (void (*)(var1##e))0))\
	if(var2 = ::xtal::tricky_cast(block_value_holder.values[1], (void (*)(var2##e))0))

/**
* @brief foreachを簡単に記述するためのマクロ
*
* 各要素を受け取る変数に型をつけることが出来る。
* @code
* Xfor3_cast(AnyPtr& v1, AnyPtr& v2, AnyPtr& v3, hoge->send("each3")){
*   // use v1, v2 and v3
* }
* @endcode
*/
#define Xfor3_cast(var1, var2, var3, tar) \
	if(bool not_end = true)\
	for(::xtal::BlockValueHolder3 block_value_holder(tar, not_end); not_end; not_end=false)\
	for(not_end=::xtal::block_next(block_value_holder, true); not_end; not_end=::xtal::block_next(block_value_holder, false))\
	if(var1 = ::xtal::tricky_cast(block_value_holder.values[0], (void (*)(var1##e))0))\
	if(var2 = ::xtal::tricky_cast(block_value_holder.values[1], (void (*)(var2##e))0))\
	if(var3 = ::xtal::tricky_cast(block_value_holder.values[2], (void (*)(var3##e))0))

/**
* @brief foreachを簡単に記述するためのマクロ
*
* 各要素を受け取る変数に型をつけることが出来る。
* 変換できない型の場合、スキップされる。
* @code
* Xfor_as(AnyPtr& value, array){
*   // use value
* }
* @endcode
*/
#define Xfor_as(var, tar) \
	if(bool not_end = true)\
	for(::xtal::BlockValueHolder1 block_value_holder(tar, not_end); not_end; not_end=false)\
	for(not_end=::xtal::block_next(block_value_holder, true); not_end; not_end=::xtal::block_next(block_value_holder, false))\
	if(var = ::xtal::tricky_as(block_value_holder.values[0], (void (*)(var##e))0))

/**
* @brief foreachを簡単に記述するためのマクロ
*
* 各要素を受け取る変数に型をつけることが出来る。
* 変換できない型の場合、スキップされる。
* @code
* Xfor2_as(AnyPtr& key, AnyPtr& value, map){
*   // use key and value
* }
* @endcode
*/
#define Xfor2_as(var1, var2, tar) \
	if(bool not_end = true)\
	for(::xtal::BlockValueHolder2 block_value_holder(tar, not_end); not_en; not_end=false)\
	for(not_end=::xtal::block_next(block_value_holder, true); not_end; not_end=::xtal::block_next(block_value_holder, false))\
	if(var1 = ::xtal::tricky_as(block_value_holder.values[0], (void (*)(var1##e))0))\
	if(var2 = ::xtal::tricky_as(block_value_holder.values[1], (void (*)(var2##e))0))

/**
* @brief foreachを簡単に記述するためのマクロ
*
* 各要素を受け取る変数に型をつけることが出来る。
* 変換できない型の場合、スキップされる。
* @code
* Xfor3_as(AnyPtr& v1, AnyPtr& v2, AnyPtr& v3, hoge->send("each3")){
*   // use v1, v2 and v3
* }
* @endcode
*/
#define Xfor3_as(var1, var2, var3, tar) \
	if(bool not_end = true)\
	for(::xtal::BlockValueHolder3 block_value_holder(tar, not_end); not_end; not_end=false)\
	for(not_end=::xtal::block_next(block_value_holder, true); not_end; not_end=::xtal::block_next(block_value_holder, false))\
	if(var1 = ::xtal::tricky_as(block_value_holder.values[0], (void (*)(var1##e))0))\
	if(var2 = ::xtal::tricky_as(block_value_holder.values[1], (void (*)(var2##e))0))\
	if(var3 = ::xtal::tricky_as(block_value_holder.values[2], (void (*)(var3##e))0))

/**
* @brief textを簡単に記述するためのマクロ
*
* @code
* AnyPtr text = Xt("Text %d %s")(10, "test");
* @endcode
*/
#define Xt(txt) ::xtal::text(XTAL_STRING(txt)) 

/**
* @brief formatを簡単に記述するためのマクロ
*
* @code
* AnyPtr fmt = Xf("Text %d %s")(10, "test");
* @endcode
*/
#define Xf(txt) ::xtal::format(XTAL_STRING(txt)) 

#ifndef XTAL_NO_PARSER

/**
* @brief Xtalのソースを簡単に記述するためのマクロ
*
* @code
* AnyPtr src = Xsrc((
*   return [0, 2, 3, 4];
* ));
* @endcode
*/
#define Xsrc(text) ::xtal::source(XTAL_STRING(#text)+1, sizeof(XTAL_STRING(#text))/sizeof(char_t)-3, __FILE__)

#endif

#ifdef XTAL_USE_COMPILED_EMB
#define Xemb(text, compiled_text) ::xtal::compiled_source(compiled_text, sizeof(compiled_text)-1, __FILE__)
#else
#define Xemb(text, compiled_text) ::xtal::source(XTAL_STRING(#text)+1, sizeof(XTAL_STRING(#text))/sizeof(char_t)-3, __FILE__)
#endif

/*@}*/

/*
* @brief インターンされた文字列を簡単に記述するためのマクロ
*
* @code
* IDPtr id = Xid(test);
* @endcode
*/
#define Xid(text) (::xtal::string_literal_to_id(XTAL_STRING(#text)))

