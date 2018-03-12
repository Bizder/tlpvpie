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
class MyTag : public Tag
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
  /**
   * Set the tag value
   * \param value The tag value.
   */
  void SetSimpleValue (uint16_t value);
  /**
   * Get the tag value
   * \return the tag value.
   */
  uint16_t GetSimpleValue (void) const;
private:
  uint16_t m_simpleValue;  //!< tag value
};

}

#endif /* PACKET_VALUE_TAG_H */