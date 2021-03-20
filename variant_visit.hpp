#ifdef SWL_CPP_LIBRARY_VARIANT_HPP

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

// for simple visitation 
template <std::size_t... Idx>
struct make_dispatcher<std::integer_sequence<std::size_t, Idx...>, false> {
	
	template <class Fn, class Var>
	static constexpr rtype_visit<Fn, Var> (*dispatcher[sizeof...(Idx)]) (Fn, Var) = {
		[] (Fn self, Var var) {
				return self( static_cast<Var&&>(var).template get<Idx>() );
		}...
	};
};
								 	
template <std::size_t... Idx>
struct make_dispatcher<std::integer_sequence<std::size_t, Idx...>, true> {
	
	template <class Fn, class Var>
	static constexpr rtype_index_visit<Fn, Var>(*dispatcher[sizeof...(Idx)]) (Fn, Var) = {
		[] (Fn fn, Var var){
			return static_cast<Fn&&>(fn)(static_cast<Var&&>(var).template get<Idx>(), std::integral_constant<unsigned, Idx>{});
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

inline namespace v2 {

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
	constexpr union_index_t sizes[] = {Sizes...};
	constexpr union_index_t total_size = (Sizes * ...);
	constexpr union_index_t num_dim = sizeof...(Sizes);
	using walker_t = union_index_t[sizeof...(Sizes)];
	array_wrapper<walker_t[total_size]> res {{0}};
	
	for (unsigned k = 0; k < total_size; k += num_dim){
		increment<0, num_dim>(res.data[k], sizes);
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

#endif