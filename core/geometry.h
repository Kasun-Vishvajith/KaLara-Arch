#pragma once
#include <array>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace kalara::geometry {
constexpr double designLimitMm = 10000000.0;
constexpr double mergeSuggestionMm = 1.0;
constexpr double snapRadiusDip = 8.0;
double epsilon(double localExtent);
struct Length {
    double mm;
    explicit Length(double value = 0);
};
struct Angle {
    double radians;
    explicit Angle(double value = 0);
};
enum class Unit { mm, cm, m, inch, foot };
struct LengthParse {
    std::optional<Length> value;
    std::string error;
};
LengthParse parseLength(std::string_view input, Unit bareUnit = Unit::mm);
std::string formatLength(Length value, Unit unit = Unit::mm, int decimals = 0);
std::string formatFeetInches(Length value, int denominator = 16);

struct Vector2 { double x = 0, y = 0; };
struct Point2 {
    double x = 0, y = 0;
    Point2(double x = 0, double y = 0);
};
Vector2 operator-(Point2 a, Point2 b);
Point2 operator+(Point2 p, Vector2 v);
Vector2 operator*(Vector2 v, double factor);
double dot(Vector2 a, Vector2 b);
double cross(Vector2 a, Vector2 b);
double norm(Vector2 v);
Length distance(Point2 a, Point2 b);
bool near(Point2 a, Point2 b, double extent = 1);
int orientation(Point2 a, Point2 b, Point2 c);
struct Segment2 {
    Point2 a, b;
    Segment2(Point2 a, Point2 b);
};
struct Circle2 {
    Point2 center;
    Length radius;
    Circle2(Point2 center, Length radius);
};
struct Arc2 {
    Circle2 circle;
    Angle start, sweep;
    Arc2(Point2 center, Length radius, Angle start, Angle sweep);
};
struct Polyline2 { std::vector<Point2> points; };
struct Polygon2 { std::vector<Point2> outer; std::vector<std::vector<Point2>> holes; };
struct AABB2 {
    Point2 min, max;
    bool contains(Point2 p) const;
};
struct Transform2 {
    // [a c tx; b d ty; 0 0 1]
    double a = 1, b = 0, c = 0, d = 1, tx = 0, ty = 0;
    Point2 apply(Point2 p) const;
    Transform2 inverse() const;
    Transform2 then(const Transform2& next) const;
    static Transform2 translation(Vector2 v);
    static Transform2 rotation(Angle angle);
    static Transform2 scaling(double x, double y);
};
double projectionParameter(Point2 p, Segment2 segment);
Point2 nearestPoint(Point2 p, Segment2 segment);
// Collinear overlap returns its two endpoints; an endpoint touch returns one.
std::vector<Point2> intersections(Segment2 first, Segment2 second);
std::vector<Point2> intersections(Segment2 line, Circle2 circle);
std::vector<Point2> intersections(Segment2 line, Arc2 arc);
bool containsAngle(const Arc2& arc, Angle angle);
double signedArea(const std::vector<Point2>& ring);
double area(const Polygon2& polygon);
double perimeter(const Polygon2& polygon);
enum class Location { outside, inside, boundary };
Location locate(Point2 point, const std::vector<Point2>& ring);
Location locate(Point2 point, const Polygon2& polygon);
// Enforces winding, simple rings, hole containment and disjoint holes.
std::optional<std::string> validate(const Polygon2& polygon);
AABB2 bounds(const std::vector<Point2>& points);
AABB2 bounds(const Arc2& arc);
Segment2 offset(Segment2 segment, Length leftDistance);
// Positive distance expands the outer boundary and shrinks holes. Invalid
// collapsed/self-intersecting results reject; no silent repair or topology guess.
Polygon2 offset(const Polygon2& polygon, Length outwardDistance);
}
