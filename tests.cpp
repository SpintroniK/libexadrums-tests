#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "libexadrums/Api/eXaDrums.hpp"
#include "libexadrums/Api/KitCreator/KitCreator_api.hpp"
#include "libexadrums/Api/Config/Config_api.hpp"
#include <libexadrums/Util/Xml.h>

#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include <iostream>
#include <stack>
#include <map>

using namespace std::string_literals;

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
using namespace tinyxml2;
using namespace eXaDrumsApi;


class Operation
{
public:
    Operation() = default;
    virtual ~Operation() = default;

    virtual bool IsInput() const  = 0;
    virtual std::string Compute() = 0;

};

using OperationPtr = std::shared_ptr<Operation>;

class Input : public Operation
{
public:
    Input() : Operation() {}
    virtual ~Input() = default;
    

    virtual bool IsInput() const final { return true; }
};


class UnaryOp : public Operation
{
public:

    UnaryOp(std::stack<Operation*>& opStack) : Operation()
    {
        child = opStack.top();
        opStack.pop();
    }

    UnaryOp(Operation* op) : Operation(), child{op} {}

    virtual ~UnaryOp() 
    { 
         delete child; 
    }

    void SetChild(Operation* node)
    {
        child = node;
    }

    virtual bool IsInput() const final { return false; }

    Operation* GetChild() const { return child; }


protected:

    Operation* child;

};


class BinaryOp : public Operation
{
public:

    BinaryOp(std::stack<Operation*>& opStack) : Operation()
    {
        op1 = opStack.top();
        opStack.pop();
        op2 = opStack.top();
        opStack.pop();
    }

    BinaryOp(Operation* op1, Operation* op2) : Operation() {}

    virtual ~BinaryOp()
    {
        delete op1;
        delete op2;
    }

    virtual bool IsInput() const final { return false; }


protected:

    Operation* op1{nullptr};
    Operation* op2{nullptr};

};

class SoundInput final : public Input
{
public:
    SoundInput() : Input() 
    {
    }

    virtual ~SoundInput() = default;

    virtual std::string Compute() final { return "Sound"; }

private:

    std::string fileLocation;
    std::string soundType;

};


class TriggerInput final : public Input
{
public:
    TriggerInput() : Input()
    {

    }

    virtual ~TriggerInput() = default;

    virtual std::string Compute() final { return "Trigger"; }

private:

};


class AmplitudeModulator final : public BinaryOp
{
public:

    AmplitudeModulator(std::stack<Operation*>& opStack) : BinaryOp(opStack)
    {

    }

    virtual ~AmplitudeModulator() = default;

    virtual std::string Compute() final { return "AmplitudeModulator \n[\n\t"s + op1->Compute() + " | "s + op2->Compute() + "\n]"s; }

};


class OperationFactory
{

public:

    OperationFactory() : operationMap{}
    {
        operationMap["Sound"] = &OperationFactory::MakeSound;
        operationMap["Trigger"] = &OperationFactory::MakeTrigger;
        operationMap["AmplitudeModulator"] = &OperationFactory::MakeAmplitudeModulator;
    }
    
    ~OperationFactory() = default;

    Operation* MakeOperation(const std::string& opName, std::stack<Operation*>& opStack) const
    {
       auto iter = operationMap.find(opName);
       if(iter != operationMap.end())
       {
           return (this->*iter->second)(opStack);
       }

       return nullptr;
    }

    Operation* MakeSound(std::stack<Operation*>&) const
    {
        return new SoundInput();
    }

    Operation* MakeTrigger(std::stack<Operation*>&) const
    {
        return new TriggerInput();
    }

    Operation* MakeAmplitudeModulator(std::stack<Operation*>& opStack) const
    {
        return new AmplitudeModulator(opStack);
    }

private:

    using FactoryPtmf = Operation*(OperationFactory::*)(std::stack<Operation*>&) const;
    using OperationMap = std::map<std::string, FactoryPtmf>;

    OperationMap operationMap;

};


OperationFactory operationFactory;


class Interpreter
{

public:
    Interpreter() = default;
    ~Interpreter() = default;

    Operation* Interpret(const Util::XmlElement& root)
    {

        for(const auto& e : root)
        {
            std::cout << e.TagName() << " / " << e.Attribute<std::string>("class") << std::endl;
            Operation* op = Parse(e);
            return op;
        }

        return nullptr;
    }

private:

    Operation* Parse(const Util::XmlElement& e, bool isRoot = true)
    {
        for(const auto& se : e)
        {
            const auto type = se.Attribute<std::string>("class");
            std::cout << se.TagName() << " / " << type << std::endl;

            Parse(se, false);

            Operation* subOp = operationFactory.MakeOperation(type, operationStack);
            operationStack.push(subOp);
        }

        if(!isRoot)
            return nullptr;

        return operationFactory.MakeOperation(e.Attribute<std::string>("class"), operationStack);
    }

    std::stack<Operation*> operationStack;

};


TEST_CASE("Xml reader test", "[xml]")  
{

    SECTION("test")
    {


        const auto configPath = std::getenv("HOME")+ "/.eXaDrums/Data/"s;
        INFO("Config path = " << configPath);

        auto exa = eXaDrums{configPath.data()};

        std::cout << exa.GetVersion() << std::endl;

        const std::string fileName = "instrument.xml";

		XMLDocument doc;

		if(doc.LoadFile(fileName.c_str()) != XML_SUCCESS)
		{
			throw -1;
		}

		XMLElement* root = doc.RootElement();
        XMLElement* instrumentRoot = root->FirstChildElement("Instrument");
        const auto instrument = Util::XmlElement{instrumentRoot};

        std::cout << "Instrument name = " << instrument.Attribute<std::string>("name") << std::endl;

        Interpreter interpreter;

        auto OpTree = std::unique_ptr<Operation>(interpreter.Interpret(instrument));

        std::cout << OpTree->Compute() << std::endl;

        // size_t i = 0;

        // auto rootNode = makeNode("root", "Output", Util::XmlElement{nullptr});

        // interpret(instrument, i, rootNode);

        // auto node = dynamic_cast<UnaryOp*>(rootNode.get());

        // node->Print();

    }
}

/*TEST_CASE("Check configuration files", "[config]")
{

    const auto configPath = std::getenv("HOME")+ "/.eXaDrums/Data/"s;

    SECTION("Check Data folder")
    {

    }

    SECTION("Check Hdd sensors data")
    {
        REQUIRE( fs::exists("/usr/share/exadrums/Data/data/out.raw") );
    }
}

TEST_CASE("eXaDrums initialization test", "[init]")  
{

    SECTION("Init test")
    {

        const auto configPath = std::getenv("HOME")+ "/.eXaDrums/Data/"s;
        INFO("Config path = " << configPath);

        auto exa = eXaDrums{configPath.data()};

        const auto error = exa.GetInitError();
        const auto message = std::string{error.message};

        CHECK( message == "" );
        REQUIRE( error.type == Util::error_type_success );

    }
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
        REQUIRE( exa.GetNumKits() > 1 );

        // Select last kit (test kit)
        REQUIRE_NOTHROW( exa.SelectKit(exa.GetNumKits() - 1) );

        REQUIRE_NOTHROW( exa.EnableRecording(true) );
        REQUIRE_NOTHROW( exa.Start() );

        sleep_for(5s);

        REQUIRE_NOTHROW( exa.Stop() );
        REQUIRE_NOTHROW( exa.EnableRecording(false) );

        REQUIRE_NOTHROW( exa.RecorderExport(configPath + "Rec/test.xml") );

        REQUIRE( fs::exists(configPath + "Rec/test.xml") );
        CHECK( fs::remove(configPath + "Rec/test.xml") );
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

}*/

/*TEST_CASE("Import and export config tests", "[config]") 
{
    SECTION("test")
    {
        const auto home = std::getenv("HOME");
        const auto configPath = home + "/.eXaDrums/Data/"s;
        auto exa = eXaDrums{configPath.data()};

        //Config::ExportConfig(home + "/.eXaDrums"s, "./test.zip");
        //Config::ImportConfig("test.zip", "test_config");

        //fs::remove("test.zip");
        fs::remove_all("test_config");
    }
}*/