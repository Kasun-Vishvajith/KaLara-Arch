#pragma once
#include "architecture/model.h"
#include <string>
#include <string_view>
#include <variant>
namespace kalara::persistence {
struct CodecFailure { std::vector<Diagnostic> diagnostics; };
using DecodeResult = std::variant<architecture::Project, CodecFailure>;
std::string encodeJson(const architecture::Project& project);
DecodeResult decodeJson(std::string_view json);
}
