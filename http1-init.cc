#include <thread>
#include <chrono>
#include <cstdlib>

#include <cocaine/framework/worker.hpp>
#include <cocaine/framework/worker/http.hpp>

using namespace cocaine::framework;
using namespace cocaine::framework::worker;

int main(int argc, char** argv) {

    //using namespace std::chrono_literals;

    worker_t worker(options_t(argc, argv));

    const std::array<size_t, 3> delays = {1,3,7};

    typedef http::event<> http_t;
    worker.on<http_t>("http", [&delays](http_t::fresh_sender tx, http_t::fresh_receiver){
        http_response_t rs;
        rs.code = 200;

        std::chrono::seconds delta(delays[ std::rand() % delays.size()]);

        // TODO: logging
        std::this_thread::sleep_for(delta);

        tx.send(std::move(rs)).get()
            .send("Hello coxxx user from C++").get();

    });

    return worker.run();
}
