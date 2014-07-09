/*
Author: Minsheng Zhang
*/

#ifndef _NDN_MC_H_
#define	_NDN_MC_H_

#include "ns3/simple-ref-count.h"
#include "ns3/node.h"
#include "ns3/ndn-mc-entry.h"
#include <vector>
#include "boost/tuple/tuple.hpp"

namespace ns3{
namespace ndn{

class Mc: public Object
{
public:
  static TypeId GetTypeId ();
  
  Mc () {}

  virtual ~Mc () {}

// exact matching
  virtual Ptr<mc::Entry>
  Find(const Name &prefix) = 0;

  virtual Ptr<mc::Entry>
  FindLongestMatch (const Name &prefix) = 0;

// notice the number of child will not change after the initialization.
  virtual Ptr<mc::Entry>
  Add (const Name prefix, bool hasChild, const Name mapping, int32_t priority, int32_t weight) = 0;

  virtual Ptr<mc::Entry>
  Add (const Ptr<const Name> &prefix, bool hasChild, const Ptr<const Name> &mapping, int32_t priority, int32_t weight) = 0;

  virtual void
  AddorUpdate(const std::string &prefix, const std::string &mapping, int32_t priority, int32_t weight) = 0;
  
  virtual void
  Remove (const std::string &prefix, const std::string &parentPrefix, bool parentHasChild) = 0;
    
  virtual uint32_t
  GetSize () const = 0;
  
  virtual void
  Print (std::ostream &os) const = 0;

  static inline Ptr<Mc>
  GetMc (Ptr<Object> node);

  inline uint64_t 
  GetSeq ();

  inline void
  SetSeq(uint64_t seq);

private:
  Mc (const Mc&) {};

protected:
  uint64_t m_seq;
}; 

std::ostream& operator<< (std::ostream& os, const Mc &Mc);

Ptr<Mc>
Mc::GetMc (Ptr<Object> node)
{
  return node->GetObject<Mc> ();
}

uint64_t
Mc::GetSeq()
{
  return m_seq;
}

void
Mc::SetSeq(uint64_t seq)
{
  m_seq = seq;
}

} // namespace ndn
} // namespace ns3

#endif