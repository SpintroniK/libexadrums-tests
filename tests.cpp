#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "libexadrums/Api/eXaDrums.hpp"

#include <string>
#include <thread>
#include <chrono>

using namespace std::string_literals;
using namespace std::chrono_literals;
using namespace std::this_thread;

TEST_CASE("eXaDrums construction") 
{
    const auto configPath = std::getenv("HOME")+ "/.eXaDrums/Data/"s;
    eXaDrumsApi::eXaDrums exa{configPath.data()};
    const auto error = exa.GetInitError();
    const auto message = std::string{error.message};

    CHECK( message.empty() );
    REQUIRE( error.type == Util::error_type_success );
}