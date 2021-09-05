#include "../PCH.h"
#include "Statistics.h"
#include "StatMacros.h"
#include <unordered_map>
#include <imgui/imgui.h>

DECLARE_FLOAT_STAT(DrawStatisticsUI, CPU)

namespace engine::Core
{
  struct KeyHash
  {
    size_t operator()(hashed_string a) const
    {
      return a.value();
    }
  };
  struct KeyEq
  {
    bool operator()(hashed_string a, hashed_string b) const
    {
      return a == b;
    }
  };
  using FloatStatPayload = std::pair<hashed_string, StatBuffer<float, STATISTICS_BUFFER_SIZE>>;

  struct StatisticsManagerData
  {
    std::unordered_map<hashed_string, FloatStatPayload, KeyHash, KeyEq> floatStats;
  };

  StatisticsManager* StatisticsManager::Get()
  {
    static StatisticsManager manager;
    return &manager;
  }

  StatisticsManager::StatisticsManager()
  {
    data = new StatisticsManagerData;
  }

  StatisticsManager::~StatisticsManager()
  {
    delete data;
  }

  void StatisticsManager::RegisterFloatStat(hashed_string statName, hashed_string groupName)
  {
    ASSERT(!data->floatStats.contains(statName));
    data->floatStats[statName] = std::make_pair(groupName, StatBuffer<float, STATISTICS_BUFFER_SIZE>());
  }

  void StatisticsManager::PushFloatStatValue(hashed_string statName, float val)
  {
    ASSERT(data->floatStats.contains(statName));
    data->floatStats[statName].second.Push(val);
  }

  const StatBuffer<float, STATISTICS_BUFFER_SIZE>& StatisticsManager::GetFloatStat(hashed_string statName)
  {
    ASSERT(data->floatStats.contains(statName));
    return data->floatStats[statName].second;
  }

  void StatisticsManager::DrawUI()
  {
    MEASURE_CPU_TIMER_STAT(DrawStatisticsUI);

    // key: group name
    // value: pair containing <stat name, pointer to stat buffer>
    // IMPORTANT: the group name and stat names are swapped compared to that in `floatStats`
    struct StatBufValues
    {
      double mean{};
      double variance{};
      double min{};
      double max{};
    };
    std::unordered_map<hashed_string, std::vector<std::pair<hashed_string, StatBufValues>>, KeyHash> groupNameToFloatStat;
    for (auto& [statName, pair] : data->floatStats)
    {
      StatBufValues vals
      {
        .mean = pair.second.Mean(),
        .variance = pair.second.Variance(),
        .min = pair.second.Min(),
        .max = pair.second.Max()
      };
      groupNameToFloatStat[pair.first].emplace_back(statName, vals);
    }

    ImGui::Begin("Statistics");
    ImGui::Indent();
    ImGui::Text("%-20s: %-10s%-10s%-10s%-10s", "Stat Name", "Average", "Variance", "Min", "Max");
    ImGui::Unindent();

    for (auto& [groupName, vec] : groupNameToFloatStat)
    {
      std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) { return a.second.max > b.second.max; });
      if (ImGui::TreeNode(groupName.data()))
      {
        for (const auto& [statName, statVals] : vec)
        {
          ImGui::Text("%-20s: %-10f%-10f%-10f%-10f", statName.data(), statVals.mean, statVals.variance, statVals.min, statVals.max);
          ImGui::Separator();
        }
        ImGui::TreePop();
      }
    }
    ImGui::End();
  }
}