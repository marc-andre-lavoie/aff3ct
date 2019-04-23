#include "Decoder_polar_MK_SCL_naive_CA_sys.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

template <typename B, typename R>
Decoder_polar_MK_SCL_naive_CA_sys<B,R>
::Decoder_polar_MK_SCL_naive_CA_sys(const int& K,
                                    const int& N,
                                    const int& L,
                                    const tools::Polar_code& code,
                                    const std::vector<bool>& frozen_bits,
                                    const std::vector<std::vector<std::function<R(const std::vector<R> &LLRs,
                                                                                  const std::vector<B> &bits)>>> &lambdas,
                                    CRC<B>& crc,
                                    const int n_frames)
: Decoder                           (K, N,                                     n_frames, 1),
  Decoder_polar_MK_SCL_naive_CA<B,R>(K, N, L, code, frozen_bits, lambdas, crc, n_frames   )
{
	const std::string name = "Decoder_polar_MK_SCL_naive_CA_sys";
	this->set_name(name);

	if (!this->code.can_be_systematic())
	{
		std::stringstream message;
		message << "This polar code does not support systematic encoding.";
		throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
	}
}

template <typename B, typename R>
Decoder_polar_MK_SCL_naive_CA_sys<B,R>
::Decoder_polar_MK_SCL_naive_CA_sys(const int& K,
                                    const int& N,
                                    const int& L,
                                    const tools::Polar_code& code,
                                    const std::vector<bool>& frozen_bits,
                                    CRC<B>& crc,
                                    const int n_frames)
: Decoder_polar_MK_SCL_naive_CA_sys<B,R>(K, N, L, code, frozen_bits, {}, crc, n_frames   )
{
}

template <typename B, typename R>
void Decoder_polar_MK_SCL_naive_CA_sys<B,R>
::select_best_path()
{
	std::vector<B> U_test;
	std::set<int> active_paths_before_crc = this->active_paths;
	for (auto path : active_paths_before_crc)
	{
		U_test.clear();

		for (auto i = 0 ; i < this->N ; i++)
			if (!this->frozen_bits[i]) U_test.push_back(this->polar_trees[path]->get_root()->get_c()->s[i]);

		bool decode_result = this->crc.check(U_test, this->get_simd_inter_frame_level());
		if (!decode_result)
			this->active_paths.erase(path);
	}

	this->Decoder_polar_MK_SCL_naive<B,R>::select_best_path();
}

template <typename B, typename R>
void Decoder_polar_MK_SCL_naive_CA_sys<B,R>
::_store(B *V, bool coded) const
{
	if (!coded)
	{
		auto k = 0;
		for (auto i = 0; i < this->N; i++)
			if (!this->frozen_bits[i])
				V[k++] = this->polar_trees[*this->active_paths.begin()]->get_root()->get_c()->s[i] ? 1 : 0;
	}
	else
		for (auto i = 0; i < this->N; i++)
			V[i] = this->polar_trees[*this->active_paths.begin()]->get_root()->get_c()->s[i] ? 1 : 0;
}

// ==================================================================================== explicit template instantiation
#include "Tools/types.h"
#ifdef AFF3CT_MULTI_PREC
template class aff3ct::module::Decoder_polar_MK_SCL_naive_CA_sys<B_8,Q_8>;
template class aff3ct::module::Decoder_polar_MK_SCL_naive_CA_sys<B_16,Q_16>;
template class aff3ct::module::Decoder_polar_MK_SCL_naive_CA_sys<B_32,Q_32>;
template class aff3ct::module::Decoder_polar_MK_SCL_naive_CA_sys<B_64,Q_64>;
#else
template class aff3ct::module::Decoder_polar_MK_SCL_naive_CA_sys<B,Q>;
#endif
// ==================================================================================== explicit template instantiation
