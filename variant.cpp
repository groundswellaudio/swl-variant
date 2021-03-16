#include <type_traits>
#include <utility>
#include <new>
#include <limits>
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

template <class... Ts>
static constexpr unsigned true_ = sizeof...(Ts) < (1000000000000000);

template <unsigned = 1>
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

template <class T>
T&& declval();

template <std::size_t N, class A>
struct overload_frag {
	using type = A;
	friend overload_frag<N, A> call(overload_frag<N, A>, A a);
};

template <class Seq, class... Args>
struct make_overload;

template <std::size_t... Idx, class... Args>
struct make_overload<std::integer_sequence<std::size_t, Idx...>, Args...>
	 : overload_frag<Idx, Args>... {};

template <class T, class... Ts>
using best_overload_match 
	= typename decltype( call(make_overload<std::make_index_sequence<sizeof...(Ts)>, Ts...>{}, std::declval<T>())
			  		   )::type;

template <class T, class... Ts>
inline constexpr bool has_non_ambiguous_match 
	= requires { call(make_overload<std::make_index_sequence<sizeof...(Ts)>, Ts...>{}, declval<T>()); };

template <std::size_t, class, class>
union variant_union;

template <class T>
struct is_variant_union : std::false_type{};

template <std::size_t N, class... Ts>
struct is_variant_union<variant_union<N, Ts...>> : std::true_type{};

template <std::size_t N>
using cst = std::integral_constant<std::size_t, N>;

template <class T, std::size_t N>
constexpr bool find_first( const T(&array)[N], const T v ){
	const T* it = &array[0];
	while (it != &array[N] and *it != v) ++it;
	return (it - &array[0]);
}

template <class T, class... Ts>
constexpr std::size_t find_type(){
	constexpr auto size = sizeof...(Ts);
	constexpr bool same[size] = {std::is_same_v<T, Ts>...};
	for (std::size_t k = 0; k < size; ++k)
		if (same[k]) return k;
	return size;
}

template <std::size_t Index>
struct in_place_index_t{};

template <std::size_t Index>
inline static constexpr in_place_index_t<Index> in_place_index;

template <std::size_t NumElems, class A, class B>
union variant_union {
	
	static constexpr auto elem_size = NumElems;
	
	constexpr variant_union() = default;
	
	template <std::size_t Index, class... Args>
		requires (Index < A::elem_size)
	constexpr variant_union(in_place_index_t<Index>, Args&&... args)
	: a{ in_place_index<Index>, static_cast<Args&&>(args)... } {}
	
	template <std::size_t Index, class... Args>
		requires (Index >= A::elem_size)
	constexpr variant_union(in_place_index_t<Index>, Args&&... args)
	: b{ in_place_index<Index - A::elem_size>, static_cast<Args&&>(args)... } {} */ 
	
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

template <std::size_t NumElems, class A, class B>
union variant_leaf {
	
	static constexpr auto elem_size = NumElems;
	
	template <class... Args>
	constexpr variant_leaf(in_place_index_t<0>, Args&&... args)
	: a{static_cast<Args&&>(args)...} {}
	
	template <class... Args>
	constexpr variant_leaf(in_place_index_t<1>, Args&&... args)
	: b{static_cast<Args&&>(args)...} {}
	
	constexpr variant_leaf() = default;
	
	template <std::size_t Index>
	constexpr auto& get(){
		if constexpr 		( Index == 0 )
			return a;
		else return b;
	}
	
	A a;
	B b;
};

template <class Impl>
struct variant_top_union{
	
	struct valueless_t{};
	
	constexpr variant_top_union(valueless_t) : dummy{} {}
	
	template <class... Args>
	constexpr variant_top_union(Args&&... args) : impl{static_cast<Args&&>(args)...} {}
	
	unsigned char dummy; // for temporary valueless construct
	Impl impl;
};

template <bool B>
struct dcall;

template <>
struct dcall<true> {
	template <template <class...> class Fn, class... Ts>
	using f = Fn<Ts...>;
};

template <>
struct dcall<false> {
	template <template <class...> class Fn, class... Ts>
	using f = void;
};

template <std::size_t Clk, std::size_t Flag = 1>
struct make_tree;

template <std::size_t Clk>
struct make_tree<Clk, 1> {
	/* 
	template <class A, class B, class... Ts>
	using f = typename make_tree<Clk - 2, sizeof...(Ts) != 0>::template f<Ts..., variant_union<2, A, B>>;  */ 
	template <class A, class B, class... Ts>
	using f = typename make_tree<Clk - 2, sizeof...(Ts) != 0>
		::template f<Ts..., variant_union<A::elem_size + B::elem_size, A, B>>; 
};

// only one type left, stop
template <>
struct make_tree<0, 0> {
	template <class A>
	using f = A;
};

// end of one pass, restart
template <>
struct make_tree<0, 1> {
	template <class... Ts>
	using f = typename make_tree<sizeof...(Ts)>::template f<Ts...>;
};

// one odd type left in the pass, put it at the back to preserve the order
template <>
struct make_tree<1, 1> {
	template <class A, class... Ts>
	using f = typename make_tree<0, sizeof...(Ts) != 0>::template f<Ts..., A>;
};

// ============================================================

template <std::size_t Clk, unsigned = 1> // just a bogus argument to enable recursive alias
struct make_tree_1st;

template <std::size_t Clk>
struct make_tree_1st<Clk, 1> {
	template <class A, class B, class... Ts>
	using f = typename make_tree_1st<Clk - 2, true_<Ts...>>::template f<Ts..., variant_leaf<2, A, B>>; 
};

// end of the 1st pass, start the tail call
template <>
struct make_tree_1st<0, 1> {
	template <class... Ts>
	using f = typename make_tree<sizeof...(Ts)>::template f<Ts...>;
};

// one odd type left in the pass, put it at the back to preserve the order
template <>
struct make_tree_1st<1, 1> {
	template <class A, class... Ts>
	using f = typename make_tree_1st<0>::template f<Ts..., variant_leaf<1, A, char>>;
};

template <class T>
struct inplace_type_t{};

//#include <memory>

	
template <class... Ts>
struct variant;

template <std::size_t Idx, class... Ts>
auto& get (variant<Ts...>& v);

template <std::size_t Num, class... Ts>
using smallest_suitable_integer_type = 
	type_pack_element<static_cast<unsigned char>(Num > (std::numeric_limits<Ts>::max() + ...)),
					  Ts...
					  >;

template <class... Ts>
struct variant {
	
	static constexpr bool has_copy_assign 		= (std::is_copy_constructible_v<Ts> && ...);
	static constexpr bool has_move_ctor			= (std::is_move_constructible_v<Ts> && ...);
	static constexpr bool has_copy_ctor			= (std::is_copy_constructible_v<Ts> && ...);
	static constexpr bool trivial_move_ctor		= has_move_ctor && (std::is_trivially_move_constructible_v<Ts> && ...);
	static constexpr bool trivial_copy_ctor 	= (std::is_trivially_copy_constructible_v<Ts> && ...);
	static constexpr bool trivial_copy_assign 	= has_copy_assign && (std::is_trivially_copy_assignable_v<Ts> && ...);
	static constexpr bool trivial_dtor 			= (std::is_trivially_destructible_v<Ts> && ...);
	
	static constexpr unsigned size = sizeof...(Ts);
	
	using index_type = smallest_suitable_integer_type<sizeof...(Ts) + 1, unsigned char, unsigned short, unsigned>;
	
	static constexpr index_type npos = -1;
	
	template <std::size_t Idx>
	using alternative = type_pack_element<Idx, Ts...>;
	
	template <class Seq, bool PassIndex>
	struct make_dispatcher;
	
	template <std::size_t... Idx>
	struct make_dispatcher<std::integer_sequence<std::size_t, Idx...>, false> {
		
		template <class T>
		using ReturnType = std::common_type_t<
			decltype( declval<T>()( get<Idx>(declval<variant<Ts...>&>()) ) )... >;
		
		template <class T>
		using fn_ptr = ReturnType<T>(*)(variant<Ts...>&, T visitor);
		
		template <class T>
		static constexpr fn_ptr<T> dispatcher[sizeof...(Ts)] = {
			[] (variant<Ts...>& var, T self) -> ReturnType<T> {
					return self(get<Idx>(var));
			}...
		};
	};
	
	template <std::size_t... Idx>
	struct make_dispatcher<std::integer_sequence<std::size_t, Idx...>, true> {
		
		template <class T>
		using ReturnType = std::common_type_t<
								std::invoke_result_t<T, 
													 variant<Ts...>&, 
										   			 std::integral_constant<std::size_t, Idx>
													>...
											>;
		
		template <class T>
		using fn_ptr = ReturnType<T>(*)(variant<Ts...>&, T visitor);
		
		template <class T>
		static constexpr fn_ptr<T> dispatcher[sizeof...(Ts)] = {
			[] (variant<Ts...>& var, T self) -> decltype(auto) {
					return self(get<Idx>(var), std::integral_constant<std::size_t, Idx>{});
			}...
		};
	};
	
	template <bool PassIndex = false>
	using make_dispatcher_t = make_dispatcher<std::make_index_sequence<sizeof...(Ts)>, PassIndex>;
	
	// default constructor
	constexpr variant() 
		noexcept 
		requires std::is_default_constructible_v<alternative<0>> 
	= default;
	
	template <std::size_t Index, class... Args>
	constexpr variant(in_place_index_t<Index> tag, Args&&... args)
	: storage{tag, static_cast<Args&&>(args)...} {}
	
	// trivial move ctor
	constexpr variant(variant&&)
		requires trivial_move_ctor
	= default;
	
	// move ctor
	constexpr variant(variant&& o)
		requires (has_move_ctor and not trivial_move_ctor)
	: storage{ storage_t::valueless_t() } 
	{
		o.visit_with_index( [this] (auto&& elem, auto index_) {
			emplace<index_>(decltype(elem)(elem));
		});
	}
	
	// copy ctor
	constexpr variant(const variant& o)
		requires has_copy_ctor
	: storage{ storage_t::valueless_t() } 
	{
		o.visit_with_index( [this] (auto&& elem, auto index_) {
			emplace<index_>(decltype(elem)(elem));
		});
	}
	
	// generic assignment 
	template <class T>
		requires has_non_ambiguous_match<T, Ts...>
	constexpr variant& operator=(T&& t) {
		using related_type = best_overload_match<T, Ts...>;
		constexpr auto new_index = find_type<related_type, Ts...>();
		if (current == new_index)
			get<new_index>(*this) = static_cast<T&&>(t);
		else 
			emplace<new_index>(static_cast<T&&>(t));
		return *this;
	}
	
	// trivial copy assignment
	constexpr variant& operator=(const variant& o)
		requires trivial_copy_assign && trivial_copy_ctor 
	= default;
	
	// copy assignment 
	constexpr variant& operator=(const variant& o)
		requires (has_copy_assign and not(trivial_copy_assign && trivial_copy_ctor))
	{	
		if (o.index() == index()){
			o.visit_with_index( [this] (const auto& elem, auto index_cst) {
				get<index_cst>(*this) = elem;
			});
		}
		else {
			// if (has nothrow_copy or hasn't move_construct)
			o.visit_with_index( [this] (const auto& elem, auto index_cst) {
				emplace<index_cst>(elem);
			});
		}
		return *this;
	}
	
	// move assignment
	constexpr variant& operator=(variant&& o){
		
		o.visit_with_index( [this] (auto&& elem, auto index_cst) {
			if (index() == index_cst)
				get<index_cst>(*this) = std::move(elem);
			else 
				emplace<index_cst>(std::move(elem));
		});
	}
	
	template <std::size_t Idx, class... Args>
	auto& emplace(Args&&... args){
		using T = std::remove_reference_t<decltype(get<Idx>(*this))>;
		if constexpr (not std::is_nothrow_constructible_v<T, Args&&...>)
			current = -1;
		new(static_cast<void*>(&get<Idx>(*this))) T (static_cast<Args&&>(args)...);
		current = Idx;
		return get<Idx>(*this);
		//std::construct_at( &get<Idx>(*this), args... );
	}
	
	template <class VisitorType>
	constexpr decltype(auto) visit(VisitorType&& fn){
		make_dispatcher_t<false>::template dispatcher<VisitorType&&>[current](*this, fn);
	}
	
	template <class VisitorType>
	constexpr decltype(auto) visit_with_index(VisitorType&& fn){ 
		make_dispatcher_t<true>::template dispatcher<VisitorType&&>[current](*this, decltype(fn)(fn));
	}
	
	// ====================== probes 
	constexpr bool valueless_by_exception() const noexcept {
		return current == npos;
	}
	
	constexpr std::size_t index() const noexcept {
		return current;
	}
	
	// trivial destructor
	constexpr ~variant() requires trivial_dtor = default;
	
	// destructor
	constexpr ~variant() requires (not trivial_dtor) {
		visit( [] (auto& elem) {
			using type = std::decay_t<decltype(elem)>;
			if constexpr ( not std::is_trivially_destructible_v<type> )
				elem.~type();
		});
	}
	
	using storage_t = variant_top_union<typename make_tree_1st<sizeof...(Ts)>::template f<Ts...>>;
	
	storage_t storage;
	index_type current;
};

template <std::size_t Idx, class... Ts>
auto& get (variant<Ts...>& v){
	static_assert( Idx < sizeof...(Ts), "Index exceeds the variant size. ");
	return (v.storage.impl.template get<Idx>());
} 

template <class T, class... Ts>
T& get (variant<Ts...>& v){
	return get<find_type<T, Ts...>()>(v);
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
}

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

#define PACK int, float*, char, bool, double, float, char
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

int main(){

	//using seq = make_index_seq<1>::template f<1024, 0>;
	
	//constexpr auto idx = first_index<char**, PACK4, char**>;
	
	//using tt = __type_pack_element<800, PACK5>;
	
	//show_type<tt>();
	
	//using t = make_tree_1st<4>::template f<int, float, char, double>;
	//show_type<t>();
	
	variant<PACK> v, z;
	
	v.emplace<2>(44);
	z.emplace<3>(true);
	
	v = z;
	v = true;
	//std::visit([] (auto x) {}, v);
	//v.visit([] (auto x) {});
	
	//v = true;
	
	//std::cout << v.index() << " " << get<3>(v) << std::endl;
	
	//auto& i = get<100>(v);
}