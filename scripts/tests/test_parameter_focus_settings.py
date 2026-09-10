"""Generate a focused JSON round-trip test from the production setting paths."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[2]
settings = (root / 'src/Magpie/AppSettings.cpp').read_text(encoding='utf-8-sig')
header = (root / 'src/Magpie/AppSettings.h').read_text(encoding='utf-8-sig')
helper = (root / 'src/Magpie/JsonHelper.cpp').read_text(encoding='utf-8-sig')
reader = helper[helper.index('bool JsonHelper::ReadBool('):helper.index('bool JsonHelper::ReadBoolFlag(')]
field = re.search(r'bool _isParameterFocusSwitchingEnabled = [^;]+;', header).group()
load = re.search(r'_isParameterFocusSwitchingEnabled = false;\s+JsonHelper::ReadBool\(root, "parameterFocusSwitching", _isParameterFocusSwitchingEnabled\);', settings).group()
save = re.search(r'writer.Key\("parameterFocusSwitching"\);\s+writer.Bool\(data._isParameterFocusSwitchingEnabled\);', settings).group()

harness = r'''
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <cassert>
#include <iostream>
#include <string>
struct JsonHelper {
    static bool ReadBool(const rapidjson::GenericObject<true, rapidjson::Value>&,
        const char*, bool&, bool required=false) noexcept;
};
READER
struct Settings {
    FIELD
    void Load(const char* text) {
        const rapidjson::Document document = [] (const char* text) {
            rapidjson::Document result; result.Parse(text); return result;
        }(text);
        assert(!document.HasParseError()); const auto root=document.GetObject();
        LOAD
    }
    std::string Save() const {
        const auto& data=*this;
        rapidjson::StringBuffer buffer; rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        writer.StartObject(); SAVE writer.EndObject();
        return buffer.GetString();
    }
};
int main() {
    Settings data; assert(!data._isParameterFocusSwitchingEnabled);
    for (const char* input : {"{}", R"({"frontEdgeSync":true})", R"({"parameterFocusSwitching":false})",
            R"({"parameterFocusSwitching":null})", R"({"parameterFocusSwitching":1})",
            R"({"parameterFocusSwitching":"true"})"}) {
        data._isParameterFocusSwitchingEnabled=true; data.Load(input);
        assert(!data._isParameterFocusSwitchingEnabled);
    }
    for (bool enabled : {true,false}) {
        data.Load(enabled ? R"({"parameterFocusSwitching":true})" : R"({"parameterFocusSwitching":false})");
        assert(data._isParameterFocusSwitchingEnabled==enabled);
        Settings reloaded; reloaded.Load(data.Save().c_str());
        assert(reloaded._isParameterFocusSwitchingEnabled==enabled);
    }
    std::cout << "PASS parameter focus setting: default/old/missing/invalid disabled, explicit opt-in and true/false JSON round-trip\n";
}
'''
output = Path(sys.argv[1]) / 'parameter_focus_settings.cpp'
output.write_text(harness.replace('READER', reader).replace('FIELD', field).replace('LOAD', load).replace('SAVE', save), encoding='utf-8')
print(output)
