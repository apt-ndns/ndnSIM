/*
Author: Minsheng Zhang
*/

#ifndef _NDN_MC_ENTRY_H_
#define _NDN_MC_ENTRY_H_

#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/ndn-name.h"
#include "ns3/ndn-limits.h"
#include "ns3/traced-value.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace ns3{
namespace ndn{

class Mc;
namespace mc{

class MappingMetric
{
public:
  MappingMetric (const Ptr<const Name> &mapping, int32_t priority, int32_t weight)
  :m_mapping (mapping)
  ,m_priority (priority)
  ,m_weight (weight)
  {}

  void
  SetPriority(int32_t priority)
  {
    m_priority = priority;
  }

  int32_t
  GetPriority() const
  {
    return m_priority;
  }

  void
  SetWeight(int32_t weight)
  {
    m_weight = weight;
  }

  int32_t
  GetWeight() const
  {
    return m_weight;
  }

  Ptr<const Name>   
  GetMappingPtr() const
  {
    return m_mapping;
  }

  const Name
  GetMapping() const
  {
    return *m_mapping;
  }

private:
  friend std::ostream& operator<< (std::ostream& os, const MappingMetric &mappingMetric);
     
private:
  const Ptr<const Name> m_mapping;
  int32_t m_priority;
  int32_t m_weight;
}; 

class i_mapping{};
class i_metric{};

struct MappingMetricContainer
{
  typedef boost::multi_index::multi_index_container<
    MappingMetric,
    boost::multi_index::indexed_by<
      boost::multi_index::ordered_unique<
        boost::multi_index::tag<i_mapping>,
        boost::multi_index::const_mem_fun<MappingMetric,const Name,&MappingMetric::GetMapping>
      >,

    boost::multi_index::ordered_non_unique<
      boost::multi_index::tag<i_metric>,
        boost::multi_index::composite_key<
          MappingMetric,
          boost::multi_index::const_mem_fun<MappingMetric,int32_t,&MappingMetric::GetPriority>,
          boost::multi_index::const_mem_fun<MappingMetric,int32_t,&MappingMetric::GetWeight>
        >  
      >
    >
  > type;
}; 

class Entry : public SimpleRefCount<Entry>
{
//public:
//  typedef Entry base_type;
     
public:
  class NoMapping {};

  Entry (Ptr<Mc> mc, bool hasChild, const Ptr<const Name> &prefix)
  : m_mc (mc)
  , m_hasChild(hasChild)
  , m_prefix (prefix)
  {
  }

  const Name&
  GetPrefix () const 
  { 
    return *m_prefix; 
  }

  bool 
  HasChild() const
  {
  	return m_hasChild;
  }

  void
  SetHasChild(bool hasChild)
  {
  	m_hasChild = hasChild;
  }

  MappingMetricContainer::type
  GetMapping () const 
  { 
  	return m_mapping;
  }

  void
  AddOrUpdateMappingMetric (const Ptr<const Name> &mapping, int32_t priority, int32_t weight);

  const Name&
  FindBestCandidate ();
     
  void
  RemoveMapping (const Ptr<const Name> &mapping)
  {
    RemoveMapping(*mapping);
  }

  void
  RemoveMapping (Name mapping)
  {
    m_mapping.erase(mapping);
  }

  void
  RemoveAll();

  Ptr<Mc>
  GetMc()
  {
    return m_mc;
  }

private:
  friend std::ostream& operator<< (std::ostream& os, const Entry &entry);

public:
  Ptr<Mc> m_mc;
  bool m_hasChild;
  Ptr<const Name> m_prefix;
  MappingMetricContainer::type m_mapping;
}; 

std::ostream& operator<< (std::ostream& os, const Entry &entry);
std::ostream& operator<< (std::ostream& os, const MappingMetric &metric);
} // namespace mc
} // namespace ndn
} // namespace ns3
#endif 