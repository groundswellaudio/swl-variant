
//#include <tuple>
//#include <variant>


template <bool B>
struct conditional;

template <>
struct conditional<true> {
	template <class A, class B>
	using f = A;
};

template <>
struct conditional<false> {
	template <class A, class B>
	using f = B;
};


#define PACK int, float, char, bool, double, float
#define PACK1 PACK, PACK
#define PACK2 PACK, PACK, PACK, PACK
#define PACK3 PACK2, PACK2, PACK2, PACK2
#define PACK4 PACK3, PACK3, PACK3, PACK3
#define PACK5 PACK4, PACK4, PACK4, PACK4
#define PACK6 PACK5, PACK5

//#include <iostream>

template <class T, class... Args>
concept constructible = requires (Args... args) { T{ args... }; };

template <unsigned X>
struct ctor_detect{
	
	/* 
	ctor_detect(){
		std::cout << "Ctor " << X << std::endl;
	}
	
	ctor_detect(ctor_detect&&){
		std::cout << "Move ctor" << X << std::endl;
	}
	
	ctor_detect(const ctor_detect&){
		std::cout << "Copy ctor" << X << std::endl;
	} */ 
	
};

//#define SWL_CPP_VARIANT_USE_STD_HASH
#include "variant.hpp"
/* 
#include <iostream>
#include <variant> */ 

//#include <variant>

//#include <string>

struct my_type{
	
	my_type(my_type&&){}
	~my_type(){ }
};

struct type1{
	virtual ~type1(){}
};
struct type2{ virtual ~type2(){} };

struct noctor {
	noctor() = delete;
};

/* 
#include <iostream>
#include <string>
#include <cassert> */ 

int main(){
	
	//swl::variant<int, std::string> vst { swl::in_place_index<1>, "hehehe" };
	
	//static_assert( not std::is_trivially_destructible_v<swl::variant<std::string, int>> );
	
	
	
	
	//vvz = "hello";
	//static_assert( swl::variant<my_type, int>::trivial_dtor );
	
	//swl::variant<int> v;
	
	
	//c = "heheeehee";
	
	using namespace swl;
	//variant<PACK> v1, v2, v3, v4, v5;
	
	variant<PACK4> vvv;
	
	//show_type< swl::variant<int, float, char>::storage_t >();
	
	//std::cout << (vr1 == vr2) << std::endl;
	
	//visit( [] (auto... args) {}, v1, v2, v3, v4);
	
	/* 
	vrx.emplace<1>(2.33f);
	
	std::variant<PACK2> vr2;
	
	vr2.emplace<0>(1); */ 
	
	
	//visit( [] (auto a) {}, vr1);
	
	//visit( [] (auto a, auto b) {}, vr1, vr2 );
	/* 
	visit( [] (auto& a, auto& b) {
		return a;
	}, vrx, vr2 ); */ 
	
	
	//visit([] (auto& v) {}, vrx);
	//using seq = make_index_seq<1>::template f<1024, 0>;
	
	//constexpr auto idx = first_index<char**, PACK4, char**>;
	
	//using tt = __type_pack_element<800, PACK5>;
	
	//show_type<tt>();
	
	//using t = make_tree_1st<4>::template f<int, float, char, double>;
	//show_type<t>();
	
	/* 
	variant<PACK> v, z;
	
	v.emplace<2>(44);
	z.emplace<3>(true);
	
	v = z;
	v = true; */ 
}