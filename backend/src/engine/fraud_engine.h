#pragma once
#include <string>
#include <vector>
#include "../db.h"

namespace Engine {

struct EvalResult {
    double      risk_score;   // 0.0 - 100.0
    std::string status;       // "approved" | "flagged" | "blocked"
    std::vector<std::string> rules_triggered;
};

EvalResult Evaluate(const DB::Transaction& txn,
                    const std::vector<DB::Transaction>& history);

} // namespace Engine
