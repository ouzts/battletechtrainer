#include "utils.h"
#include <random>

namespace bt {
    int weightedRandom(const std::vector<int>& weights) {
      static std::random_device rd;
      static std::mt19937 gen(rd());

      std::discrete_distribution<> dist(weights.begin(), weights.end());
      return dist(gen);
    }
} // namespace bt
