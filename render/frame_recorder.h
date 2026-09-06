#pragma once
#include <cstddef>
#include <vector>
namespace kalara::render {
struct FrameStatistics { std::size_t count=0;double medianMs=0,p95Ms=0,p99Ms=0,maxMs=0; };
class FrameRecorder {
public:
    explicit FrameRecorder(std::size_t capacity=7200);
    void record(double milliseconds);
    const std::vector<double>& samples()const{return samples_;}
    FrameStatistics statistics()const;
private:
    std::size_t capacity_,next_=0;bool wrapped_=false;std::vector<double> samples_;
};
}
