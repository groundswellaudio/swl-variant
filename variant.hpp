#ifndef SWL_CPP_LIBRARY_VARIANT_HPP
#define SWL_CPP_LIBRARY_VARIANT_HPP

#include <exception>
#include <type_traits>
#include <utility>
#include <new>
#include <limits>

#ifdef SWL_VARIANT_USE_STD_HASH
	#include <functional>
#endif

#ifdef SWL_VARIANT_DEBUG
	#include <iostream>
	#define DebugAssert(X) if (not (X)) std::cout << "Variant : assertion failed : [" << #X << "] at line : " << __LINE__ << std::endl;
	#undef SWL_VARIANT_DEBUG
#else 
	#define DebugAssert(X) 
#endif

namespace swl {

class bad_variant_access : std::exception {
	public : 
	bad_variant_access(const char* str) noexcept : message{str} {}
	bad_variant_access() noexcept = default;
	bad_variant_access(const bad_variant_access&) noexcept = default;
	bad_variant_access& operator= (const bad_variant_access&) noexcept = default;
	const char* what() const noexcept override { return message; }
	const char* message = " "; // llvm test requires a well formed what() on default init
};

namespace vimpl { 
	struct variant_tag{};
	struct emplacer_tag{};
}

template <class T>
struct in_place_type_t : private vimpl::emplacer_tag {};

template <std::size_t Index>
struct in_place_index_t : private vimpl::emplacer_tag {};

template <std::size_t Index>
inline static constexpr in_place_index_t<Index> in_place_index;

template <class T>
inline static constexpr in_place_type_t<T> in_place_type;

namespace vimpl {
	#include "variant_detail.hpp"
	#include "variant_visit.hpp"
	
	struct variant_npos_t {
		template <class T>
		constexpr bool operator==(T idx) const noexcept { return idx == std::numeric_limits<T>::max(); }
	};
}

template <class T>
inline constexpr bool is_variant = std::is_base_of_v<vimpl::variant_tag, std::decay_t<T>>;

inline static constexpr vimpl::variant_npos_t variant_npos;

template <class... Ts>
class variant;

// ill-formed variant, an empty specialization prevents some really bad errors messages on gcc
template <class... Ts>
	requires ( (std::is_array_v<Ts> || ...) || (std::is_reference_v<Ts> || ...) || (std::is_void_v<Ts> || ...)
				|| sizeof...(Ts) == 0 )
class variant<Ts...> {
	static_assert( not (std::is_array_v<Ts> || ...), "A variant cannot contain a raw array type, consider using std::array instead." );
	static_assert( sizeof...(Ts) > 0, "A variant cannot be empty.");
	static_assert( not (std::is_reference_v<Ts> || ...), "A variant cannot contain references, consider using reference wrappers instead." );
	static_assert( not (std::is_void_v<Ts> || ...), "A variant cannot contains void." );
};

template <class... Ts>
class variant : private vimpl::variant_tag {
	
	using storage_t = vimpl::variant_top_union<vimpl::make_tree_union<Ts...>>;
	 
	static constexpr bool is_trivial           = std::is_trivial_v<storage_t>;
	static constexpr bool has_copy_ctor        = std::is_copy_constructible_v<storage_t>;
	static constexpr bool trivial_copy_ctor    = is_trivial || std::is_trivially_copy_constructible_v<storage_t>;
	static constexpr bool has_copy_assign      = std::is_copy_constructible_v<storage_t>;
	static constexpr bool trivial_copy_assign  = is_trivial || has_copy_assign && std::is_trivially_copy_assignable_v<storage_t>;
	static constexpr bool has_move_ctor        = std::is_move_constructible_v<storage_t>;
	static constexpr bool trivial_move_ctor    = is_trivial || std::is_trivially_move_constructible_v<storage_t>;
	static constexpr bool has_move_assign      = std::is_move_assignable_v<storage_t>;
	static constexpr bool trivial_move_assign  = is_trivial || std::is_trivially_move_assignable_v<storage_t>;
	static constexpr bool trivial_dtor         = std::is_trivially_destructible_v<storage_t>;
	
	template <bool PassIndex = false>
	using make_dispatcher_t = vimpl::make_dispatcher<std::make_index_sequence<sizeof...(Ts)>, PassIndex>;
	
	public : 
	
	template <std::size_t Idx>
	using alternative = std::remove_reference_t< decltype( std::declval<storage_t&>().impl.template get<Idx>() ) >;
	
	static constexpr bool can_be_valueless = not is_trivial;
	
	static constexpr unsigned size = sizeof...(Ts);
	
	using index_type = vimpl::smallest_suitable_integer_type<sizeof...(Ts) + can_be_valueless, unsigned char, unsigned short, unsigned>;
	
	static constexpr index_type npos = -1;
	
	// ============================================= constructors (20.7.3.2)
	
	// default constructor
	constexpr variant() 
		noexcept (std::is_nothrow_default_constructible_v<alternative<0>>)
		requires std::is_default_constructible_v<alternative<0>> 
	: storage{ in_place_index<0> }, current{0}
	{}
	
	// copy constructor (trivial)
	constexpr variant(const variant&)
		requires trivial_copy_ctor
	= default;
	
	// note : both the copy and move constructor cannot be meaningfully constexpr before std::construct_at (C++20)
	// copy constructor
	constexpr variant(const variant& o)
		requires (has_copy_ctor and not trivial_copy_ctor)
	: storage{ vimpl::valueless_construct_t{} } {
		construct_from(o);
	}
	
	// move constructor (trivial)
	constexpr variant(variant&&)
		requires trivial_move_ctor
	= default;
	
	// move constructor
	constexpr variant(variant&& o)
		noexcept ((std::is_nothrow_move_constructible_v<Ts> && ...))
		requires (has_move_ctor and not trivial_move_ctor)
	: storage{ vimpl::valueless_construct_t{} } {
		construct_from(static_cast<variant&&>(o));
	}
	
	// generic constructor
	template <class T, class M = vimpl::best_overload_match<T&&, Ts...>, class D = std::decay_t<T>>
	constexpr variant(T&& t)
		noexcept ( std::is_nothrow_constructible_v<M, T&&> )
		requires ( not std::is_same_v<D, variant> and not std::is_base_of_v<vimpl::emplacer_tag, D> )
	: variant{ in_place_index< vimpl::find_type<M, Ts... >() >, static_cast<T&&>(t) }
	{}
	
	// construct at index
	template <std::size_t Index, class... Args>
		requires (Index < sizeof...(Ts) && std::is_constructible_v<alternative<Index>, Args&&...>)
	explicit constexpr variant(in_place_index_t<Index> tag, Args&&... args)
	: storage{tag, static_cast<Args&&>(args)...}, current(Index) 
	{}
	
	// construct a given type
	template <class T, class... Args>
		requires (vimpl::appears_exactly_once<T, Ts...> && std::is_constructible_v<T, Args&&...>)
	explicit constexpr variant(in_place_type_t<T> tag, Args&&... args)
	: variant{ in_place_index<vimpl::find_type<T, Ts...>()>, static_cast<Args&&>(args)... }
	{}
	
	// ================================ destructors (20.7.3.3)
	
	constexpr ~variant() requires trivial_dtor = default;
	
	constexpr ~variant() requires (not trivial_dtor) {
		reset();
	}
	
	// ================================ assignment (20.7.3.4)
	
	// copy assignment (trivial)
	constexpr variant& operator=(const variant& o)
		requires trivial_copy_assign && trivial_copy_ctor 
	= default;
	
	// copy assignment 
	constexpr variant& operator=(const variant& rhs)
		requires (has_copy_assign and not(trivial_copy_assign && trivial_copy_ctor))
	{
		assign_from(rhs, [this] (const auto& elem, auto index_cst) {
			if (index() == index_cst)
				this->unsafe_get<index_cst>() = elem;
			else{
				using type = alternative<index_cst>;
				if constexpr (std::is_nothrow_copy_constructible_v<type> 
								or not std::is_nothrow_move_constructible_v<type>)
					this->emplace<index_cst>(elem);
				else{
					alternative<index_cst> tmp = elem;
					this->emplace<index_cst>(std::move(tmp));
				}
			}
		});
		return *this;
	}
	
	// move assignment (trivial)
	constexpr variant& operator=(variant&& o)
		requires (trivial_move_assign and trivial_move_ctor and trivial_dtor)
	= default; 
	
	// move assignment
	constexpr variant& operator=(variant&& o)
		noexcept ((std::is_nothrow_move_constructible_v<Ts> && ...) && (std::is_nothrow_move_assignable_v<Ts> && ...))
		requires (has_move_assign && has_move_ctor and not(trivial_move_assign and trivial_move_ctor and trivial_dtor))
	{
		assign_from(static_cast<variant&&>(o), [this] (auto&& elem, auto index_cst) 
		{
			if (index() == index_cst)
				this->unsafe_get<index_cst>() = std::move(elem);
			else 
				this->emplace<index_cst>(std::move(elem));
		});
		return *this;
	}
	
	// generic assignment 
	template <class T>
		requires vimpl::has_non_ambiguous_match<T, Ts...>
	constexpr variant& operator=(T&& t) 
		noexcept( std::is_nothrow_assignable_v<vimpl::best_overload_match<T&&, Ts...>, T&&> 
				  && std::is_nothrow_constructible_v<vimpl::best_overload_match<T&&, Ts...>, T&&> )
	{
		using namespace vimpl;
		using related_type = best_overload_match<T&&, Ts...>;
		constexpr auto new_index = find_type<related_type, Ts...>();
		if (current == new_index)
			unsafe_get<new_index>() = static_cast<T&&>(t);
		else {
			if constexpr ( std::is_nothrow_constructible_v<related_type, T> 
						   or not std::is_nothrow_move_constructible_v<related_type> )
				emplace<new_index>( static_cast<T&&>(t) );
			else {
				related_type tmp = t;
				emplace<new_index>(std::move(tmp));
			}
		}
		return *this;
	}
	
	// ================================== modifiers (20.7.3.5)
	
	template <class T, class... Args>
		requires std::is_constructible_v<T, Args&&...>
	T& emplace(Args&&... args){
		static_assert( vimpl::appears_exactly_once<T, Ts...>,  
			"Variant emplace : the type to be emplaced must appear exactly once." ); 
		
		constexpr auto Index = vimpl::find_type<T, Ts...>();	
		(void) emplace<Index>(static_cast<Args&&>(args)...);
		return unsafe_get<Index>();
	}
	
	template <std::size_t Idx, class... Args>
		requires (Idx < size and std::is_constructible_v<alternative<Idx>, Args&&...>  )
	auto& emplace(Args&&... args){
		using T = alternative<Idx>;
		
		reset();
		
		if constexpr (not std::is_nothrow_constructible_v<T, Args&&...>)
			current = npos;
		
		emplace_no_dtor<Idx>(static_cast<Args&&>(args)...);
		return unsafe_get<Idx>();
	}
	
	// ==================================== value status (20.7.3.6)
	
	constexpr bool valueless_by_exception() const noexcept {
		if constexpr ( can_be_valueless )
			return current == npos;
		else return false;
	}
	
	constexpr index_type index() const noexcept {
		return current;
	}
	
	// =================================== swap (20.7.3.7)
	
	void swap(variant& o) 
		noexcept ( (std::is_nothrow_move_constructible_v<Ts> && ...) 
				   && (vimpl::swap_trait::template nothrow<Ts> && ...) )
		requires (has_move_ctor && (vimpl::swap_trait::template able<Ts> && ...))
	{
		if constexpr (can_be_valueless){
			if (index() == npos){
				if (o.index() == npos) 
					return;
				else {
					vimpl::visit_with_index( std::move(o), vimpl::emplace_no_dtor_from_elem<variant&>{*this} );
					o.reset_no_check();
					o.current = npos;
					return;
				}
			}
			else if (o.index() == npos){
				vimpl::visit_with_index( std::move(*this), vimpl::emplace_no_dtor_from_elem<variant&>{o} );
				reset_no_check();
				current = npos;
				return;
			}
		}
		
		DebugAssert( not (valueless_by_exception() && o.valueless_by_exception()) );
		
		if (index() == o.index()){
			vimpl::visit_with_index(o, [this] (auto& elem, auto index_) {
				using std::swap;
				swap(this->unsafe_get<index_>(), elem);
			});
		}
		else {
			// TODO : replace this by multi visit with index
			vimpl::visit_with_index( o, [&o, this] (auto&& elem, auto index_) {
				using idx_t = decltype(index_);
				vimpl::visit_with_index(*this, [this, &o, &elem, index_] (auto&& this_elem, auto this_index) {
				
					auto tmp { std::move(this_elem) };
					
					// destructing the elements right here save us another level of indirection
					vimpl::destruct<alternative<this_index>>(this_elem);
					
					if constexpr (not std::is_nothrow_move_constructible_v<alternative<idx_t::value>> )
						this->current = npos;
					this->emplace_no_dtor<idx_t::value>( std::move(elem) );
					
					vimpl::destruct<alternative<idx_t::value>>(elem);
					
					if constexpr (not std::is_nothrow_move_constructible_v<alternative<this_index>> )
						o.current = npos;
					o.template emplace_no_dtor< (unsigned)(this_index) >( std::move(tmp) );
				});
			});	
		}
	}
	
	// +================================== methods for internal use
	// these methods performs no errors checking at all
	
	template <vimpl::union_index_t Idx>
	constexpr auto& unsafe_get() & noexcept	{
		static_assert(Idx < size);
		DebugAssert(current == Idx);
		return storage.impl.template get<Idx>(); 
	}
	
	template <vimpl::union_index_t Idx>
	constexpr auto&& unsafe_get() && noexcept { 
		static_assert(Idx < size);
		DebugAssert(current == Idx);
		return std::move( storage.impl.template get<Idx>() ); 
	}
	
	template <vimpl::union_index_t Idx>
	constexpr const auto& unsafe_get() const & noexcept {
		static_assert(Idx < size);
		DebugAssert(current == Idx);
		return const_cast<variant&>(*this).unsafe_get<Idx>();
	}
	
	template <vimpl::union_index_t Idx>
	constexpr const auto&& unsafe_get() const && noexcept {
		static_assert(Idx < size);
		DebugAssert(current == Idx);
		return std::move(unsafe_get<Idx>());
	}
	
	private : 
	
	// can be used directly only when the variant is in a known empty state
	template <unsigned Idx, class... Args>
	constexpr void emplace_no_dtor(Args&&... args){
		using T = alternative<Idx>;
		new( (void*)(&storage.impl.template get<Idx>()) ) T (static_cast<Args&&>(args)...);	
		current = static_cast<index_type>(Idx);
	}
	
	// destroy the current elem IFF not valueless 
	constexpr void reset() {
		if constexpr (can_be_valueless)
			if (valueless_by_exception()) return;
		reset_no_check();
	}
	
	// destroy the current element without checking for valueless
	constexpr void reset_no_check(){
		DebugAssert( index() < size );
		if constexpr ( not trivial_dtor ){
			vimpl::visit_with_index( *this, [] (auto& elem, auto index_) {
				vimpl::destruct<alternative<index_>>(elem);
			});
		}
	}
	
	// construct this from another variant, for constructors only
	template <class Other>
	constexpr void construct_from(Other&& o){
		if constexpr (can_be_valueless)
			if (o.valueless_by_exception()){
				current = npos;
				return;
			}
		
		vimpl::visit_with_index( static_cast<Other&&>(o), vimpl::emplace_no_dtor_from_elem<variant&>{*this} );
	}
	
	// assign from another variant
	template <class Other, class Fn>
	constexpr void assign_from(Other&& o, Fn&& fn){
		if constexpr (can_be_valueless){
			if (o.index() == npos){
				if (current != npos){
					reset_no_check();
					current = npos;
				}
				return;
			}
		}
		DebugAssert(not o.valueless_by_exception());
		vimpl::visit_with_index( static_cast<Other&&>(o), static_cast<Fn&&>(fn) );
	}
	
	template <class T>
	friend struct emplace_no_dtor_from_elem;
	
	storage_t storage;
	index_type current;
};

// ================================= value access (20.7.5)

template <class T, class... Ts>
constexpr bool holds_alternative(const variant<Ts...>& v) noexcept {
	static_assert( (std::is_same_v<T, Ts> || ...), "Requested type is not contained in the variant" );
	constexpr auto Index = vimpl::find_type<T, Ts...>();
	return v.index() == Index;
}

// ========= get by index

template <std::size_t Idx, class... Ts>
constexpr auto& get (variant<Ts...>& v){
	static_assert( Idx < sizeof...(Ts), "Index exceeds the variant size. ");
	if (v.index() != Idx) throw bad_variant_access{"swl::variant : Bad variant access."};
	return (v.template unsafe_get<Idx>());
}

template <std::size_t Idx, class... Ts>
constexpr const auto& get (const variant<Ts...>& v){
	return swl::get<Idx>(const_cast<variant<Ts...>&>(v));
}

template <std::size_t Idx, class... Ts>
constexpr auto&& get (variant<Ts...>&& v){
	return std::move( swl::get<Idx>(v) );
}

template <std::size_t Idx, class... Ts>
constexpr const auto&& get (const variant<Ts...>&& v){
	return std::move( swl::get<Idx>(v) );
}

// ========= get by type

template <class T, class... Ts>
constexpr T& get (variant<Ts...>& v){
	return swl::get<vimpl::find_type<T, Ts...>()>(v);
}

template <class T, class... Ts>
constexpr const T& get (const variant<Ts...>& v){
	return swl::get<vimpl::find_type<T, Ts...>()>(v);
}

template <class T, class... Ts>
constexpr T&& get (variant<Ts...>&& v){
	return swl::get<vimpl::find_type<T, Ts...>()>( static_cast<variant<Ts...>&&>(v) );
}

template <class T, class... Ts>
constexpr const T&& get (const variant<Ts...>&& v){
	return swl::get<vimpl::find_type<T, Ts...>()>(decltype(v)(v));
}

// ===== get_if by index

template <std::size_t Idx, class... Ts>
constexpr const auto* get_if(const variant<Ts...>* v) noexcept {
	if (v == nullptr || v->index() != Idx) 
		return decltype(&v->template unsafe_get<Idx>()){nullptr};
	else return &v->template unsafe_get<Idx>();
}

template <std::size_t Idx, class... Ts>
constexpr auto* get_if(variant<Ts...>* v) noexcept {
	if (v == nullptr || v->index() != Idx) 
		return decltype(&v->template unsafe_get<Idx>()){nullptr};
	else return &v->template unsafe_get<Idx>();
}

// ====== get_if by type 

template <class T, class... Ts>
constexpr T* get_if(variant<Ts...>* v) noexcept {
	static_assert( (std::is_same_v<T, Ts> || ...), "Requested type is not contained in the variant" );
	return swl::get_if<vimpl::find_type<T, Ts...>()>(v);
}

template <class T, class... Ts>
constexpr const T* get_if(const variant<Ts...>* v) noexcept {
	static_assert( (std::is_same_v<T, Ts> || ...), "Requested type is not contained in the variant" );
	return swl::get_if<vimpl::find_type<T, Ts...>()>(v);
}

// =============================== visitation (20.7.7)

namespace v1 {

template <class Fn, class... Vs>
	requires (is_variant<Vs> && ...)
constexpr decltype(auto) visit(Fn&& fn, Vs&&... vars){
	if constexpr ( (std::decay_t<Vs>::can_be_valueless || ...) )
		if ( (vars.valueless_by_exception() || ...) ) 
			throw bad_variant_access{"swl::variant : Bad variant access in visit."};
				
	if constexpr (sizeof...(Vs) == 1){
		return vimpl::visit(static_cast<Fn&&>(fn), static_cast<Vs&&>(vars)...);
	}
	else {
		using namespace vimpl::v3;
		constexpr unsigned max_size = (std::decay_t<Vs>::size * ...);
		
		using size_seq = std::integer_sequence<unsigned, std::decay_t<Vs>::size...>;
		using dispatcher_t = typename multi_dispatcher<sizeof...(Vs), size_seq>::template with_table_size<max_size>;
		const auto table_indice = vimpl::flatten_indices<std::decay_t<Vs>::size...>(vars.index()...);
		return dispatcher_t::template impl<Fn&&, Vs&&...>[ table_indice ]( static_cast<Fn&&>(fn), static_cast<Vs&&>(vars)... );
	}
}

}

template <class Fn, class... Vs>
constexpr decltype(auto) visit(Fn&& fn, Vs&&... vs){
	if constexpr ( (std::decay_t<Vs>::can_be_valueless || ...) )
		if ( (vs.valueless_by_exception() || ...) ) 
			throw bad_variant_access{"swl::variant : Bad variant access in visit."};
	
	if constexpr (sizeof...(Vs) == 1)
		return vimpl::visit(static_cast<Fn&&>(fn), static_cast<Vs&&>(vs)...);
	else 
		return vimpl::multi_visit(static_cast<Fn&&>(fn), static_cast<Vs&&>(vs)...);
}

template <class Fn>
constexpr decltype(auto) visit(Fn&& fn){
	return static_cast<Fn&&>(fn)();
}

template <class R, class Fn, class... Vs>
	requires (is_variant<Vs> && ...)
constexpr R visit(Fn&& fn, Vs&&... vars){
	return static_cast<R>( swl::visit(static_cast<Fn&&>(fn), static_cast<Vs&&>(vars)...) );
}

// ============================== relational operators (20.7.6)

template <class... Ts>
	requires ( vimpl::has_eq_comp<Ts> && ... )
constexpr bool operator==(const variant<Ts...>& v1, const variant<Ts...>& v2){
	if (v1.index() != v2.index()) 
		return false;
	if constexpr (variant<Ts...>::can_be_valueless)
		if (v1.valueless_by_exception()) return true;
	return vimpl::visit_with_index( v2, vimpl::eq_comp<const variant<Ts...>&>{v1} );
}

template <class... Ts>
constexpr bool operator!=(const variant<Ts...>& v1, const variant<Ts...>& v2)
	requires requires { v1 == v2; } 
{
	return not(v1 == v2);
}

template <class... Ts>
	requires ( vimpl::has_lesser_comp<const Ts&> && ... )
constexpr bool operator<(const variant<Ts...>& v1, const variant<Ts...>& v2){
	if constexpr (variant<Ts...>::can_be_valueless){
		if (v2.valueless_by_exception()) return false;
		if (v1.valueless_by_exception()) return true;
	}
	if ( v1.index() == v2.index() ){
		return vimpl::visit_with_index( v1, [&v2] (auto& elem, auto index) -> bool {
			return (elem < v2.template unsafe_get<index>());
		});
	}
	else
		return (v1.index() < v2.index());
}

template <class... Ts>
constexpr bool operator>(const variant<Ts...>& v1, const variant<Ts...>& v2){
	return v2 < v1;
}

template <class... Ts>
	requires ( vimpl::has_less_or_eq_comp<const Ts&> && ... )
constexpr bool operator<=(const variant<Ts...>& v1, const variant<Ts...>& v2){
	if constexpr (variant<Ts...>::can_be_valueless){
		if (v1.valueless_by_exception()) return true;
		if (v2.valueless_by_exception()) return false;
	}
	if ( v1.index() == v2.index() ){
		return vimpl::visit_with_index( v1, [&v2] (auto& elem, auto index) -> bool {
			return (elem <= v2.template unsafe_get<index>());
		});
	}
	else
		return (v1.index() < v2.index());
}

template <class... Ts>
constexpr bool operator>=(const variant<Ts...>& v1, const variant<Ts...>& v2){
	return v2 <= v1;
}

// ===================================== monostate (20.7.8, 20.7.9)

struct monostate{};
constexpr bool operator==(monostate, monostate) noexcept { return true; }
constexpr bool operator> (monostate, monostate) noexcept { return false; }
constexpr bool operator< (monostate, monostate) noexcept { return false; }
constexpr bool operator<=(monostate, monostate) noexcept { return true; }
constexpr bool operator>=(monostate, monostate) noexcept { return true; }

// ===================================== specialized algorithms (20.7.10)

template <class... Ts>
void swap(variant<Ts...>& a, variant<Ts...>& b)
	noexcept (noexcept(a.swap(b)))
	requires requires { a.swap(b); }
{
	a.swap(b);
}

} // SWL

// ====================================== hash support (20.7.12)
#ifdef SWL_VARIANT_USE_STD_HASH

	namespace std {
		template <class... Ts>
			requires (swl::vimpl::has_std_hash<Ts> && ...)
		struct hash<swl::variant<Ts...>> {
			std::size_t operator()(const swl::variant<Ts...>& v) const {
				if constexpr ( swl::variant<Ts...>::can_be_valueless )
					if (v.valueless_by_exception()) return -1;
		
				return v.visit_with_index( [] (auto& elem, auto index_) {
					using type = std::remove_cvref_t<decltype(elem)>;
					return std::hash<type>{}(elem) + index_;
				});
			}
		};

		template <>
		struct hash<swl::monostate> {
			constexpr std::size_t operator()(swl::monostate) const noexcept { return -1; }
		};
	}

#undef SWL_VARIANT_USE_STD_HASH

#endif // std-hash

#undef DebugAssert

#endif // eof