#include "rule_high_amount.h"
// [OTEL] CHILD SPAN: "rule.high_amount"
// SetAttribute("rule.threshold", 10000.0)
// SetAttribute("transaction.amount", amount)
// SetAttribute("rule.triggered", score > 0)
namespace Rules {
double HighAmount(double amount) {
    if (amount >= 50000.0) return 40.0;
    if (amount >= 10000.0) return 25.0;
    if (amount >=  5000.0) return 10.0;
    return 0.0;
}
}
