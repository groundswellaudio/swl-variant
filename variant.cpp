
#define PACK int, float, char, bool, double
#define PACK1 PACK, PACK 
#define PACK2 PACK1, PACK1
#define PACK3 PACK2, PACK2 
#define PACK4 PACK3, PACK3  
#define PACK5 PACK4, PACK4
#define PACK6 PACK5, PACK5
#define PACK7 PACK6, PACK6

//#include <iostream>

//#define SWL_CPP_VARIANT_USE_STD_HASH
#include "variant.hpp" 
//#include <variant>
int main(){
	
	using namespace std;
	
	swl::variant<PACK1> a, b, c, d;
	
	//a.emplace<1>(2);
	
	int x = 0;
	
	swl::visit( [] (auto... args) {}, a, b, c, d );
	
} 