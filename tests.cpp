#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "libexadrums/Api/eXaDrums.hpp"
#include "libexadrums/Api/KitCreator/KitCreator_api.hpp"

#include <string>
#include <memory>
#include <iostream>

using namespace std::string_literals;
using namespace Catch::Matchers;
using namespace Catch::Generators;
using namespace eXaDrumsApi;

TEST_CASE("eXaDrums construction", "[init]")  
{

    const auto configPath = std::getenv("HOME")+ "/.eXaDrums/Data/"s;
    INFO("Config path = " << configPath);

    auto exa = eXaDrums{configPath.data()};

    const auto error = exa.GetInitError();
    const auto message = std::string{error.message};

    CHECK( message == "" );
    REQUIRE( error.type == Util::error_type_success );
}

TEST_CASE("eXaDrums tests", "[drumkits]") 
{

    const auto configPath = std::getenv("HOME")+ "/.eXaDrums/Data/"s;
    auto exa = eXaDrums{configPath.data()};

    // Make kit creator
    std::string dataFolder(exa.GetDataLocation());
    auto kitCreator = std::make_unique<KitCreator>(dataFolder.c_str());

    SECTION("Check config folder")
    {
        CHECK(1);
    }

    int numInstruments = kitCreator->GetNumInstruments();

    REQUIRE(numInstruments == 0);

    SECTION("Create a new kit with 8 instruments")
    {
        const size_t numInstrumentsToCreate = 8;
        const auto instrumentTypes = kitCreator->GetInstrumentsTypes();

        const auto instType = "Pad"s;

        REQUIRE_THAT( instrumentTypes, VectorContains(instType) );

        REQUIRE(instrumentTypes.size() > 0);

		kitCreator->CreateNewKit();
		kitCreator->SetKitName("test kit");

        for(size_t i = 0; i < numInstrumentsToCreate; ++i)
        {
            std::string instrumentName = "Instrument " + std::to_string(i + 1);
            kitCreator->CreateNewInstrument();
			kitCreator->SetInstrumentVolume(1.0f);

			REQUIRE_NOTHROW( kitCreator->SetInstrumentType(instType.c_str()) );
            REQUIRE_NOTHROW( kitCreator->SetInstrumentName(instrumentName.c_str()) );    
            REQUIRE_NOTHROW( kitCreator->AddInstrumentSound("Crash/Crash_High.wav", "DrumHead") );
            REQUIRE_NOTHROW( kitCreator->AddInstrumentTrigger(i, "DrumHead") );

            kitCreator->AddInstrumentToKit();

        }

        // All instruments have been added, save kit.
        REQUIRE_NOTHROW( kitCreator->SaveKit() );

		const auto oldKitsNames = exa.GetKitsNames();

		// Retrieve kits names
        REQUIRE_NOTHROW( exa.ReloadKits() );

		const auto newkitsNames = exa.GetKitsNames();

        REQUIRE( oldKitsNames.size() + 1 == newkitsNames.size() );

        kitCreator->CreateNewKit();


    }

    SECTION("Delete test kit")
    {
        // Delete last kit
        const size_t nbKits = exa.GetKitsNames().size();
        REQUIRE( nbKits > 1 );
        REQUIRE_NOTHROW( exa.DeleteKit(nbKits - 1) );
    }

}