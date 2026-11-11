#pragma once
#define WIN32_LEAN_AND_MEAN
#include "httplib.h"
void RegisterTransactionRoutes(httplib::Server& svr);
