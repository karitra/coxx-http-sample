#ifndef __INCL_CLI__

#include <msgpack.hpp>

#include <cocaine/framework/service.hpp>
#include <cocaine/framework/manager.hpp>

#include <cocaine/idl/node.hpp>
#include <cocaine/idl/locator.hpp>

#include <cocaine/traits/error_code.hpp>

#include <cocaine/hpack/static_table.hpp>

namespace {

		namespace cli {

			using namespace cocaine;
			using namespace cocaine::framework;

			typedef io::protocol<io::app::enqueue::dispatch_type>::scope scope;

			struct HWInfo {
				static unsigned GetCpusCount() {
					const auto hwCpu = std::thread::hardware_concurrency();
					return (hwCpu) ? hwCpu << 1 : PPNConfig::HW_CPU_DEF;
				}
			};

			task<boost::optional<std::string>>::future_type
			on_send(task<channel<io::app::enqueue>::sender_type>::future_move_type future,
			    channel<io::app::enqueue>::receiver_type rx)
			{
					std::cout << "[send]\n";
			    future.get();
			    return rx.recv();
			}

			task<boost::optional<std::string>>::future_type
			on_chunk(task<boost::optional<std::string>>::future_move_type future,
			    channel<io::app::enqueue>::receiver_type rx)
			{
			    auto result = future.get();
			    if (!result) {
							std::cout << "[chunk] no result, throwing...\n";
			        throw std::runtime_error("[chunk] the `result` must be true");
			    } else {
						std::cout << "[chunk] got result: `" << result.get() << "`\n";
					}

			    return rx.recv();
			}

			void
			on_choke(task<boost::optional<std::string>>::future_move_type future) {
			    auto result = future.get();
					if (result) {
						std::cout << "[choke] got result: `" << result.get() << "`\n";
					} else {
						std::cout << "[choke] no reasult at all!\n";
					}
			}

			task<void>::future_type
			on_invoke(task<channel<io::app::enqueue>>::future_move_type future, const std::string &msg) {

				auto channel = future.get();

				auto tx = std::move(channel.tx);
	    	auto rx = std::move(channel.rx);

				return tx.send<scope::chunk>(msg)
	        .then(trace_t::bind(&on_send,  std::placeholders::_1, rx))
	        .then(trace_t::bind(&on_chunk, std::placeholders::_1, rx))
	        .then(trace_t::bind(&on_choke, std::placeholders::_1));
				}

				task<boost::optional<std::string>>::future_type
				on_invoke_from_srv(task<channel<io::app::enqueue>>::future_move_type future, const std::string &msg) {

					auto channel = future.get();

					auto tx = std::move(channel.tx);
		    	auto rx = std::move(channel.rx);

					return tx.send<scope::chunk>(msg)
		        .then(trace_t::bind(&on_send, std::placeholders::_1, rx));
				}

		} // ns::app
}

#endif
