#include "alerts_handler.h"
#include "../db.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
// [OTEL] TRACE: span "list_alerts"

void RegisterAlertsRoutes(httplib::Server& svr) {
    svr.Get("/api/alerts", [](const httplib::Request&, httplib::Response& res) {
        auto alerts = DB::GetAlerts(50);
        json arr = json::array();
        for (const auto& a : alerts) {
            arr.push_back({
                {"id",             a.id},
                {"transaction_id", a.transaction_id},
                {"rule_triggered", a.rule_triggered},
                {"description",    a.description},
                {"timestamp",      a.timestamp},
                {"risk_score",     a.risk_score}
            });
        }
        res.set_content(arr.dump(), "application/json");
    });
}
