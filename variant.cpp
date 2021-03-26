
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
/* 
#include <iostream>
#include <variant> */ 

//#include <variant>
/*
struct porridge {
	int x, y;
}; */ 

#define DEC(N) X((N)) X((N) + 1) X((N) + 2) X((N) + 3) X((N) + 4) X((N) + 5) X((N) + 6) X((N) + 7) X((N) + 8) X((N) + 9)

#define SEQ30(N) DEC( (N) + 0 ) DEC( (N) + 10 ) DEC( (N) + 20 ) 

#define SEQ100(N) SEQ30((N) + 0) SEQ30((N) + 30) SEQ30((N) + 60) DEC((N) + 90) 

#define SEQ200(N) SEQ100((N) + 0) SEQ100((N) + 100)

#define SEQ400(N) SEQ200((N) + 0) SEQ200((N) + 200)

#define SEQ800(N) SEQ400((N) + 0) SEQ400((N) + 400)

#define SEQ1600(N) SEQ800((N) + 0) SEQ800((N) + 800)

#define DOUBLE(X) SEQ##X((N) + 0) SEQ##X((N) + X)

#define SEQ3200(N) DOUBLE(1600)

#define SEQ6400(N) DOUBLE(3200)


#if defined(__GNUC__) || defined( __clang__ ) || defined( __INTEL_COMPILER )
	#define DeclareUnreachable __builtin_unreachable()
#elif defined (_MSC_VER)
	#define DeclareUnreachable __assume(false)
#endif

using vt = swl::variant<PACK6>;

#if 1
	float func(vt& vvv){
	
		float x = 0;
	
		visit( [&x] (auto z) { x += static_cast<float>(z); }, vvv );
	
		return x;
	}
#endif


template <unsigned Idx, class V>
auto&& elem_get(V&& v){
	if constexpr (Idx < std::decay_t<V>::size)
		return v.template unsafe_get<Idx>();
	else 
		return v.template unsafe_get<0>();
}


template <unsigned Offset, class Fn, class V>
constexpr decltype(auto) visit_tail(Fn&& fn, V&& v){

	constexpr auto var_size = std::decay_t<V>::size - Offset;
	
	#define X(N) case (N + Offset) : \
		if constexpr (N + Offset < var_size) { \
			return static_cast<Fn&&>(fn)( elem_get<N + Offset>( static_cast<V&&>(v) ) ); \
			break; \
		} else DeclareUnreachable;
	
	if constexpr (var_size <= 200) {
		switch( v.index() ){
			SEQ200(0)
			default : 
				DeclareUnreachable;
		}
	}
	else if constexpr (var_size <= 800) {
		switch( v.index() ){
			SEQ800(0)
			default : 
				DeclareUnreachable;
		}
	}
	else {
		switch( v.index() ){
			SEQ1600(0)
			default : 
				if constexpr (var_size > 1600)
					return visit_tail<Offset + 1600>(static_cast<Fn&&>(fn), static_cast<V&&>(v));
				else 
					DeclareUnreachable;
		}
	}
	
	#undef X
}

template <unsigned Offset, class Fn, class... Args>
constexpr decltype(auto) multi_visit_tail(Fn&& fn, Vs&&... vs){
	
	constexpr auto total_size = (std::decay_t<Vs>::size * ...);
	const auto flat_idx = swl::vimpl::flatten_indices< std::decay_t<Vs>::size... >(vs.index()...);
	
	#define X(N) case (N + Offset) : \
		if constexpr (N + Offset < total_size) { \
			constexpr auto var_idx = unflatten(N, var_sizes);
			return static_cast<Fn&&>(fn)( elem_get<var_idx.data[Vx]>( static_cast<Vs&&>(v) )... ); \
			break; \
		} else 
			DeclareUnreachable;
		
	switch( flat_idx ) {
		
		SEQ1600(0)
		
		default : 
			if constexpr (total_size > 1600)
				return multi_visit_tail<Offset + 1600>(static_cast<Fn&&>(fn), static_cast<Vs&&>(vs)...);
			else
				DeclareUnreachable;
	}
}

template <class Fn, class V>
constexpr decltype(auto) visit(Fn&& fn, V&& v){
	return visit_tail<0>(static_cast<Fn&&>(fn), static_cast<V&&>(v));
}

template <class Fn, class V, class... Vs>
constexpr decltype(auto) visit(Fn&& fn, V&& v, Vs&&... vs){
	return multi_visit_tail<0>(static_cast<Fn&&>(fn), static_cast<V&&>(v), static_cast<Vs&&>(vs)...);
}

#if 0
float func(vt& vvv){
	
	float x = 0;
	
	::visit( [&x] (auto z) { x += static_cast<int>(z); }, vvv );
	
	#undef X
	
	return x;
}
#endif

/* 
int main(){
	
	//swl::variant<int, std::string> vst { swl::in_place_index<1>, "hehehe" };
	
	//static_assert( not std::is_trivially_destructible_v<swl::variant<std::string, int>> );
	
	
	//vvz = "hello";
	//static_assert( swl::variant<my_type, int>::trivial_dtor );
	
	//swl::variant<int> v;
	
	
	//c = "heheeehee";
	
	using namespace swl;
	
	//std::cout << (vr1 == vr2) << std::endl;
	
	//visit( [] (auto... args) {}, v1, v2, v3, v4);
	
	
	//variant<PACK6> a, b, c, d;
	
	//variant<int, porridge> vvv;
	
	variant<PACK4> vvv;
	
	vvv.emplace<4>(2);
	
	int x = 0;
	
	swl::visit( [&x] (auto z) { x += static_cast<int>(z); }, vvv );
	
	//visit( [] (auto... args) {}, a);
	
	//visit( [] (auto a, auto b) {}, vr1, vr2 );
	
	//visit([] (auto& v) {}, vrx);
	//using seq = make_index_seq<1>::template f<1024, 0>;
	
	//constexpr auto idx = first_index<char**, PACK4, char**>;
	
	//using tt = __type_pack_element<800, PACK5>;
	
	//show_type<tt>();
	
	//using t = make_tree_1st<4>::template f<int, float, char, double>;
	//show_type<t>();
	
	
} */ 