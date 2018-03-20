#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "packet-value-tag.h"
#include <iostream>

namespace ns3 {

TypeId PacketValueTag::GetTypeId (void)
{
  static TypeId tid = TypeId (" ns3::PacketValueTag")
    .SetParent<Tag> ()
    .AddConstructor<PacketValueTag> ()
    .AddAttribute ("PacketValue",
                   "Packet Value",
                   EmptyAttributeValue(),
                   MakeUintegerAccessor (&PacketValueTag::GetPacketValue),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("DelayClass",
                   "DelayClass",
                   EmptyAttributeValue(),
                   MakeUintegerAccessor (&PacketValueTag::GetDelayClass),
                   MakeUintegerChecker<uint32_t>())
  ;
  return tid;
}

TypeId PacketValueTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t PacketValueTag::GetSerializedSize (void) const
{
  return 5;
}

void PacketValueTag::Serialize (TagBuffer i) const
{
  i.WriteU32 (m_packetValue);
  i.WriteU8 (m_delayClass);
}

void PacketValueTag::Deserialize (TagBuffer i)
{
  m_packetValue = i.ReadU32();
  m_delayClass = i.ReadU8();
}

void PacketValueTag::Print (std::ostream &os) const
{
  os << "pv=" << (uint32_t)m_packetValue;
  os << "dc=" << (uint32_t)m_delayClass;
}

void PacketValueTag::SetPacketValue (uint32_t value)
{
  m_packetValue = value;
}

uint32_t PacketValueTag::GetPacketValue (void) const
{
  return m_packetValue;
}

void PacketValueTag::SetDelayClass(uint8_t delayClass)
{
  m_delayClass = delayClass;
}

uint8_t PacketValueTag::GetDelayClass(void) const
{
  return m_delayClass;
}

}