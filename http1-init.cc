#include <thread>
#include <chrono>

#include <cocaine/framework/worker.hpp>
#include <cocaine/framework/worker/http.hpp>

using namespace cocaine::framework;
using namespace cocaine::framework::worker;

int main(int argc, char** argv) {

    //using namespace std::chrono_literals;

    worker_t worker(options_t(argc, argv));

    std::array<int, 3> deleays = {1,3,7};

    typedef http::event<> http_t;
    worker.on<http_t>("http", [](http_t::fresh_sender tx, http_t::fresh_receiver){
        http_response_t rs;
        rs.code = 200;

        std::chrono::seconds delta(2);
        std::this_thread::sleep_for(delta);

        tx.send(std::move(rs)).get()
            .send("Hello coxxx user from C++").get();

    });

    return worker.run();
}
