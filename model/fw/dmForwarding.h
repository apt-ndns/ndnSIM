/*
Author: Minsheng Zhang
*/

#ifndef NDN_DM_FORWARDING_STRATEGY_H
#define NDN_DM_FORWARDING_STRATEGY_H

#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/ndn-forwarding-strategy.h"
#include "ns3/ndn-name.h"

namespace ns3 {
namespace ndn {

namespace fw {
}

class Dm;
namespace dm { class Entry; }

class dmForwarding :
    public ForwardingStrategy
{
public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  dmForwarding ();
  virtual ~dmForwarding ();

  virtual void
  OnInterest (Ptr<Face> face,
              Ptr<Interest> interest);

 
  virtual bool
  DoPropagateInterest (Ptr<Face> inFace,
                       Ptr<const Interest> interest,
                       Ptr<pit::Entry> pitEntry);

protected:
  // inherited from Object class
  virtual void NotifyNewAggregate (); 
  virtual void DoDispose (); 

protected:
  Ptr<Dm> m_dm;
};

} // namespace ndn
} // namespace ns3

#endif /* NDN_FORWARDING_STRATEGY_H */
