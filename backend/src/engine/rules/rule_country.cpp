#include "rule_country.h"
#include <unordered_set>
// [OTEL] CHILD SPAN: "rule.country_risk"
// SetAttribute("transaction.country", country)
// SetAttribute("rule.triggered", score > 0)
namespace Rules {
double Country(const std::string& country) {
    // Países de alto riesgo para demo
    static const std::unordered_set<std::string> HIGH_RISK = {
        "NG", "KP", "IR", "RU", "BY", "MM", "CU"
    };
    static const std::unordered_set<std::string> MED_RISK = {
        "VE", "SD", "SY", "LB", "IQ"
    };
    if (HIGH_RISK.count(country)) return 25.0;
    if (MED_RISK.count(country))  return 15.0;
    return 0.0;
}
}
