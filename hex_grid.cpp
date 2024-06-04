#include <emscripten.h>
#include <emscripten/html5.h>
#include <cmath>
#include <string>
#include <assert.h>
#include <unordered_set>
#include <cstdio>
#include <vector>
#include <random>
#include <queue>

const int canvasWidth = 800;
const int canvasHeight = 600;
const int map_size = 5; // Adjust as needed
const int hexSize = 30;

struct Point
{
  const double x;
  const double y;
  Point(double x_, double y_): x(x_), y(y_) {}
};

struct FractionalHex
{
  const double q;
  const double r;
  const double s;
  FractionalHex(double q_, double r_, double s_): q(q_), r(r_), s(s_) {
    if (round(q + r + s) != 0) throw "q + r + s must be 0";
  }
};


struct Hex {
  static inline int instances = 0;
  int q;
  int r;
  int s;
  mutable int facing=-1;

  void setFacing(int f) const { facing = f; }
  int id;
  std::string type;
  std::string level;
  int height;
  Hex(int q_, int r_, int s_): q(q_), r(r_), s(s_) {
    if (q + r + s != 0) throw "q + r + s must be 0";

    type = getType();
    level = getLevel(type);
    id = instances++;
//    printf("Created hex with ID %d\n", id);
  }

  Hex(const Hex& other) = default;
  Hex& operator=(const Hex& other) = default;

  std::string getType() {
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

  // Function to generate weighted random numbers
  int weightedRandom(std::vector<int> weights) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::discrete_distribution<> dist(weights.begin(), weights.end());
    return dist(gen);
  }

  std::string getLevel(const std::string& type) {
    if (type == "water") {
      int depth = weightedRandom({60, 30, 7, 3});
      height = -depth;
      return "Depth: " + std::to_string(depth);
    } else {
      int elevation = weightedRandom({40, 30, 15, 10, 3, 2});
      height = elevation;
      return "Elev: " + std::to_string(elevation);
    }
  }

bool operator==(const Hex& other) const {
  return q == other.q && r == other.r && s == other.s;
}
};

namespace std {
  template <>
    struct hash<Hex> {
      std::size_t operator()(const Hex& h) const {
        return std::hash<int>()(h.q) ^ std::hash<int>()(h.r) ^ std::hash<int>()(h.s);
      }
    };
}

Hex hex_add(Hex a, Hex b) {
  return Hex(a.q + b.q, a.r + b.r, a.s + b.s);
}


Hex hex_subtract(Hex a, Hex b) {
  return Hex(a.q - b.q, a.r - b.r, a.s - b.s);
}


Hex hex_scale(Hex a, int k) {
  return Hex(a.q * k, a.r * k, a.s * k);
}


Hex hex_rotate_left(Hex a) {
  return Hex(-a.s, -a.q, -a.r);
}

int hex_length(Hex hex) {
  return int((abs(hex.q) + abs(hex.r) + abs(hex.s)) / 2);
}

Hex hex_rotate_right(Hex a) {
  return Hex(-a.r, -a.s, -a.q);
}

const std::vector<Hex> hex_directions = {Hex(1, 0, -1), Hex(1, -1, 0), Hex(0, -1, 1), Hex(-1, 0, 1), Hex(-1, 1, 0), Hex(0, 1, -1)};
Hex hex_direction(int direction) {
  return hex_directions[direction];
}


Hex hex_neighbor(Hex hex, int direction) {
  return hex_add(hex, hex_direction(direction));
}


const std::vector<Hex> hex_diagonals = {Hex(2, -1, -1), Hex(1, -2, 1), Hex(-1, -1, 2), Hex(-2, 1, 1), Hex(-1, 2, -1), Hex(1, 1, -2)};
Hex hex_diagonal_neighbor(Hex hex, int direction) {
  return hex_add(hex, hex_diagonals[direction]);
}

Hex hex_round(FractionalHex h) {
  int qi = int(round(h.q));
  int ri = int(round(h.r));
  int si = int(round(h.s));
  double q_diff = abs(qi - h.q);
  double r_diff = abs(ri - h.r);
  double s_diff = abs(si - h.s);
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

int hex_distance(Hex a, Hex b) {
  return hex_length(hex_subtract(a, b));
}

FractionalHex hex_lerp(FractionalHex a, FractionalHex b, double t) {
  return FractionalHex(a.q * (1.0 - t) + b.q * t, a.r * (1.0 - t) + b.r * t, a.s * (1.0 - t) + b.s * t);
}

std::vector<std::pair<Hex, int>> hex_linedraw(Hex a, Hex b)
{
  int N = hex_distance(a, b);
  FractionalHex a_nudge = FractionalHex(a.q + 1e-06, a.r + 1e-06, a.s - 2e-06);
  FractionalHex b_nudge = FractionalHex(b.q + 1e-06, b.r + 1e-06, b.s - 2e-06);
  FractionalHex c_nudge = FractionalHex(a.q - 1e-06, a.r - 1e-06, a.s + 2e-06);
  FractionalHex d_nudge = FractionalHex(b.q - 1e-06, b.r - 1e-06, b.s + 2e-06);
  std::vector<std::pair<Hex, int>> results = {};
  double step = 1.0 / std::max(N, 1);
  for (int i = 0; i <= N; i++)
  {
    auto h1 = hex_round(hex_lerp(a_nudge, b_nudge, step * i));
    auto h2 = hex_round(hex_lerp(c_nudge, d_nudge, step * i));
    if (h1 == h2) results.push_back({h1, 0});
    else {
      results.push_back({h1, 1});
      results.push_back({h2, 1});
    }
  }
  return results;
}
std::unordered_set<Hex> map;

std::string getHexColor(const Hex& hex) {
  if (hex.type == "ground") {
    if (hex.height == 0) return "wheat";
    if (hex.height == 1) return "tan";
    if (hex.height == 2) return "burlywood";
    if (hex.height == 3) return "saddlebrown";
    if (hex.height == 4) return "sienna";
    if (hex.height == 5) return "brown";
  }
  if (hex.type == "water") {
    if (hex.height == 0) return "lightblue";
    if (hex.height == -1) return "skyblue";
    if (hex.height == -2) return "blue";
    if (hex.height == -3) return "darkblue";
  }
  if (hex.type == "rubble") return "grey";
  if (hex.type == "light woods") return "lightgreen";
  if (hex.type == "heavy woods") return "darkgreen";
  if (hex.type == "mech") return "red";
  return "";
}


struct Orientation
{
  const double f0;
  const double f1;
  const double f2;
  const double f3;
  const double b0;
  const double b1;
  const double b2;
  const double b3;
  const double start_angle;
  Orientation(double f0_, double f1_, double f2_, double f3_, double b0_, double b1_, double b2_, double b3_, double start_angle_): f0(f0_), f1(f1_), f2(f2_), f3(f3_), b0(b0_), b1(b1_), b2(b2_), b3(b3_), start_angle(start_angle_) {}
};

struct Layout
{
  const Orientation orientation;
  const Point size;
  const Point origin;
  Layout(Orientation orientation_, Point size_, Point origin_): orientation(orientation_), size(size_), origin(origin_) {}
};

const Orientation layout_pointy = Orientation(sqrt(3.0), sqrt(3.0) / 2.0, 0.0, 3.0 / 2.0, sqrt(3.0) / 3.0, -1.0 / 3.0, 0.0, 2.0 / 3.0, 0.5);

Point hex_to_pixel(Layout layout, Hex h)
{
  Orientation M = layout.orientation;
  Point size = layout.size;
  Point origin = layout.origin;
  double x = (M.f0 * h.q + M.f1 * h.r) * size.x;
  double y = (M.f2 * h.q + M.f3 * h.r) * size.y;
  return Point(x + origin.x, y + origin.y);
}

Point hex_corner_offset(Layout layout, int corner)
{
  Orientation M = layout.orientation;
  Point size = layout.size;
  double angle = 2.0 * M_PI * (M.start_angle - corner) / 6.0;
  return Point(size.x * cos(angle), size.y * sin(angle));
}

std::vector<Point> polygon_corners(Layout layout, Hex h)
{
  std::vector<Point> corners = {};
  Point center = hex_to_pixel(layout, h);
  for (int i = 0; i < 6; i++)
  {
    Point offset = hex_corner_offset(layout, i);
    corners.push_back(Point(center.x + offset.x, center.y + offset.y));
  }
  return corners;
}


void EMSCRIPTEN_KEEPALIVE drawHexagon(float x, float y, const Layout& layout, const Hex& h) {
  std::vector<Point> corners = polygon_corners(layout, h);
  std::string script = "var ctx = Module['canvas'].getContext('2d'); ctx.beginPath();";
  for (int i = 0; i < 6; i++) {
    script += "ctx.lineTo(" + std::to_string(corners[i].x) + ", " + std::to_string(corners[i].y) + ");";
  }
  script += "ctx.closePath();";
  script += "ctx.fillStyle = '" + getHexColor(h) + "'; ctx.fill();";
  script += "ctx.strokeStyle = 'black'; ctx.stroke();";


  if (h.type == "mech") {
    printf("highlighting mech border\n");
  int hl = h.facing;
  script += "ctx.lineWidth = 7;";
  script += "ctx.beginPath(); ctx.moveTo(" + std::to_string(corners[hl].x) + ", " + std::to_string(corners[hl].y) + ");";
  script += "ctx.lineTo(" + std::to_string(corners[(hl + 1) % 6].x) + ", " + std::to_string(corners[(hl + 1) % 6].y) + ");";
  script += "ctx.strokeStyle = 'yellow'; ctx.stroke();";
  script += "ctx.lineWidth = 1;";
  }

  Point center = hex_to_pixel(layout, h);
  std::string topText = std::to_string(h.id);
  std::string middleText = h.type;
  std::string bottomText = h.level;

  if (!topText.empty()) {
    script += "ctx.fillStyle = 'black'; ctx.font = '12px Arial';";
    script += "ctx.textAlign = 'center'; ctx.textBaseline = 'top';";
    script += "ctx.fillText('" + topText + "', " + std::to_string(center.x) + ", " + std::to_string(center.y - 15) + ");";
  }

  // Add middle text if provided
  if (!middleText.empty()) {
    script += "ctx.fillStyle = 'black'; ctx.font = '10px Arial';";
    script += "ctx.textAlign = 'center'; ctx.textBaseline = 'middle';";
    script += "ctx.fillText('" + middleText + "', " + std::to_string(center.x) + ", " + std::to_string(center.y) + ");";
  }

  // Add bottom text if provided
  if (!bottomText.empty()) {
    script += "ctx.fillStyle = 'black'; ctx.font = '12px Arial';";
    script += "ctx.textAlign = 'center'; ctx.textBaseline = 'bottom';";
    script += "ctx.fillText('" + bottomText + "', " + std::to_string(center.x) + ", " + std::to_string(center.y + 15) + ");";
  }

  emscripten_run_script(script.c_str());
}

void drawGrid() {
  emscripten_set_canvas_element_size("#canvas", canvasWidth, canvasHeight);

  std::string clearCanvas = "var ctx = Module['canvas'].getContext('2d'); ctx.fillStyle = 'white'; ctx.fillRect(0, 0, "
    + std::to_string(canvasWidth) + ", " + std::to_string(canvasHeight) + "); ctx.strokeStyle = 'black';";
  emscripten_run_script(clearCanvas.c_str());


  const Orientation o(sqrt(3.0), sqrt(3.0) / 2.0, 0.0, 3.0 / 2.0, sqrt(3.0) / 3.0, -1.0 / 3.0, 0.0, 2.0 / 3.0, 0.5);
  Layout l(o, Point(hexSize, hexSize), Point(300, 300));

  for (const auto& h : map) {
    drawHexagon(0, 0, l, h);
  }
}

auto getRandomHex() {
  int max = map.size();
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, max-1);
  int rval = dis(gen);
//  static bool once = false;
//  if (!once) { rval = 60; once = true; }
//  else rval = 68;
  printf("Selected %d\n", rval);
  auto it = std::find_if(map.begin(), map.end(), [&](const Hex&h) { return h.id == rval && h.type != "mech"; });
  if (it == map.end()) {
    printf("what?\n");
  }
  return it;
}


bool is_in_firing_arc(Hex mech, int facing, Hex target) {
  std::vector<int> front_directions = {
    facing, (facing + 1) % 6, (facing - 1 + 6) % 6
  };

  std::unordered_set<Hex> arc_hexes;
  std::unordered_map<Hex, bool> seen;
  std::queue<Hex> q;
  q.push(mech);

  while (!q.empty()) {
    auto cur = q.front();
    seen[cur] = true;
    q.pop();
    if (cur == target) return true;
    for (int dir : front_directions) {
      auto it = std::find_if(map.begin(), map.end(), [&](const Hex &hx) { return hx == hex_neighbor(cur, dir); });
      if (it == map.end()) continue;
      Hex current = *it;
      if (!seen.count(current))
        q.push(current);
    }
  }
  return false;
}



// place mechs
void bar() {
  auto it = getRandomHex();
  assert(it != map.end());
  auto hex = *it;
  map.erase(it);
  hex.type = "mech";
  hex.setFacing(rand() % 6);
  hex.level = "";
  map.insert(hex);

  it = getRandomHex();
  assert(it != map.end());
  auto hex2 = *it;
  map.erase(it);
  hex2.type = "mech";
  hex2.setFacing(rand() % 6);
  hex2.level = "";
  map.insert(hex2);

  auto intersecting_hexes = hex_linedraw(hex, hex2);
  printf("Calculating intersection from %d to %d\n", hex.id, hex2.id);
  auto first_non_self = *std::find_if(map.begin(), map.end(), [&](const Hex &hx) { return hx == intersecting_hexes[1].first; });

  int neighbor1 = hex.facing - 1;
  if (neighbor1 < 0) neighbor1 = 5;
  int neighbor2 = hex.facing + 1;
  if (neighbor2 > 5) neighbor2 = 0;

  printf("Checking neighbors: %d %d %d\n", hex.facing, neighbor1, neighbor2);
  auto hex_neighbor1 = *std::find_if(map.begin(), map.end(), [&](const Hex& hx) { return hx == hex_neighbor(hex, hex.facing); });
  auto hex_neighbor2 = *std::find_if(map.begin(), map.end(), [&](const Hex& hx) { return hx == hex_neighbor(hex, neighbor1); });
  auto hex_neighbor3 = *std::find_if(map.begin(), map.end(), [&](const Hex& hx) { return hx == hex_neighbor(hex, neighbor2); });

  printf("Frontal arc neighbors of %d are %d, %d, %d\n", hex.id, hex_neighbor1.id, hex_neighbor2.id, hex_neighbor3.id);

  printf("First non self ID is %d\n", first_non_self.id);
  if (first_non_self.id == hex_neighbor1.id ||
      first_non_self.id == hex_neighbor2.id ||
      first_non_self.id == hex_neighbor3.id) {
    // this isn't right, bug!
    printf("***************** %d is in frontal arc of %d\n", hex2.id, hex.id);
  }
    printf("########## %d is in firing arc of %d?: %d\n", hex2.id, hex.id, is_in_firing_arc(hex, hex.facing, hex2));




  // if facing is 0, frontal arc is a path starting with 0, 1, or 5
  // if facing is 1, frontal arc is a path starting with 1, 0, or 2
  // if facing is 2, frontal arc is a path starting with 2, 1, or 3
  // if facing is 3, frontal arc is a path starting with 3, 2 or 4
  // if facing is 4, frontal arc is a path starting with 4, 3 or 5
  // if facing is 5, frontal arc is a path starting with 5, 4 or 6

  for (auto &h: map) {
    for (int dir = 0; dir < 6; dir++) {
      auto neighbor = hex_neighbor(hex, dir);
      auto found_it = std::find_if(map.begin(), map.end(), [&](const Hex& hxx) { return hxx == neighbor; });
      auto found = *found_it;
      //      printf("Neighbor of %d in direction %d is hex %d\n", hex.id, dir, found.id);
    }
  }



  for (auto &[h, partial] : intersecting_hexes) {
    auto it = std::find_if(map.begin(), map.end(), [&](const Hex &h2) { return h == h2; });
    if (it != map.end()) {
      if (partial) printf("PARTIAL ");
      printf("Intersects: %d\n", it->id);

    } else { printf("no matches\n"); }
  }
  drawGrid();
}

extern "C" {
  void EMSCRIPTEN_KEEPALIVE myFunction() { bar(); } }

  int main() {
    map.clear();
    Hex::instances = 0;
    srand(time(NULL));
    for (int q = -map_size; q <= map_size; q++) {
      int r1 = std::max(-map_size, -q - map_size);
      int r2 = std::min(map_size, -q + map_size);
      for (int r = r1; r <= r2; r++) {
        Hex h(q, r, -q-r);
        map.insert(h);
      }
    }
    drawGrid();
  }

