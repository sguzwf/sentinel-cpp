#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "sentinel-core/flow/flow_rule_manager.h"
#include "sentinel-core/init/init_target_registry.h"
#include "sentinel-core/log/log_base.h"
#include "sentinel-core/log/logger.h"
#include "sentinel-core/log/metric/metric_log_task.h"
#include "sentinel-core/public/sph_u.h"
#include "sentinel-core/transport/command/http_server_init_target.h"

std::atomic<int> g_count{0};

void doEntry(const char* resource) {
  while (g_count < 10000) {
    auto r = Sentinel::SphU::Entry(resource);
    if (r->IsBlocked()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      r->Exit();
    }
    g_count.store(g_count.load() + 1);
  }
}

void doOneEntry() { doEntry("my_open_api_abc"); }

void doAnotherEntry() { doEntry("big_brother_service:foo()"); }

int main() {
  // Initialize for Sentinel.
  Sentinel::Log::Logger::InitDefaultLogger();
  Sentinel::Transport::HttpCommandCenterInitTarget command_center_init;
  command_center_init.Initialize();
  Sentinel::Log::MetricLogTask metric_log_task;
  metric_log_task.Initialize();

  Sentinel::Flow::FlowRule rule1{"my_open_api_abc"};
  rule1.set_metric_type(Sentinel::Flow::FlowMetricType::kThreadCount);
  rule1.set_count(2);
  Sentinel::Flow::FlowRuleManager::GetInstance().LoadRules({rule1});
  std::thread t1(doOneEntry);
  std::thread t2(doOneEntry);
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  std::thread t3(doOneEntry);
  std::thread t4(doAnotherEntry);
  std::thread t5(doOneEntry);
  std::thread t6(doOneEntry);
  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  t6.join();
  return 0;
}
