/*
Author:  Minsheng Zhang <mzhang4@memphis.edu>
*/

#include "trForwarding.h"

#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-fib.h"
#include "ns3/ndn-content-store.h"
#include "ns3/ndn-face.h"
#include "ns3/name-component.h"
#include "ns3/random-variable.h"
#include "ns3/ndn-mc.h"
#include "ns3/ndn-mc-entry.h"

#include "ns3/assert.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/string.h"

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"

#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (trForwarding);

NS_LOG_COMPONENT_DEFINE (trForwarding::GetLogName ().c_str ());

std::string
trForwarding::GetLogName ()
{
  return "ndn.trfw";
}

TypeId trForwarding::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::trForwarding")
    .SetGroupName ("Ndn")
    .SetParent<ForwardingStrategy> ()
    .AddConstructor <trForwarding> ()
    ;
  return tid;
}

trForwarding::trForwarding ()
{
}

void
trForwarding::NotifyNewAggregate ()
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
  if (m_mc == 0)
    {
      m_mc = GetObject<Mc> ();
    }

  Object::NotifyNewAggregate ();
}

void
trForwarding::DoDispose ()
{
  m_pit = 0;
  m_contentStore = 0;
  m_fib = 0;
  m_mc = 0;

  Object::DoDispose ();
}

void
trForwarding::OnInterest (Ptr<Face> inFace,
                                Ptr<Interest> interest)
{
  NS_LOG_FUNCTION (inFace << interest->GetName ());
  m_inInterests (interest, inFace);
  int check = 0;

  Ptr<pit::Entry> pitEntry = m_pit->Lookup (*interest);
  bool similarInterest = true;

  if (pitEntry == 0)
  {
    similarInterest = false;
    pitEntry = m_pit->CreateWithFh (interest);
  }

  if (pitEntry == 0)
  {
    Ptr<mc::Entry> mcEntry = m_mc->FindLongestMatch(interest->GetName());
    if(mcEntry != 0)
    {
      Name forwardinghint = mcEntry->FindBestCandidate();

      interest->SetForwardinghint(forwardinghint);
    } 
    else  // else change the forwarding hint to /dm and generate a new interest
    {
      interest->SetForwardinghint(Name("/dm"));
      check = 1;
    }
    pitEntry = m_pit->CreateByFh(interest);
  }
    
  if (pitEntry != 0)
  {
    DidCreatePitEntry (inFace, interest, pitEntry);
  }
  else
  {
    FailedToCreatePitEntry (inFace, interest);
    return;
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
        
    WillSatisfyPendingInterest (0, pitEntry);
        
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

  if(check == 1)
  {
    Name queryName = Name("/query/mapping") + interest->GetName();
    Ptr<Interest> queryInterest = Create<Interest>();
    queryInterest->SetName(queryName);
    queryInterest->SetInterestLifetime(interest->GetInterestLifetime());
    UniformVariable non(0, std::numeric_limits<uint32_t>::max ());
    queryInterest->SetNonce(non.GetValue());
    queryInterest->SetForwardinghint(Name("/dm"));
            
    Ptr<pit::Entry> queryPitEntry = m_pit->CreateByFh(queryInterest);
    PropagateInterest (inFace, queryInterest, queryPitEntry);
    queryPitEntry->RemoveIncoming (inFace);  
  }
    
  PropagateInterest (inFace, interest, pitEntry);
}


void
trForwarding::OnData (Ptr<Face> inFace,
                            Ptr<Data> data)
{
  NS_LOG_FUNCTION (inFace << data->GetName ());
  m_inData (data, inFace);
            
  // Lookup PIT entry
  // filter for data to deal with the query data
  Name dataName = data->GetName();
  if(dataName.getPrefix(2) == Name("/query/mapping"))
  {
    // operate with the payload and write the corresponding mapping to the mapping cache;
    Ptr<const Packet> payload  = data->GetPayload();

    uint8_t *buffer = new uint8_t [payload->GetSize ()]; 
    payload->CopyData(buffer,payload->GetSize());
    std::stringstream strOStream;
    strOStream.str("");
    strOStream.clear();
    strOStream << buffer;
    delete buffer;
    
    std::string mm = strOStream.str();
    
    std::string size;
    std::getline(strOStream,size,'\n');
    int n;
    std::stringstream(size) >> n;

    std::string mappingContent;
    std::string mappingprefix;
    int numC;
    int numP;
    std::string mappingInfo;
    int p;
    int w;
    Ptr<Name> currentName = Create<Name>();

    for(int i=0;i<n;i++)
    {
      std::getline(strOStream,mappingContent,'\n');
      std::stringstream stream(mappingContent);

      stream >> mappingprefix;
      stream >> numC;
      stream >> numP;
      Ptr<const Name> prefix = Create<const Name>(currentName->append(mappingprefix));

      if(numP == 0)
        m_mc->Add(prefix, numC,0,0,0);
      else
      {
        for(int j=0;j<numP;j++)
        {
          stream >> mappingInfo;
          stream >> p;
          stream >> w;
          m_mc->Add(prefix, numC, Create<const Name>(mappingInfo), p, w);
        }
      }
    }

    if(currentName->size() != dataName.size()-2 && numC != 0)
    {
      m_mc->Add(Create<const Name>(dataName.getPrefix(currentName->size()+1,2)),0, 0, 0, 0);
    }
  }
  else
  {
    Ptr<pit::Entry> pitEntry = m_pit->Find (dataName);
    if (pitEntry == 0)
    {
      bool cached = false;
                  
      if (m_cacheUnsolicitedData || (m_cacheUnsolicitedDataFromApps && (inFace->GetFlags () | Face::APPLICATION)))
      {
        // Optimistically add or update entry in the content store
        cached = m_contentStore->Add (data);
      }
      else
      {
        // Drop data packet if PIT entry is not found
        // (unsolicited data packets should not "poison" content store)
                      
        //drop dulicated or not requested data packet
        m_dropData (data, inFace);
      }
                  
      DidReceiveUnsolicitedData (inFace, data, cached);
      return;
    }
    else
    {
      bool cached = m_contentStore->Add (data);
      DidReceiveSolicitedData (inFace, data, cached);
    }
              
    while (pitEntry != 0)
    {
      // Do data plane performance measurements
      WillSatisfyPendingInterest (inFace, pitEntry);
                  
      // Actually satisfy pending interest
      SatisfyPendingInterest (inFace, data, pitEntry);
                  
      // Lookup another PIT entry
      pitEntry = m_pit->Lookup (*data);
    }
  }
}
    
bool
trForwarding::DoPropagateInterest (Ptr<Face> inFace,
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
}

} // namespace ndn
} // namespace ns3
