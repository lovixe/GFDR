/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "gpsr-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("GpsrPacket");

namespace ns3 {
namespace gpsr {

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

TypeHeader::TypeHeader (MessageType t = GPSRTYPE_HELLO)
  : m_type (t),
    m_valid (true)
{
}

TypeId
TypeHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::gpsr::TypeHeader")
    .SetParent<Header> ()
    .AddConstructor<TypeHeader> ()
  ;
  return tid;
}

TypeId
TypeHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
TypeHeader::GetSerializedSize () const
{
  return 5;
}

void
TypeHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 ((uint8_t) m_type);
  i.WriteU32(srcID);
}

uint32_t
TypeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t type = i.ReadU8 ();
  srcID = i.ReadU32();
  m_valid = true;
  switch (type)
    {
    case GPSRTYPE_HELLO:
    case GPSRTYPE_FD:
    case GPSRTYPE_POS:
      {
        m_type = (MessageType) type;
        break;
      }
    default:
      m_valid = false;
    }
  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
TypeHeader::Print (std::ostream &os) const
{
  switch (m_type)
    {
    case GPSRTYPE_HELLO:
      {
        os << "HELLO";
        break;
      }
    case GPSRTYPE_POS:
      {
        os << "POSITION";
        break;
      }
    default:
      os << "UNKNOWN_TYPE";
    }
}

bool
TypeHeader::operator== (TypeHeader const & o) const
{
  return (m_type == o.m_type && m_valid == o.m_valid);
}

std::ostream &
operator<< (std::ostream & os, TypeHeader const & h)
{
  h.Print (os);
  return os;
}

//-----------------------------------------------------------------------------
// HELLO
//-----------------------------------------------------------------------------
HelloHeader::HelloHeader (uint64_t originPosx, uint64_t originPosy)
  : m_originPosx (originPosx),
    m_originPosy (originPosy)
{
  csCount = recvCount = 0;
}

NS_OBJECT_ENSURE_REGISTERED (HelloHeader);

TypeId
HelloHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::gpsr::HelloHeader")
    .SetParent<Header> ()
    .AddConstructor<HelloHeader> ()
  ;
  return tid;
}

TypeId
HelloHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
HelloHeader::GetSerializedSize () const
{
  uint32_t result = 16 + 8 + 8 + csCount * 16 + recvCount * 8 + 8;
  return result;
}

void
HelloHeader::Serialize (Buffer::Iterator i) const
{
  NS_LOG_DEBUG ("Serialize X " << m_originPosx << " Y " << m_originPosy);
  i.WriteHtonU64 (m_originPosx);
  i.WriteHtonU64 (m_originPosy);

  //wirte selfID
  i.WriteHtonU64(selfID);

  //write cs to buf
  i.WriteHtonU64(csCount);

  for(uint32_t j = 0; j < csCount; j++){
    i.WriteHtonU64(cs[j].id);
    i.WriteHtonU64(cs[j].p);
  }

  //write recvList to buf
  i.WriteHtonU64 (recvCount);  //write count

  for(uint32_t j = 0; j < recvCount; j++){
    i.WriteHtonU64(recvList[j]);
  }
}

uint32_t
HelloHeader::Deserialize (Buffer::Iterator start)
{

  Buffer::Iterator i = start;

  m_originPosx = i.ReadNtohU64 ();
  m_originPosy = i.ReadNtohU64 ();
  selfID = i.ReadNtohU64();

  csCount = i.ReadNtohU64();
  for(uint32_t j = 0; j < csCount; j++){
    cs[j].id = i.ReadNtohU64();
    cs[j].p = i.ReadNtohU64();
  }
  recvCount = i.ReadNtohU64();
  for(uint32_t j = 0; j < recvCount; j++){
    recvList[j] = i.ReadNtohU64();
  }


  NS_LOG_DEBUG ("Deserialize X " << m_originPosx << " Y " << m_originPosy);

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
HelloHeader::Print (std::ostream &os) const
{
  os << " PositionX: " << m_originPosx
     << " PositionY: " << m_originPosy;
}

std::ostream &
operator<< (std::ostream & os, HelloHeader const & h)
{
  h.Print (os);
  return os;
}



bool
HelloHeader::operator== (HelloHeader const & o) const
{
  return (m_originPosx == o.m_originPosx && m_originPosy == o.m_originPosy);
}


//-----------------------------------------------------------------------------
// FD
//-----------------------------------------------------------------------------

FDHeader::FDHeader()
{
}
NS_OBJECT_ENSURE_REGISTERED (FDHeader);
TypeId FDHeader::GetTypeId(){
  static TypeId tid = TypeId ("ns3::gpsr::FDHeader")
    .SetParent<Header> ()
    .AddConstructor<FDHeader> ()
  ;
  return tid;
}

void FDHeader::Print(std::ostream &os) const
{
   os << " History Count: "  << pCount;
}

TypeId FDHeader::GetInstanceTypeId() const
{
  return GetTypeId();
}

uint32_t
FDHeader::GetSerializedSize () const
{
  uint32_t result = 1 + 4 * pCount + 16;
  return result;
}

void
FDHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU64(dstPosition.x);
  i.WriteU64(dstPosition.y);
  i.WriteU8(pCount);
  for(int j = 0; j < (int)pCount; j++){
    i.WriteU16(idList[j]);
    i.WriteU16(jumpList[j]);
  }
}

uint32_t
FDHeader::Deserialize (Buffer::Iterator start)
{

  Buffer::Iterator i = start;
  dstPosition.x = i.ReadU64();
  dstPosition.y = i.ReadU64();

  pCount = i.ReadU8();
  uint32_t dist;
  for(uint32_t j = 0; j < pCount; j++){
    idList[j] = i.ReadU16();
    jumpList[j] = i.ReadU16();
  }
  dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

bool
FDHeader::operator== (FDHeader const & o) const
{
  return (pCount == o.pCount);
}

//-----------------------------------------------------------------------------
// Position
//-----------------------------------------------------------------------------
PositionHeader::PositionHeader (uint64_t dstPosx, uint64_t dstPosy, uint32_t updated, uint64_t recPosx, uint64_t recPosy, uint8_t inRec, uint64_t lastPosx, uint64_t lastPosy)
  : m_dstPosx (dstPosx),
    m_dstPosy (dstPosy),
    m_updated (updated),
    m_recPosx (recPosx),
    m_recPosy (recPosy),
    m_inRec (inRec),
    m_lastPosx (lastPosx),
    m_lastPosy (lastPosy)
{
}

NS_OBJECT_ENSURE_REGISTERED (PositionHeader);

TypeId
PositionHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::gpsr::PositionHeader")
    .SetParent<Header> ()
    .AddConstructor<PositionHeader> ()
  ;
  return tid;
}

TypeId
PositionHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
PositionHeader::GetSerializedSize () const
{
  return 53;
}

void
PositionHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU64 (m_dstPosx);
  i.WriteU64 (m_dstPosy);
  i.WriteU32 (m_updated);
  i.WriteU64 (m_recPosx);
  i.WriteU64 (m_recPosy);
  i.WriteU8 (m_inRec);
  i.WriteU64 (m_lastPosx);
  i.WriteU64 (m_lastPosy);
}

uint32_t
PositionHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_dstPosx = i.ReadU64 ();
  m_dstPosy = i.ReadU64 ();
  m_updated = i.ReadU32 ();
  m_recPosx = i.ReadU64 ();
  m_recPosy = i.ReadU64 ();
  m_inRec = i.ReadU8 ();
  m_lastPosx = i.ReadU64 ();
  m_lastPosy = i.ReadU64 ();

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
PositionHeader::Print (std::ostream &os) const
{
  os << " PositionX: "  << m_dstPosx
     << " PositionY: " << m_dstPosy
     << " Updated: " << m_updated
     << " RecPositionX: " << m_recPosx
     << " RecPositionY: " << m_recPosy
     << " inRec: " << m_inRec
     << " LastPositionX: " << m_lastPosx
     << " LastPositionY: " << m_lastPosy;
}

std::ostream &
operator<< (std::ostream & os, PositionHeader const & h)
{
  h.Print (os);
  return os;
}



bool
PositionHeader::operator== (PositionHeader const & o) const
{
  return (m_dstPosx == o.m_dstPosx && m_dstPosy == o.m_dstPosy && m_updated == o.m_updated && m_recPosx == o.m_recPosx && m_recPosy == o.m_recPosy && m_inRec == o.m_inRec && m_lastPosx == o.m_lastPosx && m_lastPosy == o.m_lastPosy);
}


}
}





