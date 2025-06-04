#include "../include/Log.h"
#include "../include/http_server.h"
#include "../include/httplib.h"
#include <atomic>
#include <chrono>
#include <iomanip>
#include <limits>
#include <sstream>
#include <thread>
#include <vector>

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

#define XX(...) #__VA_ARGS__

// 性能指标结构体，扩展新指标
struct ServerMetrics {
  std::atomic<int> active_connections{0};
  std::atomic<long> total_requests{0};
  std::atomic<double> avg_response_time_ms{0.0};
  std::atomic<double> max_response_time_ms{0.0};
  std::atomic<double> min_response_time_ms{std::numeric_limits<double>::max()};
  std::atomic<long> timeout_connections{0};
  std::atomic<long> error_connections{0};
  std::atomic<long> response_count{0};

  void record_request(double response_time_ms, bool is_timeout = false,
                      bool is_error = false) {
    total_requests++;
    response_count++;

    // 更新平均时延
    double old_avg = avg_response_time_ms.load();
    long count = response_count.load();
    double new_avg = (old_avg * (count - 1) + response_time_ms) / count;
    avg_response_time_ms.store(new_avg);

    // 更新最大/最小时延
    double current_max = max_response_time_ms.load();
    while (response_time_ms > current_max &&
           !max_response_time_ms.compare_exchange_weak(current_max,
                                                       response_time_ms)) {
    }

    double current_min = min_response_time_ms.load();
    while (response_time_ms < current_min &&
           !min_response_time_ms.compare_exchange_weak(current_min,
                                                       response_time_ms)) {
    }

    // 记录超时和错误
    if (is_timeout)
      timeout_connections++;
    if (is_error)
      error_connections++;

    SYLAR_LOG_DEBUG(g_logger)
        << "Recorded request, time_ms: " << response_time_ms
        << ", avg: " << new_avg << ", max: " << max_response_time_ms
        << ", min: " << min_response_time_ms;
  }

  std::string to_json() {
    std::stringstream ss;
    ss << "{";
    ss << "\"active_connections\":" << active_connections << ",";
    ss << "\"total_requests\":" << total_requests << ",";
    ss << "\"avg_response_time_ms\":" << std::fixed << std::setprecision(2)
       << avg_response_time_ms << ",";
    ss << "\"max_response_time_ms\":" << std::fixed << std::setprecision(2)
       << max_response_time_ms << ",";
    ss << "\"min_response_time_ms\":" << std::fixed << std::setprecision(2)
       << min_response_time_ms << ",";
    ss << "\"timeout_connections\":" << timeout_connections << ",";
    ss << "\"error_connections\":" << error_connections;
    ss << "}";
    return ss.str();
  }
};

// Web服务器，展示美化后的监控页面
void start_web_server(ServerMetrics &metrics) {
  httplib::Server svr;

  // 提供监控页面
  svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
    std::string html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HTTP Server Metrics</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://cdn.tailwindcss.com"></script>
    <style>
        body { background-color: #f3f4f6; }
        .card { transition: transform 0.2s; }
        .card:hover { transform: scale(1.05); }
    </style>
</head>
<body class="min-h-screen flex flex-col items-center py-8">
    <h1 class="text-3xl font-bold text-gray-800 mb-6">HTTP Server Performance Dashboard</h1>
    <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6 w-full max-w-6xl">
        <div class="card bg-white p-6 rounded-lg shadow-lg">
            <h2 class="text-lg font-semibold text-gray-700">Active Connections</h2>
            <p id="connections" class="text-2xl text-blue-600">0</p>
        </div>
        <div class="card bg-white p-6 rounded-lg shadow-lg">
            <h2 class="text-lg font-semibold text-gray-700">Total Requests</h2>
            <p id="requests" class="text-2xl text-blue-600">0</p>
        </div>
        <div class="card bg-white p-6 rounded-lg shadow-lg">
            <h2 class="text-lg font-semibold text-gray-700">Avg Response Time (ms)</h2>
            <p id="avg_response_time" class="text-2xl text-blue-600">0</p>
        </div>
        <div class="card bg-white p-6 rounded-lg shadow-lg">
            <h2 class="text-lg font-semibold text-gray-700">Max Response Time (ms)</h2>
            <p id="max_response_time" class="text-2xl text-blue-600">0</p>
        </div>
        <div class="card bg-white p-6 rounded-lg shadow-lg">
            <h2 class="text-lg font-semibold text-gray-700">Min Response Time (ms)</h2>
            <p id="min_response_time" class="text-2xl text-blue-600">0</p>
        </div>
        <div class="card bg-white p-6 rounded-lg shadow-lg">
            <h2 class="text-lg font-semibold text-gray-700">Timeout Connections</h2>
            <p id="timeout_connections" class="text-2xl text-red-600">0</p>
        </div>
        <div class="card bg-white p-6 rounded-lg shadow-lg">
            <h2 class="text-lg font-semibold text-gray-700">Error Connections</h2>
            <p id="error_connections" class="text-2xl text-red-600">0</p>
        </div>
    </div>
    <div class="w-full max-w-6xl mt-8">
        <div class="bg-white p-6 rounded-lg shadow-lg">
            <h2 class="text-lg font-semibold text-gray-700 mb-4">Response Time Trends</h2>
            <canvas id="responseTimeChart"></canvas>
        </div>
    </div>
    <script>
        const ctx = document.getElementById('responseTimeChart').getContext('2d');
        const chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: 'Avg Response Time (ms)',
                        data: [],
                        borderColor: '#3b82f6',
                        fill: false
                    },
                    {
                        label: 'Max Response Time (ms)',
                        data: [],
                        borderColor: '#ef4444',
                        fill: false
                    },
                    {
                        label: 'Min Response Time (ms)',
                        data: [],
                        borderColor: '#10b981',
                        fill: false
                    }
                ]
            },
            options: {
                responsive: true,
                scales: {
                    x: { title: { display: true, text: 'Time' } },
                    y: { title: { display: true, text: 'Response Time (ms)' } }
                }
            }
        });

        const source = new EventSource('/metrics');
        source.onmessage = function(event) {
            const data = JSON.parse(event.data);
            document.getElementById('connections').textContent = data.active_connections;
            document.getElementById('requests').textContent = data.total_requests;
            document.getElementById('avg_response_time').textContent = data.avg_response_time_ms.toFixed(2);
            document.getElementById('max_response_time').textContent = data.max_response_time_ms.toFixed(2);
            document.getElementById('min_response_time').textContent = data.min_response_time_ms.toFixed(2);
            document.getElementById('timeout_connections').textContent = data.timeout_connections;
            document.getElementById('error_connections').textContent = data.error_connections;

            const now = new Date().toLocaleTimeString();
            chart.data.labels.push(now);
            chart.data.datasets[0].data.push(data.avg_response_time_ms);
            chart.data.datasets[1].data.push(data.max_response_time_ms);
            chart.data.datasets[2].data.push(data.min_response_time_ms);
            if (chart.data.labels.length > 20) {
                chart.data.labels.shift();
                chart.data.datasets.forEach(dataset => dataset.data.shift());
            }
            chart.update();
        };
    </script>
</body>
</html>
        )";
    res.set_content(html, "text/html");
  });

  // SSE端点，推送扩展指标
  svr.Get("/metrics", [&](const httplib::Request &, httplib::Response &res) {
    res.set_header("Content-Type", "text/event-stream");
    res.set_header("Cache-Control", "no-cache");
    res.set_header("Connection", "keep-alive");

    res.set_content_provider(
        "text/event-stream", [&](size_t /*offset*/, httplib::DataSink &sink) {
          while (true) {
            std::string data = "data: " + metrics.to_json() + "\n\n";
            if (!sink.write(data.c_str(), data.size())) {
              break;
            }
            sink.done();
            std::this_thread::sleep_for(std::chrono::seconds(1));
          }
          return true;
        });
  });

  SYLAR_LOG_INFO(g_logger) << "Web server running on port 8080";
  svr.listen("0.0.0.0", 8080);
}

void run() {
  // 实例化性能指标
  ServerMetrics metrics;

  // 超时阈值（毫秒）
  const double TIMEOUT_THRESHOLD_MS = 500.0;

  // 启动HTTP服务器
  HttpServer::ptr server(new HttpServer(true));
  Address::ptr addr = Address::LookupAnyIPAddress("0.0.0.0:8020");
  while (!server->bind(addr)) {
    sleep(2);
  }
  auto sd = server->getServletDispatch();

  // 默认Servlet，恢复Nginx HTML
  sd->addServlet("/test", [&metrics, TIMEOUT_THRESHOLD_MS](
                              HttpRequest::ptr req, HttpResponse::ptr rsp,
                              HttpSession::ptr session) {
    try {
      SYLAR_LOG_DEBUG(g_logger)
          << "Starting /* servlet, path: " << (req ? req->getPath() : "null");
      if (!req || !rsp) {
        SYLAR_LOG_ERROR(g_logger) << "Invalid req or rsp in /* servlet";
        rsp->setBody("Internal Server Error");
        rsp->setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        metrics.error_connections++;
        return 500;
      }

      metrics.active_connections++;
      SYLAR_LOG_DEBUG(g_logger) << "Incremented active_connections";

      auto start = std::chrono::high_resolution_clock::now();
      SYLAR_LOG_DEBUG(g_logger) << "Got start time";

      // 设置Nginx HTML
      rsp->setHeader("Content-Type", "text/html");
      rsp->setBody(R"rawliteral(<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
<style>
html { color-scheme: light dark; }
body { width: 35em; margin: 0 auto;
font-family: Tahoma, Verdana, Arial, sans-serif; }
</style>
</head>
<body>
<h1>Welcome to nginx!</h1>
<p>If you see this page, the nginx web server is successfully installed and
working. Further configuration is required.</p>
<p>For online documentation and support please refer to
<a href="http://nginx.org/">nginx.org</a>.<br/>
Commercial support is available at
<a href="http://nginx.com/">nginx.com</a>.</p>
<p><em>Thank you for using nginx.</em></p>
</body>
</html>)rawliteral");
      SYLAR_LOG_DEBUG(g_logger) << "Set body for /* servlet";

      auto end = std::chrono::high_resolution_clock::now();
      double response_time_ms =
          std::chrono::duration<double, std::milli>(end - start).count();
      SYLAR_LOG_DEBUG(g_logger)
          << "Calculated response time: " << response_time_ms;

      // 记录指标，检查超时
      bool is_timeout = response_time_ms > TIMEOUT_THRESHOLD_MS;
      metrics.record_request(response_time_ms, is_timeout, false);
      SYLAR_LOG_DEBUG(g_logger) << "Recorded metrics for /* servlet";

      metrics.active_connections--;
      SYLAR_LOG_DEBUG(g_logger) << "Decremented active_connections";

      SYLAR_LOG_DEBUG(g_logger)
          << "Processed /* request, response_time_ms: " << response_time_ms;
      return 0;
    } catch (const std::exception &e) {
      SYLAR_LOG_ERROR(g_logger) << "Exception in /* servlet: " << e.what();
      if (rsp) {
        rsp->setBody("Internal Server Error");
        rsp->setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
      }
      metrics.error_connections++;
      metrics.active_connections--;
      return 500;
    } catch (...) {
      SYLAR_LOG_ERROR(g_logger) << "Unknown exception in /* servlet";
      if (rsp) {
        rsp->setBody("Internal Server Error");
        rsp->setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
      }
      metrics.error_connections++;
      metrics.active_connections--;
      return 500;
    }
  });

  // 404 Servlet
  sd->addGlobServlet("/error", [&metrics, TIMEOUT_THRESHOLD_MS](
                                   HttpRequest::ptr req, HttpResponse::ptr rsp,
                                   HttpSession::ptr session) {
    try {
      SYLAR_LOG_DEBUG(g_logger) << "Starting /sylarx/* servlet, path: "
                                << (req ? req->getPath() : "null");
      if (!req || !rsp) {
        SYLAR_LOG_ERROR(g_logger) << "Invalid req or rsp in /sylarx/* servlet";
        rsp->setBody("Internal Server Error");
        rsp->setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        metrics.error_connections++;
        return 500;
      }

      metrics.active_connections++;
      auto start = std::chrono::high_resolution_clock::now();

      rsp->setHeader("Content-Type", "text/html");
      rsp->setBody(XX(<html><head><title> 404 Not Found</ title></ head><body>
                          <center><h1> 404 Not Found</ h1></ center><hr><center>
                              nginx /
                          1.16.0 <
                      / center > </ body></ html><!--padding-->));

      auto end = std::chrono::high_resolution_clock::now();
      double response_time_ms =
          std::chrono::duration<double, std::milli>(end - start).count();

      bool is_timeout = response_time_ms > TIMEOUT_THRESHOLD_MS;
      metrics.record_request(response_time_ms, is_timeout, false);

      metrics.active_connections--;
      SYLAR_LOG_DEBUG(g_logger)
          << "Processed /sylarx/* request, response_time_ms: "
          << response_time_ms;
      return 0;
    } catch (const std::exception &e) {
      SYLAR_LOG_ERROR(g_logger)
          << "Exception in /sylarx/* servlet: " << e.what();
      if (rsp) {
        rsp->setBody("Internal Server Error");
        rsp->setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
      }
      metrics.error_connections++;
      metrics.active_connections--;
      return 500;
    } catch (...) {
      SYLAR_LOG_ERROR(g_logger) << "Unknown exception in /sylarx/* servlet";
      if (rsp) {
        rsp->setBody("Internal Server Error");
        rsp->setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
      }
      metrics.error_connections++;
      metrics.active_connections--;
      return 500;
    }
  });

  // 启动Web监控服务器
  std::thread web_thread(start_web_server, std::ref(metrics));
  web_thread.detach();

  // 启动HTTP服务器
  server->start();
}

int main(int argc, char **argv) {
  IOManager iom(6, "main");
  iom.schedule(run);
  return 0;
}
