#include <iostream>
#include <chrono>
#include <exception>
#include <thread>
#include <vector>

#include <cstdlib>

//#include <cocaine/framework/detail/log.hpp>
#include <cocaine/framework/service.hpp>
#include <cocaine/framework/manager.hpp>
#include <cocaine/idl/node.hpp>
#include <cocaine/traits/error_code.hpp>

#include <boost/range/irange.hpp>
#include <boost/lexical_cast.hpp>

using namespace cocaine;
using namespace cocaine::framework;

namespace {

	// const auto APP_NAME = "PingPongNative";
	const auto APP_NAME_DEF = "ppn"; // ppn => PingPingNative
	const unsigned HW_CPU_DEF = 2;
	const int TRIES_DEF = 4;

	namespace app {

		typedef io::protocol<io::app::enqueue::dispatch_type>::scope scope;

		task<boost::optional<std::string>>::future_type
		on_send(task<channel<io::app::enqueue>::sender_type>::future_move_type future,
		    channel<io::app::enqueue>::receiver_type rx)
		{
		    future.get();
		    return rx.recv();
		}

		task<boost::optional<std::string>>::future_type
		on_chunk(task<boost::optional<std::string>>::future_move_type future,
		    channel<io::app::enqueue>::receiver_type rx)
		{
		    auto result = future.get();
		    if (!result) {
		        throw std::runtime_error("the `result` must be true");
		    }
		    return rx.recv();
		}

		void
		on_choke(task<boost::optional<std::string>>::future_move_type future) {
		    auto result = future.get();
				if (!result) {
					std::cout << "got result: " << result.get() << '\n';
				} else {
					std::cout << "no reasult at all!\n";
				}
		}


		task<void>::future_type
		on_invoke(task<channel<io::app::enqueue>>::future_move_type future) {

			auto channel = future.get();

			auto tx = std::move(channel.tx);
    	auto rx = std::move(channel.rx);

			return tx.send<scope::chunk>("100500")
        .then(trace_t::bind(&on_send,  std::placeholders::_1, rx))
        .then(trace_t::bind(&on_chunk, std::placeholders::_1, rx))
        .then(trace_t::bind(&on_choke, std::placeholders::_1));
			}
	} // ns::app
} // ns::

int main(int argc, char *argv[]) {

	using namespace std;
	// using namespace std::chrono_literals;

	int tries = (argc > 1) ? boost::lexical_cast<int>(argv[1]) : TRIES_DEF;
	auto srvName = (argc > 2) ? string(argv[2]) : APP_NAME_DEF;

	const auto hwCpu = std::thread::hardware_concurrency();
	unsigned cpuCount = (hwCpu) ? hwCpu << 1 : HW_CPU_DEF;

	cout << "initializing service manager\n";

	service_manager_t manager(cpuCount);

	std::string event = "ping";

	auto echo = manager.create<cocaine::io::app_tag>(srvName);

	trace_t trace = trace_t::generate("main");
	trace_t::restore_scope_t scope(trace);

	try {

			cout << "connecting to [" << srvName << "] ...\n";
			echo.connect().get();

			cout << "sending " << tries << " ping(s)...\n";
			std::vector<task<void>::future_type> futs;
			futs.reserve(tries);

			for(const auto &i : boost::irange(0, tries)) {
				cout << "invoking request: " << i << '\n';

				futs.emplace_back(
					echo.invoke<cocaine::io::app::enqueue>(event)
						.then( trace_t::bind(&app::on_invoke, std::placeholders::_1) )
				);
			}

			std::this_thread::sleep_for ( chrono::seconds(1) );

			for(const auto &f : futs) {
					f.wait();
			}

		} catch(const std::exception &e) {
			cerr << "Something went wrong: " << e.what() << '\n';
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
}
