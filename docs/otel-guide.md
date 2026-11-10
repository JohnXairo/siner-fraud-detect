# Guía de Instrumentación OpenTelemetry — Backend C++

Esta guía te lleva paso a paso para agregar trazas, métricas y logs
al `fraud-engine.exe` usando el SDK oficial de OpenTelemetry para C++.

## 1. Agregar dependencias OTel al proyecto

En `backend/vcpkg.json` agregar la dependencia:

```json
{
  "name": "fraud-engine",
  "version": "1.0.0",
  "dependencies": [
    "sqlite3",
    "nlohmann-json",
    "opentelemetry-cpp"
  ]
}
```

En `backend/CMakeLists.txt` descomentar la sección `[OTEL]`:

```cmake
find_package(opentelemetry-cpp CONFIG REQUIRED)
target_link_libraries(fraud-engine PRIVATE
    opentelemetry-cpp::api
    opentelemetry-cpp::sdk
    opentelemetry-cpp::otlp_grpc_exporter
)
```

## 2. Crear otel_init.h / otel_init.cpp

Crear `backend/src/otel_init.h`:

```cpp
#pragma once
void InitOtel(const std::string& endpoint = "localhost:4317",
              const std::string& service_name = "fraud-engine");
```

Crear `backend/src/otel_init.cpp`:

```cpp
#include "otel_init.h"
#include <opentelemetry/exporters/otlp/otlp_grpc_exporter_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/sdk/resource/resource.h>

void InitOtel(const std::string& endpoint, const std::string& service_name) {
    namespace otlp  = opentelemetry::exporter::otlp;
    namespace sdk   = opentelemetry::sdk::trace;
    namespace trace = opentelemetry::trace;

    otlp::OtlpGrpcExporterOptions opts;
    opts.endpoint = endpoint;

    auto exporter  = otlp::OtlpGrpcExporterFactory::Create(opts);
    auto processor = sdk::SimpleSpanProcessorFactory::Create(std::move(exporter));

    auto resource = opentelemetry::sdk::resource::Resource::Create({
        {"service.name",        service_name},
        {"service.version",     "1.0.0"},
        {"deployment.environment", "demo"}
    });

    auto provider = sdk::TracerProviderFactory::Create(std::move(processor), resource);
    trace::Provider::SetTracerProvider(std::move(provider));
}
```

## 3. Llamar InitOtel en main.cpp

En `src/main.cpp`, dentro de `ServiceMain()` busca el comentario
`// [OTEL] INIT` y agrega:

```cpp
#include "otel_init.h"
// ...
InitOtel("localhost:4317", "fraud-engine");
```

## 4. Instrumentar POST /api/transaction

En `src/routes/transaction_handler.cpp`, busca `// [OTEL] TRACE: evaluate_transaction`:

```cpp
auto tracer = opentelemetry::trace::Provider::GetTracerProvider()
                ->GetTracer("fraud-engine", "1.0.0");

auto span = tracer->StartSpan("evaluate_transaction");
span->SetAttribute("transaction.id",      txn.id);
span->SetAttribute("transaction.amount",  txn.amount);
span->SetAttribute("transaction.country", txn.country);
span->SetAttribute("account.id",          txn.account_id);
opentelemetry::trace::Scope scope(span);
```

Al final del handler:

```cpp
span->SetAttribute("fraud.risk_score", result.risk_score);
span->SetAttribute("fraud.status",     result.status);
span->End();
```

## 5. Instrumentar cada regla del motor

Cada regla en `src/engine/rules/` tiene comentario `// [OTEL] CHILD SPAN`.
Ejemplo para `rule_high_amount.cpp`:

```cpp
auto tracer = opentelemetry::trace::Provider::GetTracerProvider()
                ->GetTracer("fraud-engine");
auto span = tracer->StartSpan("rule.high_amount",
    {{{"rule.threshold", 10000.0}, {"transaction.amount", amount}}});
span->SetAttribute("rule.triggered", triggered);
span->End();
```

## 6. Métricas

```cpp
#include <opentelemetry/metrics/provider.h>

auto meter   = opentelemetry::metrics::Provider::GetMeterProvider()
                 ->GetMeter("fraud-engine", "1.0.0");
auto counter = meter->CreateUInt64Counter("fraud.transactions.total",
                 "Total de transacciones procesadas");
counter->Add(1, {{"status", txn.status}});

auto histogram = meter->CreateDoubleHistogram("fraud.risk_score.distribution");
histogram->Record(risk_score, {{"country", txn.country}});
```

## 7. Verificar en SigNoz

1. Abre SigNoz → **Traces** → filtra por `service.name = fraud-engine`
2. Cada transacción aparece como un trace con spans anidados por regla
3. En **Metrics** verás `fraud.transactions.total` y `fraud.risk_score.distribution`
4. En **Logs** los eventos con contexto `trace_id` vinculado
