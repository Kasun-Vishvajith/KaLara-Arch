#include "kalara/exporters/dxf_writer.hpp"
#include <cmath>

namespace kalara::exporters {

DxfWriter::DxfWriter() {
    m_stream << std::fixed << std::setprecision(2);
}

void DxfWriter::writeGroup(int code, const std::string& value) {
    m_stream << code << "\n" << value << "\n";
}

void DxfWriter::writeGroup(int code, int value) {
    m_stream << code << "\n" << value << "\n";
}

void DxfWriter::writeGroup(int code, int64_t value) {
    m_stream << code << "\n" << value << "\n";
}

void DxfWriter::writeGroup(int code, double value) {
    m_stream << code << "\n" << value << "\n";
}

void DxfWriter::beginSection(const std::string& sectionName) {
    writeGroup(0, "SECTION");
    writeGroup(2, sectionName);
}

void DxfWriter::endSection() {
    writeGroup(0, "ENDSEC");
}

void DxfWriter::writeHeader(const kalara::core::geometry::Point2D& extMin,
                            const kalara::core::geometry::Point2D& extMax) {
    beginSection("HEADER");

    // AutoCAD Release 12 / 2000 compatibility
    writeGroup(9, "$ACADVER");
    writeGroup(1, "AC1009");

    // Canonical internal units = Millimetres (Section 2, Principle 3)
    // 1 = Inches, 2 = Feet, 4 = Millimeters, 6 = Meters
    writeGroup(9, "$INSUNITS");
    writeGroup(70, 4);

    // Measurement system: 1 = Metric, 0 = Imperial
    writeGroup(9, "$MEASUREMENT");
    writeGroup(70, 1);

    // Linear units: 2 = Decimal
    writeGroup(9, "$LUNITS");
    writeGroup(70, 2);

    // Angular units: 0 = Decimal degrees
    writeGroup(9, "$AUNITS");
    writeGroup(70, 0);

    // Drawing extents
    writeGroup(9, "$EXTMIN");
    writeGroup(10, extMin.x);
    writeGroup(20, extMin.y);
    writeGroup(30, 0.0);

    writeGroup(9, "$EXTMAX");
    writeGroup(10, extMax.x);
    writeGroup(20, extMax.y);
    writeGroup(30, 0.0);

    endSection();
}

void DxfWriter::writeTables(const std::vector<DxfLayerDef>& layers) {
    beginSection("TABLES");

    // VPORT table
    writeGroup(0, "TABLE");
    writeGroup(2, "VPORT");
    writeGroup(70, 0);
    writeGroup(0, "ENDTAB");

    // LTYPE table
    writeGroup(0, "TABLE");
    writeGroup(2, "LTYPE");
    writeGroup(70, 1);
    writeGroup(0, "LTYPE");
    writeGroup(2, "CONTINUOUS");
    writeGroup(70, 64);
    writeGroup(3, "Solid line");
    writeGroup(72, 65);
    writeGroup(73, 0);
    writeGroup(40, 0.0);
    writeGroup(0, "ENDTAB");

    // LAYER table
    writeGroup(0, "TABLE");
    writeGroup(2, "LAYER");
    writeGroup(70, static_cast<int>(layers.size()));

    for (const auto& l : layers) {
        writeGroup(0, "LAYER");
        writeGroup(2, l.name);
        writeGroup(70, 64);
        writeGroup(62, l.color);
        writeGroup(6, l.linetype);
    }
    writeGroup(0, "ENDTAB");

    endSection();
}

void DxfWriter::writeBlocks() {
    beginSection("BLOCKS");
    endSection();
}

void DxfWriter::beginEntities() {
    beginSection("ENTITIES");
}

void DxfWriter::endEntities() {
    endSection();
}

void DxfWriter::writeLine(const std::string& layer,
                          const kalara::core::geometry::Point2D& p1,
                          const kalara::core::geometry::Point2D& p2,
                          double z) {
    writeGroup(0, "LINE");
    writeGroup(8, layer);
    writeGroup(10, p1.x);
    writeGroup(20, p1.y);
    writeGroup(30, z);
    writeGroup(11, p2.x);
    writeGroup(21, p2.y);
    writeGroup(31, z);
}

void DxfWriter::writePolyline(const std::string& layer,
                              const std::vector<kalara::core::geometry::Point2D>& points,
                              bool closed,
                              double z) {
    if (points.empty()) return;

    writeGroup(0, "POLYLINE");
    writeGroup(8, layer);
    writeGroup(66, 1); // Vertices follow flag
    writeGroup(70, closed ? 1 : 0); // 1 = Closed polyline
    writeGroup(10, 0.0);
    writeGroup(20, 0.0);
    writeGroup(30, z);

    for (const auto& pt : points) {
        writeGroup(0, "VERTEX");
        writeGroup(8, layer);
        writeGroup(10, pt.x);
        writeGroup(20, pt.y);
        writeGroup(30, z);
    }

    writeGroup(0, "SEQEND");
}

void DxfWriter::writeArc(const std::string& layer,
                         const kalara::core::geometry::Point2D& center,
                         double radius,
                         double startAngleDeg,
                         double endAngleDeg,
                         double z) {
    writeGroup(0, "ARC");
    writeGroup(8, layer);
    writeGroup(10, center.x);
    writeGroup(20, center.y);
    writeGroup(30, z);
    writeGroup(40, radius);
    writeGroup(50, startAngleDeg);
    writeGroup(51, endAngleDeg);
}

void DxfWriter::writeCircle(const std::string& layer,
                           const kalara::core::geometry::Point2D& center,
                           double radius,
                           double z) {
    writeGroup(0, "CIRCLE");
    writeGroup(8, layer);
    writeGroup(10, center.x);
    writeGroup(20, center.y);
    writeGroup(30, z);
    writeGroup(40, radius);
}

void DxfWriter::writeText(const std::string& layer,
                         const kalara::core::geometry::Point2D& pos,
                         double height,
                         const std::string& text,
                         double rotationDeg) {
    writeGroup(0, "TEXT");
    writeGroup(8, layer);
    writeGroup(10, pos.x);
    writeGroup(20, pos.y);
    writeGroup(30, 0.0);
    writeGroup(40, height);
    writeGroup(1, text);
    if (std::abs(rotationDeg) > 0.001) {
        writeGroup(50, rotationDeg);
    }
}

void DxfWriter::writeDimension(const std::string& layer,
                              const kalara::core::geometry::Point2D& p1,
                              const kalara::core::geometry::Point2D& p2,
                              const kalara::core::geometry::Point2D& textPos,
                              const std::string& textOverride) {
    // Write decomposed visual dimension primitives for 100% universal CAD compatibility:
    // 1. Dimension line connecting offset points
    // 2. Extension lines from witness points
    // 3. Tick marks at dimension ends
    // 4. Centered text
    kalara::core::geometry::Vector2D dir = p2 - p1;
    double len = dir.length();
    if (len < 0.001) return;

    kalara::core::geometry::Vector2D normal = dir.normalized().perpendicular();
    // Project textPos to normal to get signed offset
    double signedOffset = (textPos - p1).dot(normal);
    kalara::core::geometry::Vector2D offsetVec = normal * signedOffset;

    kalara::core::geometry::Point2D d1 = p1 + offsetVec;
    kalara::core::geometry::Point2D d2 = p2 + offsetVec;

    // Dimension line
    writeLine(layer, d1, d2);

    // Witness / extension lines
    writeLine(layer, p1, d1 + normal * 100.0);
    writeLine(layer, p2, d2 + normal * 100.0);

    // 45-degree architectural slash ticks (200 mm length)
    kalara::core::geometry::Vector2D tick = (dir.normalized() + normal).normalized() * 100.0;
    writeLine(layer, d1 - tick, d1 + tick);
    writeLine(layer, d2 - tick, d2 + tick);

    // Dimension text placed slightly above dimension line
    kalara::core::geometry::Point2D midDim = kalara::core::geometry::Point2D{(d1.x + d2.x) * 0.5, (d1.y + d2.y) * 0.5} + normal * 120.0;
    double rotDeg = std::atan2(dir.dy, dir.dx) * (180.0 / 3.14159265358979323846);
    if (rotDeg > 90.0 || rotDeg < -90.0) {
        rotDeg += 180.0;
    }
    writeText(layer, midDim, 180.0, textOverride.empty() ? std::to_string(static_cast<int>(std::round(len))) : textOverride, rotDeg);
}

void DxfWriter::writeEof() {
    writeGroup(0, "EOF");
}

} // namespace kalara::exporters
