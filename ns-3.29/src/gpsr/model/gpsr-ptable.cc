#include "gpsr-ptable.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("GpsrTable");


namespace ns3 {
namespace gpsr {

/*
  GPSR position table
*/

PositionTable::PositionTable ()
{
  m_txErrorCallback = MakeCallback (&PositionTable::ProcessTxError, this);
  m_entryLifeTime = Seconds (2); //FIXME fazer isto parametrizavel de acordo com tempo de hello

  for (int i = 0; i < MAX_CS_COUNT; i++)
  {
    cs[i].connectID = NO_CONNECT;
    cs[i].count = 0; //初始化，保持干净
    for(int j = 0; j < MAX_CS_COUNT; j++){
      cs[i].cs[j].id = NO_CONNECT;
    }

    lastRecv[i] = NO_CONNECT;
  }
}

Time 
PositionTable::GetEntryUpdateTime (Ipv4Address id)
{
  if (id == Ipv4Address::GetZero ())
    {
      return Time (Seconds (0));
    }
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find (id);
  return i->second.second;
}

/**
 * \brief Adds entry in position table
 */
void 
PositionTable::AddEntry (Ipv4Address id, Vector position)
{
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
    {
      m_table.erase (id);
      m_table.insert (std::make_pair (id, std::make_pair (position, Simulator::Now ())));
      return;
    }
  

  m_table.insert (std::make_pair (id, std::make_pair (position, Simulator::Now ())));
}

void PositionTable::updateOneCycle()
{
  //calc cs
  for(int i = 0; i < MAX_CS_COUNT; i++){
    if(cs[i].connectID != NO_CONNECT){
      //use int instead float
      cs[i].p = cs[i].p * 506 / 512;
    }

    lastRecv[i] = NO_CONNECT; //clear
  }
}

/**
 * \brief recv hello, update cs
 */
void PositionTable::AddRecv(T_Connect *recvCs, int csCount, uint32_t *recvs, int recvCount)
{
  //At first, add to CS
  int csIndex = 0;
  for(csIndex = 0; csIndex < MAX_CS_COUNT; csIndex++){
    if(cs[csIndex].connectID == recvCs->connectID){
      break;
    }
  }
  if(csIndex == MAX_CS_COUNT){
    for(csIndex = 0; csIndex < MAX_CS_COUNT; csIndex++){
      if(cs[csIndex].connectID == NO_CONNECT){
        cs[csIndex].connectID = recvCs->connectID;
        break;
      }
    }
  }
  assert(csIndex < MAX_CS_COUNT);
  

  cs[csIndex].count = recvCs->count;
  for(int i = 0; i < csCount; i++){
    cs[csIndex].cs[i].id = recvCs->cs[i].id;
    cs[csIndex].cs[i].p = recvCs->cs[i].p;
  }

  int recvIndex;
  for(recvIndex = 0 ; recvIndex < MAX_CS_COUNT; recvIndex++){
    if(lastRecv[recvIndex] == recvCs->connectID){
      break;
    }
  }

  if(recvIndex == MAX_CS_COUNT){
    for(int i = 0; i < MAX_CS_COUNT; i++){
      if(lastRecv[i] == NO_CONNECT){
        lastRecv[i] = recvCs->connectID;
        cs[csIndex].p += 25;
        break;
      }
    }
  }
}

/**
 * \brief Deletes entry in position table
 */
void PositionTable::DeleteEntry (Ipv4Address id)
{
  m_table.erase (id);
}

/**
 * \brief Gets position from position table
 * \param id Ipv4Address to get position from
 * \return Position of that id or NULL if not known
 */
Vector 
PositionTable::GetPosition (Ipv4Address id)
{

  NodeList::Iterator listEnd = NodeList::End ();
  for (NodeList::Iterator i = NodeList::Begin (); i != listEnd; i++)
    {
      Ptr<Node> node = *i;
      if (node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () == id)
        {
          return node->GetObject<MobilityModel> ()->GetPosition ();
        }
    }
  return PositionTable::GetInvalidPosition ();

}

/**
 * \brief Checks if a node is a neighbour
 * \param id Ipv4Address of the node to check
 * \return True if the node is neighbour, false otherwise
 */
bool
PositionTable::isNeighbour (Ipv4Address id)
{

 std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
    {
      return true;
    }

  return false;
}


/**
 * \brief remove entries with expired lifetime
 */
void 
PositionTable::Purge ()
{

  if(m_table.empty ())
    {
      return;
    }

  std::list<Ipv4Address> toErase;

  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.begin ();
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator listEnd = m_table.end ();
  
  for (; !(i == listEnd); i++)
    {

      if (m_entryLifeTime + GetEntryUpdateTime (i->first) <= Simulator::Now ())
        {
          toErase.insert (toErase.begin (), i->first);

        }
    }
  toErase.unique ();

  std::list<Ipv4Address>::iterator end = toErase.end ();

  for (std::list<Ipv4Address>::iterator it = toErase.begin (); it != end; ++it)
    {

      m_table.erase (*it);

    }
}

/**
 * \brief clears all entries
 */
void 
PositionTable::Clear ()
{
  m_table.clear ();
}

void PositionTable::AddIpv4IDTable(Ipv4Address ip, uint32_t id)
{
  ids.insert(std::make_pair(ip, id));
}

uint32_t PositionTable::GetIpv4ID(Ipv4Address ip)
{
  std::map<Ipv4Address, uint32_t>::iterator i = ids.find(ip);
  return i->second;
}

//It is used to check whether a node has location information. This is the place to turn off the GPS signal manually
bool PositionTable::HasValidPosition(Vector position)
{
  int radio = 130;
  int centerPositionX = 145;
  int centerPositionY = 150;
  if(position.x < centerPositionX - radio || position.x > centerPositionX +radio || position.y < centerPositionY - radio || position.y > centerPositionY + radio){
    return true;
  }
  return false;
}

//Add the current CS to the P set
void PositionTable::addCurCs(T_Connect *param)
{
  for(uint32_t i = 0; i < param->count; i++){
    for(uint32_t j = 0; j < MAX_CS_COUNT; j++){
      if(cs[j].connectID == param->cs[i].id){
        for(uint32_t k = 0; k < cs[j].count; k++){
          if(!HasSameNode(param, cs[j].cs[k].id)){
            param->cs[param->count].id = cs[j].cs[k].id;
            param->cs[param->count++].jump = 0;
          }
        }
        break;
      }
    }
  }
}
//Get the node with location information in the line
void PositionTable::getAllCsWithPosition(T_Connect *param)
{
  param->count = 0;
  NodeList::Iterator listEnd = NodeList::End ();
  for (NodeList::Iterator i = NodeList::Begin (); i != listEnd; i++)
  {
    Ptr<Node> node = *i;
    for(uint32_t index = 0; index < MAX_CS_COUNT; index++){
      if(cs[index].connectID == node->GetId()){
        Vector position = node->GetObject<MobilityModel>()->GetPosition();
        if(HasValidPosition(position))
        {
          param->cs[param->count].id = node->GetId();
          param->cs[param->count++].jump = 0;
        }
      }
    }
  }
}

bool PositionTable::HasSameNode(T_Connect *param0, uint32_t nodeid)
{
  for(uint32_t i = 0; i < param0->count; i++){
    if(param0->cs[i].id == nodeid){
      return true;
    }
  }
  return false;
}

Ipv4Address PositionTable::GetNextFDNode(T_Connect *his)
{
  for(int i = 0; i < MAX_CS_COUNT; i++){
    if(cs[i].connectID != NO_CONNECT){
      if(!HasSameNode(his, cs[i].connectID)){
        his->cs[his->count].id = cs[i].connectID;
        his->cs[his->count++].jump = 0;
      }
    }
  }
  if(!HasSameNode(his, selfID)){
    his->cs[his->count].id = selfID;
    his->cs[his->count++].jump = 0;
  }

  uint32_t shortList[MAX_CS_COUNT];
  uint32_t shortListValue[MAX_CS_COUNT];
  for(uint32_t i = 0; i < MAX_CS_COUNT; i++){
    shortList[i] = NO_CONNECT;
    shortListValue[i] = 0x00;
  }

  for(uint32_t i = 0; i < MAX_CS_COUNT; i++){
    if(cs[i].connectID != NO_CONNECT){
      shortListValue[i] = getSub(his, i);
      shortList[i] = i;
    }
  }

  //get max avail node count
  uint32_t max = 0;
  uint32_t targetID = NO_CONNECT;
  for(uint32_t i = 0; i < MAX_CS_COUNT; i++){
    if(shortListValue[i] > max){
      max = shortListValue[i];
    }
  }

  //Get send times, in this simulation. Two-way model is different from the actual situation. There will be no route quagmire or two hops better than one hop.
  
  
  uint32_t minTimes = 0xFFFFFFFF;
  for(uint32_t i = 0; i < MAX_CS_COUNT; i++){
    //shortListSendTimes[i] = NO_CONNECT;
    if(shortListValue[i] == max){
      //calculate the send times. Find the p value min is.
      if(cs[shortList[i]].p < minTimes){
        targetID = cs[shortList[i]].connectID;
        minTimes = cs[shortList[i]].p;
      } 
    }
  }

  if(targetID != NO_CONNECT){
    std::map<Ipv4Address, uint32_t>::iterator i;
    for(i = ids.begin(); i != ids.end(); i++){
      if(i->second == targetID){
        return i->first;
      }
    }
  }
  return Ipv4Address::GetZero ();
}

uint32_t PositionTable::getSub(T_Connect *his, uint32_t index)
{
  T_Connect *a = &cs[index];
  uint32_t result = 0;

  for(uint32_t i = 0; i < a->count; i++){
    uint32_t j = 0;
    for(; j < his->count; j++){
      if(his->cs[j].id == a->cs[i].id)
      {
        break;
      }
    }
    if(j == his->count){
      result++;
    }
  }
  return result;
}


/**
 * \brief Gets next hop according to GPSR protocol
 * \param position the position of the destination node
 * \param nodePos the position of the node that has the packet
 * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
 */
Ipv4Address 
PositionTable::BestNeighbor (Vector position, Vector nodePos)
{
  Purge ();

  double initialDistance = CalculateDistance (nodePos, position);

  if (m_table.empty ())
    {
      NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << position);
      return Ipv4Address::GetZero ();
    }     //if table is empty (no neighbours)

  Ipv4Address bestFoundID = m_table.begin ()->first;
  double bestFoundDistance = CalculateDistance (m_table.begin ()->second.first, position);
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i;
  for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      if (bestFoundDistance > CalculateDistance (i->second.first, position))
        {
          bestFoundID = i->first;
          bestFoundDistance = CalculateDistance (i->second.first, position);
        }
    }

  if(initialDistance > bestFoundDistance)
    return bestFoundID;
  else
    return Ipv4Address::GetZero (); //so it enters Recovery-mode

}


/**
 * \brief Gets next hop according to GPSR recovery-mode protocol (right hand rule)
 * \param previousHop the position of the node that sent the packet to this node
 * \param nodePos the position of the destination node
 * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
 */
Ipv4Address
PositionTable::BestAngle (Vector previousHop, Vector nodePos)
{
  Purge ();

  if (m_table.empty ())
    {
      NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << nodePos);
      return Ipv4Address::GetZero ();
    }     //if table is empty (no neighbours)

  double tmpAngle;
  Ipv4Address bestFoundID = Ipv4Address::GetZero ();
  double bestFoundAngle = 360;
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i;

  for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      tmpAngle = GetAngle(nodePos, previousHop, i->second.first);
      if (bestFoundAngle > tmpAngle && tmpAngle != 0)
	{
	  bestFoundID = i->first;
	  bestFoundAngle = tmpAngle;
	}
    }
  if(bestFoundID == Ipv4Address::GetZero ()) //only if the only neighbour is who sent the packet
    {
      bestFoundID = m_table.begin ()->first;
    }
  return bestFoundID;
}


//Gives angle between the vector CentrePos-Refpos to the vector CentrePos-node counterclockwise
double 
PositionTable::GetAngle (Vector centrePos, Vector refPos, Vector node){
  double const PI = 4*atan(1);

  std::complex<double> A = std::complex<double>(centrePos.x,centrePos.y);
  std::complex<double> B = std::complex<double>(node.x,node.y);
  std::complex<double> C = std::complex<double>(refPos.x,refPos.y);   //Change B with C if you want angles clockwise

  std::complex<double> AB; //reference edge
  std::complex<double> AC;
  std::complex<double> tmp;
  std::complex<double> tmpCplx;

  std::complex<double> Angle;

  AB = B - A;
  AB = (real(AB)/norm(AB)) + (std::complex<double>(0.0,1.0)*(imag(AB)/norm(AB)));

  AC = C - A;
  AC = (real(AC)/norm(AC)) + (std::complex<double>(0.0,1.0)*(imag(AC)/norm(AC)));

  tmp = log(AC/AB);
  tmpCplx = std::complex<double>(0.0,-1.0);
  Angle = tmp*tmpCplx;
  Angle *= (180/PI);
  if (real(Angle)<0)
    Angle = 360+real(Angle);

  return real(Angle);

}





/**
 * \ProcessTxError
 */
void PositionTable::ProcessTxError (WifiMacHeader const & hdr)
{
}



//FIXME ainda preciso disto agr que o LS ja n está aqui???????

/**
 * \brief Returns true if is in search for destionation
 */
bool PositionTable::IsInSearch (Ipv4Address id)
{
  return false;
}

bool PositionTable::HasPosition (Ipv4Address id)
{
  return true;
}


}   // gpsr
} // ns3
