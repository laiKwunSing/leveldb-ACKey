//
// Created by Guancheng Lai on 10/21/20.
//

#ifndef LEVELDB_METRICS_HPP
#define LEVELDB_METRICS_HPP

#include <fstream>
#include <unordered_map>
#include <iostream>
#include <chrono>
#include <ctime>

class metrics {
public:
  static metrics& GetMetrics() {
    static metrics instance;
    return instance;
  }

  void AddProperty(const std::string& title, int value) {
    property[title] = value;
  }

  void AddCount(const std::string& cacheType, const std::string& activity)  {
    cacheActivityCount[cacheType][activity]++;
  }

  void AddUsage(const std::string& p, double usage) {
    cacheAvgUsage[p].first += usage / static_cast<double>(property["Cache Size"]);
    cacheAvgUsage[p].second++;
  }

private:
  metrics(metrics const&) = delete;

  void operator=(metrics const&) = delete;

  metrics() {
    start = std::chrono::system_clock::now();
  }

  ~metrics() {
    end = std::chrono::system_clock::now();

    std::chrono::duration<float> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    fs = std::fstream("metrics.txt", std::fstream::app);
    if (!fs.good()) {
      std::cout << "Failed to open metrics.txt" << std::endl;
      return;
    }

    std::cout << "Writing to metrics.txt" << std::endl;
    fs << "\n---------------------------------------------\n";
    fs << "Finished computation at " << std::ctime(&end_time);
    fs << "Elapsed time: " << elapsed_seconds.count() << "s\n";

    for (const auto& it : property) {
      fs << it.first << " = " << it.second << std::endl;
    }

    for (const auto& it : cacheActivityCount) {
      std::string cacheType = it.first;
      uint64_t totalActivityCount = 0;
      for (const auto& activity : it.second) {
        totalActivityCount += activity.second;
      }
      fs << "\n--------------" << cacheType << "--------------\n";
      for (const auto& activity : it.second) {
        fs << activity.first << " rate = " << activity.second / static_cast<double>(totalActivityCount) << std::endl;
      }
      fs << "Avg usage = " << cacheAvgUsage[cacheType].first / static_cast<double>(cacheAvgUsage[cacheType].second) << std::endl;
      fs << "--------------" << cacheType << "--------------\n";
    }

    fs << "\n---------------------------------------------" << std::endl;
  }

  std::fstream fs;
  std::unordered_map<std::string, int> property;
  std::unordered_map<std::string, std::unordered_map<std::string, uint64_t>> cacheActivityCount;
  std::unordered_map<std::string, std::pair<double,uint64_t>> cacheAvgUsage;
  std::chrono::system_clock::time_point start, end;
};

#endif //LEVELDB_METRICS_HPP
