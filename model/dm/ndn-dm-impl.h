/*
Author: Minsheng Zhang
*/

#ifndef _NDN_DM_IMPL_H_
#define _NDN_DM_IMPL_H_

#include "ns3/ndn-dm.h"
#include "ns3/ndn-name.h"
#include "../../utils/trie/trie-with-policy.h"
#include "../../utils/trie/counting-policy.h"
#include <utility>
#include "boost/tuple/tuple.hpp"

namespace ns3{
namespace ndn{
namespace dm{

class EntryImpl : public Entry
{
public:
  typedef ndnSIM::trie_with_policy<
    Name,
    ndnSIM::smart_pointer_payload_traits<EntryImpl>,
    ndnSIM::counting_policy_traits
    > trie;

  EntryImpl (Ptr<Dm> dm, const Ptr<const Name> &prefix)
  :Entry (dm, prefix)
  ,item_ (0)
  {
  }

  void
  SetTrie (trie::iterator item)
  {
    item_ = item;
  }

  trie::iterator to_iterator () { return item_; }
  trie::const_iterator to_iterator () const { return item_; }
  
private:
  trie::iterator item_;
};

class DmImpl : public Dm,
              protected ndnSIM::trie_with_policy< Name,
                                                  ndnSIM::smart_pointer_payload_traits< EntryImpl >,
                                                  ndnSIM::counting_policy_traits >
{
public:
	typedef ndnSIM::trie_with_policy< Name,
                                    ndnSIM::smart_pointer_payload_traits<EntryImpl>,
                                    ndnSIM::counting_policy_traits > super;

  static TypeId GetTypeId ();

  DmImpl ();

  virtual Ptr<Entry>
  Find(const Name &prefix);
	
	virtual Ptr<Entry>
	FindLongestMatch(const Name &prefix);

  virtual std::vector<boost::tuple<name::Component, int32_t, int32_t, Ptr<Entry> > >
  FindAllComponents(const Name &prefix);

  virtual Ptr<dm::Entry>
  Add (const Name prefix, const Name &mapping, int32_t priority, int32_t weight);

	virtual Ptr<Entry>
  Add (const Ptr<const Name> &prefix,const Ptr<const Name> &mapping, int32_t priority, int32_t weight);

	virtual void
	Remove (const Ptr<const Name> &prefix);

  virtual bool
  Remove (std::string &prefix, std::string &parentPrefix, bool parentHasChild);

	virtual uint32_t
	GetSize () const;
    
  virtual void
  Print (std::ostream &os) const;

	static inline Ptr<Dm>
	GetDm (Ptr<Object> node);
  
protected:
  // inherited from Object class
  virtual void NotifyNewAggregate (); 
  virtual void DoDispose ();   
};

} // namespace dm
} // namespace ndn
} // namespace ns3
#endif // _NDN_DM_IMPL_H_