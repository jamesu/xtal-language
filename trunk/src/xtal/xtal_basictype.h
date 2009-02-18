
#pragma once

namespace xtal{

class Int : public Any{
public:

	Int(int_t val = 0)
		:Any(val){}

public:
	
	int_t to_i(){
		return ivalue(*this);
	}

	float_t to_f(){
		return (float_t)ivalue(*this);
	}

	StringPtr to_s(){
		return Any::to_s();
	}

	bool op_in(const IntRangePtr& range);

	bool op_in(const FloatRangePtr& range);

	IntRangePtr op_range(int_t right, int_t kind);

	FloatRangePtr op_range(float_t right, int_t kind);
};

class Float : public Any{
public:

	Float(float_t val = 0)
		:Any(val){}

public:

	int_t to_i(){
		return (int_t)fvalue(*this);
	}

	float_t to_f(){
		return fvalue(*this);
	}

	StringPtr to_s(){
		return Any::to_s();
	}

	bool op_in(const IntRangePtr& range);

	bool op_in(const FloatRangePtr& range);

	FloatRangePtr op_range(int_t right, int_t kind);

	FloatRangePtr op_range(float_t right, int_t kind);
};



class Bool : public Any{};

class Range : public Base{
public:

	Range(const AnyPtr& left, const AnyPtr& right, int_t kind = CLOSED)
		:left_(left), right_(right), kind_(kind){}

	const AnyPtr& left(){ return left_; }

	const AnyPtr& right(){ return right_; }

	int_t kind(){ return kind_; }

	bool is_left_closed(){ return (kind_&(1<<1))==0; }

	bool is_right_closed(){ return (kind_&(1<<0))==0; }

	enum{
		CLOSED = (0<<1) | (0<<0),
		LEFT_CLOSED_RIGHT_OPEN = (0<<1) | (1<<0),
		LEFT_OPEN_RIGHT_CLOSED = (1<<1) | (0<<0),
		OPEN = (1<<1) | (1<<0)
	};

protected:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & left_ & right_;
	}

	AnyPtr left_;
	AnyPtr right_;
	int_t kind_;
};

class IntRange : public Range{
public:

	IntRange(int_t left, int_t right, int_t kind = CLOSED)
		:Range(left, right, kind){}

public:

	/**
	* @brief �͈͂̊J�n
	*
	* begin�͍��E�J��� [begin, end) �œ��邱�Ƃ��o���� 
	*/
	int_t begin(){ return is_left_closed() ? left() : left()+1; }

	/**
	* @brief �͈͂̏I�[
	*
	* end�͍��E�J��� [begin, end) �œ��邱�Ƃ��o���� 
	*/
	int_t end(){ return is_right_closed() ? right()+1 : right(); }

	int_t left(){ return ivalue(left_); }

	int_t right(){ return ivalue(right_); }

	AnyPtr each();
};

class IntRangeIter : public Base{
public:

	IntRangeIter(const IntRangePtr& range)
		:it_(range->begin()), end_(range->end()){
	}

	void block_next(const VMachinePtr& vm);

private:
	int_t it_, end_;
};

class FloatRange : public Range{
public:

	FloatRange(float_t left, float_t right, int_t kind = CLOSED)
		:Range(left, right, kind){}

public:

	float_t left(){ return fvalue(left_); }

	float_t right(){ return fvalue(right_); }

	AnyPtr each();
};

class HaveParent : public Base{
public:

	HaveParent()
		:parent_(0), force_(0){}

	HaveParent(const HaveParent& a);

	HaveParent& operator=(const HaveParent& a);

	~HaveParent();

	virtual const ClassPtr& object_parent();

	virtual int_t object_parent_force();

	virtual void set_object_parent(const ClassPtr& parent, int_t force);

protected:

	Class* parent_;
	int_t force_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & parent_;
	}	
};


class GCObserver : public Base{
public:
	GCObserver();
	GCObserver(const GCObserver& v);
	virtual ~GCObserver();
	virtual void before_gc(){}
	virtual void after_gc(){}
};


}