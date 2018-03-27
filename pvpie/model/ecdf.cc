#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ecdf.h"
#include "math.h"
#include <algorithm>
#include <iostream>

namespace ns3 {


TypeId eCDF::GetTypeId (void)
{
  static TypeId tid = TypeId (" ns3::eCDF")
    .SetParent<Object> ()
    .AddConstructor<eCDF> ()
    .AddAttribute ("TimeDelta",
                   "time windows used to monitor data",
                    TimeValue(MilliSeconds(100)),
                    MakeTimeAccessor (&eCDF::timedelta),
                    MakeTimeChecker ())
  ;
  return tid;
}

TypeId eCDF::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

eCDF::eCDF()
{
}

uint32_t eCDF::GetThresholdValue(double pdrop)
{
  std::vector<uint32_t> sorted_values;

  for ( uint32_t i = 0; i < values.size() ; ++i )
  {
    sorted_values.push_back(values[i].packet_value);
  }

  std::sort(sorted_values.begin(), sorted_values.end());
  return sorted_values[ceil(sorted_values.size() * pdrop)];
}

void eCDF::RemoveOldValues()
{
  double now = Simulator::Now().GetSeconds();

  int threshold_index = 0;
  for (uint32_t i = 0; i < values.size() ; ++i ) {
    if ( now - timedelta.GetSeconds() > values[i].receive_time)
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

} /* ns3 namespace */