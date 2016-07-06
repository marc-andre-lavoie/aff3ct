#include "Channel_AWGN_std_LR.hpp"

#include <cassert>
#include <cmath>

template <typename R>
Channel_AWGN_std_LR<R>
::Channel_AWGN_std_LR(const R& sigma, const int seed, const R scaling_factor)
: Channel_AWGN_std_LLR<R>(sigma, seed, scaling_factor)
{
}

template <typename R>
Channel_AWGN_std_LR<R>
::~Channel_AWGN_std_LR()
{
}

template <typename R>
void Channel_AWGN_std_LR<R>
::add_noise(const mipp::vector<R>& X_N, mipp::vector<R>& Y_N)
{
	assert(X_N.size() == Y_N.size());

	Channel_AWGN_std_LLR<R>::add_noise(X_N, Y_N);
	for (unsigned i = 0; i < Y_N.size(); i++)
		Y_N[i] = std::exp(Y_N[i]);
}

// ==================================================================================== explicit template instantiation 
#include "../../../Tools/types.h"
#ifdef MULTI_PREC
template class Channel_AWGN_std_LR<R_32>;
template class Channel_AWGN_std_LR<R_64>;
#else
template class Channel_AWGN_std_LR<R>;
#endif
// ==================================================================================== explicit template instantiation