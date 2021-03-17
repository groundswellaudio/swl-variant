#include <type_traits>
#include <utility>
#include <new>
#include <limits>

namespace swl {

namespace variant__ {
	
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
}

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
using type_pack_element = typename variant__::find_type_i<(K != 0 and variant__::true_<Ts...>)>::template f<K, Ts...>;

template <class T>
T&& declval();

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

template <class T, class... Ts>
inline constexpr bool has_non_ambiguous_match 
	= requires { make_overload<std::make_index_sequence<sizeof...(Ts)>, Ts...>{}(declval<T>()); };
	
// =================== base variant storage type 
// this type is used to build a N-ary tree of union. 

struct dummy_type{}; // used to fill the back of union nodes

template <bool IsTerminal, class A, class B>
union variant_union;

template <class T>
struct inplace_type_t{};

template <std::size_t Index>
inline static constexpr in_place_index_t<Index> in_place_index;

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

// ============ faster variadic common_type implementation 

constexpr unsigned char next_common_type(unsigned x){
	return x >= 16 ? 3 : x > 8 ? 2 : (x != 0);
}

template <unsigned char Go>
struct common_type;

template <>
struct common_type<1> {
	template <class A, class B, class... Ts>
	using f = typename common_type<next_common_type(sizeof...(Ts))>::template f<std::common_type_t<A, B>, Ts...>;
};

template <>
struct common_type<0>{
	template <class A>
	using f = A;
};

template <>
struct common_type<2>{
	
	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class... Ts>
	using f = typename common_type<next_common_type(sizeof...(Ts))>
::template f< 
	
	std::common_type_t<T1, 
		std::common_type_t<T2, 
			std::common_type_t<T3,
				std::common_type_t<T4,
					std::common_type_t<T5,
						std::common_type_t<T6,
							std::common_type_t<T7, T8>
						>
					>
				>
			>
		>
	>, Ts...>;			
};

template <>
struct common_type<3>{
	
	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8,
			  class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16,
			  class... Ts>
	using f = typename common_type<next_common_type(sizeof...(Ts))>
::template f< 
	
	#define ITER(X, ...) std::common_type_t<T##X, __VA_ARGS__>
	
	ITER(1, 
		ITER(2, 
			ITER(3, 
				ITER(4, 
					ITER(5, 
						ITER(6, 
							ITER(7, 
								ITER(8, 
									ITER(9, 
										ITER(10, 
											ITER(11, 
												ITER(12, 
													ITER(13,
														ITER(14, 
																std::common_type_t<T15, T16>
															)
														)
													)
												)
											)
										)
									)
								)
							)
						)
					)
				)
			)
		), Ts...>;
	
	#undef ITER	
};

template <class... Ts>
using common_type_t = typename common_type<next_common_type(sizeof...(Ts) - 1)>::template f<Ts...>;

// ============================================================

template <class... Ts>
struct variant;

template <std::size_t Idx, class... Ts>
auto& get (variant<Ts...>& v);

template <std::size_t Num, class... Ts>
using smallest_suitable_integer_type = 
	type_pack_element<static_cast<unsigned char>(Num > (std::numeric_limits<Ts>::max() + ...)),
					  Ts...
					  >;

// ========================= visit dispatcher

template <class Rt, class Variant, class Fn, unsigned Idx>
constexpr Rt visit__(Variant v, Fn fn){
	return fn( get<Idx>(v) );
}

template <bool B>
struct common_type_impl;

template <>
struct common_type_impl<true> {
	template <class A, class... Ts>
	using f = A;
};

template <>
struct common_type_impl<false> {
	template <class A, class... Ts>
	using f = std::common_type_t<A, Ts...>;
};

template <bool B>
struct all_same;

template <>
struct all_same<true>{
	template <class A, class... Ts>
	static constexpr bool v = (std::is_same_v<A, Ts> && ...);
};

template <class... Ts>
using common_type__ = typename common_type_impl<all_same<(sizeof...(Ts) != 0)>::template v<Ts...>>::template f<Ts...>;

template <class Seq, bool PassIndex>
struct make_dispatcher;

template <std::size_t... Idx>
struct make_dispatcher<std::integer_sequence<std::size_t, Idx...>, false> {
	
	template <class Variant, class Fn>
	using ReturnType = //common_type__<decltype( declval<Fn>()( get<Idx>(declval<Variant>()) ) )...>;
		 std::common_type_t< decltype(
			declval<Fn>()( get<Idx>(declval<Variant>()) )
		)
		...  
	>; 
	
	template <class Variant, class Visitor>
	using fn_ptr = ReturnType<Variant, Visitor>(*)(Variant, Visitor visitor);
	
	template <class Variant, class Fn>
	static constexpr fn_ptr<Variant, Fn> dispatcher[sizeof...(Idx)] = {
		//&visit__<ReturnType<Variant, Fn>, Variant, Fn, Idx>...
		[] (Variant var, Fn self) {
				return self(get<Idx>(var));
		}...
	};
};

template <std::size_t... Idx>
struct make_dispatcher<std::integer_sequence<std::size_t, Idx...>, true> {
	
	template <class Variant, class Fn>
	using ReturnType = //common_type__<decltype( declval<Visitor>()( get<Idx>(declval<Variant>()) ) )...>;
		 std::common_type_t< decltype(
			declval<Fn>()( get<Idx>(declval<Variant>(), std::integral_constant<std::size_t, Idx>{}) )
		)
		... 
	>;  
	
	template <class Variant, class Visitor>
	using fn_ptr = ReturnType<Variant, Visitor>(*)(Variant, Visitor visitor);
	
	template <class Variant, class Visitor>
	static constexpr fn_ptr<Variant, Visitor> dispatcher[sizeof...(Idx)] = {
		[] (Variant var, Visitor self) {
				return self(get<Idx>(var), std::integral_constant<std::size_t, Idx>{});
		}...
	};
};

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
		make_dispatcher_t<false>::template dispatcher<variant&, VisitorType&&>[current](*this, decltype(fn)(fn));
	}
	
	template <class VisitorType>
	constexpr decltype(auto) visit_with_index(VisitorType&& fn){ 
		make_dispatcher_t<true>::template dispatcher<variant&, VisitorType&&>[current](*this, decltype(fn)(fn));
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
	
	using storage_t = variant_top_union<make_tree_union<Ts...>>;
	
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


}