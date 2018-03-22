#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ecdf.h"
#include "math.h"
#include <algorithm>
#include <iostream>

namespace ns3 {

uint32_t eCDF::GetThresholdValue(double pdrop)
{
  std::vector<PacketValueRecord> sorted_values = values;
  std::sort(sorted_values.begin(), sorted_values.end());
  return sorted_values[ceil(sorted_values.size() * pdrop)].packet_value;
}

void eCDF::RemoveOldValues()
{
  double now = Simulator::Now().GetSeconds();

  int threshold_index = 0;
  for (int i = 0; i < values.size() ; ++i ) {
    if ( now - timedelta > values[i].receive_time)
    {
      threshold_index = i;
    }
  }

  values.erase(values.begin(),values.begin()+threshold_index);
}

void eCDF::AddValue(Time time, uint32_t packet_value)
{
  values.push_back(PacketValueRecord(time, packet_value));
}

void SetTimeDelta(Time timedelta)
{
  this.timedelta = timedelta;
}

Time GetTimeDelta(void)
{
  return timedelta;
}


}