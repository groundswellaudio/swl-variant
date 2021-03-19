
//#include <tuple>
//#include <variant>

/* 
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

template <std::size_t, class, class>
union variant_union;

template <class T, std::size_t N>
constexpr bool find_first( const T(&array)[N], const T v ){
	const T* it = &array[0];
	while (it != &array[N] and *it != v) ++it;
	return (it - &array[0]);
}


template <class T>
constexpr void show_type(){
	static_assert( std::is_same_v<T, void> );
}

template <class T>
struct array_wrapper {
    T data;
};

template <unsigned char Idx, unsigned Max>
constexpr void increment(auto& walker, const auto& sizes){
    ++walker[Idx];
    if (walker[Idx] == sizes[Idx]){
        if constexpr (Idx + 1 < Max){
            walker[Idx] = 0;
            return increment<Idx + 1, Max>(walker, sizes);
        }
    }
}

template <unsigned... Sizes>
constexpr auto make_flat_sequence(){
    constexpr unsigned sizes[] = {Sizes...};
    constexpr unsigned total_size = (Sizes * ...);
    constexpr unsigned num_dim = sizeof...(Sizes);
    using walker_t = unsigned[sizeof...(Sizes)];
    array_wrapper<walker_t[total_size]> res {{0}};
    
    
    for (unsigned k = 0; k < total_size; k += num_dim){
        increment<0, num_dim>(res.data[k], sizes);
    }
    
    return res;
} */ 

/* 
template <class Visitor, class... Vs, unsigned... Idx>
constexpr auto visit_impl(std::integer_sequence<std::size_t, Idx...>, Visitor&& v, Vs&&... var){
	constexpr auto index_table = make_flat_sequence<std::decay_t<Visitor>::size...>();
	using fn_ptr = void(*)(Visitor&&, Vs&&...);
	
	constexpr fn_ptr dispatcher[] = { 
		[] (Visitor&& v, Vs&&... var) {
			v( get< index_table[Idx][Vseq] >(var)... );
		}...
	};
	
	dispatcher[ make_flat_indice(var.index()...) ](v, var...);
} 

template <class Visitor, class... Vs>
auto visit(Visitor&& v, Vs&&... variants){
	constexpr auto total_size = (std::decay_t<Variants>::size * ...);
	constexpr auto dispatcher = make_table<Visitor&&, Vs&&...>(
		std::make_index_sequence<total_size>{}, 
		std::make_index_sequence<sizeof...(Vs)>{},
		std::integer_sequence<unsigned, std::decay_t<Variants>::size...>{},
		static_cast<V&&>(v), 
		static_cast<Vs&&>(variants)...
	);
} */ 

#define PACK int, float, char, bool, double, float
#define PACK1 PACK, PACK
#define PACK2 PACK, PACK, PACK, PACK
#define PACK3 PACK2, PACK2, PACK2, PACK2
#define PACK4 PACK3, PACK3, PACK3, PACK3
#define PACK5 PACK4, PACK4, PACK4, PACK4
#define PACK6 PACK5, PACK5

//#include <iostream>

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

#include "variant.hpp"
#include <iostream>
#include <variant>

int main(){
	
	//using namespace swl;
	std::variant<int, unsigned, float, char> vrx { 3 };
	
	visit( [] (auto x) { std::cout << x << std::endl;}, vrx );
	
	//show_type< swl::variant<int, float, char>::storage_t >();
	
	std::variant<PACK> vr1, vr2;
	
	vr1.emplace<0>(4);
	vr2.emplace<0>(4);
	
	//std::cout << (vr1 == vr2) << std::endl;
	
	visit( [] (auto v) {}, vr1 );
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