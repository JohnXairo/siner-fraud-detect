#include "rule_velocity.h"
#include <algorithm>
// [OTEL] CHILD SPAN: "rule.velocity"
// SetAttribute("account.id", account_id)
// SetAttribute("velocity.txn_count_last_hour", count)
// SetAttribute("rule.triggered", score > 0)
namespace Rules {
double Velocity(const std::string& account_id, const std::vector<DB::Transaction>& history) {
    // Contar transacciones del mismo account en el historial reciente (últimas 10)
    int count = (int)std::count_if(history.begin(), history.end(),
        [&](const DB::Transaction& t){ return t.account_id == account_id; });
    if (count >= 5) return 30.0;
    if (count >= 3) return 15.0;
    return 0.0;
}
}
