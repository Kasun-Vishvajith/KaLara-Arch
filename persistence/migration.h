#pragma once
#include "persistence/json_codec.h"
namespace kalara::persistence {
// Migration is deliberately explicit. Version 1 is the first editable format;
// unsupported versions return diagnostics without mutating the source document.
DecodeResult migrateAndDecode(std::string_view source);
}
