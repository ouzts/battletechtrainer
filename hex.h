#pragma once
#include "fractional_hex.h"
#include <string>
#include <vector>

namespace bt {
  class Hex {
    template<typename T> friend class std::hash;
  public:
    Hex(int q, int r, int s);

    Hex(const Hex& other) = default;
    Hex& operator=(const Hex& other) = default;

    [[nodiscard]] std::string getType() const;
    [[nodiscard]] std::string getLevel(const std::string& type) const;

    bool operator==(const Hex& other) const;

    [[nodiscard]] int length() const;
    [[nodiscard]] Hex neighbor(int direction) const;
    [[nodiscard]] int distance(const Hex& other) const;

    [[nodiscard]] int height() const { return height_; }
    [[nodiscard]] int q() const { return q_; }
    [[nodiscard]] int r() const { return r_; }
    [[nodiscard]] int s() const { return s_; }
    [[nodiscard]] int id() const { return id_; }
    [[nodiscard]] int facing() const { return facing_; }
    [[nodiscard]] std::string type() const { return type_; }
    [[nodiscard]] std::string level() const { return level_; }
    [[nodiscard]] Hex add(const Hex &other) const {
      return Hex(q_ + other.q_, r_ + other.r_, s_ + other.s_);
    }

    [[nodiscard]] Hex subtract(const Hex& other)  const {
      return Hex(q_ - other.q_, r_ - other.r_, s_ - other.s_);
    }


    [[nodiscard]] Hex hex_scale(int k) const {
      return Hex(q_ * k, r_ * k, s_ * k);
    }

    [[nodiscard]] std::vector<std::pair<bt::Hex, int>> linedraw(const Hex& other) const;
    void setFacing(int f) const { facing_ = f; }
    void setLevel(const std::string &s) const { level_ = s; }
    void setType(const std::string &s) const { type_ = s; }

    // TODO: private or do away with it
    static inline int instances = 0;

  private:
    int q_;
    int r_;
    int s_;
    mutable int facing_=-1;

    int id_;
    mutable std::string type_;
    mutable std::string level_;
    mutable int height_;
  };


} // namespace bt

namespace std {
  template <>
    struct hash<bt::Hex> {
      std::size_t operator()(const bt::Hex& h) const {
        return std::hash<int>()(h.q_) ^ std::hash<int>()(h.r_) ^ std::hash<int>()(h.s_);
      }
    };
}
