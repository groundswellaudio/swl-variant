#ifdef SWL_CPP_LIBRARY_VARIANT_HPP

// ========================= visit dispatcher

template <class Seq, bool PassIndex>
struct make_dispatcher;

template <class Fn, class... Vars>
using rtype_visit = decltype( ( std::declval<Fn>()( std::declval<Vars>().template unsafe_get<0>()... ) ) );

template <class Fn, class Var>
using rtype_index_visit = decltype( ( std::declval<Fn>()( std::declval<Var>().template unsafe_get<0>(), 
								 	  std::integral_constant<std::size_t, 0>{} ) ) 
								  );

// for simple visitation 
template <std::size_t... Idx>
struct make_dispatcher<std::integer_sequence<std::size_t, Idx...>, false> {
	
	template <class Fn, class Var>
	static constexpr rtype_visit<Fn, Var> (*dispatcher[sizeof...(Idx)]) (Fn, Var) = {
		+[] (Fn fn, Var var) -> decltype(auto) {
				return static_cast<Fn&&>(fn)( static_cast<Var&&>(var).template unsafe_get<Idx>() );
		}...
	};
};
								 	
template <std::size_t... Idx>
struct make_dispatcher<std::integer_sequence<std::size_t, Idx...>, true> {
	
	template <class Fn, class Var>
	static constexpr rtype_index_visit<Fn, Var>(*dispatcher[sizeof...(Idx)]) (Fn, Var) = {
		+[] (Fn fn, Var var) -> decltype(auto) {
			return static_cast<Fn&&>(fn)(static_cast<Var&&>(var).template unsafe_get<Idx>(), std::integral_constant<unsigned, Idx>{});
		}... 
	};
};

template <unsigned... Sizes>
constexpr unsigned flatten_indices(const auto... args){	
	unsigned res = 0;
	([&] () { res *= Sizes; res += args; }(), ...);
	return res;
}

//  for visit of two variants of size 24 : clang ~1.2 sec, gcc ~4.45 sec

namespace v2 {

template <unsigned char Idx>
constexpr void increment(auto& walker, const auto& sizes, unsigned k){
	walker[k][Idx] = walker[k - 1][Idx] + 1;
	if (walker[k][Idx] == sizes[Idx]){
		if constexpr (Idx > 0){
			walker[k][Idx] = 0;
			return increment<Idx - 1>(walker, sizes, k);
		}
	}
	else {
		for (unsigned x = 1; x <= Idx; ++x)
			walker[k][Idx - x] = walker[k - 1][Idx - x];
	}
}

template <unsigned... Sizes>
constexpr auto make_flat_sequence(){
	constexpr union_index_t sizes[] = {Sizes...};
	constexpr unsigned total_size = (Sizes * ...);
	constexpr union_index_t num_dim = sizeof...(Sizes);
	using walker_t = union_index_t[sizeof...(Sizes)];
	array_wrapper<walker_t[total_size]> res {{0}};
	auto& tab = res.data;
	
	for (unsigned k = 1; k < total_size; ++k){
		increment<num_dim-1>(tab, sizes, k);
	}
	
	return res;
}

// this build an array of array of all the possibles indices (make_indices)
// and use it to build the dispatch table (with_table_size::impl)
template <unsigned NumVariants, class Seq = std::make_integer_sequence<unsigned, NumVariants>>
struct multi_dispatcher;

template <unsigned NumVariants, unsigned... Vx>
struct multi_dispatcher<NumVariants, std::integer_sequence<unsigned, Vx...>> {
	
	template <unsigned... VarSizes>
	struct make_indices {
	
		static constexpr union_index_t total_size = (VarSizes * ...);
		static constexpr union_index_t var_sizes[sizeof...(VarSizes)] = {VarSizes...};
		
		static constexpr auto indices = make_flat_sequence<VarSizes...>();
	};
	
	template <unsigned Size, class Seq = std::make_integer_sequence<unsigned, Size>>
	struct with_table_size;
	
	template <unsigned FlatIdx, class Fn, class... Vars>
	static constexpr decltype(auto) func (Fn fn, Vars... vars){
		constexpr auto& coord = make_indices<std::decay_t<Vars>::size...>::indices.data[FlatIdx];
		return static_cast<Fn&&>(fn)( static_cast<Vars&&>(vars).template unsafe_get<coord[Vx]>()... );
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

inline namespace v3 {


template <std::size_t N>
constexpr auto unflatten(unsigned Idx, const unsigned(&sizes)[N]){
	array_wrapper<unsigned[N]> res;
	auto& data = res.data;
	for (unsigned k = 1; k < N; ++k){
		const auto prev = Idx;
		Idx /= sizes[N - k];
		data[N - k] = prev - Idx * sizes[k];
	}
	data[0] = Idx;
	return res;
}

// this build an array of array of all the possibles indices (make_indices)
// and use it to build the dispatch table (with_table_size::impl)
template <unsigned NumVariants, class SizeSeq, class AccessSeq = std::make_integer_sequence<unsigned, NumVariants>>
struct multi_dispatcher;

template <unsigned NumVariants, unsigned... VarSizes, unsigned... Vx>
struct multi_dispatcher
	<NumVariants, 
	std::integer_sequence<unsigned, VarSizes...>, 
	std::integer_sequence<unsigned, Vx...>
	> 
{	
	static constexpr unsigned var_sizes[] = {VarSizes...};
	
	template <unsigned Size, class Seq = std::make_integer_sequence<unsigned, Size>>
	struct with_table_size;
	
	template <unsigned FlatIdx, class Fn, class... Vars>
	static constexpr decltype(auto) func (Fn fn, Vars... vars){
		constexpr auto coord = unflatten(FlatIdx, var_sizes);
		return static_cast<Fn&&>(fn)( static_cast<Vars&&>(vars).template unsafe_get<coord.data[Vx]>()... );
	}
		
	template <unsigned TableSize, unsigned... Idx>
	struct with_table_size<TableSize, std::integer_sequence<unsigned, Idx...> > {
		
		template <class Fn, class... Vars>
		static constexpr rtype_visit<Fn, Vars...>( *impl[TableSize] )(Fn, Vars...) = {
			func<Idx, Fn, Vars...>...
		};
	};
}; 

}// v3

template <class Fn, class V>
constexpr decltype(auto) visit(Fn&& fn, V&& v){
	DebugAssert(not v.valueless_by_exception());
	using seq = std::make_index_sequence< std::remove_reference_t<V>::size >;
	return make_dispatcher<seq, false>::template dispatcher<Fn&&, V&&>[v.index()]
		( static_cast<Fn&&>(fn), static_cast<V&&>(v) );
}

template <class Fn, class V>
constexpr decltype(auto) visit_with_index(Fn&& fn, V&& v){
	DebugAssert(not v.valueless_by_exception());
	using seq = std::make_index_sequence< std::remove_reference_t<V>::size >;
	return make_dispatcher<seq, true>::template dispatcher<Fn&&, V&&>[v.index()]
		( static_cast<Fn&&>(fn), static_cast<V&&>(v) );
}


#endif