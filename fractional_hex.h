#pragma once

namespace bt {
  class Hex;
  class FractionalHex
  {
  public:
    FractionalHex(double q_, double r_, double s_);
    [[nodiscard]] Hex round() const;

    [[nodiscard]] FractionalHex lerp(const FractionalHex& other, double t) const;
  private:
    const double q;
    const double r;
    const double s;
  };

} // namespace bt
