#include <Geode/Geode.hpp>
#include <Geode/modify/CCLabelBMFont.hpp>
#include <Geode/loader/Event.hpp>
#include <matjson.hpp>
#include <fstream>
#include <iostream>

using namespace geode::prelude;

// Custom class to handle translations
class $modify(MyCCLabelBMFont, CCLabelBMFont) {
public:
    // Override the create method to include translation
    static CCLabelBMFont* create(const char* str, const char* fntFile) {
        // Load and parse the JSON file for translations
        auto resourcesDir = Mod::get()->getResourcesDir();

        std::string lang;

        if (Mod::get()->hasSavedValue("lang")) {
            lang = Mod::get()->getSavedValue<std::string>("lang");
        } else {
            lang = "en";
            Mod::get()->setSavedValue<std::string>("lang", lang);
        }

        if (lang == "en") {
            return CCLabelBMFont::create(str, fntFile);
        } else {
            std::string jsonFile = Mod::get()->getSavedValue<std::string>("lang") + ".json";
            std::string jsonFilePath = (resourcesDir / jsonFile).string();

            // Read the JSON file into a string
            std::ifstream file(jsonFilePath);
            if (!file) {
                std::cerr << "Failed to open file: " << jsonFilePath << std::endl;
                return CCLabelBMFont::create(str, fntFile); // Return original if file fails
            }
            std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            // Parse the JSON file
            matjson::Value jsonObject;
            try {
                jsonObject = matjson::parse(fileContents);
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
                return CCLabelBMFont::create(str, fntFile); // Return original if parsing fails
            }

            // Translate the input string
            std::string translatedStr = translateString(jsonObject, str);

            // Call the original create method with the translated string
            return CCLabelBMFont::create(translatedStr.c_str(), fntFile);
        }
    }

private:
    static std::string translateString(const matjson::Value& jsonObject, const char* originalStr) {
        // Check if the JSON object contains the "phrases" key
        if (jsonObject.is_object() && jsonObject.contains("phrases")) {
            auto& phrases = jsonObject["phrases"];
            if (phrases.is_object()) {
                // Use the key to get the translation
                auto it = phrases.as_object().find(originalStr);
                if (it != phrases.as_object().end() && it->second.is_string()) {
                    return it->second.as_string();
                }
            }
        }
        // If JSON is not an object or key not found, return the original string
        return originalStr;
    }
};