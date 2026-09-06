#include "core/geometry.h"
#include <algorithm>
#include <iomanip>
#include <limits>
#include <locale>
#include <numbers>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace kalara::geometry {
namespace {
constexpr double tau = 2 * std::numbers::pi;
void finite(double value) { if (!std::isfinite(value)) throw std::invalid_argument("A finite value is required"); }
double factor(Unit unit) {
    switch (unit) { case Unit::cm: return 10; case Unit::m: return 1000; case Unit::inch: return 25.4; case Unit::foot: return 304.8; default: return 1; }
}
double number(const std::string& text) {
    std::istringstream stream(text);
    stream.imbue(std::locale::classic());
    double value;
    stream >> value;
    if (!stream || !stream.eof()) throw std::invalid_argument("Invalid number");
    finite(value);
    return value;
}
double fraction(const std::string& text) {
    static const std::regex pattern(R"(^\s*(?:(\d+)\s+)?(\d+)\s*/\s*(\d+)\s*$)");
    std::smatch match;
    if (!std::regex_match(text, match, pattern)) return number(text);
    const double denominator = number(match[3]);
    const double numerator = number(match[2]);
    if (denominator <= 0) throw std::invalid_argument("Fraction denominator must be positive");
    return (match[1].matched ? number(match[1]) : 0) + numerator / denominator;
}
double normalized(double angle) {
    double result = std::fmod(angle, tau);
    return result < 0 ? result + tau : result;
}
Point2 along(Segment2 line, double t) { return line.a + (line.b - line.a) * t; }
bool onSegment(Point2 point, Segment2 line) {
    if (orientation(line.a, line.b, point) != 0) return false;
    const double eps = epsilon(norm(line.b-line.a));
    return point.x >= std::min(line.a.x,line.b.x)-eps && point.x <= std::max(line.a.x,line.b.x)+eps &&
           point.y >= std::min(line.a.y,line.b.y)-eps && point.y <= std::max(line.a.y,line.b.y)+eps;
}
void uniquePush(std::vector<Point2>& points, Point2 point, double extent) {
    for (auto p : points) if (near(p, point, extent)) return;
    points.push_back(point);
}
double ringPerimeter(const std::vector<Point2>& ring) {
    double result = 0;
    for (std::size_t i = 0; i < ring.size(); ++i) result += distance(ring[i],ring[(i+1)%ring.size()]).mm;
    return result;
}
bool ringsIntersect(const std::vector<Point2>& first, const std::vector<Point2>& second) {
    for (std::size_t i=0; i<first.size(); ++i) for (std::size_t j=0; j<second.size(); ++j)
        if (!intersections({first[i],first[(i+1)%first.size()]}, {second[j],second[(j+1)%second.size()]}).empty()) return true;
    return false;
}
std::optional<std::string> validRing(const std::vector<Point2>& ring, bool outer) {
    if (ring.size()<3) return "A ring needs at least three vertices";
    for (auto p : ring) { Point2 checked(p.x,p.y); (void)checked; }
    const auto box = bounds(ring);
    const double extent = distance(box.min,box.max).mm;
    const double signedValue = signedArea(ring);
    if (std::abs(signedValue) <= epsilon(extent)*extent) return "Ring area is degenerate";
    if ((signedValue > 0) != outer) return "Outer rings must be counterclockwise and holes clockwise";
    for (std::size_t i=0; i<ring.size(); ++i) {
        const Segment2 edge(ring[i],ring[(i+1)%ring.size()]);
        const auto previous = ring[(i+ring.size()-1)%ring.size()];
        if (orientation(previous,edge.a,edge.b)==0 && dot(previous-edge.a,edge.b-edge.a)>0) return "Adjacent edges overlap";
        for (std::size_t j=i+1; j<ring.size(); ++j) {
            if (j==i+1 || (i==0 && j==ring.size()-1)) continue;
            if (!intersections(edge,{ring[j],ring[(j+1)%ring.size()]}).empty()) return "Ring self-intersects";
        }
    }
    return {};
}
}
double epsilon(double extent) { finite(extent); return std::max(1e-6,1e-12*std::abs(extent)); }
Length::Length(double value) : mm(value) { finite(value); }
Angle::Angle(double value) : radians(value) { finite(value); }
LengthParse parseLength(std::string_view input, Unit bareUnit) {
    try {
        std::string text(input);
        const auto begin = text.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos) return {{},"Enter a length"};
        text = text.substr(begin, text.find_last_not_of(" \t\r\n")-begin+1);
        std::transform(text.begin(),text.end(),text.begin(),[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        double sign=1;
        if (text.front()=='-' || text.front()=='+') { sign=text.front()=='-'?-1:1; text.erase(0,1); }
        static const std::regex feet(R"(^\s*(\d+(?:\.\d+)?)\s*(?:ft|')\s*(?:(\d+(?:\.\d+)?(?:\s+\d+\s*/\s*\d+)?|\d+\s*/\s*\d+)\s*(?:in|")?)?\s*$)");
        std::smatch match;
        double mm;
        if (std::regex_match(text,match,feet)) {
            const double inches = match[2].matched ? fraction(match[2]) : 0;
            if (inches >= 12) return {{},"Inches in feet-and-inches input must be less than 12"};
            mm = number(match[1])*304.8 + inches*25.4;
        } else {
            static const std::regex simple(R"(^\s*(\d+(?:\.\d*)?(?:[eE][+-]?\d+)?|\.\d+(?:[eE][+-]?\d+)?|\d+\s+\d+\s*/\s*\d+|\d+\s*/\s*\d+)\s*(mm|cm|m|in|ft|"|')?\s*$)");
            if (!std::regex_match(text,match,simple)) return {{},"Use a number with mm, cm, m, in, ft or feet-inch fractions"};
            const auto suffix=match[2].str();
            Unit unit=bareUnit;
            if(suffix=="mm")unit=Unit::mm; else if(suffix=="cm")unit=Unit::cm; else if(suffix=="m")unit=Unit::m;
            else if(suffix=="in"||suffix=="\"")unit=Unit::inch; else if(suffix=="ft"||suffix=="'")unit=Unit::foot;
            mm=fraction(match[1])*factor(unit);
        }
        return {Length(sign*mm),{}};
    } catch (const std::exception& error) { return {{},error.what()}; }
}
std::string formatLength(Length value, Unit unit, int decimals) {
    finite(value.mm);
    if (decimals<0 || decimals>9) throw std::invalid_argument("Display decimals must be between zero and nine");
    const char* suffix[] = {" mm"," cm"," m"," in"," ft"};
    std::ostringstream out; out.imbue(std::locale::classic());
    const double scale=std::pow(10.0,decimals);
    double rounded=std::round(value.mm/factor(unit)*scale)/scale;
    if(rounded==0)rounded=0;
    out<<std::fixed<<std::setprecision(decimals)<<rounded<<suffix[static_cast<int>(unit)];
    return out.str();
}
std::string formatFeetInches(Length value, int denominator) {
    finite(value.mm);
    if (denominator<=0 || denominator>1024 || (denominator&(denominator-1))) throw std::invalid_argument("Use a power-of-two fraction denominator through 1024");
    const double scaled=std::abs(value.mm)/25.4*denominator;
    if(scaled > static_cast<double>(std::numeric_limits<long long>::max()/2)) throw std::invalid_argument("Length too large to format");
    const auto ticks=static_cast<long long>(std::round(scaled));
    const auto feet=ticks/(12*denominator), inches=ticks/denominator%12;
    auto numerator=ticks%denominator;
    while(numerator && numerator%2==0 && denominator%2==0) {numerator/=2;denominator/=2;}
    std::ostringstream out; out.imbue(std::locale::classic());
    if(value.mm<0 && ticks)out<<'-';
    out<<feet<<"' "<<inches;
    if(numerator)out<<' '<<numerator<<'/'<<denominator;
    out<<'"'; return out.str();
}
Point2::Point2(double xValue,double yValue):x(xValue),y(yValue) {
    finite(x); finite(y);
    if(std::abs(x)>designLimitMm || std::abs(y)>designLimitMm) throw std::invalid_argument("Point is outside the +/-10,000,000 mm design envelope");
}
Vector2 operator-(Point2 a,Point2 b){return {a.x-b.x,a.y-b.y};}
Point2 operator+(Point2 p,Vector2 v){return {p.x+v.x,p.y+v.y};}
Vector2 operator*(Vector2 v,double factor){finite(factor);return {v.x*factor,v.y*factor};}
double dot(Vector2 a,Vector2 b){return std::fma(a.x,b.x,a.y*b.y);}
double cross(Vector2 a,Vector2 b){const double p=a.y*b.x;return std::fma(a.x,b.y,-p)-std::fma(a.y,b.x,-p);}
double norm(Vector2 v){return std::hypot(v.x,v.y);}
Length distance(Point2 a,Point2 b){return Length(norm(a-b));}
bool near(Point2 a,Point2 b,double extent){return distance(a,b).mm<=epsilon(extent);}
int orientation(Point2 a,Point2 b,Point2 c){
    const auto u=b-a,v=c-a;
    const double extent=std::max(norm(u),norm(v));
    const double determinant=cross(u,v),limit=epsilon(extent)*extent;
    return determinant>limit?1:determinant < -limit?-1:0;
}
Segment2::Segment2(Point2 start,Point2 end):a(start),b(end){if(near(a,b,norm(b-a)))throw std::invalid_argument("Segment has zero numerical length");}
Circle2::Circle2(Point2 p,Length r):center(p),radius(r){if(r.mm<=epsilon(r.mm))throw std::invalid_argument("Radius must be positive and nondegenerate");}
Arc2::Arc2(Point2 p,Length r,Angle a,Angle s):circle(p,r),start(a),sweep(s){if(std::abs(s.radians)<=1e-12 || std::abs(s.radians)>tau)throw std::invalid_argument("Arc sweep must be nonzero and at most one revolution");}
bool AABB2::contains(Point2 p)const{return p.x>=min.x&&p.x<=max.x&&p.y>=min.y&&p.y<=max.y;}
Point2 Transform2::apply(Point2 p)const{return {std::fma(a,p.x,std::fma(c,p.y,tx)),std::fma(b,p.x,std::fma(d,p.y,ty))};}
Transform2 Transform2::inverse()const{
    for(double value:{a,b,c,d,tx,ty})finite(value);
    const double determinant=cross({a,b},{c,d});
    const double scale=std::max({std::abs(a),std::abs(b),std::abs(c),std::abs(d)});
    if(scale==0 || std::abs(determinant)<=1e-14*scale*scale)throw std::invalid_argument("Transform is singular or ill-conditioned");
    return {d/determinant,-b/determinant,-c/determinant,a/determinant,(c*ty-d*tx)/determinant,(b*tx-a*ty)/determinant};
}
Transform2 Transform2::then(const Transform2& n)const{return {n.a*a+n.c*b,n.b*a+n.d*b,n.a*c+n.c*d,n.b*c+n.d*d,n.a*tx+n.c*ty+n.tx,n.b*tx+n.d*ty+n.ty};}
Transform2 Transform2::translation(Vector2 v){finite(v.x);finite(v.y);return {1,0,0,1,v.x,v.y};}
Transform2 Transform2::rotation(Angle angle){const double c=std::cos(angle.radians),s=std::sin(angle.radians);return {c,s,-s,c,0,0};}
Transform2 Transform2::scaling(double x,double y){finite(x);finite(y);return {x,0,0,y,0,0};}
double projectionParameter(Point2 p,Segment2 s){auto v=s.b-s.a;return dot(p-s.a,v)/dot(v,v);}
Point2 nearestPoint(Point2 p,Segment2 s){return along(s,std::clamp(projectionParameter(p,s),0.0,1.0));}
std::vector<Point2> intersections(Segment2 first,Segment2 second){
    std::vector<Point2> result;
    const auto u=first.b-first.a,v=second.b-second.a,w=second.a-first.a;
    const double extent=std::max(norm(u),norm(v)),denominator=cross(u,v);
    if(std::abs(denominator)<=epsilon(extent)*extent){
        for(auto p:{first.a,first.b})if(onSegment(p,second))uniquePush(result,p,extent);
        for(auto p:{second.a,second.b})if(onSegment(p,first))uniquePush(result,p,extent);
        return result;
    }
    const double t=cross(w,v)/denominator,s=cross(w,u)/denominator;
    const double tEps=epsilon(extent)/norm(u),sEps=epsilon(extent)/norm(v);
    if(t>=-tEps&&t<=1+tEps&&s>=-sEps&&s<=1+sEps)result.push_back(along(first,std::clamp(t,0.0,1.0)));
    return result;
}
std::vector<Point2> intersections(Segment2 line,Circle2 circle){
    const auto delta=line.b-line.a;
    const double t=projectionParameter(circle.center,line);
    // Calculate the projection vector without constructing an out-of-envelope point.
    const Vector2 relative{line.a.x+delta.x*t-circle.center.x,line.a.y+delta.y*t-circle.center.y};
    const double residual=circle.radius.mm*circle.radius.mm-dot(relative,relative);
    const double eps=epsilon(circle.radius.mm)*circle.radius.mm;
    if(residual < -eps)return {};
    const double dt=std::sqrt(std::max(0.0,residual))/norm(delta);
    std::vector<Point2> result;
    const double te=epsilon(norm(delta))/norm(delta);
    for(double param:{t-dt,t+dt})if(param>=-te&&param<=1+te)uniquePush(result,along(line,std::clamp(param,0.0,1.0)),circle.radius.mm);
    return result;
}
bool containsAngle(const Arc2& arc,Angle angle){
    const double delta=normalized(arc.sweep.radians>0?angle.radians-arc.start.radians:arc.start.radians-angle.radians);
    return delta<=std::abs(arc.sweep.radians)+1e-12 || tau-delta<=1e-12;
}
std::vector<Point2> intersections(Segment2 line,Arc2 arc){
    auto points=intersections(line,arc.circle);
    std::erase_if(points,[&](Point2 p){return !containsAngle(arc,Angle(std::atan2(p.y-arc.circle.center.y,p.x-arc.circle.center.x)));});return points;
}
double signedArea(const std::vector<Point2>& ring){
    if(ring.size()<3)return 0;
    double sum=0,correction=0;
    for(std::size_t i=1;i+1<ring.size();++i){const double value=cross(ring[i]-ring[0],ring[i+1]-ring[0])-correction;const double next=sum+value;correction=(next-sum)-value;sum=next;}
    return sum/2;
}
double area(const Polygon2& polygon){if(auto error=validate(polygon))throw std::invalid_argument(*error);double result=signedArea(polygon.outer);for(const auto& hole:polygon.holes)result+=signedArea(hole);return result;}
double perimeter(const Polygon2& polygon){if(auto error=validate(polygon))throw std::invalid_argument(*error);double result=ringPerimeter(polygon.outer);for(const auto& hole:polygon.holes)result+=ringPerimeter(hole);return result;}
Location locate(Point2 p,const std::vector<Point2>& ring){
    if(ring.size()<3)return Location::outside;
    bool inside=false;
    for(std::size_t i=0,j=ring.size()-1;i<ring.size();j=i++){
        const Segment2 edge(ring[j],ring[i]);
        if(onSegment(p,edge))return Location::boundary;
        if((edge.a.y>p.y)!=(edge.b.y>p.y)){
            const double x=edge.a.x+(p.y-edge.a.y)*(edge.b.x-edge.a.x)/(edge.b.y-edge.a.y);
            if(p.x<x)inside=!inside;
        }
    }
    return inside?Location::inside:Location::outside;
}
Location locate(Point2 p,const Polygon2& polygon){
    auto where=locate(p,polygon.outer);if(where!=Location::inside)return where;
    for(const auto& hole:polygon.holes){where=locate(p,hole);if(where==Location::boundary)return where;if(where==Location::inside)return Location::outside;}
    return Location::inside;
}
std::optional<std::string> validate(const Polygon2& polygon){
    try{
        if(auto error=validRing(polygon.outer,true))return error;
        for(std::size_t i=0;i<polygon.holes.size();++i){const auto& hole=polygon.holes[i];
            if(auto error=validRing(hole,false))return error;
            if(locate(hole[0],polygon.outer)!=Location::inside||ringsIntersect(polygon.outer,hole))return "Hole must lie strictly inside its outer ring";
            for(std::size_t j=0;j<i;++j)if(ringsIntersect(hole,polygon.holes[j])||locate(hole[0],polygon.holes[j])!=Location::outside||locate(polygon.holes[j][0],hole)!=Location::outside)return "Holes must be disjoint and cannot nest";
        }
        return {};
    }catch(const std::exception& error){return error.what();}
}
AABB2 bounds(const std::vector<Point2>& points){
    if(points.empty())throw std::invalid_argument("Cannot bound an empty point set");
    AABB2 box{points[0],points[0]};for(auto p:points){box.min.x=std::min(box.min.x,p.x);box.min.y=std::min(box.min.y,p.y);box.max.x=std::max(box.max.x,p.x);box.max.y=std::max(box.max.y,p.y);}return box;
}
AABB2 bounds(const Arc2& arc){
    std::vector<Point2> points;
    auto point=[&](double angle){return arc.circle.center+Vector2{std::cos(angle)*arc.circle.radius.mm,std::sin(angle)*arc.circle.radius.mm};};
    points.push_back(point(arc.start.radians));points.push_back(point(arc.start.radians+arc.sweep.radians));
    for(int i=0;i<4;++i){const double angle=i*std::numbers::pi/2;if(containsAngle(arc,Angle(angle)))points.push_back(point(angle));}return bounds(points);
}
Segment2 offset(Segment2 s,Length leftDistance){const auto v=s.b-s.a;const Vector2 n{-v.y/norm(v)*leftDistance.mm,v.x/norm(v)*leftDistance.mm};return {s.a+n,s.b+n};}
Polygon2 offset(const Polygon2& polygon,Length outwardDistance){
    if(auto error=validate(polygon))throw std::invalid_argument(*error);
    auto ringOffset=[&](const std::vector<Point2>& ring){
        std::vector<Point2> result;
        for(std::size_t i=0;i<ring.size();++i){
            const auto before=offset({ring[(i+ring.size()-1)%ring.size()],ring[i]},Length(-outwardDistance.mm));
            const auto after=offset({ring[i],ring[(i+1)%ring.size()]},Length(-outwardDistance.mm));
            const auto u=before.b-before.a,v=after.b-after.a;
            const double denominator=cross(u,v);
            if(std::abs(denominator)<=epsilon(std::max(norm(u),norm(v)))*std::max(norm(u),norm(v))){result.push_back(after.a);continue;}
            const double t=cross(after.a-before.a,v)/denominator;
            result.push_back(along(before,t));
        }
        return result;
    };
    Polygon2 result{ringOffset(polygon.outer),{}};
    for(const auto& hole:polygon.holes)result.holes.push_back(ringOffset(hole));
    if(auto error=validate(result))throw std::invalid_argument("Offset requires a topology change: "+*error);
    // A valid-looking inverted collapse must not produce a new phantom region.
    if(outwardDistance.mm<0){for(auto p:result.outer)if(locate(p,polygon)!=Location::inside)throw std::invalid_argument("Offset collapsed the region");
        for(auto p:result.outer){double closest=std::numeric_limits<double>::max();for(std::size_t i=0;i<polygon.outer.size();++i)closest=std::min(closest,distance(p,nearestPoint(p,{polygon.outer[i],polygon.outer[(i+1)%polygon.outer.size()]})).mm);if(closest+epsilon(closest)<-outwardDistance.mm)throw std::invalid_argument("Offset collapsed the region");}}
    if(outwardDistance.mm>0) {
        for(std::size_t h=0;h<polygon.holes.size();++h) {
            const auto& original=polygon.holes[h];
            for(auto p:result.holes[h]) {
                if(locate(p,original)!=Location::inside)throw std::invalid_argument("Offset collapsed a hole");
                double closest=std::numeric_limits<double>::max();
                for(std::size_t i=0;i<original.size();++i)closest=std::min(closest,distance(p,nearestPoint(p,{original[i],original[(i+1)%original.size()]})).mm);
                if(closest+epsilon(closest)<outwardDistance.mm)throw std::invalid_argument("Offset collapsed a hole");
            }
        }
    }
    return result;
}
}

