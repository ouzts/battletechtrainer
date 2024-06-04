#include "fractional_hex.h"
#include "hex.h"

namespace bt {
  FractionalHex::FractionalHex(double q_, double r_, double s_): q(q_), r(r_), s(s_) {
    if (std::round(q + r + s) != 0) throw "q + r + s must be 0";
  }

  Hex FractionalHex::round() const {
    int qi = int(std::round(q));
    int ri = int(std::round(r));
    int si = int(std::round(s));
    double q_diff = abs(qi - q);
    double r_diff = abs(ri - r);
    double s_diff = abs(si - s);
    if (q_diff > r_diff && q_diff > s_diff)
    {
      qi = -ri - si;
    }
    else
      if (r_diff > s_diff)
      {
        ri = -qi - si;
      }
      else
      {
        si = -qi - ri;
      }
    return Hex(qi, ri, si);

  }

   FractionalHex FractionalHex::lerp(const FractionalHex& other, double t) const {
      return FractionalHex(q * (1.0 - t) + other.q * t, r * (1.0 - t) + other.r * t, s * (1.0 - t) + other.s * t);
    }

} // namespace bt
