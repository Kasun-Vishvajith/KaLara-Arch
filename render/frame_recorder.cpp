#include "render/frame_recorder.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace kalara::render {
FrameRecorder::FrameRecorder(std::size_t capacity):capacity_(capacity){if(!capacity)throw std::invalid_argument("Frame capacity must be positive");samples_.reserve(capacity);}
void FrameRecorder::record(double value){if(!std::isfinite(value)||value<0)throw std::invalid_argument("Frame duration must be finite and nonnegative");if(samples_.size()<capacity_)samples_.push_back(value);else{samples_[next_]=value;wrapped_=true;}next_=(next_+1)%capacity_;}
FrameStatistics FrameRecorder::statistics()const{if(samples_.empty())return{};auto sorted=samples_;std::sort(sorted.begin(),sorted.end());auto quantile=[&](double q){return sorted[static_cast<std::size_t>(std::ceil(q*sorted.size()))-1];};return{sorted.size(),quantile(.5),quantile(.95),quantile(.99),sorted.back()};}
}
