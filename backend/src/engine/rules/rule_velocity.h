#pragma once
#include <string>
#include <vector>
#include "../../../src/db.h"
namespace Rules { double Velocity(const std::string& account_id, const std::vector<DB::Transaction>& history); }
