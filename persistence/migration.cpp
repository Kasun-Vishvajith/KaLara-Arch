#include "persistence/migration.h"
namespace kalara::persistence { DecodeResult migrateAndDecode(std::string_view source){return decodeJson(source);} }
