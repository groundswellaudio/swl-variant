#ifdef SWL_CPP_LIBRARY_VARIANT_HPP

template <class T>
T&& declval();

template <class T>
struct array_wrapper { 
	T data;
};

// find the first position of the type
template <class T, class... Ts>
constexpr std::size_t find_type(){
	constexpr auto size = sizeof...(Ts);
	constexpr bool same[size] = {std::is_same_v<T, Ts>...};
	for (std::size_t k = 0; k < size; ++k)
		if (same[k]) return k;
	return size;
}

// ============= type pack element 

template <class... Ts>
static constexpr unsigned true_ = sizeof...(Ts) < (1000000000000000);

template <unsigned char = 1>
struct find_type_i;

template <>
struct find_type_i<1> {
	template <std::size_t Idx, class T, class... Ts>
	using f = typename find_type_i<(Idx != 1)>::template f<Idx - 1, Ts...>;
};

template <>
struct find_type_i<0> {
	template <std::size_t, class T, class... Ts>
	using f = T;
};

template <std::size_t K, class... Ts>
using type_pack_element = typename find_type_i<(K != 0 and true_<Ts...>)>::template f<K, Ts...>;

// ============= overload match detector. to be used for variant generic assignment

template <std::size_t N, class A>
struct overload_frag {
	using type = A;
	overload_frag<N, A> operator()(A a);
};

template <class Seq, class... Args>
struct make_overload;

template <std::size_t... Idx, class... Args>
struct make_overload<std::integer_sequence<std::size_t, Idx...>, Args...>
	 : overload_frag<Idx, Args>... { 
	using overload_frag<Idx, Args>::operator()...;
};

template <class T, class... Ts>
using best_overload_match 
	= typename decltype( make_overload<std::make_index_sequence<sizeof...(Ts)>, Ts...>{}(std::declval<T>()) 
					   )::type;

// ================================== rel ops

template <class T>
concept has_eq_comp = requires (T a, T b) { bool(a == b); };

template <class T>
concept has_lesser_comp = requires (T a, T b) { bool(a < b); };

template <class T>
concept has_lesser_than_comp = requires (T a, T b) { bool(a <= b); };

template <class A>
struct eq_comp {
	constexpr bool operator()(const auto& elem, auto index) const noexcept { 
		return (a.template get<index>() == elem); 
	}
	const A& a;
};

template <class A>
struct emplace_into {
	constexpr void operator()(auto&& elem, auto index) const {
		a.template emplace<index>(decltype(elem)(elem));
	}
	A& a;
};
	
// =============================== variant union types
	
// =================== base variant storage type 
// this type is used to build a N-ary tree of union. 

struct dummy_type{}; // used to fill the back of union nodes

template <bool IsTerminal, class... Ts>
union variant_union;

template <class A, class B>
union variant_union<false, A, B> {
	
	static constexpr auto elem_size = A::elem_size + B::elem_size;
	
	constexpr variant_union() = default;
	
	template <std::size_t Index, class... Args>
		requires (Index < A::elem_size)
	constexpr variant_union(in_place_index_t<Index>, Args&&... args)
	: a{ in_place_index<Index>, static_cast<Args&&>(args)... } {}
	
	template <std::size_t Index, class... Args>
		requires (Index >= A::elem_size)
	constexpr variant_union(in_place_index_t<Index>, Args&&... args)
	: b{ in_place_index<Index - A::elem_size>, static_cast<Args&&>(args)... } {} 
	
	template <std::size_t Index>
	constexpr auto& get(){
		if constexpr 		( Index < A::elem_size )
			return a.template get<Index>();
		else 
			return b.template get<Index - A::elem_size>();
	}
	
	A a;
	B b;
};

template <class A, class B>
union variant_union<true, A, B> {
	
	static constexpr unsigned short elem_size = not( std::is_same_v<B, dummy_type> ) ? 2 : 1;
	
	constexpr variant_union() = default;
	
	template <class... Args>
	constexpr variant_union(in_place_index_t<0>, Args&&... args)
	: a{static_cast<Args&&>(args)...} {}
	
	template <class... Args>
	constexpr variant_union(in_place_index_t<1>, Args&&... args)
	: b{static_cast<Args&&>(args)...} {}
	
	template <std::size_t Index>
	constexpr auto& get(){
		if constexpr 		( Index == 0 )
			return a;
		else return b;
	}
	
	A a;
	B b;
};

struct valueless_construct_t{};

template <class Impl>
union variant_top_union{
	constexpr variant_top_union() = default;
	constexpr variant_top_union(valueless_construct_t) : dummy{} {}
	
	template <class... Args>
	constexpr variant_top_union(Args&&... args) : impl{static_cast<Args&&>(args)...} {}
	
	Impl impl;
	dummy_type dummy;
};

// =================== algorithm to build the tree of unions 
// take a sequence of types and perform an order preserving fold until only one type is left
// the first parameter is the numbers of types remaining for the current pass

constexpr unsigned char pick_next(unsigned remaining){
	return remaining >= 2 ? 2 : remaining;
}

template <unsigned char Pick, unsigned char GoOn = 1, bool FirstPass = false>
struct make_tree;

template <bool IsFirstPass>
struct make_tree<2, 1, IsFirstPass> {
	template <unsigned Remaining, class A, class B, class... Ts>
	using f = typename make_tree<pick_next(Remaining - 2), 
								 sizeof...(Ts) != 0,
								 IsFirstPass
								 >::template f< Remaining - 2, 
								 				Ts..., 
								 				variant_union<IsFirstPass, A, B>
								 			  >; 
};

// only one type left, stop
template <bool F>
struct make_tree<0, 0, F> {
	template <unsigned, class A>
	using f = A;
};

// end of one pass, restart
template <bool IsFirstPass>
struct make_tree<0, 1, IsFirstPass> {
	template <unsigned Remaining, class... Ts>
	using f = typename make_tree<pick_next(sizeof...(Ts)), 
								 (sizeof...(Ts) != 1), 
								 false // <- both first pass and tail call recurse into a tail call
								>::template f<sizeof...(Ts), Ts...>;
};

// one odd type left in the pass, put it at the back to preserve the order
template <>
struct make_tree<1, 1, false> {
	template <unsigned Remaining, class A, class... Ts>
	using f = typename make_tree<0, sizeof...(Ts) != 0, false>::template f<0, Ts..., A>;
};

// one odd type left in the first pass, wrap it in an union
template <>
struct make_tree<1, 1, true> {
	template <unsigned, class A, class... Ts>
	using f = typename make_tree<0, sizeof...(Ts) != 0, false>
		::template f<0, Ts..., 
					 variant_union<true, A, dummy_type>
					>;
};

template <class... Ts>
using make_tree_union = typename 
	make_tree<pick_next(sizeof...(Ts)), 1, true>::template f<sizeof...(Ts), Ts...>;


template <class T, class... Ts>
inline constexpr bool has_non_ambiguous_match 
	= requires { make_overload<std::make_index_sequence<sizeof...(Ts)>, Ts...>{}(declval<T>()); };

// ============================================================

// Ts... must be sorted in ascending size 
template <std::size_t Num, class... Ts>
using smallest_suitable_integer_type = 
	type_pack_element<(static_cast<unsigned char>(Num > std::numeric_limits<Ts>::max()) + ...),
					  Ts...
					  >;
					  
#endif
					  