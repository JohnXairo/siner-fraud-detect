// WIN32_LEAN_AND_MEAN evita que <windows.h> incluya <winsock.h> v1,
// que entra en conflicto con <winsock2.h> que usa httplib.h.
// Debe definirse ANTES de cualquier include de Windows.
#define WIN32_LEAN_AND_MEAN

// httplib.h debe incluirse ANTES de <windows.h> para que maneje
// correctamente el orden de winsock2.
#include "httplib.h"

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <windows.h>

#include "db.h"
#include "routes/transaction_handler.h"
#include "routes/alerts_handler.h"
#include "routes/stats_handler.h"

// [OTEL] INIT: #include "otel_init.h" y llamar InitOtel() en ServiceMain()

static const int    PORT         = 8081;
static const char*  SERVICE_NAME = "FraudEngine";

std::atomic<bool>     g_running{true};
SERVICE_STATUS        g_status{};
SERVICE_STATUS_HANDLE g_status_handle{};

void StartServer() {
    DB::Init();

    httplib::Server svr;

    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin",  "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        if (req.method == "OPTIONS") {
            res.status = 204;
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(
            R"({"status":"ok","service":"fraud-engine","version":"1.0.0"})",
            "application/json");
    });

    // [OTEL] TRACE: cada handler registra un span raíz con nombre de la operación
    RegisterTransactionRoutes(svr);
    RegisterAlertsRoutes(svr);
    RegisterStatsRoutes(svr);

    std::cout << "[fraud-engine] Escuchando en http://0.0.0.0:" << PORT << "\n";
    svr.listen("0.0.0.0", PORT);
}

void ReportStatus(DWORD state, DWORD exit_code = NO_ERROR, DWORD wait_hint = 0) {
    g_status.dwCurrentState  = state;
    g_status.dwWin32ExitCode = exit_code;
    g_status.dwWaitHint      = wait_hint;
    SetServiceStatus(g_status_handle, &g_status);
}

void WINAPI ServiceCtrlHandler(DWORD ctrl) {
    if (ctrl == SERVICE_CONTROL_STOP || ctrl == SERVICE_CONTROL_SHUTDOWN) {
        ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);
        g_running = false;
    }
}

void WINAPI ServiceMain(DWORD, LPSTR*) {
    g_status_handle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    g_status.dwServiceType      = SERVICE_WIN32_OWN_PROCESS;
    g_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ReportStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // [OTEL] INIT: llamar InitOtel("localhost:4317", "fraud-engine") aquí

    ReportStatus(SERVICE_RUNNING);

    std::thread t(StartServer);
    t.detach();

    while (g_running)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ReportStatus(SERVICE_STOPPED);
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--console") {
        std::cout << "[fraud-engine] Modo consola (desarrollo)\n";
        StartServer();
        return 0;
    }
    SERVICE_TABLE_ENTRY table[] = {
        { const_cast<LPSTR>(SERVICE_NAME), ServiceMain },
        { nullptr, nullptr }
    };
    StartServiceCtrlDispatcher(table);
    return 0;
}
