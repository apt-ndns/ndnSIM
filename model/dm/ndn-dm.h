/*
  Minsheng Zhang
*/

#ifndef _NDN_DM_H_
#define	_NDN_DM_H_

#include "ns3/simple-ref-count.h"
#include "ns3/node.h"
#include "ns3/ndn-dm-entry.h"
#include <vector>
#include "boost/tuple/tuple.hpp"

namespace ns3{
namespace ndn{

class Dm: public Object
{
public:
  static TypeId GetTypeId ();
  
  Dm () {}

  virtual ~Dm () {}

  virtual Ptr<dm::Entry>
  Find(const Name &prefix) = 0;

  virtual Ptr<dm::Entry>
  FindLongestMatch (const Name &prefix) = 0;

  virtual std::vector<boost::tuple<name::Component, int32_t, int32_t, Ptr<dm::Entry> > >
  FindAllComponents(const Name &prefix) = 0;

  virtual Ptr<dm::Entry>
  Add (const Name prefix, const Name &mapping, int32_t priority, int32_t weight) = 0;

  virtual Ptr<dm::Entry>
  Add (const Ptr<const Name> &prefix, const Ptr<const Name> &mapping, int32_t priority, int32_t weight) = 0;

  // remove the entry from the DM,
  virtual void
  Remove (const Ptr<const Name> &prefix) = 0;
    
  virtual bool
  Remove (std::string &prefix, std::string &parentPrefix, bool &parentHasChild) = 0;
  
  virtual uint32_t
  GetSize () const = 0;
  
  virtual void
  Print (std::ostream &os) const = 0;

  static inline Ptr<Dm>
  GetDm (Ptr<Object> node);

private:
  Dm (const Dm&) {};
}; 

std::ostream& operator<< (std::ostream& os, const Dm &dm);

Ptr<Dm>
Dm::GetDm (Ptr<Object> node)
{
  return node->GetObject<Dm> ();
}
} // namespace ndn
} // namespace ns3
#endif // _NDN_DM_H_