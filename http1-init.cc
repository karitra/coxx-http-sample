#include <thread>
#include <iostream>
#include <chrono>

#include <cstdlib>

#include <cocaine/framework/worker.hpp>
#include <cocaine/framework/worker/http.hpp>
#include <cocaine/framework/trace.hpp>
#include <cocaine/framework/trace_logger.hpp>

using namespace cocaine;
using namespace cocaine::framework;
using namespace cocaine::framework::worker;

namespace {
  const auto APP_NAME = "PingPong";
  const auto VAR_NAME = "TEST1";
}

int main(int argc, char** argv) {

    //using namespace std::chrono_literals;
    using namespace std;

    const auto var1 = getenv(VAR_NAME);

    cout << "init var " << VAR_NAME << " => " << var1 << '\n'; 

    worker_t worker(options_t(argc, argv));

    auto trace = trace_t::generate( string(APP_NAME).append("::main") );
    trace_t::restore_scope_t scope(trace);

    //
    // Entry web point
    // TODO:
    //    - port selection
    //    - parameters
    //
    typedef http::event<> http_t;
    worker.on<http_t>("http", [](http_t::fresh_sender tx, http_t::fresh_receiver){
        http_response_t rs;
        rs.code = 200;

        cout << "on web request\n";

        // TODO: logging
        // std::this_thread::sleep_for(delta);

        tx.send(std::move(rs)).get()
            .send("Hello coxxx user from C++").get();

        cout << "web request done\n";
    });

     // worker.on

    return worker.run();
}
