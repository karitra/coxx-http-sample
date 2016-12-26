#include <iostream>
#include <chrono>
#include <exception>
#include <thread>
#include <vector>

#include <cstdlib>

// not in boost 1.54
// #include <boost/program_options/postitional_options.hpp>

#include <msgpack.hpp>

#include <boost/range/irange.hpp>
#include <boost/lexical_cast.hpp>

#include "config.hpp"
#include "cli.hpp"

using namespace cocaine;
using namespace cocaine::framework;

namespace {
	const int TRIES_DEF = 4;
	const int COUNTER_DEF = 7;
} // ns::

int main(int argc, char *argv[]) {

	using namespace std;
	// using namespace std::chrono_literals;

	/** postitional_options not in boost 1.54
	namespace po = boost::program_options;

	po::options_description desc("cli options");
	po::positional_options_description p;

	p.add("srv", 1);
	p.add("method", 1);
	p.add("param", 1);
	p.add("conns", 1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).
	          options(desc).positional(p).run(), vm);
	po::notify(vm);

	int tries    = vm.count("conns") ? vm["conns"].as<int>() : TRIES_DEF;
	auto srvName = vm.count("srv")   ? vm["srv"].as<string>() : PPNConfig::APP_NAME_DEF;
	auto cntStr  = vm.count("param") ? vm["param"].as<string>() : boost::lexical_cast<string>(COUNTER_DEF);
	auto event   = vm.count("method") ? vm["method"].as<string>() : PPNConfig::CLI_MATHOD_DEF;
	*/

	auto srvName = (argc > 1) ? argv[1] : PPNConfig::APP_NAME_DEF;
	auto event   = (argc > 2) ? argv[2] : PPNConfig::CLI_MATHOD_DEF;
	auto cntStr  = (argc > 3) ? argv[3] : boost::lexical_cast<string>(COUNTER_DEF);
	auto tries   = (argc > 4) ? boost::lexical_cast<int>(argv[4]) : TRIES_DEF;

	cout << "initializing service manager\n";

	service_manager_t manager(cli::HWInfo::GetCpusCount());
	auto echo = manager.create<cocaine::io::app_tag>(srvName);

	trace_t trace = trace_t::generate("cli::main");
	trace_t::restore_scope_t scope(trace);

	try {

			cout << "connecting to [" << srvName << "] ...\n";
			echo.connect().get();

			cout << "sending " << tries << " ping(s)...\n";
			std::vector<task<void>::future_type> futs;
			futs.reserve(tries);

			auto msg = cntStr;

			if (event == string("getfile")) {

				auto ns  = string{"store"};
				auto key = string{"test.txt"};

				auto pos = cntStr.find('/');
				if (pos != string::npos) {
					ns  = cntStr.substr(0,pos);
					key = cntStr.substr(pos+1);
				}

				std::stringstream obuff;
				msgpack::type::tuple<string,string> src(ns,key);
				msgpack::pack(obuff, src);

				obuff.seekg(0);

				msg = obuff.str();
				cout << "packed message " << msg << '\n';
			}

			for(const auto &i : boost::irange(0, tries)) {
				cout << "invoking request num. " << i << " for method " << event << " with message " << msg << '\n';

				futs.emplace_back(
					echo.invoke<cocaine::io::app::enqueue>(event)
						.then( trace_t::bind(&::cli::on_invoke, std::placeholders::_1, msg) )
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
