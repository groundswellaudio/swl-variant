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
		static constexpr rtype_visit<Fn, Vars...>( *impl[TableSize] )(Fn, Vars...) = {
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

// this build an array of array of all the possibles indices (make_indices)
// and use it to build the dispatch table (with_table_size::impl)
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

#endif