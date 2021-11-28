 
//#define TEST_STD

#ifdef TEST_STD
	#include <variant>
	#define swl std
#else
	#include <swl/variant.hpp>
#endif

#define PACK int, float, char, bool, double
#define PACK10 PACK, PACK 
#define PACK20 PACK10, PACK10
#define PACK40 PACK20, PACK20 
#define PACK80 PACK40, PACK40
#define PACK160 PACK80, PACK80
#define PACK320 PACK160, PACK160
#define PACK640 PACK320, PACK320

//#define TEST_MULTI_VISIT

int main(){
	
	swl::variant<PACK160> a, b, c, d;
	
	#ifndef TEST_MULTI_VISIT
	
		swl::visit( [] (auto x) {}, a );
	
	#else
		
		swl::visit( [] (auto... args) {}, a, b, c );
		
	#endif
}