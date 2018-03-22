#ifndef ECDF_H
#define ECDF_H

#include "ns3/uinteger.h"
#include "ns3/timer.h"
#include <vector>
#include <iostream>

namespace ns3 {


struct PacketValueRecord {
  PacketValueRecord(Time time, uint32_t packet_value) : receive_time(time.GetSeconds()), packet_value(packet_value) {};

  double receive_time;
  uint32_t packet_value;

  bool operator < (const PacketValueRecord &rhs) const
  {
    return receive_time < rhs.receive_time;
  }

};

class eCDF
{
  public:
    uint32_t GetThresholdValue(double);
    void AddValue(Time, uint32_t);
    void RemoveOldValues(Time currentTime);

    void SetTimeDelta(Time);
    Time GetTimeDelta(void);


  private:
    std::vector<PacketValueRecord> values;
    Time timedelta;

};

}

#endif /* ECDF_H */