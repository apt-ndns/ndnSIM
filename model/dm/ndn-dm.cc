/*
  Author: Minsheng Zhang
*/

#include "ndn-dm.h"

#include "ns3/node.h"
#include "ns3/names.h"

namespace ns3 {
namespace ndn {

TypeId 
Dm::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::Dm")
    .SetParent<Object> ()
  	.SetGroupName ("Ndn")
  ;
  return tid;
}

std::ostream&
operator<< (std::ostream& os, const Dm &dm)
{
  os << "Node " << Names::FindName (dm.GetObject<Node>()) << "\n";
  os << "+-------------+--------------------------------------+\n";

  dm.Print (os);
  return os;
}

} // namespace ndn
} // namespace ns3