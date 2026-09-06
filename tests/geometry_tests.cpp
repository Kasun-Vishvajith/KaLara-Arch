#include "core/geometry.h"
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>
using namespace kalara::geometry;
namespace {
int checks=0;
void check(bool condition,const char* label){++checks;if(!condition)throw std::runtime_error(label);}
void close(double actual,double expected,double tolerance,const char* label){check(std::abs(actual-expected)<=tolerance,label);}
template<class F> void rejects(F operation,const char* label){bool rejected=false;try{operation();}catch(const std::invalid_argument&){rejected=true;}check(rejected,label);}
}
int main(){try{
    for(auto input:{"25.4 mm","2.54 cm","0.0254 m","1 in","1\""}){auto parsed=parseLength(input);check(parsed.value.has_value(),input);close(parsed.value->mm,25.4,1e-10,input);}
    close(parseLength("5' 7 1/2\"").value->mm,1714.5,1e-9,"feet fraction");
    close(parseLength("-5 ft 7 1/2 in").value->mm,-1714.5,1e-9,"negative feet fraction");
    close(parseLength("1/2 in").value->mm,12.7,1e-10,"fraction");
    close(parseLength("3",Unit::foot).value->mm,914.4,1e-9,"bare units");
    for(auto input:{"nan","inf","1/0 in","5' 12\"","","3 bananas","1e999 m","--3","1 2"})check(!parseLength(input).value,input);
    check(formatLength(Length(1234.56))=="1235 mm","display round only");
    check(formatLength(Length(1234.56),Unit::m,3)=="1.235 m","decimal display");
    check(formatFeetInches(Length(1714.5))=="5' 7 1/2\"","fraction formatting");
    close(distance({0,0},{3000,4000}).mm,5000,1e-9,"3-4-5 distance");
    rejects([]{Point2 p(10000001,0);},"design envelope");
    rejects([]{Length x(std::numeric_limits<double>::infinity());},"nonfinite length");
    rejects([]{Point2 p(0,std::numeric_limits<double>::quiet_NaN());},"nonfinite point");
    rejects([]{Segment2 s({0,0},{0,0});},"degenerate segment");
    Segment2 tiny({0,0},{0.001,0});close(distance(tiny.a,tiny.b).mm,0.001,1e-12,"tiny valid segment");
    check(orientation({9999000,9999000},{10000000,9999000},{10000000,10000000})==1,"large-coordinate orientation");
    check(orientation({0,0},{1000,0},{1000,0.0000001})==0,"numeric tolerance");
    check(mergeSuggestionMm>epsilon(1000)&&snapRadiusDip==8,"separate tolerances");
    close(nearestPoint({4,7},{{0,0},{10,0}}).x,4,1e-12,"projection");
    auto crossing=intersections(Segment2({0,0},{10,10}),Segment2({0,10},{10,0}));check(crossing.size()==1&&near(crossing[0],{5,5}),"diagonal crossing");
    check(intersections(Segment2({0,0},{10,0}),Segment2({5,0},{15,0})).size()==2,"overlap endpoints");
    check(intersections(Segment2({0,0},{10,0}),Segment2({10,0},{20,0})).size()==1,"endpoint intersection");
    check(intersections(Segment2({0,0},{10,0}),Segment2({0,1},{10,1.000001})).empty(),"near parallel no intersection");
    Circle2 circle({0,0},Length(5));
    check(intersections(Segment2({-10,0},{10,0}),circle).size()==2,"line circle secant");
    check(intersections(Segment2({-10,5},{10,5}),circle).size()==1,"line circle tangent");
    Arc2 arc({0,0},Length(5),Angle(0),Angle(std::numbers::pi));
    check(intersections(Segment2({0,-10},{0,10}),arc).size()==1,"line arc filtering");
    const auto arcBox=bounds(arc);close(arcBox.min.x,-5,1e-12,"arc min x");close(arcBox.max.y,5,1e-12,"arc max y");
    const auto transform=Transform2::scaling(1.7,0.4).then(Transform2::rotation(Angle(0.7))).then(Transform2::translation({300000,700000}));
    for(Point2 p:std::vector<Point2>{{0,0},{6000,4000},{-10000,99999}})check(near(transform.inverse().apply(transform.apply(p)),p,100000),"inverse roundtrip");
    rejects([]{Transform2::scaling(0,1).inverse();},"singular transform");
    Polygon2 rectangle{{{0,0},{6000,0},{6000,4000},{0,4000}}, {}};
    check(!validate(rectangle),"rectangle validation");close(area(rectangle),24000000,1e-5,"centerline area");
    close(perimeter(rectangle),20000,1e-9,"centerline perimeter");
    const auto inner=offset(rectangle,Length(-100));close(area(inner),22040000,1e-5,"clear room area 22.04 m2");close(perimeter(inner),19200,1e-9,"clear perimeter");
    rejects([&]{offset(rectangle,Length(-3000));},"inward collapse rejection");
    Polygon2 courtyard=rectangle;courtyard.holes={{{1000,1000},{1000,2000},{2000,2000},{2000,1000}}};
    check(!validate(courtyard),"courtyard winding");close(area(courtyard),23000000,1e-5,"courtyard hole area");
    check(locate({1500,1500},courtyard)==Location::outside,"hole excluded");check(locate({1000,1500},courtyard)==Location::boundary,"hole boundary");
    close(area(offset(courtyard,Length(100))),25400000,1e-5,"courtyard outward offset");
    rejects([&]{offset(courtyard,Length(600));},"hole collapse rejection");
    for(int i=0;i<1000;++i) {
        Point2 point(i*17.3-5000,i*3.123-9000);
        check(near(transform.inverse().apply(transform.apply(point)),point,100000),"inverse fixture sweep");
    }
    Polygon2 lshape{{{0,0},{4000,0},{4000,2000},{2000,2000},{2000,4000},{0,4000}}, {}};
    close(area(lshape),12000000,1e-5,"L area");
    check(validate({{{0,0},{10,10},{0,10},{10,0}}, {}}).has_value(),"self intersection rejection");
    courtyard.holes.push_back(courtyard.holes[0]);check(validate(courtyard).has_value(),"overlapping holes reject");
    std::cout<<"geometry-v1: "<<checks<<" numerical checks PASS; diagonal=5000 mm; inner area=22040000 mm2; inner perimeter=19200 mm\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}

