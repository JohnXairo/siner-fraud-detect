using System;
using System.Collections.Generic;

namespace FraudDashboard.Models {

    public class Transaction {
        public string Id          { get; set; }
        public string AccountId   { get; set; }
        public double Amount      { get; set; }
        public string Country     { get; set; }
        public string Currency    { get; set; }
        public string Timestamp   { get; set; }
        public double RiskScore   { get; set; }
        public string Status      { get; set; }

        public string StatusClass => Status switch {
            "approved" => "status-approved",
            "flagged"  => "status-flagged",
            "blocked"  => "status-blocked",
            _          => ""
        };
        public string StatusLabel => Status switch {
            "approved" => "✓ Aprobada",
            "flagged"  => "⚠ Revisión",
            "blocked"  => "✗ Bloqueada",
            _          => Status
        };
    }

    public class Alert {
        public string Id            { get; set; }
        public string TransactionId { get; set; }
        public string RuleTriggered { get; set; }
        public string Description   { get; set; }
        public string Timestamp     { get; set; }
        public double RiskScore     { get; set; }
    }

    public class Stats {
        public int    TotalTransactions { get; set; }
        public int    Approved          { get; set; }
        public int    Flagged           { get; set; }
        public int    Blocked           { get; set; }
        public double AvgRiskScore      { get; set; }
        public double TotalAmount       { get; set; }
    }

    public class DashboardViewModel {
        public Stats                  Stats        { get; set; }
        public List<Transaction>      Transactions { get; set; }
        public List<Alert>            Alerts       { get; set; }
    }
}
