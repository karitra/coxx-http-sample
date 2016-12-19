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


  enum MethodName {
    PINGNameId,
    PONGNameId,
    MethodsCount
  };

  const char *METHS_NAMES[] = {
    "ping",
    "pong",
  };


  namespace util {

    using namespace std;

    struct call_sentry_logger_t {

        call_sentry_logger_t(ostream &os, const string &pfx, const string &inMsg, const string &outMsg) :
          osRef(os),
          pfx(pfx),
          inMsg(inMsg),
          outMsg(outMsg) { osRef << '[' << pfx << "] on enter: " << inMsg; }

        ~call_sentry_logger_t() { osRef << '[' << pfx << "] on exit: " << outMsg; }

        ostream &osRef;

        const string pfx;
        const string inMsg;
        const string outMsg;
    };
  }

}

int main(int argc, char** argv) {

    //using namespace std::chrono_literals;
    using namespace std;
    using namespace ::util;

    const auto var1 = getenv(VAR_NAME);

    if (var1 != nullptr)
      cout << "init var " << VAR_NAME << " => " << var1 << '\n';

    worker_t worker(options_t(argc, argv));

#if 0
    auto trace = trace_t::generate( string(APP_NAME).append("::main") );
    trace_t::restore_scope_t scope(trace);
#endif

#if 0
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
#endif

    const auto pingNamePStr = METHS_NAMES[PINGNameId];
    worker.on( pingNamePStr , [pingNamePStr] (worker::sender tx, worker::receiver rx) {
        call_sentry_logger_t cs(cout, pingNamePStr, "init", "done");

        if (auto msg = rx.recv().get()) {
            std::cout << "After chunk: '" << *msg << "'\n";
            tx.write(*msg).get();
            std::cout << "After write\n";
          }
    });

    const auto pongNamePStr = METHS_NAMES[PONGNameId];
    worker.on(pongNamePStr, [pongNamePStr] (worker::sender tx, worker::receiver rx) {
        call_sentry_logger_t cs(cout, pongNamePStr, "init", "done");

        if (auto msg = rx.recv().get()) {
            std::cout << "After chunk: '" << *msg << "'\n";
            tx.write("").get();
            std::cout << "After write\n";
          }
    });

    return worker.run();
}
