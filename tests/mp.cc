#include <iostream>
#include <sstream>
#include <tuple>

#include <cstdlib>

#include <msgpack.hpp>

using namespace std;

int main(int argc, char *argv[])
{
	cout << "starting tests\n";

	// typedef std::tuple<std::string,std::string> TStdMsgType;
	typedef msgpack::type::tuple<std::string,std::string> TMsgType;

	// auto t = make_tuple("hello", "msg");
	// msgpack::type::tuple<string, string> src1("hello", "message");
	const auto src1 = msgpack::type::make_tuple<string,string>("hello", "message4");

	stringstream s;
	msgpack::pack(s, src1);
	auto packedMsg = s.str();

	cout << "packed msg: " << s.str() << '\n';

	// auto off = size_t{};
	msgpack::unpacked unp;
	msgpack::unpack( &unp, packedMsg.data(), packedMsg.size() );
	msgpack::object obj = unp.get();

	cout << "obj " << obj << '\n';

	const auto dst = obj.as<TMsgType>();

	cout << "fst: " << dst.get<0>() << '\n';
	cout << "snd: " << dst.get<1>() << '\n';

	cout << "tests done\n";
	return EXIT_SUCCESS;
}
