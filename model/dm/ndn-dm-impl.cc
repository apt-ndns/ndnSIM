/*
Author: Minsheng
*/

#include "ndn-dm-impl.h"


#include "ns3/ndn-name.h"
#include "ns3/node.h"
#include "ns3/names.h"
#include "ns3/log.h"

#include <boost/ref.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <vector>
#include <utility>
#include "boost/tuple/tuple.hpp"

namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE ("ndn.dm.DmImpl");

namespace ns3{
namespace ndn{
namespace dm{
NS_OBJECT_ENSURE_REGISTERED (DmImpl);

TypeId 
DmImpl::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::dm::Default") // cheating ns3 object system
    .SetParent<Dm> ()
    .SetGroupName ("Ndn")
    .AddConstructor<DmImpl> ()
  ;
  return tid;
}

DmImpl::DmImpl()
{}

void
DmImpl::NotifyNewAggregate ()
{
  Object::NotifyNewAggregate ();
}

void 
DmImpl::DoDispose (void)
{
  clear ();
  Object::DoDispose ();
}

Ptr<Entry>
DmImpl::Find(const Name &prefix)
{
  super::iterator item = super::find_exact (prefix);

  if (item == super::end ())
    return 0;
  else
    return item->payload ();
}

Ptr<Entry>
DmImpl::FindLongestMatch (const Name &prefix)
{
  super::iterator item = super::longest_prefix_match (prefix);

  if (item == super::end ())
    return 0;
  else
    return item->payload ();
}

std::vector<boost::tuple<name::Component, int32_t, int32_t, Ptr<Entry> > >
DmImpl::FindAllComponents(const Name &prefix)
{
  std::vector<boost::tuple<name::Component, int32_t, int32_t, Ptr<Entry> > > records;

  super::iterator foundItem, lastItem;
  bool reachLast;
  boost::tie (foundItem, reachLast, lastItem) = super::getTrie().find (prefix);

  if(foundItem != super::getTrie().end() )
  {
    while(lastItem != 0)
    {
      if(lastItem->key().size()!= 0){
        if(lastItem->payload() != 0)
        {      
          records.push_back(boost::make_tuple(lastItem->key(),lastItem->numChild(), lastItem->payload()->GetMapping().size(), lastItem->payload()));
        }
        else
          records.push_back(boost::make_tuple(lastItem->key(),lastItem->numChild(), 0 , lastItem->payload()));
      }
      
      lastItem = lastItem->GetParent();
    }
  }
  
  return records;
}

Ptr<dm::Entry>
DmImpl::Add (const Name prefix, const Name &mapping, int32_t priority, int32_t weight)
{
  return Add(Create<Name>(prefix), Create<Name>(mapping), priority, weight);
}

Ptr<Entry>
DmImpl::Add (const Ptr<const Name> &prefix,const Ptr<const Name> &mapping, int32_t priority, int32_t weight)
{
  NS_LOG_FUNCTION (this->GetObject<Node> ()->GetId () << boost::cref(*prefix) << boost::cref(*mapping) << priority << weight);

  // will add entry if doesn't exists, or just return an iterator to the existing entry
  std::pair< super::iterator, bool > result = super::insert (*prefix, 0);
  if (result.first != super::end ())
    {
      if (result.second)
        {
          Ptr<EntryImpl> newEntry = Create<EntryImpl> (this, prefix);
          newEntry->SetTrie (result.first);
          result.first->set_payload (newEntry);
        }
      
      super::modify (result.first, ll::bind (&Entry::AddOrUpdateMappingMetric, ll::_1, mapping, priority, weight));
      return result.first->payload ();
    }
  else
    return 0;
}

void
DmImpl::Remove (const Ptr<const Name> &prefix)
{
  NS_LOG_FUNCTION (this->GetObject<Node> ()->GetId () << boost::cref(*prefix));

  super::iterator dmEntry = super::find_exact (*prefix);
  if (dmEntry != super::end ())
    {
      super::erase (dmEntry);
    }
}

void
DmImpl::Remove (std::string &prefix, std::string &parentPrefix, bool parentHasChild)
{
  Ptr<const Name> p = Create<const Name>(prefix);
  super::iterator dmEntry = super::find_exact (*p);
  if (dmEntry != super::end ())
    {
      super::erase (dmEntry);
    }

  super::iterator parentDmEntry = super::longest_prefix_match(*p);
  parentPrefix = parentDmEntry->payload()->GetPrefix().toUri();
  parentHasChild = parentDmEntry->numChild(); 
}

void
DmImpl::Print (std::ostream &os) const
{
  // !!! unordered_set imposes "random" order of item in the same level !!!
  super::parent_trie::const_recursive_iterator item (super::getTrie ());
  super::parent_trie::const_recursive_iterator end (0);
  for (; item != end; item++)
    {
      if (item->payload () == 0) continue;

      os << item->payload ()->GetPrefix () << "\t" << *item->payload () << "\n";
    }
}

uint32_t
DmImpl::GetSize () const
{
	return super::getPolicy ().size ();
}

} // namespace dm
} // namespace ndn
} // namespace ns3