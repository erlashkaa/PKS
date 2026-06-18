#include "TrafficStrategies.h"
#include <sstream>

// =============================================================================
// ConstantBitRateStrategy
// =============================================================================

ConstantBitRateStrategy::ConstantBitRateStrategy(double interval,
                                                   uint32_t payloadSize)
    : interval_(interval),
      payloadSize_(payloadSize),
      lastSentTime_(-interval) 
{}

std::optional<Packet> ConstantBitRateStrategy::generate(
    double             currentTime,
    const std::string& srcMAC,
    const std::string& srcIP,
    const std::string& dstMAC,
    const std::string& dstIP)
{
    if (currentTime - lastSentTime_ < interval_) {
        return std::nullopt; 
    }

    lastSentTime_ = currentTime;

    // Формируем payload с меткой времени
    std::ostringstream oss;
    oss << "CBR-data@t=" << currentTime;

    return Packet(srcMAC, dstMAC, srcIP, dstIP, oss.str(), payloadSize_, currentTime);
}

BurstyStrategy::BurstyStrategy(int      burstSize,
                                 double   burstGap,
                                 double   pauseTime,
                                 uint32_t payloadSize)
    : burstSize_(burstSize),
      burstGap_(burstGap),
      pauseTime_(pauseTime),
      payloadSize_(payloadSize),
      sentInBurst_(0),
      lastEventTime_(-burstGap),
      inPause_(false)
{}


std::optional<Packet> BurstyStrategy::generate(
    double             currentTime,
    const std::string& srcMAC,
    const std::string& srcIP,
    const std::string& dstMAC,
    const std::string& dstIP)
{
    if (inPause_) {
        if (currentTime - lastEventTime_ >= pauseTime_) {
            inPause_       = false;
            sentInBurst_   = 0;
            lastEventTime_ = currentTime;
        }
        return std::nullopt;
    }

    if (currentTime - lastEventTime_ < burstGap_) {
        return std::nullopt; 
    }

    lastEventTime_ = currentTime;
    ++sentInBurst_;

    std::ostringstream oss;
    oss << "BURST[" << sentInBurst_ << "/" << burstSize_ << "]@t=" << currentTime;

    Packet pkt(srcMAC, dstMAC, srcIP, dstIP, oss.str(), payloadSize_, currentTime);

    if (sentInBurst_ >= burstSize_) {
        inPause_       = true;
        lastEventTime_ = currentTime; 
    }

    return pkt;
}
