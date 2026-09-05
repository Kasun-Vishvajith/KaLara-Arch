#pragma once
#include "kalara/core/geometry/point_vector.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace kalara::exporters {

/// Standard AutoCAD Color Index (ACI) numbers for architectural layers.
namespace DxfColor {
    constexpr int Red = 1;
    constexpr int Yellow = 2;
    constexpr int Green = 3;
    constexpr int Cyan = 4;
    constexpr int Blue = 5;
    constexpr int Magenta = 6;
    constexpr int White = 7;
    constexpr int Gray = 8;
    constexpr int LightGray = 9;
}

/// Representation of a CAD layer definition in the DXF TABLES section.
struct DxfLayerDef {
    std::string name;
    int color = DxfColor::White;
    std::string linetype = "CONTINUOUS";
};

/// Low-level ASCII DXF generation engine with canonical millimetre units ($INSUNITS = 4).
class DxfWriter {
public:
    DxfWriter();

    /// Append raw group code and string/numeric value
    void writeGroup(int code, const std::string& value);
    void writeGroup(int code, int value);
    void writeGroup(int code, int64_t value);
    void writeGroup(int code, double value);

    /// Section helpers
    void beginSection(const std::string& sectionName);
    void endSection();

    /// Standard architectural DXF header with explicit mm unit assignment ($INSUNITS = 4, $MEASUREMENT = 1)
    void writeHeader(const kalara::core::geometry::Point2D& extMin = {-10000.0, -10000.0},
                     const kalara::core::geometry::Point2D& extMax = {50000.0, 50000.0});

    /// Tables section with standard layers and linetypes
    void writeTables(const std::vector<DxfLayerDef>& layers);

    /// Blocks section (empty base block table)
    void writeBlocks();

    /// Begin and end entities section
    void beginEntities();
    void endEntities();

    /// Entity output primitives (all in real-scale millimetres)
    void writeLine(const std::string& layer,
                   const kalara::core::geometry::Point2D& p1,
                   const kalara::core::geometry::Point2D& p2,
                   double z = 0.0);

    void writePolyline(const std::string& layer,
                       const std::vector<kalara::core::geometry::Point2D>& points,
                       bool closed = false,
                       double z = 0.0);

    void writeArc(const std::string& layer,
                  const kalara::core::geometry::Point2D& center,
                  double radius,
                  double startAngleDeg,
                  double endAngleDeg,
                  double z = 0.0);

    void writeCircle(const std::string& layer,
                     const kalara::core::geometry::Point2D& center,
                     double radius,
                     double z = 0.0);

    void writeText(const std::string& layer,
                   const kalara::core::geometry::Point2D& pos,
                   double height,
                   const std::string& text,
                   double rotationDeg = 0.0);

    /// Writes dimension entity / decomposed associative linework
    void writeDimension(const std::string& layer,
                        const kalara::core::geometry::Point2D& p1,
                        const kalara::core::geometry::Point2D& p2,
                        const kalara::core::geometry::Point2D& textPos,
                        const std::string& textOverride);

    /// Finalize DXF file with 0 EOF
    void writeEof();

    /// Retrieve full DXF output string
    [[nodiscard]] std::string str() const { return m_stream.str(); }

private:
    std::ostringstream m_stream;
};

} // namespace kalara::exporters
