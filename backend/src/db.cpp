#include "db.h"
#include <sqlite3.h>
#include <stdexcept>
#include <iostream>
#include <sstream>

// [OTEL] SPAN: añadir spans de DB con atributo db.statement en cada función

static sqlite3* g_db = nullptr;

static void Exec(const char* sql) {
    char* err = nullptr;
    if (sqlite3_exec(g_db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error("DB error: " + msg);
    }
}

namespace DB {

void Init() {
    if (sqlite3_open("fraud.db", &g_db) != SQLITE_OK)
        throw std::runtime_error("No se pudo abrir fraud.db");

    Exec(R"sql(
        CREATE TABLE IF NOT EXISTS transactions (
            id           TEXT PRIMARY KEY,
            account_id   TEXT NOT NULL,
            amount       REAL NOT NULL,
            country      TEXT NOT NULL,
            currency     TEXT NOT NULL DEFAULT 'USD',
            timestamp    TEXT NOT NULL,
            risk_score   REAL NOT NULL DEFAULT 0.0,
            status       TEXT NOT NULL DEFAULT 'approved'
        );
    )sql");

    Exec(R"sql(
        CREATE TABLE IF NOT EXISTS alerts (
            id              TEXT PRIMARY KEY,
            transaction_id  TEXT NOT NULL,
            rule_triggered  TEXT NOT NULL,
            description     TEXT NOT NULL,
            timestamp       TEXT NOT NULL,
            risk_score      REAL NOT NULL,
            FOREIGN KEY(transaction_id) REFERENCES transactions(id)
        );
    )sql");

    std::cout << "[DB] Inicializada: fraud.db\n";
}

void InsertTransaction(const Transaction& t) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO transactions VALUES (?,?,?,?,?,?,?,?);";
    sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt,  1, t.id.c_str(),         -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,  2, t.account_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt,3, t.amount);
    sqlite3_bind_text(stmt,  4, t.country.c_str(),    -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,  5, t.currency.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,  6, t.timestamp.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt,7, t.risk_score);
    sqlite3_bind_text(stmt,  8, t.status.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void InsertAlert(const Alert& a) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO alerts VALUES (?,?,?,?,?,?);";
    sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt,   1, a.id.c_str(),             -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   2, a.transaction_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   3, a.rule_triggered.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   4, a.description.c_str(),    -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   5, a.timestamp.c_str(),      -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 6, a.risk_score);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<Transaction> GetTransactions(int limit) {
    std::vector<Transaction> result;
    std::string sql = "SELECT * FROM transactions ORDER BY timestamp DESC LIMIT " + std::to_string(limit) + ";";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return result;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Transaction t;
        t.id         = (const char*)sqlite3_column_text(stmt, 0);
        t.account_id = (const char*)sqlite3_column_text(stmt, 1);
        t.amount     = sqlite3_column_double(stmt, 2);
        t.country    = (const char*)sqlite3_column_text(stmt, 3);
        t.currency   = (const char*)sqlite3_column_text(stmt, 4);
        t.timestamp  = (const char*)sqlite3_column_text(stmt, 5);
        t.risk_score = sqlite3_column_double(stmt, 6);
        t.status     = (const char*)sqlite3_column_text(stmt, 7);
        result.push_back(t);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Alert> GetAlerts(int limit) {
    std::vector<Alert> result;
    std::string sql = "SELECT * FROM alerts ORDER BY timestamp DESC LIMIT " + std::to_string(limit) + ";";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return result;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Alert a;
        a.id              = (const char*)sqlite3_column_text(stmt, 0);
        a.transaction_id  = (const char*)sqlite3_column_text(stmt, 1);
        a.rule_triggered  = (const char*)sqlite3_column_text(stmt, 2);
        a.description     = (const char*)sqlite3_column_text(stmt, 3);
        a.timestamp       = (const char*)sqlite3_column_text(stmt, 4);
        a.risk_score      = sqlite3_column_double(stmt, 5);
        result.push_back(a);
    }
    sqlite3_finalize(stmt);
    return result;
}

Stats GetStats() {
    Stats s{};
    auto qi = [&](const char* sql, int& out) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) out = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
        }
    };
    auto qd = [&](const char* sql, double& out) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) out = sqlite3_column_double(stmt, 0);
            sqlite3_finalize(stmt);
        }
    };
    qi("SELECT COUNT(*) FROM transactions;",                         s.total_transactions);
    qi("SELECT COUNT(*) FROM transactions WHERE status='approved';", s.approved);
    qi("SELECT COUNT(*) FROM transactions WHERE status='flagged';",  s.flagged);
    qi("SELECT COUNT(*) FROM transactions WHERE status='blocked';",  s.blocked);
    qd("SELECT COALESCE(AVG(risk_score),0) FROM transactions;",      s.avg_risk_score);
    qd("SELECT COALESCE(SUM(amount),0)     FROM transactions;",      s.total_amount);
    return s;
}

} // namespace DB
