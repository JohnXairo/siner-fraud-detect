cmake_minimum_required(VERSION 3.20)
project(fraud-engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/routes/*.cpp"
    "src/engine/*.cpp"
    "src/engine/rules/*.cpp"
)

add_executable(fraud-engine ${SOURCES})

target_include_directories(fraud-engine PRIVATE src third_party)

target_link_libraries(fraud-engine PRIVATE
    unofficial::sqlite3::sqlite3
    nlohmann_json::nlohmann_json
)

if(WIN32)
    target_link_libraries(fraud-engine PRIVATE ws2_32 advapi32)
endif()

install(TARGETS fraud-engine RUNTIME DESTINATION .)

# [OTEL] Descomentar cuando agregues OpenTelemetry (ver docs/otel-guide.md):
# find_package(opentelemetry-cpp CONFIG REQUIRED)
# target_link_libraries(fraud-engine PRIVATE
#     opentelemetry-cpp::api
#     opentelemetry-cpp::sdk
#     opentelemetry-cpp::otlp_grpc_exporter
# )
