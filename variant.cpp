
#define PACK int, float, char, bool, double
#define PACK10 PACK, PACK 
#define PACK20 PACK10, PACK10
#define PACK40 PACK20, PACK20 
#define PACK80 PACK40, PACK40
#define PACK160 PACK80, PACK80
#define PACK320 PACK160, PACK160
#define PACK640 PACK320, PACK320

//#include <iostream>

//#define SWL_CPP_VARIANT_USE_STD_HASH
//#include "variant.hpp" 
#include <variant>
int main(){
	
	using namespace std;
	
	std::variant<PACK640> a;
	
	//a.emplace<1>(2);
	
	int x = 0;
	
	std::visit( [] (auto... args) {}, a );
} 