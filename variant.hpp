#include <type_traits>
#include <utility>
#include <new>
#include <limits>

namespace swl {

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

// ============= overload match detector to be used for variant generic assignment 

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
	


}