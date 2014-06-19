/*
Author: Minsheng Zhang
*/

#include "dmForwarding.h"

#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-fib.h"
#include "ns3/ndn-content-store.h"
#include "ns3/ndn-face.h"
#include "ns3/ndn-dm.h"
#include "ns3/ndn-dm-entry.h"

#include "ns3/assert.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/string.h"

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"

#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (dmForwarding);

NS_LOG_COMPONENT_DEFINE (dmForwarding::GetLogName ().c_str ());

std::string
dmForwarding::GetLogName ()
{
  return "ndn.fw.dmForwarding";
}

TypeId dmForwarding::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::dmForwarding")
    .SetGroupName ("Ndn")
    .SetParent<ForwardingStrategy> ()
    .AddConstructor <dmForwarding> ()
    .AddAttribute ("dmForwarding::Filename", "Output file.",
                   StringValue (""),
                   MakeStringAccessor (&dmForwarding::m_fileName),
                   MakeStringChecker())
    ;
  return tid;
}

dmForwarding::dmForwarding ()
{
  m_cacheMissInterest = 0;
}

dmForwarding::~dmForwarding ()
{
}

void
dmForwarding::NotifyNewAggregate ()
{
  if (m_pit == 0)
    {
      m_pit = GetObject<Pit> ();
    }
  if (m_fib == 0)
    {
      m_fib = GetObject<Fib> ();
    }
  if (m_contentStore == 0)
    {
      m_contentStore = GetObject<ContentStore> ();
    }
  if (m_dm == 0)
    {
      m_dm = GetObject<Dm> ();
    }

  Object::NotifyNewAggregate ();
}

void
dmForwarding::DoDispose ()
{
  if(m_fileName != "")
  {
    Ptr<Node> node = this->GetObject<Node> ();

    std::ofstream output;
    output.open(m_fileName,std::ios::app);
    if(!output.is_open())
      std::cerr << "Cannot open the file: " << m_fileName << std::endl;
    else
    {
      output << node->GetId() << " Normal Interest: " << m_normalInterest << std::endl;
      output << node->GetId() << " Cache Miss Interest: " << m_cacheMissInterest << std::endl;
      output << node->GetId() << " Query Interest: " << m_queryInterest << std::endl;
    }
    output.close();
  }

  m_pit = 0;
  m_contentStore = 0;
  m_fib = 0;
  m_dm = 0;

  Object::DoDispose ();
}
    
void
dmForwarding::OnInterest (Ptr<Face> inFace,
                                Ptr<Interest> interest)
{ 
  NS_LOG_FUNCTION (inFace << interest->GetName ());
  m_inInterests (interest, inFace);
  // a filter to do with the query mapping interest.
  Name interestName = interest->GetName();
  if( interestName.size()>2&&interestName.getPrefix(2) == Name("/query/mapping"))
  {
    m_queryInterest++;
    Name queryName = interestName.getPrefix(interestName.size()-2,2);
    std::vector<boost::tuple<name::Component, int, int, Ptr<dm::Entry> > > records = m_dm->FindAllComponents(queryName);

    std::string mappingString = boost::lexical_cast<std::string>(records.size()) + "\n";
    name::Component prefix;
    int numChild;
    int nMapping;
    Ptr<dm::Entry> entry;

    for(int i=records.size()-1; i>=0;i--)
    {
      boost::tie (prefix, numChild, nMapping, entry) = records[i];

      mappingString += prefix.toUri() + " " + boost::lexical_cast<std::string>(numChild) + " " + boost::lexical_cast<std::string>(nMapping) + " ";
      if(entry != 0)
      {
        dm::MappingMetricContainer::type mapping = entry->GetMapping();
        for(dm::MappingMetricContainer::type::iterator metric= mapping.begin();metric!=mapping.end();++metric)
        {
          if (metric != mapping.begin ())
            mappingString += "  ";
          mappingString += metric->GetMappingPtr()->toUri() + " " + boost::lexical_cast<std::string>(metric->GetPriority()) + " " + boost::lexical_cast<std::string>(metric->GetWeight()) + " ";
        }
        mappingString += "\n";
      }
      else
        mappingString += "\n";
    }
    
    Ptr<ndn::Data> mappingdata = Create<ndn::Data>();
    mappingdata->SetName(interestName);
           
    Ptr<Packet> pkt = Create<Packet> ((uint8_t*) (mappingString.c_str()),mappingString.length()+1);
    mappingdata->SetPayload(pkt);
        
    inFace->SendData(mappingdata); 
    m_outData (mappingdata, 0, inFace);

    return;
  }
  else
  {
    Ptr<pit::Entry> pitEntry = m_pit->Lookup (*interest);
    bool similarInterest = true;
    if (pitEntry == 0)
    {
      similarInterest = false;
      pitEntry = m_pit->CreateWithFh (interest);
      if(pitEntry == 0)
      {
        m_cacheMissInterest++;
        interest->SetForwardinghint(m_dm->FindLongestMatch(interest->GetName())->FindBestCandidate());
        pitEntry = m_pit->CreateByFh (interest);
      }
      else
      {
        m_normalInterest++;
      }

      if (pitEntry != 0)
      {
        DidCreatePitEntry (inFace, interest, pitEntry);
      }
      else
      {
        FailedToCreatePitEntry (inFace, interest);//Commented out by Yaoqing Liu
        return;
      }
    }
            
    bool isDuplicated = true;
    if (!pitEntry->IsNonceSeen (interest->GetNonce ()))
    {
      pitEntry->AddSeenNonce (interest->GetNonce ());
      isDuplicated = false;
    }
            
    if (isDuplicated)
    {
      DidReceiveDuplicateInterest (inFace, interest, pitEntry);
      return;
    }
            
    Ptr<Data> contentObject;
    contentObject = m_contentStore->Lookup (interest);
    if (contentObject != 0)
    {
      FwHopCountTag hopCountTag;
      if (interest->GetPayload ()->PeekPacketTag (hopCountTag))
      {
        contentObject->GetPayload ()->AddPacketTag (hopCountTag);
      }
                
      pitEntry->AddIncoming (inFace/*, Seconds (1.0)*/);
                
      // Do data plane performance measurements
      WillSatisfyPendingInterest (0, pitEntry);
                
      // Actually satisfy pending interest
      SatisfyPendingInterest (0, contentObject, pitEntry);
      return;
    }
            
    if (similarInterest && ShouldSuppressIncomingInterest (inFace, interest, pitEntry))
    {
      pitEntry->AddIncoming (inFace/*, interest->GetInterestLifetime ()*/);
      // update PIT entry lifetime
      pitEntry->UpdateLifetime (interest->GetInterestLifetime ());
                
      // Suppress this interest if we're still expecting data from some other face
      NS_LOG_DEBUG ("Suppress interests");
      m_dropInterests (interest, inFace);
                
      DidSuppressSimilarInterest (inFace, interest, pitEntry);
      return;
    }
            
    if (similarInterest)
    {
      DidForwardSimilarInterest (inFace, interest, pitEntry);
    }
      PropagateInterest (inFace, interest, pitEntry); 
  }
}

bool
dmForwarding::DoPropagateInterest (Ptr<Face> inFace,
                                Ptr<const Interest> interest,
                                Ptr<pit::Entry> pitEntry)
{
    NS_LOG_FUNCTION (this);
    
    int propagatedCount = 0;
    
    BOOST_FOREACH (const fib::FaceMetric &metricFace, pitEntry->GetFibEntry ()->m_faces.get<fib::i_metric> ())
    {
        NS_LOG_DEBUG ("Trying " << boost::cref(metricFace));
        if (metricFace.GetStatus () == fib::FaceMetric::NDN_FIB_RED) // all non-read faces are in the front of the list
            break;
        
        if (!TrySendOutInterest (inFace, metricFace.GetFace (), interest, pitEntry))
        {
            continue;
        }
        propagatedCount++;
    }
    
    NS_LOG_INFO ("Propagated to " << propagatedCount << " faces");
    return propagatedCount > 0;
    return false;
}


} // namespace ndn
} // namespace ns3
