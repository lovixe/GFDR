/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "constant-position-mobility-model.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ConstantPositionMobilityModel);

TypeId
ConstantPositionMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ConstantPositionMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<ConstantPositionMobilityModel> ()
  ;
  return tid;
}

ConstantPositionMobilityModel::ConstantPositionMobilityModel ()
{
}
ConstantPositionMobilityModel::~ConstantPositionMobilityModel ()
{
}

Vector
ConstantPositionMobilityModel::DoGetPosition (void) const
{
  //int radio = 50;
  /*if(radio * radio> (435 - m_position.x) * (435 - m_position.x) + (435 - m_position.y) * (435 - m_position.y)){
    return Vector(0,0, 65530);
  }
  */
 //方形的，也就是每次都是一定的，所以可以30个30个的变动

  /*if(m_position.x < 145 - radio || m_position.x > 145 + radio || m_position.y < 145 - radio || m_position.y > 145 + radio){
      return m_position;
  }
  else{
      return Vector(-1, -1, 65530);
  }
  return m_position;
  */

 //先做一个测试
 
  //if(m_position.x < 145 - radio || m_position.x > 145 + radio || m_position.y > 8){
    //  return m_position;
  //}
  //else{
    //  return Vector(-1, -1, 65530);
  //}
  return m_position;
}
void
ConstantPositionMobilityModel::DoSetPosition (const Vector &position)
{
  m_position = position;
  NotifyCourseChange ();
}
Vector
ConstantPositionMobilityModel::DoGetVelocity (void) const
{
  return Vector (0.0, 0.0, 0.0);
}

} // namespace ns3