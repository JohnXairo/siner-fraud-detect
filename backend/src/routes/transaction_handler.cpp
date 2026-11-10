#include "transaction_handler.h"
#include "../db.h"
#include "../engine/fraud_engine.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>

using json = nlohmann::json;

// [OTEL] TRACE: CreateRootSpan("http.post /api/transaction") aquí
// SetAttribute("http.method", "POST")
// SetAttribute("http.route",  "/api/transaction")

static std::string GenerateId() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);
    std::ostringstream ss;
    ss << std::hex << dist(rng) << dist(rng);
    return ss.str();
}

static std::string NowISO() {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

void RegisterTransactionRoutes(httplib::Server& svr) {

    // POST /api/transaction — recibir y evaluar una transacción
    svr.Post("/api/transaction", [](const httplib::Request& req, httplib::Response& res) {
        // [OTEL] TRACE: span raíz "evaluate_transaction"
        try {
            auto body = json::parse(req.body);

            DB::Transaction txn;
            txn.id         = GenerateId();
            txn.account_id = body.value("account_id", "UNKNOWN");
            txn.amount     = body.value("amount",     0.0);
            txn.country    = body.value("country",    "US");
            txn.currency   = body.value("currency",   "USD");
            txn.timestamp  = body.value("timestamp",  NowISO());

            // [OTEL] SetAttribute("transaction.id",      txn.id)
            // [OTEL] SetAttribute("transaction.amount",  txn.amount)
            // [OTEL] SetAttribute("transaction.country", txn.country)

            auto history = DB::GetTransactions(20);
            auto result  = Engine::Evaluate(txn, history);

            txn.risk_score = result.risk_score;
            txn.status     = result.status;

            DB::InsertTransaction(txn);

            // Generar alertas para transacciones sospechosas
            if (txn.status != "approved") {
                for (const auto& rule : result.rules_triggered) {
                    DB::Alert alert;
                    alert.id              = GenerateId();
                    alert.transaction_id  = txn.id;
                    alert.rule_triggered  = rule;
                    alert.description     = "Regla disparada: " + rule
                                          + " | Cuenta: " + txn.account_id
                                          + " | Monto: $" + std::to_string((int)txn.amount);
                    alert.timestamp       = txn.timestamp;
                    alert.risk_score      = txn.risk_score;
                    // [OTEL] LOG warn: "Alerta generada" con transaction.id, rule, risk_score
                    DB::InsertAlert(alert);
                }
            }

            json resp;
            resp["id"]              = txn.id;
            resp["status"]          = txn.status;
            resp["risk_score"]      = txn.risk_score;
            resp["rules_triggered"] = result.rules_triggered;
            resp["timestamp"]       = txn.timestamp;

            // [OTEL] SetAttribute("fraud.status",     txn.status)
            // [OTEL] SetAttribute("fraud.risk_score", txn.risk_score)

            res.set_content(resp.dump(), "application/json");

        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    // GET /api/transactions — historial
    svr.Get("/api/transactions", [](const httplib::Request&, httplib::Response& res) {
        // [OTEL] TRACE: span "list_transactions"
        auto txns = DB::GetTransactions(50);
        json arr = json::array();
        for (const auto& t : txns) {
            arr.push_back({
                {"id",         t.id},
                {"account_id", t.account_id},
                {"amount",     t.amount},
                {"country",    t.country},
                {"currency",   t.currency},
                {"timestamp",  t.timestamp},
                {"risk_score", t.risk_score},
                {"status",     t.status}
            });
        }
        res.set_content(arr.dump(), "application/json");
    });
}
