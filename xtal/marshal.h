
#pragma once

#include "any.h"
#include "array.h"
#include "mapimpl.h"
#include "stream.h"

namespace xtal{

class Marshal{
public:

	Marshal(const Stream& s);

	void dump(const Any& v);

	Any load();

	void to_script(const Any& v);

private:

	void inner_dump(const Any& v);
	Any inner_load();
	void inner_to_script(const Any& v, int_t tab);

	Any demangle(int_t n);

	int_t register_dvalue(const Any& v, bool& added);
	int_t register_lvalue(const Any& v);

	void put_tab(int_t tab);

	enum{ VALUE, LIB, REF, TNULL, INT, FLOAT, STRING, TID, ARRAY, MAP };

	StrictMap dmap_;
	Array dvalues_;

	StrictMap lmap_;
	Array lvalues_;

	Stream stream_;
};

}
