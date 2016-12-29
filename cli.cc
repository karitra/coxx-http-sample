#include <iostream>
#include <chrono>
#include <fstream>
#include <exception>
#include <iterator>
#include <sstream>
#include <thread>
#include <vector>

#include <cstdlib>

// not in boost 1.54
// #include <boost/program_options/postitional_options.hpp>

#include <msgpack.hpp>

#include <boost/range/irange.hpp>
#include <boost/lexical_cast.hpp>
// #include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>

#include "config.hpp"
#include "cli.hpp"

#include <cocaine/traits/attributes.hpp>
#include <cocaine/traits/enum.hpp>
#include <cocaine/traits/tuple.hpp>
#include <cocaine/traits/vector.hpp>
#include <cocaine/traits/endpoint.hpp>
#include <cocaine/traits/graph.hpp>

using namespace cocaine;
using namespace cocaine::framework;

namespace {
	const int TRIES_DEF = 4;
	const int COUNTER_DEF = 7;

	typedef boost::asio::ip::tcp::endpoint endpoint_t;

	std::string slurp(std::ifstream &&is) {
			std::ostringstream os;
			os << is.rdbuf();
			return os.str();
	}


	struct GraphPrinter {

		// GraphPrinter(const graph_root_t &r) : root(r) {}

		void print(std::ostream &os, const cocaine::io::graph_root_t &r) {

			for(const auto &p : r) {
					const auto &v =  p.second;
					os << "key: " << p.first << '\n'
							<< "  name: " << get<0>(v) << '\n';

							const auto &m1 = get<1>(v);
							const auto &m2 = get<2>(v);

							if (m1) { GraphPrinter::print(os, 2, m1); }
							if (m2) { GraphPrinter::print(os, 2, m2); }
			}

		}


 	private:

		template<class GraphNode>
		static void
		print(std::ostream &os, const int depth, const GraphNode &gn) {

			std::string tabs(depth, ' ');

			for(const auto &p : gn) {
				os << tabs << "id: " << p.first << '\n';
				if (p.second) {
					print(os, depth << 1, *p.second);
				}
			}
		}

	private:
		// const graph_root_t &root;
	};

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

#if 1
	auto point = slurp( std::move(ifstream("../endpoints.list") ) );
	boost::trim(point);
	cout << "endpoints: ( [" << point << "], "
			<< PPNConfig::DEFAULT_RT_PORT << " )\n";

	std::vector<endpoint_t> v;
	v.emplace_back(
			boost::asio::ip::address::from_string(point),
      PPNConfig::DEFAULT_RT_PORT );

	service_manager_t manager( move(v), cli::HWInfo::GetCpusCount());
#else
	service_manager_t manager( cli::HWInfo::GetCpusCount());
#endif

	cout << "creating link to app [" << srvName << "]\n";

	auto echo = manager.create<cocaine::io::app_tag>(srvName);

	auto locator = manager.create<cocaine::io::locator_tag>(PPNConfig::DEFAULT_LOCATOR);
	auto logger = manager.create<cocaine::io::log_tag>("logging");

	trace_t trace = trace_t::generate("cli::main");
	trace_t::restore_scope_t scope(trace);

	try {

		  echo.connect().get();
			locator.connect().get();
			logger.connect().get();

			logger.invoke<cocaine::io::log::emit>(logging::info, "cli1", "** client online" );

			cout << "sending " << tries << " ping(s)...\n";

			std::vector<task<void>::future_type> futs;
			futs.reserve(tries);

			auto msg = cntStr;

			if (event == string{"getfile"}) {

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

			} else if (event == string{"locate"}) {

				cout << "requesing info from locator @ " << PPNConfig::DEFAULT_LOCATOR << '\n'
						 << "\tfor app [" << srvName << "]\n";

				auto t = locator.invoke<cocaine::io::locator::resolve>(srvName).get();

				const auto endPts = std::get<0>(t);
				const auto ver    = std::get<1>(t);
				const auto gr     = std::get<2>(t);

				typedef std::remove_reference<decltype(endPts)>::type::value_type TEndpoint;

				cout << "proto version: " << ver << '\n';
				// cout << "graph: " << gr << '\n';

				copy( begin(endPts), end(endPts), ostream_iterator<TEndpoint>(cout, "\n") );

				return EXIT_SUCCESS;
			}

			for(const auto &i : boost::irange(0, tries)) {
				cout << "invoking request num. " << i << " for method " << event << '\n';

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
			logger.invoke<cocaine::io::log::emit>(logging::error, "cli1", e.what() );

			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
}
