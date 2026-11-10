# siner-fraud-detect

Motor de detección de fraude bancario en tiempo real — Demo de observabilidad con OpenTelemetry y SigNoz.

## Stack

| Capa | Tecnología |
|---|---|
| Backend | C++17, `cpp-httplib`, SQLite3, Windows Service (puerto 8081) |
| Frontend | ASP.NET MVC 5 + Razor, IIS, .NET Framework 4.x |
| Observabilidad | OpenTelemetry C++ SDK → OTel Collector → SigNoz |

## Estructura

```
siner-fraud-detect/
├── backend/              # C++17 — fraud-engine.exe (Windows Service)
│   ├── src/
│   │   ├── main.cpp
│   │   ├── service.cpp/.h
│   │   ├── db.cpp/.h
│   │   ├── routes/
│   │   └── engine/rules/
│   ├── third_party/      # httplib.h, sqlite3, nlohmann/json
│   ├── CMakeLists.txt
│   └── vcpkg.json
├── frontend/             # ASP.NET MVC 5 + Razor
│   ├── Controllers/
│   ├── Models/
│   ├── Views/
│   └── FraudDashboard.csproj
├── docs/
│   └── otel-guide.md
└── scripts/
    ├── install-service.bat
    └── uninstall-service.bat
```

## Requisitos (Windows Server 2019)

### Backend C++
- Visual Studio 2022 Build Tools con workload **Desktop development with C++**
- CMake >= 3.20
- vcpkg (`C:\vcpkg`)

### Frontend ASP.NET
- .NET Framework 4.8 (ya instalado)
- Visual Studio 2022 o MSBuild
- IIS habilitado

## Compilar el backend

```bat
cd backend
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build --config Release
```

El ejecutable queda en `backend\build\Release\fraud-engine.exe`.

## Modo consola (desarrollo)

```bat
fraud-engine.exe --console
```

## Instalar como Windows Service

```bat
scripts\install-service.bat
```

## API del backend (puerto 8081)

| Método | Endpoint | Descripción |
|---|---|---|
| `POST` | `/api/transaction` | Enviar transacción para análisis |
| `GET` | `/api/transactions` | Historial de transacciones (últimas 50) |
| `GET` | `/api/alerts` | Alertas de fraude detectadas |
| `GET` | `/api/stats` | Estadísticas globales |
| `GET` | `/health` | Health check |

## Instrumentación OpenTelemetry

Ver [docs/otel-guide.md](docs/otel-guide.md) para la guía paso a paso.
