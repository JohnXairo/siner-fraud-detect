#include "fraud_engine.h"
#include "rules/rule_high_amount.h"
#include "rules/rule_velocity.h"
#include "rules/rule_odd_hours.h"
#include "rules/rule_country.h"
#include <iostream>

// [OTEL] TRACE: Evaluate() crea el span raíz "engine.evaluate"
// Cada regla crea un child span — ver docs/otel-guide.md sección 5

namespace Engine {

EvalResult Evaluate(const DB::Transaction& txn,
                    const std::vector<DB::Transaction>& history) {
    EvalResult result;
    result.risk_score = 0.0;

    // Ejecutar reglas — cada una devuelve un score parcial (0-25)
    // [OTEL] CHILD SPAN: rule_high_amount
    double score_amount = Rules::HighAmount(txn.amount);

    // [OTEL] CHILD SPAN: rule_velocity
    double score_velocity = Rules::Velocity(txn.account_id, history);

    // [OTEL] CHILD SPAN: rule_odd_hours
    double score_hours = Rules::OddHours(txn.timestamp);

    // [OTEL] CHILD SPAN: rule_country
    double score_country = Rules::Country(txn.country);

    result.risk_score = score_amount + score_velocity + score_hours + score_country;

    if (score_amount  > 0) result.rules_triggered.push_back("high_amount");
    if (score_velocity> 0) result.rules_triggered.push_back("velocity");
    if (score_hours   > 0) result.rules_triggered.push_back("odd_hours");
    if (score_country > 0) result.rules_triggered.push_back("country_risk");

    // Umbral: <30 aprobada, 30-69 revisión, >=70 bloqueada
    if      (result.risk_score >= 70.0) result.status = "blocked";
    else if (result.risk_score >= 30.0) result.status = "flagged";
    else                                result.status = "approved";

    std::cout << "[engine] txn=" << txn.id
              << " score=" << result.risk_score
              << " status=" << result.status << "\n";

    return result;
}

} // namespace Engine
