/*
  Author: Minsheng Zhang
*/

#include "ndn-mc.h"

#include "ns3/node.h"
#include "ns3/names.h"

namespace ns3 {
namespace ndn {

TypeId 
Mc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::Mc")
    .SetParent<Object> ()
  	.SetGroupName ("Ndn")
  ;
  return tid;
}

std::ostream&
operator<< (std::ostream& os, const Mc &mc)
{
  os << "Node " << Names::FindName (mc.GetObject<Node>()) << "\n";
  os << "+-------------+--------------------------------------+\n";

  mc.Print (os);
  return os;
}

} // namespace ndn
} // namespace ns3