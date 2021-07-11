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

enum class NodeType
{
    Sound, 
    Trigger,
    AmplitudeModulator,
    Output
};

class AbstractNode
{
public:
    AbstractNode(const std::string& name, NodeType type): name{name}, type{type} {}

    virtual bool IsLeaf() const  = 0;
    virtual void Print() const = 0;

    virtual void Compute() = 0;

    std::string GetName() const { return name; }
    virtual void SetPrecedence(size_t p) { precedence = p; }

protected:

    std::string name;
    size_t precedence{0};
    NodeType type{NodeType::Sound};
};

using AbstractNodePtr = std::shared_ptr<AbstractNode>;

class Node : public AbstractNode
{
public:
    Node(const std::string& name, NodeType type) : AbstractNode(name, type) {}
    

    void AddChild(AbstractNodePtr node)
    {
        childs.emplace_back(node);
    }

    virtual bool IsLeaf() const final { return false; }

    const std::vector<AbstractNodePtr>& GetChilds() const { return childs; }
    AbstractNodePtr GetChild(size_t i) const { return childs[i]; }
    auto GetNumChilds() const { return childs.size(); }


    virtual void Compute() override {}
    virtual void Print() const override
    {
        std::cout << std::string(precedence, ' ') << name << ": " << std::endl;
        for(const auto& child : childs)
        {
            child->Print();
        }
    }

private:

    std::vector<AbstractNodePtr> childs;

};

class Input : public AbstractNode
{
public:
    Input(const std::string& name, NodeType type) : AbstractNode(name, type) {}
    

    virtual bool IsLeaf() const final { return true; }
    virtual void Compute() override {}
    virtual void Print() const override
    {
        std::cout << std::string(precedence, ' ') << name << std::endl;
    }
};


class SoundNode : public Node
{
public:
    SoundNode(const std::string& name, const Util::XmlElement& element) : Node(name, NodeType::Sound) 
    {
        fileLocation = std::string{element.GetText()};
        soundType = element.Attribute<std::string>("type");
    }

private:

    std::string fileLocation;
    std::string soundType;

};

using TriggerNode = Input;
using AmplitudeModulator = Node;
using Output = Node;

AbstractNodePtr makeNode(const std::string& name, const std::string& type, const Util::XmlElement& element)
{
    if(type == "Sound")
    {
        return std::make_shared<SoundNode>(name, element);
    }

    if(type == "Trigger")
    {
        return std::make_shared<TriggerNode>(name, NodeType::Trigger);
    }

    if(type == "AmplitudeModulator")
    {
        return std::make_shared<AmplitudeModulator>(name, NodeType::AmplitudeModulator);
    }

    if(type == "Output")
    {
        return std::make_shared<Output>(name, NodeType::Output);
    }
    throw "Error";
}


void interpret(const Util::XmlElement& e, size_t& i, AbstractNodePtr node)
{

    Node* currentNode = dynamic_cast<Node*>(node.get());
    const auto name = e.TagName();

    if(!e.HasChildren())
    {
        const auto type = e.Attribute<std::string>("class");
        std::cout << "Leaf: " << std::string(i, ' ') << " " << type << std::endl;

        currentNode->AddChild(makeNode(type, type, e));
        currentNode->GetChilds().back()->SetPrecedence(i);
        return;
    }

    if(i != 0)
    {
        const auto type = e.Attribute<std::string>("class");
        std::cout << "Node: " << std::string(i, ' ') << " " << type << std::endl;

        currentNode->AddChild(makeNode(type, type, e));
        currentNode->GetChilds().back()->SetPrecedence(i);
    }


    i++;
    for(const auto& s : Util::XmlElement{e})
    {

        if(!node->IsLeaf() && i != 1)
        {            
            node = currentNode->GetChilds().back();
        }

        interpret(s, i, node);
    }
    i--;
}

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
        size_t i = 0;

        auto rootNode = makeNode("root", "Output", Util::XmlElement{nullptr});

        interpret(instrument, i, rootNode);

        auto node = dynamic_cast<Node*>(rootNode.get());

        node->Print();

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