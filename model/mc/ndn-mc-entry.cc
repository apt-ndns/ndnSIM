/*
Author: Minsheng
*/

#include "ndn-mc-entry.h"
#include "ndn-mc.h"

#include "ns3/ndn-name.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include <boost/ref.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <ctime>
#include <utility>

namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE ("ndn.mc.Entry");

namespace ns3{
namespace ndn{
namespace mc{

struct MappingMetricByMapping
{
  typedef MappingMetricContainer::type::index<i_mapping>::type
  type;
};

struct MappingMetricByMetric
{
  typedef MappingMetricContainer::type::index<i_metric>::type
  type;
};

void
Entry::AddOrUpdateMappingMetric (const Ptr<const Name> &mapping, int32_t priority, int32_t weight)
{
  NS_LOG_FUNCTION (this);

  if(mapping == 0) 
    return;

  MappingMetricByMapping::type::iterator record = m_mapping.get<i_mapping> ().find (*mapping);
  if (record == m_mapping.get<i_mapping> ().end ())
    {
      m_mapping.insert (MappingMetric (mapping, priority, weight));
    }
  else
  {
    m_mapping.modify (record,
      ll::bind (&MappingMetric::SetPriority, ll::_1, priority));

    m_mapping.modify (record,
      ll::bind (&MappingMetric::SetWeight, ll::_1, weight));  
  }
}

const Name&
Entry::FindBestCandidate ()
{
  Ptr<const Name> result;
  if(m_mapping.size() == 0) throw Entry::NoMapping ();
  
  MappingMetricByMetric::type &mappingByMetric = m_mapping.get<i_metric>();
  int32_t priority = (--mappingByMetric.end())->GetPriority();
  int32_t sumWeight = 0;
  int32_t firstWeight = 0;

  std::pair<MappingMetricByMetric::type::iterator, MappingMetricByMetric::type::iterator> records = mappingByMetric.equal_range(priority);
  firstWeight = records.first->GetWeight();

  for(MappingMetricByMetric::type::iterator it = records.first; it != records.second; ++it)
  {
    sumWeight += it->GetWeight();
  }

  std::srand(std::time(0));
  int32_t targetWeight = rand()%sumWeight;

  if(sumWeight != firstWeight)
  {
    int32_t tempSum = 0;
    for(MappingMetricByMetric::type::iterator it = records.first; it != records.second; ++it)
    {
      if(targetWeight <= tempSum + it->GetWeight())
      {
        result = it->GetMappingPtr();
        break;
      }
      else
        tempSum += it->GetWeight();
    }
  }
  else 
    result = records.first->GetMappingPtr();

  return *result;
}

std::ostream& operator<< (std::ostream& os, const Entry &entry)
{
  os << *entry.m_prefix;
  if(entry.m_hasChild == true)
	 os << " has child ";
  else
    os <<  " not has child ";

  for (MappingMetricContainer::type::index<i_metric>::type::iterator metric =
          entry.m_mapping.get<i_metric> ().begin ();
       metric != entry.m_mapping.get<i_metric> ().end ();
       metric++)
    {
      if (metric != entry.m_mapping.get<i_metric> ().begin ())
        os << ", ";

      os << *metric;
    }
  return os;
}

std::ostream& operator<< (std::ostream& os, const MappingMetric &metric)
{
  os << *metric.m_mapping << "(" << metric.m_priority << ","<< metric.m_weight << ")";
  return os;
}

} // namespace dm
} // namespace ndn
} // namespace ns3