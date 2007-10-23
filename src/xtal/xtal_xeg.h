
#pragma once

#include "xtal.h"

namespace xtal{ namespace xeg{

class Parts;

/**
* @brief 
*/
class Scanner : public Base{
public:

	Scanner(){
		buf_ = xnew<Array>(256);
		pos_ = 0;
		read_ = 0;
		marked_ = 0; //~0;

		mm_ = xnew<MemoryStream>();

		newline_ch_ = "\n";
		lineno_ = 1;

		results_ = xnew<Map>();
	}

	void seek(uint_t pos){
		if(pos_ < pos){
			peek(pos - pos_);
		}
		pos_ = pos;
	}

	uint_t pos(){
		return pos_;
	}

	uint_t lineno(){
		return lineno_;
	}

	/**
	* @brief n個先の要素を覗き見る
	*/
	const AnyPtr& peek(uint_t n = 0){
		uint_t bufsize = buf_->size();
		uint_t bufmask = bufsize - 1;
		uint_t rpos = read_&bufmask;

		while(pos_+n >= read_){
			uint_t now_read = 0;
			if(marked_ != ~0){
				uint_t mpos = marked_&bufmask;

				if(rpos<=mpos && ((rpos+n)&bufmask)>mpos){
					// マーク中の領域を侵犯しようとしているので、リングバッファを倍に拡大
					buf_->resize(bufsize*2);
					bufsize = buf_->size();
					bufmask = bufsize - 1;
					rpos = read_&bufmask;
					mpos = marked_&bufmask;
				}

				if(mpos>rpos){
					now_read = do_read(buf_->data()+rpos, mpos-rpos);
				}else{
					now_read = do_read(buf_->data()+rpos, bufsize-rpos);
				}
			}else{
				now_read = do_read(buf_->data()+rpos, bufsize-rpos);
			}

			if(now_read==0){
				return nop;
			}

			read_ += now_read;
			rpos = read_&bufmask;
		}
		
		return buf_->at((pos_+n)&bufmask);
	}

	/**
	* @brief ひとつ読み取る
	*/
	const AnyPtr& read(){
		const AnyPtr& ret = peek();
		if(raweq(ret, newline_ch_)){
			lineno_++;
		}
		pos_ += 1;
		return  ret;
	}

	/**
	* @brief valueと等しい場合に読み飛ばす
	*/
	bool eat(const AnyPtr& value){
		const AnyPtr& ret = peek();
		if(raweq(ret, value)){
			read();
			return true;
		}
		return false;
	}

	/**
	* @brief 一つ先読みし、終了しているか調べる
	*/
	bool eof(){
		return raweq(peek(), nop);
	}

	/**
	* @brief n文字読み飛ばす
	*/
	void skip(uint_t n){
		for(uint_t i=0; i<n; ++i){
			read();
		}
	}

public:

	void push_result(const AnyPtr& v){
		//results_->push_back(v);
	}

public:

	StringPtr capture(int_t begin, int_t end){
		uint_t mask = buf_->size()-1;
		mm_->clear();
		for(uint_t i=begin; i<end; ++i){
			mm_->put_s(buf_->at(i & mask)->to_s());
		}
		return mm_->to_s();
	}

protected:

	virtual int_t do_read(AnyPtr* buffer, int_t max) = 0;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & buf_ & mm_ & results_ & newline_ch_;
	}

private:
	
	MemoryStreamPtr mm_;
	StringPtr newline_ch_;
	MapPtr results_;
	ArrayPtr buf_;

	uint_t pos_;
	uint_t read_;
	uint_t marked_;
	uint_t lineno_;
};

class StreamScanner : public Scanner{
public:

	StreamScanner(const StreamPtr& stream)
		:stream_(stream){}

	virtual int_t do_read(AnyPtr* buffer, int_t max);

private:
	StreamPtr stream_;

	virtual void visit_members(Visitor& m){
		Scanner::visit_members(m);
		m & stream_;
	}
};

class IteratorScanner : public Scanner{
public:

	IteratorScanner(const AnyPtr& iter)
		:iter_(iter){}

	virtual int_t do_read(AnyPtr* buffer, int_t max);

private:
	AnyPtr iter_;

	virtual void visit_members(Visitor& m){
		Scanner::visit_members(m);
		m & iter_;
	}
};

typedef SmartPtr<Scanner> ScannerPtr;
typedef SmartPtr<StreamScanner> StreamScannerPtr;
typedef SmartPtr<IteratorScanner> IteratorScannerPtr;


ScannerPtr parse_scanner(const AnyPtr& pattern, const ScannerPtr& scanner);

inline ScannerPtr parse_stream(const AnyPtr& pattern, const StreamPtr& stream){
	return parse_scanner(pattern, xnew<StreamScanner>(stream));
}

inline ScannerPtr parse_string(const AnyPtr& pattern, const AnyPtr& string){
	return parse_stream(pattern, xnew<StringStream>(string->to_s()));
}

inline ScannerPtr parse_iterator(const AnyPtr& pattern, const AnyPtr& iter){
	return parse_scanner(pattern, xnew<IteratorScanner>(iter));
}

}}

