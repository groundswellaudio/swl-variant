#include <type_traits>
#include <utility>
#include <new>
#include <limits>

namespace swl {

template <class T>
struct array_wrapper { 
	T data;
};

template <class T, class... Ts>
constexpr std::size_t find_type(){
	constexpr auto size = sizeof...(Ts);
	constexpr bool same[size] = {std::is_same_v<T, Ts>...};
	for (std::size_t k = 0; k < size; ++k)
		if (same[k]) return k;
	return size;
}

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

template <bool IsTerminal, class... Ts>
union variant_union;

template <class T>
struct inplace_type_t{};

template <std::size_t Index>
struct in_place_index_t{};

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

// ============================================================

template <std::size_t Num, class... Ts>
using smallest_suitable_integer_type = 
	type_pack_element<static_cast<unsigned char>(Num > (std::numeric_limits<Ts>::max() + ...)),
					  Ts...
					  >;
					  
template <class... Ts>
struct variant;

// ========================= visit dispatcher

template <unsigned Idx, class Fn, class Var>
constexpr decltype(auto) visit__private_simple__(Fn fn, Var var){
	return static_cast<Fn&&>(fn)( static_cast<Var&&>(var) );
}

template <unsigned Idx, class Fn, class Var>
constexpr decltype(auto) visit_with_index__(Fn fn, Var v){
	return static_cast<Fn&&>(fn)(static_cast<Var&&>(v).template get<Idx>(), std::integral_constant<unsigned, Idx>{});
}

template <class Seq, bool PassIndex>
struct make_dispatcher;

template <class Fn, class... Vars>
using rtype_visit = decltype( declval<Fn>()( declval<Vars>().template get<0>()... ) );

template <class Fn, class Var>
using rtype_index_visit = decltype( declval<Fn>()( declval<Var>().template get<0>(), 
								 	std::integral_constant<std::size_t, 0>{} ) );
	
template <std::size_t... Idx>
struct make_dispatcher<std::integer_sequence<std::size_t, Idx...>, false> {
	
	template <class Fn, class Var>
	static constexpr rtype_visit<Fn, Var> (*dispatcher[sizeof...(Idx)]) (Fn, Var) = {
		[] (Fn self, Var var) {
				return self( static_cast<Var&&>(var).template get<Idx>() );
		}...
		//::swl::visit__private_simple__<Idx, Fn, Var>...
	};
};
								 	
template <std::size_t... Idx>
struct make_dispatcher<std::integer_sequence<std::size_t, Idx...>, true> {
	
	template <class Fn, class Var>
	static constexpr rtype_index_visit<Fn, Var>(*dispatcher[sizeof...(Idx)]) (Fn, Var) = {
		[] (Fn fn, Var var){
			return static_cast<Fn&&>(fn)(static_cast<Var&&>(var).template get<Idx>(), std::integral_constant<unsigned, Idx>{});
		}... 
		//::swl::visit_with_index__<Idx, Fn, Var>...
	};
};

template <std::size_t N>
constexpr auto unflatten(unsigned Idx, const unsigned(&sizes)[N]){
	array_wrapper<unsigned[N]> res;
	for (unsigned k = 1; k < N; ++k){
		const auto prev = Idx;
		Idx /= sizes[k];
		res.data[N - k] = prev - Idx * sizes[k];
	}
	res.data[0] = Idx;
	return res;
}

template <unsigned... Sizes>
constexpr unsigned flatten_indices(const auto... args){	
	unsigned res = 0;
	([&] () { res *= Sizes; res += args; }(), ...);
	return res;
}

namespace v1 {

template <unsigned NumVariants, class Seq = std::make_integer_sequence<unsigned, NumVariants>>
struct multi_dispatcher;

template <unsigned NumVariants, unsigned... Vx>
struct multi_dispatcher<NumVariants, std::integer_sequence<unsigned, Vx...>> {

	template <unsigned Size, class Seq = std::make_integer_sequence<unsigned, Size>>
	struct with_table_size;
	
	template <unsigned TableSize, unsigned... Idx>
	struct with_table_size<TableSize, std::integer_sequence<unsigned, Idx...> > {
		
		template <class... Vars>
		static constexpr unsigned var_sizes[sizeof...(Vars)] = {std::decay_t<Vars>::size...};
		
		template <unsigned Indice, class Fn, class... Vars>
		static constexpr decltype(auto) func(Fn fn, Vars... vars) {
			constexpr auto seq = unflatten<sizeof...(Vars)>(Indice, var_sizes<Vars...>);
			return fn( get<seq.data[Vx]>(vars)... );
		}
		
		template <class Fn, class... Vars>
		using ReturnType = decltype( func<0, Fn, Vars...>(declval<Fn>(), declval<Vars>()...) );
		
		// ugly typename, but GCC doesn't like using an alias here?
		template <class Fn, class... Vars>
		static constexpr ReturnType<Fn, Vars...>( *impl[TableSize] )(Fn, Vars...) = {
			func<Idx, Fn, Vars...>... 
		};
		
	};
};

} // V1

//  for visit of two variants of size 24 : clang ~1.2 sec, gcc ~4.45 sec

inline namespace v2 {

template <unsigned N, unsigned TotalSize>
constexpr auto make_unflattened_indices(const unsigned(&strides)[N]){
	array_wrapper<unsigned[TotalSize][N]> res;
	
	float walker[N - 1] {0};
	const auto delta_walk = [&strides] () {
		array_wrapper<float[N - 1]> res {{ 1.f / static_cast<float>(strides[N - 1]) }};
		for (unsigned k = 0; k < N - 2; ++k)
			res.data[N - k - 3] /= (static_cast<float>(strides[N - 1 - k]) * res.data[N - k - 2]);
		return res;
	}();
	
	unsigned walk_lsb = 0;
	for (unsigned k = 0; k < TotalSize; ++k){
		for (unsigned x = 0; x < N - 1; ++x){
			res.data[k][x] = walker[x];
			walker[x] += delta_walk.data[x];
			if (static_cast<unsigned>(walker[x]) >= strides[x]) walker[x] = 0;
		}
		res.data[k][N - 1] = walk_lsb;
		++walk_lsb;
		if (walk_lsb >= strides[N - 1]) walk_lsb = 0;
	}
	return res;
}

template <unsigned NumVariants, class Seq = std::make_integer_sequence<unsigned, NumVariants>>
struct multi_dispatcher;

template <unsigned NumVariants, unsigned... Vx>
struct multi_dispatcher<NumVariants, std::integer_sequence<unsigned, Vx...>> {
	
	template <unsigned... VarSizes>
	struct make_indices {
	
		static constexpr unsigned total_size = (VarSizes * ...);
		static constexpr unsigned var_sizes[sizeof...(VarSizes)] = {VarSizes...};
		
		static constexpr array_wrapper<unsigned[total_size][sizeof...(VarSizes)]> indices = 
			make_unflattened_indices<sizeof...(VarSizes), total_size>(var_sizes);
	};
	
	template <unsigned Size, class Seq = std::make_integer_sequence<unsigned, Size>>
	struct with_table_size;
	
	template <unsigned FlatIdx, class Fn, class... Vars>
		static constexpr decltype(auto) func (Fn fn, Vars... vars){
		constexpr auto& coord = make_indices<std::decay_t<Vars>::size...>::indices.data[FlatIdx];
		return static_cast<Fn&&>(fn)( static_cast<Vars&&>(vars).template get<coord[Vx]>()... );
	}
		
	template <unsigned TableSize, unsigned... Idx>
	struct with_table_size<TableSize, std::integer_sequence<unsigned, Idx...> > {
		
		template <class Fn, class... Vars>
		static constexpr rtype_visit<Fn, Vars...>( *impl[TableSize] )(Fn, Vars...) = {
			func<Idx, Fn, Vars...>...
		};
	};
};

} // V2 

struct variant_detector_t{};

template <class T>
inline constexpr bool is_variant = std::is_base_of_v<variant_detector_t, std::decay_t<T>>;

template <class... Ts>
struct variant : private variant_detector_t {

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
	
	// generic constructor
	template <class T>
		requires ( !std::is_same_v<std::decay_t<T>, variant> && has_non_ambiguous_match<T, Ts...> )
	constexpr variant(T&& t) {
			
	}
	
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
	: storage{ valueless_construct_t{} } 
	{
		o.visit_with_index( [this] (auto&& elem, auto index_) {
			emplace<index_>(decltype(elem)(elem));
		});
	}
	
	// copy ctor
	constexpr variant(const variant& o)
		requires has_copy_ctor
	: storage{ valueless_construct_t{} } 
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
		o.visit_with_index( [this] (const auto& elem, auto index_cst) {
			if (index() == index_cst)
				get<index_cst>(*this) = elem;
			else
				emplace<index_cst>(elem);
		});
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
		using T = std::remove_reference_t<decltype(get<Idx>())>;
		if constexpr (not std::is_nothrow_constructible_v<T, Args&&...>)
			current = npos;
		new(static_cast<void*>(&get<Idx>())) T (static_cast<Args&&>(args)...);
		current = static_cast<index_type>(Idx);
		return get<Idx>();
	}
	
	template <class VisitorType>
	constexpr decltype(auto) visit(VisitorType&& fn){
		return make_dispatcher_t<false>::template dispatcher<VisitorType&&, variant&>[current](decltype(fn)(fn), *this);
	}
	
	template <class VisitorType>
	constexpr decltype(auto) visit_with_index(VisitorType&& fn){ 
		return make_dispatcher_t<true>::template dispatcher<VisitorType&&, variant&>[current](decltype(fn)(fn), *this);
	}
	
	template <class VisitorType>
	constexpr decltype(auto) visit_with_index(VisitorType&& fn) const { 
		return make_dispatcher_t<true>::template dispatcher<VisitorType&&, const variant&>[current](decltype(fn)(fn), *this);
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
	
	template <unsigned Idx>
	inline constexpr auto& get() & noexcept	{ 
		return storage.impl.template get<Idx>(); 
	}
	
	template <unsigned Idx>
	inline constexpr auto&& get() && noexcept { 
		return std::move(storage.impl.template get<Idx>()); 
	}
	
	template <unsigned Idx>
	inline constexpr const auto& get() const noexcept { 
		return const_cast<variant&>(*this).get<Idx>();
	}
	
	using storage_t = variant_top_union<make_tree_union<Ts...>>;
	
	storage_t storage;
	index_type current;
};

template <std::size_t Idx, class... Ts>
constexpr auto& get (variant<Ts...>& v){
	static_assert( Idx < sizeof...(Ts), "Index exceeds the variant size. ");
	return (v.template get<Idx>());
}

template <std::size_t Idx, class... Ts>
constexpr const auto& get (const variant<Ts...>& v){
	static_assert( Idx < sizeof...(Ts), "Index exceeds the variant size. ");
	return (v.template get<Idx>());
}

template <std::size_t Idx, class... Ts>
constexpr auto&& get (variant<Ts...>&& v){
	static_assert( Idx < sizeof...(Ts), "Index exceeds the variant size. ");
	return ( std::move(v.template get<Idx>()) );
}

template <std::size_t Idx, class... Ts>
constexpr auto* get_if(variant<Ts...>& v){
	if (v.index() == Idx) return &v.template get<Idx>();
	else return nullptr;
}

template <std::size_t Idx, class... Ts>
constexpr const auto* get_if(const variant<Ts...>& v){
	if (v.index() == Idx) return &v.template get<Idx>();
	else return nullptr;
}

template <class T, class... Ts>
constexpr T& get (variant<Ts...>& v){
	return get<find_type<T, Ts...>()>(v);
} 

template <class Fn, class Var>
constexpr decltype(auto) visit(Fn&& fn, Var&& var){
	return var.visit(static_cast<Fn&&>(fn));
}

/* 
template <class Fn, class... Vs>
constexpr decltype(auto) visit(Fn&& fn, Vs&&... vars){
	constexpr unsigned max_size = (std::decay_t<Vs>::size * ...);
	using dispatcher_t = typename multi_dispatcher<sizeof...(Vs)>::template with_table_size<max_size>;
	const auto table_indice = flatten_indices<std::decay_t<Vs>::size...>(vars.index()...);
	return dispatcher_t::template impl<Fn&&, Vs&&...>[ table_indice ]( static_cast<Fn&&>(fn), static_cast<Vs&&>(vars)... );
} */ 

template <class A>
struct eq_comp {
	constexpr bool operator()(const auto& elem, auto index) const noexcept { 
		return (a.template get<index>() == elem); 
	}
	const A& a;
};

template <class... Ts>
constexpr bool operator==(const variant<Ts...>& v1, const variant<Ts...>& v2){
	if (v1.index() != v2.index()) return false;
	if (v1.valueless_by_exception()) return true;
	else return v1.visit_with_index( eq_comp<const variant<Ts...>&>{v1} );
}

template <class T, class... Ts>
constexpr bool holds_alternative(const variant<Ts...>& v){
	return (get_if<T>(v) != nullptr);
}

} // SWL