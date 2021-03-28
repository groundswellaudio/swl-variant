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

namespace v3 {

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

}// v3

inline namespace v1 {

#if defined(__GNUC__) || defined( __clang__ ) || defined( __INTEL_COMPILER )
	#define DeclareUnreachable __builtin_unreachable()
#elif defined (_MSC_VER)
	#define DeclareUnreachable __assume(false)
#endif

#define DEC(N) X((N)) X((N) + 1) X((N) + 2) X((N) + 3) X((N) + 4) X((N) + 5) X((N) + 6) X((N) + 7) X((N) + 8) X((N) + 9)

#define SEQ30(N) DEC( (N) + 0 ) DEC( (N) + 10 ) DEC( (N) + 20 ) 
#define SEQ100(N) SEQ30((N) + 0) SEQ30((N) + 30) SEQ30((N) + 60) DEC((N) + 90) 
#define SEQ200(N) SEQ100((N) + 0) SEQ100((N) + 100)
#define SEQ400(N) SEQ200((N) + 0) SEQ200((N) + 200)
#define SEQ800(N) SEQ400((N) + 0) SEQ400((N) + 400)
#define SEQ1600(N) SEQ800((N) + 0) SEQ800((N) + 800)
#define CAT(M, N) M##N
#define CAT2(M, N) CAT(M, N)
#define INJECTSEQ(N) CAT2(SEQ, N)(0)

// single-visitation

template <unsigned Offset, class Rtype, class Fn, class V>
constexpr Rtype single_visit_tail(Fn&& fn, V&& v){

	constexpr auto var_size = std::decay_t<V>::size - Offset;
	
	#define X(N) case (N + Offset) : \
		if constexpr (N + Offset < var_size) { \
			return static_cast<Fn&&>(fn)( static_cast<V&&>(v).template unsafe_get<N+Offset>() ); \
			break; \
		} else DeclareUnreachable;

	#define SEQSIZE 400
	
	switch( v.index() ){
	
		INJECTSEQ(SEQSIZE)
		
		default : 
			if constexpr (var_size - Offset > SEQSIZE)
				return single_visit_tail<Offset + SEQSIZE, Rtype>(static_cast<Fn&&>(fn), static_cast<V&&>(v));
			else 
				DeclareUnreachable;
	}
	
	#undef X
	#undef SEQSIZE
}

template <unsigned Offset, class Rtype, class Fn, class V>
constexpr Rtype single_visit_w_index_tail(Fn&& fn, V&& v){

	constexpr auto var_size = std::decay_t<V>::size;
	
	#define X(N) case (N + Offset) : \
		if constexpr (N + Offset < var_size) { \
			return static_cast<Fn&&>(fn)( static_cast<V&&>(v).template unsafe_get<N+Offset>(), std::integral_constant<unsigned, N+Offset>{} ); \
			break; \
		} else DeclareUnreachable;
	
	#define SEQSIZE 400
	
	switch( v.index() ){
	
		INJECTSEQ(SEQSIZE)
		
		default : 
			if constexpr (var_size - Offset > SEQSIZE)
				return single_visit_w_index_tail<Offset + SEQSIZE, Rtype>(static_cast<Fn&&>(fn), static_cast<V&&>(v));
			else 
				DeclareUnreachable;
	}
	
	#undef X
	#undef SEQSIZE
}

template <class Fn, class V>
constexpr decltype(auto) visit(Fn&& fn, V&& v){
	return single_visit_tail<0, rtype_visit<Fn&&, V&&>>(static_cast<Fn&&>(fn), static_cast<V&&>(v));
}

template <class Fn, class V>
constexpr decltype(auto) visit_with_index(Fn&& fn, V&& v){
	return single_visit_w_index_tail<0, rtype_index_visit<Fn&&, V&&>>(static_cast<Fn&&>(fn), static_cast<V&&>(v));
}

template <unsigned Offset, class Rtype, class Fn, class... Vs, std::size_t... Vx>
constexpr Rtype multi_visit_tail(std::integer_sequence<std::size_t, Vx...> seq, Fn&& fn, Vs&&... vs){
	
	constexpr unsigned var_sizes[sizeof...(Vs)] = {std::decay_t<Vs>::size...};
	constexpr auto total_size = (std::decay_t<Vs>::size * ...);
	
	const auto flat_idx = flatten_indices< std::decay_t<Vs>::size... >(vs.index()...);
	
	#define X(N) case (N + Offset) :  \
		if constexpr (N + Offset < total_size) { \
			constexpr auto var_idx = unflatten(N, var_sizes); \
			return static_cast<Fn&&>(fn)( static_cast<Vs&&>(vs).template unsafe_get<var_idx.data[Vx]>()... ); \
			break; \
		} else DeclareUnreachable;
	
	#define SEQSIZE 200
	
	switch( flat_idx ) {
		
		default : 
			if constexpr (total_size - Offset > SEQSIZE)
				return multi_visit_tail<Offset + SEQSIZE, Rtype>(seq, static_cast<Fn&&>(fn), static_cast<Vs&&>(vs)...);
			else
				DeclareUnreachable;
		
		INJECTSEQ(SEQSIZE)
	}
	
	#undef X
	#undef SEQSIZE
}

template <class Fn, class... Vs>
constexpr decltype(auto) multi_visit(Fn&& fn, Vs&&... vs){
	using seq = std::make_index_sequence<sizeof...(Vs)>;
	return multi_visit_tail<0, rtype_visit<Fn&&, Vs&&...>>(seq{}, static_cast<Fn&&>(fn), static_cast<Vs&&>(vs)...);
}

#undef DEC
#undef SEQ30
#undef SEQ100
#undef SEQ200
#undef SEQ400
#undef SEQ800
#undef SEQ1600
#undef DeclareUnreachable
#undef CAT
#undef CAT2
#undef INJECTSEQ

}


#endif