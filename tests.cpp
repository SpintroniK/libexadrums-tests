#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "libexadrums/Api/eXaDrums.hpp"

#include <string>
#include <memory>
#include <iostream>

using namespace std::string_literals;
using namespace eXaDrumsApi;


TEST_CASE("eXaDrums construction", "[init]")  
{
    const auto configPath = std::getenv("HOME")+ "/.eXaDrums/Data/"s;
    std::cout << "Config path = " << configPath << std::endl;

    auto exa = eXaDrums{configPath.data()};

    const auto error = exa.GetInitError();
    const auto message = std::string{error.message};

    CHECK( message == "" );
    REQUIRE( error.type == Util::error_type_success );
}

TEST_CASE("eXaDrums tests", "[tests]") 
{
    const auto configPath = std::getenv("HOME")+ "/.eXaDrums/Data/"s;
    auto exa = eXaDrums{configPath.data()};

    REQUIRE_NOTHROW( exa.Start() );
    REQUIRE_NOTHROW( exa.Stop() );
}