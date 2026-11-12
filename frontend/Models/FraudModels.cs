using System.Collections.Generic;

namespace FraudDashboard.Models {

    public class Transaction {
        public string Id         { get; set; }
        public string AccountId  { get; set; }
        public double Amount     { get; set; }
        public string Country    { get; set; }
        public string Currency   { get; set; }
        public string Timestamp  { get; set; }
        public double RiskScore  { get; set; }
        public string Status     { get; set; }

        public string StatusClass {
            get {
                if (Status == "approved") return "status-approved";
                if (Status == "flagged")  return "status-flagged";
                if (Status == "blocked")  return "status-blocked";
                return "";
            }
        }

        public string StatusLabel {
            get {
                if (Status == "approved") return "✓ Aprobada";
                if (Status == "flagged")  return "⚠ Revisión";
                if (Status == "blocked")  return "✗ Bloqueada";
                return Status;
            }
        }
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
        public Stats             Stats        { get; set; }
        public List<Transaction> Transactions { get; set; }
        public List<Alert>       Alerts       { get; set; }
    }
}
