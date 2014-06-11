/*
Author: Minsheng Zhang
*/

#ifndef _NDN_MC_IMPL_H_
#define _NDN_MC_IMPL_H_

#include "ns3/ndn-mc.h"
#include "ns3/ndn-name.h"
#include "../../utils/trie/trie-with-policy.h"
#include "../../utils/trie/counting-policy.h"
#include "ns3/ndn-name.h"
#include "ns3/node.h"
#include "ns3/names.h"
#include "ns3/log.h"

#include <boost/ref.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include <string>

namespace ll = boost::lambda;

namespace ns3{
namespace ndn{
namespace mc{

template<class MC>
class EntryImpl : public Entry
{
public:
  EntryImpl (Ptr<Mc> mc, bool hasChild, const Ptr<const Name> &prefix)
  : Entry (mc, hasChild, prefix)
  , item_(0)
  {}

  void
  SetTrie (typename MC::super::iterator item)
  {
    item_ = item;
  }

  typename MC::super::iterator to_iterator () { return item_; }
  typename MC::super::const_iterator to_iterator () const { return item_; }

private:
  typename MC::super::iterator item_; 
}; // EntryImpl

template<class Policy>
class McImpl : public Mc,
               protected ndnSIM::trie_with_policy< Name,
                                                   ndnSIM::smart_pointer_payload_traits< EntryImpl< McImpl< Policy > >, Entry >,
                                                   Policy > 
{
public:
  typedef ndnSIM::trie_with_policy< Name,
                                    ndnSIM::smart_pointer_payload_traits< EntryImpl< McImpl< Policy > >, Entry >,
                                    Policy > super;

  typedef EntryImpl< McImpl< Policy > > entry;

  static TypeId GetTypeId ();

  McImpl() {};
  virtual ~McImpl() {};

  virtual Ptr<Entry>
  Find(const Name &prefix);

  virtual Ptr<Entry>
  FindLongestMatch (const Name &prefix);

  virtual Ptr<mc::Entry>
  Add (const Name prefix, bool hasChild, const Name mapping, int32_t priority, int32_t weight);

  virtual Ptr<Entry>
  Add (const Ptr<const Name> &prefix, bool hasChild, const Ptr<const Name> &mapping, int32_t priority, int32_t weight);
    
  virtual void
  AddorUpdate(const Ptr<const Name> &prefix, const Ptr<const Name>&mapping, int32_t priority, int32_t weight);

  virtual void
  Remove (const Ptr<const Name> &prefix);
  
  virtual uint32_t
  GetSize () const;
  
  virtual void
  Print (std::ostream &os) const;

  static inline Ptr<Mc>
  GetMc (Ptr<Object> node);

private:
  void
  SetMaxSize (uint32_t maxSize);

  uint32_t
  GetMaxSize () const;

private:
  static LogComponent g_log;
  TracedCallback< Ptr<const Entry> > m_didAddEntry;
};

//////////////////////////////////////////
////////// Implementation ////////////////
//////////////////////////////////////////


template<class Policy>
LogComponent McImpl< Policy >::g_log = LogComponent (("ndn.mc." + Policy::GetName ()).c_str ());

template<class Policy>
TypeId
McImpl< Policy >::GetTypeId ()
{
  static TypeId tid = TypeId (("ns3::ndn::mc::"+Policy::GetName ()).c_str ())
    .SetGroupName ("Ndn")
    .SetParent<Mc> ()
    .AddConstructor< McImpl< Policy > > ()
    .AddAttribute ("MaxSize",
                   "Set maximum number of entries in Mapping Cache. If 0, limit is not enforced",
                   StringValue ("100"),
                   MakeUintegerAccessor (&McImpl< Policy >::GetMaxSize,
                                         &McImpl< Policy >::SetMaxSize),
                   MakeUintegerChecker<uint32_t> ())

    .AddTraceSource ("DidAddEntry", "Trace fired every time entry is successfully added to the cache",
                     MakeTraceSourceAccessor (&McImpl< Policy >::m_didAddEntry))
    ;

  return tid;
}

template<class Policy>
Ptr<Entry>
McImpl< Policy > :: Find(const Name &prefix)
{
  typename super::iterator item = super::find_exact (prefix);

  if (item == super::end ())
    return 0;
  else
    return item->payload ();
}

template<class Policy>
Ptr<Entry>
McImpl< Policy > :: FindLongestMatch (const Name &prefix)
{
  typename super::iterator foundItem, lastItem;
  bool reachLast;
  boost::tie (foundItem, reachLast, lastItem) = super::getTrie().find (prefix);

  if(foundItem == super::getTrie().end())
    return 0;

  if(lastItem->payload()->HasChild() == true && lastItem->payload()->GetPrefix() != prefix)
  {
    return 0;
  }
  else
  {
    while(lastItem->payload()->GetMapping().size() == 0)
    {
      lastItem = lastItem->GetParent();
    }
    return lastItem->payload();
  }
}

template<class Policy>
Ptr<mc::Entry>
McImpl< Policy > :: Add (const Name prefix, bool hasChild, const Name mapping, int32_t priority, int32_t weight)
{
  return Add(Create<Name>(prefix), hasChild, Create<Name>(mapping), priority, weight);
}

template<class Policy>
Ptr<Entry>
McImpl< Policy > :: Add (const Ptr<const Name> &prefix, bool hasChild, const Ptr<const Name> &mapping, int32_t priority, int32_t weight)
{
  //NS_LOG_FUNCTION (this->GetObject<Node> ()->GetId () << boost::cref(*prefix) << numChild << boost::cref(*mapping) << priority << weight);

  // will add entry if doesn't exists, or just return an iterator to the existing entry
  std::pair< typename super::iterator, bool > result = super::insert (*prefix, 0);

  if (result.first != super::end ())
    {
      if (result.second)
        {
          Ptr<EntryImpl<McImpl< Policy > > > newEntry = Create<EntryImpl<McImpl< Policy > > > (this, hasChild, prefix);
          newEntry->SetTrie (result.first);
          result.first->set_payload (newEntry);
        }
      
      //super::modify (result.first, ll::bind (&Entry::SetHasChild, ll::_1, hasChild));
      super::modify (result.first, ll::bind (&Entry::AddOrUpdateMappingMetric, ll::_1, mapping, priority, weight));
      return result.first->payload ();
    }
  else
    return 0;
}
    
//These two next operations are for updates.    
template<class Policy>
void
McImpl< Policy >::AddorUpdate(const Ptr<const Name> &prefix, const Ptr<const Name>&mapping, int32_t priority, int32_t weight)
{}

template<class Policy>
void
McImpl< Policy > :: Remove (const Ptr<const Name> &prefix)
{
  //NS_LOG_FUNCTION (this->GetObject<Node> ()->GetId () << boost::cref(*prefix));

  typename super::iterator mcEntry = super::find_exact (*prefix);
  if (mcEntry != super::end ())
    {
      super::erase (mcEntry);
    }
}

template<class Policy>
uint32_t  
McImpl< Policy > :: GetSize () const
{
  return this->getPolicy().size();
}
  
template<class Policy>
void
McImpl< Policy > :: Print (std::ostream &os) const
{
  typename super::parent_trie::const_recursive_iterator item (super::getTrie ());
  typename super::parent_trie::const_recursive_iterator end (0);
  for (; item != end; item++)
    {
      if (item->payload () == 0) continue;

      os << item->payload ()->GetPrefix () << "\t" << *item->payload () << "\n";
    }
}

template<class Policy>
void
McImpl< Policy >::SetMaxSize(uint32_t maxSize)
{
  this->getPolicy().set_max_size(maxSize);
}

template<class Policy>
uint32_t
McImpl< Policy >::GetMaxSize() const
{
  return this->getPolicy().get_max_size();
}

} // namespace mc
} // namespace ndn
} // namespace ns3

#endif