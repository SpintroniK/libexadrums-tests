#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "libexadrums/Api/eXaDrums.hpp"
#include "libexadrums/Api/KitCreator/KitCreator_api.hpp"
#include "libexadrums/Api/Config/Config_api.hpp"

#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include <iostream>

#if __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif

using namespace std::string_literals;
using namespace std::chrono_literals;
using namespace std::this_thread;
using namespace Catch::Matchers;
using namespace eXaDrumsApi;


TEST_CASE("Check configuration files", "[config]")
{
    SECTION("Check Data folder")
    {

    }

    SECTION("Check for Hdd sensors data")
    {

    }
}

TEST_CASE("eXaDrums initialization test", "[init]")  
{

    const auto configPath = std::getenv("HOME")+ "/.eXaDrums/Data/"s;
    INFO("Config path = " << configPath);

    auto exa = eXaDrums{configPath.data()};

    const auto error = exa.GetInitError();
    const auto message = std::string{error.message};

    CHECK( message == "" );
    REQUIRE( error.type == Util::error_type_success );
}

TEST_CASE("eXaDrums drum kits tests", "[drumkit]") 
{

    const auto configPath = std::getenv("HOME")+ "/.eXaDrums/Data/"s;
    auto exa = eXaDrums{configPath.data()};

    // Make kit creator
    std::string dataFolder(exa.GetDataLocation());
    auto kitCreator = std::make_unique<KitCreator>(dataFolder.c_str());

    // Make config manager
    auto config = Config(exa);

    SECTION("Sensors configuration")
    {
        REQUIRE_NOTHROW( config.LoadTriggersConfig() );
        const auto sensorsTypes = config.GetSensorsTypes();


        REQUIRE_THAT( sensorsTypes, VectorContains("Hdd"s) );
        REQUIRE_NOTHROW( config.SetSensorsType("Hdd"s) );

        REQUIRE_NOTHROW( config.SaveSensorsConfig() );
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
		kitCreator->SetKitName("test_kit");

        for(size_t i = 0; i < numInstrumentsToCreate; ++i)
        {
            std::string instrumentName = "Instrument " + std::to_string(i + 1);
            kitCreator->CreateNewInstrument();
			kitCreator->SetInstrumentVolume(0.1f);

			REQUIRE_NOTHROW( kitCreator->SetInstrumentType(instType.c_str()) );
            REQUIRE_NOTHROW( kitCreator->SetInstrumentName(instrumentName.c_str()) );    
            REQUIRE_NOTHROW( kitCreator->AddInstrumentSound("SnareDrum/Snr_Acou_01.wav", "DrumHead") );
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

    SECTION("Modify test kit instrument")
    {

    }

    SECTION("Recorder test")
    {
        REQUIRE_NOTHROW( kitCreator->CreateFromModel("test_kit.xml") );
        const size_t nbInst = kitCreator->GetNumInstruments();

        // Keep only one instrument
        for(size_t i = 0; i < nbInst - 1; ++i)
        {
            kitCreator->RemoveLastInstrument();
        }

        REQUIRE_NOTHROW( kitCreator->SaveKit() );
        REQUIRE( exa.GetNumKits() > 1);

        // Select last kit (test kit)
        REQUIRE_NOTHROW( exa.SelectKit(exa.GetNumKits() - 1) );

        REQUIRE_NOTHROW( exa.EnableRecording(true) );
        REQUIRE_NOTHROW( exa.Start() );

        sleep_for(5s);

        REQUIRE_NOTHROW( exa.Stop() );
        REQUIRE_NOTHROW( exa.EnableRecording(false) );

        REQUIRE_NOTHROW( exa.RecorderExport(configPath + "Rec/test.xml") );

        REQUIRE( fs::exists("test.xml") );
    }

    SECTION("Delete test kit")
    {
        // Delete last kit
        const size_t nbKits = exa.GetKitsNames().size();
        REQUIRE( nbKits > 1 );
        REQUIRE_NOTHROW( exa.DeleteKit(nbKits - 1) );
    }

    SECTION("Reset sensors configuration")
    {
        REQUIRE_NOTHROW( config.SetSensorsType("Virtual"s) );
        REQUIRE_NOTHROW( config.SaveSensorsConfig() );
    }

}