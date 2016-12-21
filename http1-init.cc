#include <thread>
#include <iostream>
#include <chrono>

#include <cstdlib>

#include <blackhole/attribute.hpp>
#include <blackhole/extensions/writer.hpp>

#include <cocaine/idl/node.hpp>

#include <cocaine/framework/worker.hpp>
#include <cocaine/framework/worker/http.hpp>
#include <cocaine/framework/trace.hpp>
#include <cocaine/framework/trace_logger.hpp>

#include <cocaine/framework/manager.hpp>
#include <cocaine/framework/service.hpp>

#include <cocaine/traits/attributes.hpp>
#include <cocaine/traits/enum.hpp>
#include <cocaine/traits/tuple.hpp>
#include <cocaine/traits/vector.hpp>



// #include <cocaine/framework/detail/log.hpp>

#include "config.hpp"
#include "cli.hpp"

using namespace cocaine;
using namespace cocaine::framework;
using namespace cocaine::framework::worker;

namespace {

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

  // namespace cocaine {
  //   namespace io { struct app_tag; }
  // }

  struct PingPongMethod {

    template<class Service, class LogWrapper>
    static void remoteInvoke(Service &srv, LogWrapper &logg, std::string name, worker::sender &tx, worker::receiver &rx) {

      using namespace std;
      // using namespace cocaine;
      // using namespace cocaine::framework;
      // using namespace cocaine::framework::worker;

      if (auto msg = rx.recv().get()) {

          ostringstream os;
          os << "get request for method: '" << *msg << "'\n";
          logg.info(os.str());

          const auto cnt = boost::lexical_cast<unsigned>(*msg);
          if (cnt > 0) {

            auto ppnSrv = srv.template create<cocaine::io::app_tag>(PPNConfig::APP_NAME_DEF);

            {
              ostringstream os;
              os << "calling method " << name << " with cnt: " << (cnt - 1);
              logg.info(os.str());
            }

            ppnSrv.connect().get();

            auto res = ppnSrv.template invoke<io::app::enqueue>(name)
                .then( trace_t::bind(&::cli::on_invoke_from_srv, std::placeholders::_1, boost::lexical_cast<string>(cnt - 1) ) );

            const auto v = res.get();

            {
              ostringstream os;
              os << "get response from method " << name << " call: " << v;
              logg.info(os.str());
            }

          } else {

            ostringstream os;
            os << "request sequence done";
            logg.info(os.str());

            tx.write("done").get();
          }
        }
    }
  };

  namespace util {

    using namespace std;

    template<class LogWrapper>
    struct call_sentry_logger_t {

        call_sentry_logger_t(LogWrapper &l, const string &pfx, const string &inMsg, const string &outMsg) :
          logRef(l),
          pfx(pfx),
          inMsg(inMsg),
          outMsg(outMsg) {
            ostringstream os;
            os << '[' << pfx << "] on enter: " << inMsg;
            logRef.info(os.str());
          }

        ~call_sentry_logger_t() {
          ostringstream os;
          os << '[' << pfx << "] on exit: " << outMsg;
          logRef.info(os.str());
        }

        LogWrapper &logRef;

        const string pfx;
        const string inMsg;
        const string outMsg;
    };

    //
    // template<class Logger, class... Args>
    // void
    // log_msg(Logger &l, const std::string &msg, const Args&... arg) { l.template invoke<io::log::emit>(msg, arg).get(); }
    //

    template<class LogRef>
    struct logger_wrapper_t {
        logger_wrapper_t(LogRef &l, const std::string &app) : logger(l), appName(app) {}

        void info(const string &str) {
          logger.template invoke<io::log::emit>( logging::info ,appName, str).get();
        }

      private:
        LogRef &logger;
        const string appName;
    };

  } // ns::cli
}


int main(int argc, char** argv) {

    //using namespace std::chrono_literals;
    using namespace std;
    using namespace ::util;

    const auto var1 = getenv(VAR_NAME);

    if (var1 != nullptr)
      cout << "init var " << VAR_NAME << " => " << var1 << '\n';

    const auto appName = string(PPNConfig::APP_NAME_DEF) + "::main";

    worker_t worker(options_t(argc, argv));
    cout << "initializing tracer...\n";

    auto trace = trace_t::generate(appName);
    trace_t::restore_scope_t scope(trace);

    cout << "initializing logger...\n";

    typedef decltype(*worker.manager().logger()) TLogger;
    typedef logger_wrapper_t<TLogger> TLoggerWrapper;

    TLoggerWrapper logg( *worker.manager().logger(), appName );

    logg.info("service ready");

    {
      ostringstream os;
      os << "init var " << VAR_NAME << " => " << var1;
      logg.info( os.str() );
    }

#if 1
    //
    // Entry web point
    // TODO:
    //    - port selection
    //    - parameters
    //
    typedef http::event<> http_t;
    worker.on<http_t>("http", [&logg](http_t::fresh_sender tx, http_t::fresh_receiver){
        http_response_t rs;
        rs.code = 200;

        logg.info("on web request");

        // std::this_thread::sleep_for(delta);

        tx.send(std::move(rs)).get()
            .send("Hello coxxx user from C++").get();

        logg.info("web request done");
    });
#endif

    const auto pingNamePStr = METHS_NAMES[PINGNameId];
    const auto pongNamePStr = METHS_NAMES[PONGNameId];

    logg.info("creating clients service manager...");
    service_manager_t manager(cli::HWInfo::GetCpusCount());

    auto storage    = manager.create<cocaine::io::app_tag>("storage");

    worker.on( "getfile", [&logg, &storage] (worker::sender tx, worker::receiver rx) {
        auto key = rx.recv().get();
        if (key) {

          {
            ostringstream os;
            os << "got storage request key = " << *key;
            logg.info(os.str());
          }

          auto ch = storage.invoke<io::app::enqueue>("store", *key).get();
          auto something = ch.rx.recv().get();

          {
            ostringstream os;
            os << "got from storage ";
            if (something) {
              os << *something;
              tx.write(*something).get();
            } else {
              os << "n/a";
            }

            logg.info( os.str() );
          }

        } else {
          logg.info("no key at all");
        }
    });

    worker.on( pingNamePStr , [&manager, &logg, pingNamePStr, pongNamePStr] (worker::sender tx, worker::receiver rx) {
        call_sentry_logger_t<decltype(logg)> cs(logg, pingNamePStr, "init", "done");
        PingPongMethod::remoteInvoke(manager, logg, pongNamePStr, tx, rx);
    });

    worker.on( pongNamePStr, [&manager, &logg, pingNamePStr, pongNamePStr] (worker::sender tx, worker::receiver rx) {
        call_sentry_logger_t<decltype(logg)> cs(logg, pongNamePStr, "init", "done");
        PingPongMethod::remoteInvoke(manager, logg, pingNamePStr, tx, rx);
    });

    worker.on( "test", [&logg] (worker::sender tx, worker::receiver rx) {
        call_sentry_logger_t<decltype(logg)> cs(logg, "test", "init", "done");

        if (auto msg = rx.recv().get()) {

          ostringstream os;
          os << "test request: " << *msg;
          logg.info(os.str());

          tx.write("done").get();
        }
    });

    worker.on("meta", [&logg](worker::sender tx, worker::receiver rx) {
        call_sentry_logger_t<decltype(logg)> cs(logg, "test", "init", "done");

        logg.info("meta request");

        // std::cout << "After invoke. Headers count: "
        //           << rx.invocation_headers().get_headers().size() << std::endl;

        if (auto frame = rx.recv<worker::frame_t>().get()) {
                std::cout << "After chunk: '" << frame->data << "'" << std::endl;
                tx.write(frame->data).get();
                std::cout << "After write" << std::endl;
        }

        std::cout << "After close" << std::endl;
    });

    return worker.run();
}
