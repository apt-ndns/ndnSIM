/*
Author: Minsheng Zhang
*/

#ifndef NDN_tr_FORWARDING_STRATEGY_H
#define NDN_tr_FORWARDING_STRATEGY_H

#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/ndn-forwarding-strategy.h"

namespace ns3 {
namespace ndn {

namespace fw {
}

class Mc;
namespace mc { class Entry; }

class trForwarding :
    public ForwardingStrategy
{
public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  trForwarding ();

  virtual void
  OnInterest (Ptr<Face> face,
              Ptr<Interest> interest);

  virtual void
  OnData (Ptr<Face> face,
          Ptr<Data> data);

 
  virtual bool
  DoPropagateInterest (Ptr<Face> inFace,
                       Ptr<const Interest> interest,
                       Ptr<pit::Entry> pitEntry);

protected:
  // inherited from Object class
  virtual void NotifyNewAggregate (); ///< @brief Even when object is aggregated to another Object
  virtual void DoDispose (); ///< @brief Do cleanup

protected:
  Ptr<Mc> m_mc;
private:
  std::string m_fileName;
  uint64_t m_cachehit;
  uint64_t m_cachemiss;
};

} // namespace ndn
} // namespace ns3

#endif /* NDN_FORWARDING_STRATEGY_H */
