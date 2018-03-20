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
    .AddAttribute ("SimpleValue",
                   "A simple value",
                   EmptyAttributeValue (),
                   MakeUintegerAccessor (&PacketValueTag::GetSimpleValue),
                   MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

TypeId PacketValueTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t PacketValueTag::GetSerializedSize (void) const
{
  return 2;
}

void PacketValueTag::Serialize (TagBuffer i) const
{
  i.WriteU16 (m_simpleValue);
}

void PacketValueTag::Deserialize (TagBuffer i)
{
  m_simpleValue = i.ReadU16();
}

void PacketValueTag::Print (std::ostream &os) const
{
  os << "v=" << (uint32_t)m_simpleValue;
}

void PacketValueTag::SetSimpleValue (uint16_t value)
{
  m_simpleValue = value;
}

uint16_t PacketValueTag::GetSimpleValue (void) const
{
  return m_simpleValue;
}

}