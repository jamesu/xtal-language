#include "xtal.h"

namespace xtal{

////////////////////////////////////////////////

StringPtr ChCodeLib::ch_inc(const char_t* data, int_t data_size){
	if(data_size>6){
		return xnew<ID>(data, data_size);
	}

	uchar_t buf[8] = {0};
	std::memcpy(buf+1, data, data_size*sizeof(uchar_t));

	for(int_t i=data_size; i>=0; --i){
		buf[i]++;
		if(buf[i]==0){
			continue;
		}
		break;
	}

	if(buf[0]==0){
		return xnew<ID>((char_t*)buf+1, data_size);
	}
	else{
		return xnew<ID>((char_t*)buf, data_size+1);
	}
}

int_t ChCodeLib::ch_cmp(const char_t* a, int_t asize, const char_t* b, int_t bsize){
	if(asize==bsize){
		for(int_t i=0; i<asize; ++i){
			if((uchar_t)a[i]<(uchar_t)b[i]){
				return -1;
			}
			else if((uchar_t)a[i]>(uchar_t)b[i]){
				return 1;
			}
		}
		return 0;
	}
	else if(asize<bsize){
		return -1;
	}
		
	return 1;
}

void ChMaker::add(char_t ch){
	buf_[pos_++] = ch;
	if(pos_==1){
		len_ = ch_len(ch);
	}
	else if(pos_ == -len_){
		len_ = ch_len2(buf_);
	}
}

const IDPtr& ChMaker::to_s(){
	switch(pos_){
	case 1: temp_ = xnew<ID>(buf_[0]);
	case 2: temp_ = xnew<ID>(buf_[0], buf_[1]);
	case 3: temp_ = xnew<ID>(buf_[0], buf_[1], buf_[2]);
	default: temp_ = xnew<ID>(&buf_[0], pos_);
	}

	return temp_;
}

int_t ch_len(char_t lead){
	return core()->chcode_lib()->ch_len(lead);
}

int_t ch_len2(const char_t* str){
	return core()->chcode_lib()->ch_len2(str);
}

StringPtr ch_inc(const char_t* data, int_t data_size){
	return core()->chcode_lib()->ch_inc(data, data_size);
}

int_t ch_cmp(const char_t* a, uint_t asize, const char_t* b, uint_t bsize){
	return core()->chcode_lib()->ch_cmp(a, asize, b, bsize);
}

void edit_distance_helper(const void* data1, uint_t size1, const void* data2, uint_t size2, uint_t* buf, uint_t k, int_t offset){
	uint_t v1 = buf[k-1+offset]+1;
	uint_t v2 = buf[k+1+offset];
	uint_t x = v1>v2 ? v1 : v2;
	uint_t y = x-k;
	while(x<size1 && y<size2 && ((u8*)data1)[x-1]==((u8*)data2)[y-1]){
		++x;
		++y;
	}
	buf[k+offset] = x;
}

uint_t edit_distance(const void* data1, uint_t data_size1, const void* data2, uint_t data_size2){
	if(data_size1<data_size2){
		return edit_distance(data2, data_size2, data1, data_size1);
	}

	uint_t size1 = data_size1 + 1;
	uint_t size2 = data_size2 + 1;

	uint_t buf_size = size1+size2+6;
	uint_t* buf = (uint_t*)user_malloc(sizeof(uint_t)*buf_size);
	
	for(uint_t i=0; i<buf_size; ++i){
		buf[i] = 0;
	}

	int_t offset = size2+1, delta = size1-size2;
	for(int_t p=0;;++p){
		for(int_t k=-p; k<delta; ++k){
			edit_distance_helper(data1, size1, data2, size2, buf, k, offset);
		}

		for(int_t k=delta+p; k>delta; --k){
			edit_distance_helper(data1, size1, data2, size2, buf, k, offset);
		}

		edit_distance_helper(data1, size1, data2, size2, buf, delta, offset);

		if(buf[delta+offset]==size1){
			user_free(buf);
			return p+delta;
		}
	}
	return 0;
}

}