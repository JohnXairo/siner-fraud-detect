using System;
using System.Collections.Generic;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Threading.Tasks;
using System.Web.Configuration;
using System.Web.Mvc;
using FraudDashboard.Models;
using Newtonsoft.Json;

namespace FraudDashboard.Controllers {

    public class DashboardController : Controller {

        private static readonly string BackendUrl =
            WebConfigurationManager.AppSettings["FraudEngineUrl"] ?? "http://localhost:8081";

        private static readonly HttpClient Http = new HttpClient {
            Timeout = TimeSpan.FromSeconds(10)
        };

        static DashboardController() {
            Http.DefaultRequestHeaders.Accept.Add(
                new MediaTypeWithQualityHeaderValue("application/json"));
        }

        // GET /
        public async Task<ActionResult> Index() {
            var vm = new DashboardViewModel {
                Stats        = await GetStats(),
                Transactions = await GetTransactions(),
                Alerts       = await GetAlerts()
            };
            return View(vm);
        }

        // POST /Dashboard/Submit
        [HttpPost]
        public async Task<ActionResult> Submit(string account_id, double amount,
                                               string country, string currency) {
            var payload = new {
                account_id = account_id,
                amount     = amount,
                country    = country  ?? "US",
                currency   = currency ?? "USD"
            };
            var content = new StringContent(
                JsonConvert.SerializeObject(payload),
                System.Text.Encoding.UTF8, "application/json");

            try {
                var resp = await Http.PostAsync(BackendUrl + "/api/transaction", content);
                resp.EnsureSuccessStatusCode();
            } catch (Exception ex) {
                TempData["Error"] = "Error al enviar transacción: " + ex.Message;
            }

            return RedirectToAction("Index");
        }

        private async Task<Stats> GetStats() {
            try {
                var json = await Http.GetStringAsync(BackendUrl + "/api/stats");
                var raw  = JsonConvert.DeserializeAnonymousType(json, new {
                    total_transactions = 0, approved = 0, flagged = 0, blocked = 0,
                    avg_risk_score = 0.0, total_amount = 0.0
                });
                return new Stats {
                    TotalTransactions = raw.total_transactions,
                    Approved          = raw.approved,
                    Flagged           = raw.flagged,
                    Blocked           = raw.blocked,
                    AvgRiskScore      = raw.avg_risk_score,
                    TotalAmount       = raw.total_amount
                };
            } catch { return new Stats(); }
        }

        private async Task<List<Transaction>> GetTransactions() {
            try {
                var json = await Http.GetStringAsync(BackendUrl + "/api/transactions");
                var raw  = JsonConvert.DeserializeAnonymousType(json,
                    new[] { new { id = "", account_id = "", amount = 0.0, country = "",
                                  currency = "", timestamp = "", risk_score = 0.0, status = "" } });
                var list = new List<Transaction>();
                foreach (var r in raw)
                    list.Add(new Transaction {
                        Id        = r.id,
                        AccountId = r.account_id,
                        Amount    = r.amount,
                        Country   = r.country,
                        Currency  = r.currency,
                        Timestamp = r.timestamp,
                        RiskScore = r.risk_score,
                        Status    = r.status
                    });
                return list;
            } catch { return new List<Transaction>(); }
        }

        private async Task<List<Alert>> GetAlerts() {
            try {
                var json = await Http.GetStringAsync(BackendUrl + "/api/alerts");
                var raw  = JsonConvert.DeserializeAnonymousType(json,
                    new[] { new { id = "", transaction_id = "", rule_triggered = "",
                                  description = "", timestamp = "", risk_score = 0.0 } });
                var list = new List<Alert>();
                foreach (var r in raw)
                    list.Add(new Alert {
                        Id            = r.id,
                        TransactionId = r.transaction_id,
                        RuleTriggered = r.rule_triggered,
                        Description   = r.description,
                        Timestamp     = r.timestamp,
                        RiskScore     = r.risk_score
                    });
                return list;
            } catch { return new List<Alert>(); }
        }
    }
}
