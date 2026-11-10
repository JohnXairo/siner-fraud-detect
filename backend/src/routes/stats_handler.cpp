#include "stats_handler.h"
#include "../db.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
// [OTEL] TRACE: span "get_stats"
// SetAttribute("stats.total_transactions", s.total_transactions)
// SetAttribute("stats.blocked", s.blocked)

void RegisterStatsRoutes(httplib::Server& svr) {
    svr.Get("/api/stats", [](const httplib::Request&, httplib::Response& res) {
        auto s = DB::GetStats();
        json resp = {
            {"total_transactions", s.total_transactions},
            {"approved",           s.approved},
            {"flagged",            s.flagged},
            {"blocked",            s.blocked},
            {"avg_risk_score",     s.avg_risk_score},
            {"total_amount",       s.total_amount}
        };
        res.set_content(resp.dump(), "application/json");
    });
}
