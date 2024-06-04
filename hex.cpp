#include "hex.h"
#include "utils.h"
#include <random>

namespace bt {
  Hex::Hex(int q, int r, int s): q_(q), r_(r), s_(s) {
    if (q_ + r_ + s_ != 0) throw "q + r + s must be 0";

    type_ = getType();
    level_ = getLevel(type_);
    id_ = instances++;
  }

  std::string Hex::getType() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);

    double randomValue = dis(gen);

    if (randomValue < 0.6) return "ground";
    if (randomValue < 0.7) return "light woods";
    if (randomValue < 0.75) return "heavy woods";
    if (randomValue < 0.8) return "rubble";
    return "water";
  }

  std::string Hex::getLevel(const std::string& type) const {
    if (type == "water") {
      int depth = weightedRandom({60, 30, 7, 3});
      height_ = -depth;
      return "Depth: " + std::to_string(depth);
    } else {
      int elevation = weightedRandom({40, 30, 15, 10, 3, 2});
      height_ = elevation;
      return "Elev: " + std::to_string(elevation);
    }
  }

  bool Hex::operator==(const Hex& other) const {
    return q_ == other.q_ && r_ == other.r_ && s_ == other.s_;
  }

  int Hex::length() const {
    return int((abs(q_) + abs(r_) + abs(s_)) / 2);
  }

  Hex Hex::neighbor(int direction) const {
    static const std::vector<Hex> hex_directions = {Hex(1, 0, -1), Hex(1, -1, 0), Hex(0, -1, 1), Hex(-1, 0, 1), Hex(-1, 1, 0), Hex(0, 1, -1)};
    return add(hex_directions[direction]);
  }

  int Hex::distance(const Hex& other) const {
    return subtract(other).length();
  }

    std::vector<std::pair<bt::Hex, int>> Hex::linedraw(const Hex& other) const {
      int N = distance(other);
      FractionalHex a_nudge = FractionalHex(q_ + 1e-06, r_ + 1e-06, s_ - 2e-06);
      FractionalHex b_nudge = FractionalHex(other.q_ + 1e-06, other.r_ + 1e-06, other.s_ - 2e-06);
      FractionalHex c_nudge = FractionalHex(q_ - 1e-06, r_ - 1e-06, s_ + 2e-06);
      FractionalHex d_nudge = FractionalHex(other.q_ - 1e-06, other.r_ - 1e-06, other.s_ + 2e-06);
      std::vector<std::pair<Hex, int>> results = {};
      double step = 1.0 / std::max(N, 1);
      for (int i = 0; i <= N; i++) {
        auto h1 = a_nudge.lerp(b_nudge, step * i).round();
        auto h2 = c_nudge.lerp(d_nudge, step * i).round();
        if (h1 == h2) results.push_back({h1, 0});
        else {
          results.push_back({h1, 1});
          results.push_back({h2, 1});
        }
      }
      return results;
    }


} // namespace bt
