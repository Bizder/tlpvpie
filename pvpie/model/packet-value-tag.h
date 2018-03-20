#ifndef PACKET_VALUE_TAG_H
#define PACKET_VALUE_TAG_H

#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <iostream>

namespace ns3 {

/**
 * \ingroup network
 * A simple example of an Tag implementation
 */
class PacketValueTag : public Tag
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  // these are our accessors to our tag structure
  void SetPacketValue (uint32_t);
  uint32_t GetPacketValue (void) const;

  void SetDelayClass (uint8_t);
  uint8_t GetDelayClass (void) const;

private:
  uint32_t m_packetValue;  //!< tag packet value
  uint8_t m_delayClass;   //!< tag delay class
};

}

#endif /* PACKET_VALUE_TAG_H */