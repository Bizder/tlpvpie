#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "packet-value-tag.h"
#include <iostream>

namespace ns3 {

TypeId MyTag::GetTypeId (void)
{
  static TypeId tid = TypeId (" ns3::MyTag")
    .SetParent<Tag> ()
    .AddConstructor<MyTag> ()
    .AddAttribute ("SimpleValue",
                   "A simple value",
                   EmptyAttributeValue (),
                   MakeUintegerAccessor (&MyTag::GetSimpleValue),
                   MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

TypeId MyTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t MyTag::GetSerializedSize (void) const
{
  return 2;
}

void MyTag::Serialize (TagBuffer i) const
{
  i.WriteU16 (m_simpleValue);
}

void MyTag::Deserialize (TagBuffer i)
{
  m_simpleValue = i.ReadU16();
}

void MyTag::Print (std::ostream &os) const
{
  os << "v=" << (uint32_t)m_simpleValue;
}

void MyTag::SetSimpleValue (uint16_t value)
{
  m_simpleValue = value;
}

uint16_t MyTag::GetSimpleValue (void) const
{
  return m_simpleValue;
}

}