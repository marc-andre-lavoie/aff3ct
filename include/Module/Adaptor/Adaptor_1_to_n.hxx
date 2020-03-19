#include <string>
#include <sstream>

#include "Tools/Exception/exception.hpp"
#include "Module/Adaptor/Adaptor_1_to_n.hpp"

namespace aff3ct
{
namespace module
{

Task& Adaptor_1_to_n
::operator[](const adp::tsk t)
{
	return Module::operator[]((size_t)t);
}

Socket& Adaptor_1_to_n
::operator[](const adp::sck::push_1 s)
{
	return Module::operator[]((size_t)adp::tsk::push_1)[(size_t)s];
}

Socket& Adaptor_1_to_n
::operator[](const adp::sck::pull_n s)
{
	return Module::operator[]((size_t)adp::tsk::pull_n)[(size_t)s];
}

Adaptor_1_to_n
::Adaptor_1_to_n(const size_t n_elmts,
                 const std::type_index datatype,
                 const size_t buffer_size,
                 const bool active_waiting,
                 const int n_frames)
: Adaptor(n_elmts, datatype, buffer_size, n_frames),
  active_waiting(active_waiting),
  cnd_pull(new std::vector<std::condition_variable>(1000)),
  mtx_pull(new std::vector<std::mutex             >(1000)),
  cnd_put (new             std::condition_variable (    )),
  mtx_put (new             std::mutex              (    ))
{
	this->init();
}

Adaptor_1_to_n
::Adaptor_1_to_n(const std::vector<size_t> &n_elmts,
                 const std::vector<std::type_index> &datatype,
                 const size_t buffer_size,
                 const bool active_waiting,
                 const int n_frames)
: Adaptor(n_elmts, datatype, buffer_size, n_frames),
  active_waiting(active_waiting),
  cnd_pull(new std::vector<std::condition_variable>(1000)),
  mtx_pull(new std::vector<std::mutex             >(1000)),
  cnd_put (new             std::condition_variable (    )),
  mtx_put (new             std::mutex              (    ))
{
	this->init();
}

void Adaptor_1_to_n
::init()
{
	const std::string name = "Adaptor_1_to_n";
	this->set_name(name);
	this->set_short_name(name);

	std::function<size_t(Task&, const size_t, const std::type_index&, const std::string&)> create_socket_in =
		[this](Task& p, const size_t n_elmts, const std::type_index& datatype, const std::string& n)
		{
			     if (datatype == typeid(int8_t )) return this->template create_socket_in<int8_t >(p, n, n_elmts);
			else if (datatype == typeid(int16_t)) return this->template create_socket_in<int16_t>(p, n, n_elmts);
			else if (datatype == typeid(int32_t)) return this->template create_socket_in<int32_t>(p, n, n_elmts);
			else if (datatype == typeid(int64_t)) return this->template create_socket_in<int64_t>(p, n, n_elmts);
			else if (datatype == typeid(float  )) return this->template create_socket_in<float  >(p, n, n_elmts);
			else if (datatype == typeid(double )) return this->template create_socket_in<double >(p, n, n_elmts);
			else
			{
				std::stringstream message;
				message << "This should never happen.";
				throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
			}
		};

	std::function<size_t(Task&, const size_t, const std::type_index&, const std::string&)> create_socket_out =
		[this](Task& p, const size_t n_elmts, const std::type_index& datatype, const std::string& n)
		{
			     if (datatype == typeid(int8_t )) return this->template create_socket_out<int8_t >(p, n, n_elmts);
			else if (datatype == typeid(int16_t)) return this->template create_socket_out<int16_t>(p, n, n_elmts);
			else if (datatype == typeid(int32_t)) return this->template create_socket_out<int32_t>(p, n, n_elmts);
			else if (datatype == typeid(int64_t)) return this->template create_socket_out<int64_t>(p, n, n_elmts);
			else if (datatype == typeid(float  )) return this->template create_socket_out<float  >(p, n, n_elmts);
			else if (datatype == typeid(double )) return this->template create_socket_out<double >(p, n, n_elmts);
			else
			{
				std::stringstream message;
				message << "This should never happen.";
				throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
			}
		};

	auto &p1 = this->create_task("push_1", (int)adp::tsk::push_1);
	std::vector<size_t> p1s_in;
	for (size_t s = 0; s < this->n_sockets; s++)
		p1s_in.push_back(create_socket_in(p1, this->n_elmts[s], this->datatype[s], "in" + std::to_string(s+1)));

	this->create_codelet(p1, [p1s_in](Module &m, Task &t) -> int
	{
		auto &adp = static_cast<Adaptor_1_to_n&>(m);
		if (adp.is_no_copy_push())
		{
			adp.wait_push();
			// for debug mode coherence
			for (size_t s = 0; s < t.sockets.size() -1; s++)
				t.sockets[s]->bind(adp.get_empty_buffer(s));
		}
		else
		{
			std::vector<const int8_t*> sockets_dataptr(p1s_in.size());
			for (size_t s = 0; s < p1s_in.size(); s++)
				sockets_dataptr[s] = static_cast<const int8_t*>(t[p1s_in[s]].get_dataptr());
			adp.push_1(sockets_dataptr);
		}
		return status_t::SUCCESS;
	});

	auto &p2 = this->create_task("pull_n", (int)adp::tsk::pull_n);
	std::vector<size_t> p2s_out;
	for (size_t s = 0; s < this->n_sockets; s++)
		p2s_out.push_back(create_socket_out(p2, this->n_elmts[s], this->datatype[s], "out" + std::to_string(s+1)));

	this->create_codelet(p2, [p2s_out](Module &m, Task &t) -> int
	{
		auto &adp = static_cast<Adaptor_1_to_n&>(m);
		if (adp.is_no_copy_pull())
		{
			adp.wait_pull();
			// for debug mode coherence
			for (size_t s = 0; s < t.sockets.size() -1; s++)
				t.sockets[s]->bind(adp.get_filled_buffer(s));
		}
		else
		{
			std::vector<int8_t*> sockets_dataptr(p2s_out.size());
			for (size_t s = 0; s < p2s_out.size(); s++)
				sockets_dataptr[s] = static_cast<int8_t*>(t[p2s_out[s]].get_dataptr());
			adp.pull_n(sockets_dataptr);
		}
		return status_t::SUCCESS;
	});
}
}
}