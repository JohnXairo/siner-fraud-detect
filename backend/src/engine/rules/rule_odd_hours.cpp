#include "rule_odd_hours.h"
#include <ctime>
#include <string>
// [OTEL] CHILD SPAN: "rule.odd_hours"
// SetAttribute("transaction.hour", hour)
// SetAttribute("rule.triggered", score > 0)
namespace Rules {
double OddHours(const std::string& timestamp) {
    // timestamp formato ISO: "2024-11-10T03:22:00Z"
    // Extraer hora (posición 11-12)
    int hour = 12; // default
    if (timestamp.size() >= 13) {
        try { hour = std::stoi(timestamp.substr(11, 2)); }
        catch (...) {}
    }
    // Horas de madrugada (0-5) son sospechosas
    if (hour >= 0 && hour <= 5) return 20.0;
    return 0.0;
}
}
