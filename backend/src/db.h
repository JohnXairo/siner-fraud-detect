#pragma once
#include <string>
#include <vector>

namespace DB {

struct Transaction {
    std::string id;
    std::string account_id;
    double      amount;
    std::string country;
    std::string currency;
    std::string timestamp;
    double      risk_score;
    std::string status;   // "approved" | "flagged" | "blocked"
};

struct Alert {
    std::string id;
    std::string transaction_id;
    std::string rule_triggered;
    std::string description;
    std::string timestamp;
    double      risk_score;
};

struct Stats {
    int    total_transactions;
    int    approved;
    int    flagged;
    int    blocked;
    double avg_risk_score;
    double total_amount;
};

void Init();
void InsertTransaction(const Transaction& t);
void InsertAlert(const Alert& a);
std::vector<Transaction> GetTransactions(int limit = 50);
std::vector<Alert>       GetAlerts(int limit = 50);
Stats                    GetStats();

} // namespace DB
