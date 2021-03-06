/*
Author:  Minsheng Zhang <mzhang4@memphis.edu>
*/

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/names.h"
#include "ns3/packet-socket-factory.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/callback.h"
#include "ns3/node.h"
#include "ns3/core-config.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/callback.h"

#include "ns3/ndn-net-device-face.h"
#include "ns3/ndn-l3-protocol.h"

#include "ns3/ndn-forwarding-strategy.h"
#include "ns3/ndn-fib.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-name.h"
#include "ns3/ndn-content-store.h"
#include "ns3/ndn-dm.h"

#include "ns3/node-list.h"
#include "ns3/data-rate.h"

#include "ns3/ndn-face-container.h"
#include "ndn-dm-stack-helper.h"

#include <limits>
#include <map>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

NS_LOG_COMPONENT_DEFINE ("ndn.DmStackHelper");

namespace ns3 {
namespace ndn {

DmStackHelper::DmStackHelper ()
  : m_limitsEnabled (false)
  , m_needSetDefaultRoutes (false)
{
  m_ndnFactory.         SetTypeId ("ns3::ndn::L3Protocol");
  m_strategyFactory.    SetTypeId ("ns3::ndn::fw::Flooding");
  m_contentStoreFactory.SetTypeId ("ns3::ndn::cs::Lru");
  m_fibFactory.         SetTypeId ("ns3::ndn::fib::Default");
  m_pitFactory.         SetTypeId ("ns3::ndn::pit::Persistent");
  m_dmFactory.          SetTypeId ("ns3::ndn::dm::Default");

  m_netDeviceCallbacks.push_back (std::make_pair (PointToPointNetDevice::GetTypeId (), MakeCallback (&DmStackHelper::PointToPointNetDeviceCallback, this)));
}

DmStackHelper::~DmStackHelper ()
{
}

void
DmStackHelper::SetStackAttributes (const std::string &attr1, const std::string &value1,
                                 const std::string &attr2, const std::string &value2,
                                 const std::string &attr3, const std::string &value3,
                                 const std::string &attr4, const std::string &value4)
{
  if (attr1 != "")
      m_ndnFactory.Set (attr1, StringValue (value1));
  if (attr2 != "")
      m_ndnFactory.Set (attr2, StringValue (value2));
  if (attr3 != "")
      m_ndnFactory.Set (attr3, StringValue (value3));
  if (attr4 != "")
      m_ndnFactory.Set (attr4, StringValue (value4));
}

void
DmStackHelper::SetForwardingStrategy (const std::string &strategy,
                                    const std::string &attr1, const std::string &value1,
                                    const std::string &attr2, const std::string &value2,
                                    const std::string &attr3, const std::string &value3,
                                    const std::string &attr4, const std::string &value4)
{
  if (strategy != "")
    m_strategyFactory.SetTypeId (strategy);
  if (attr1 != "")
      m_strategyFactory.Set (attr1, StringValue (value1));
  if (attr2 != "")
      m_strategyFactory.Set (attr2, StringValue (value2));
  if (attr3 != "")
      m_strategyFactory.Set (attr3, StringValue (value3));
  if (attr4 != "")
      m_strategyFactory.Set (attr4, StringValue (value4));
}

void
DmStackHelper::SetContentStore (const std::string &contentStore,
                              const std::string &attr1, const std::string &value1,
                              const std::string &attr2, const std::string &value2,
                              const std::string &attr3, const std::string &value3,
                              const std::string &attr4, const std::string &value4)
{
  m_contentStoreFactory.SetTypeId (contentStore);
  if (attr1 != "")
      m_contentStoreFactory.Set (attr1, StringValue (value1));
  if (attr2 != "")
      m_contentStoreFactory.Set (attr2, StringValue (value2));
  if (attr3 != "")
      m_contentStoreFactory.Set (attr3, StringValue (value3));
  if (attr4 != "")
      m_contentStoreFactory.Set (attr4, StringValue (value4));
}

void
DmStackHelper::SetPit (const std::string &pitClass,
                     const std::string &attr1, const std::string &value1,
                     const std::string &attr2, const std::string &value2,
                     const std::string &attr3, const std::string &value3,
                     const std::string &attr4, const std::string &value4)
{
  m_pitFactory.SetTypeId (pitClass);
  if (attr1 != "")
      m_pitFactory.Set (attr1, StringValue (value1));
  if (attr2 != "")
      m_pitFactory.Set (attr2, StringValue (value2));
  if (attr3 != "")
      m_pitFactory.Set (attr3, StringValue (value3));
  if (attr4 != "")
      m_pitFactory.Set (attr4, StringValue (value4));
}

void
DmStackHelper::SetDm (const std::string &dmClass,
                      const std::string &attr1, const std::string &value1,
                      const std::string &attr2, const std::string &value2,
                      const std::string &attr3, const std::string &value3,
                      const std::string &attr4, const std::string &value4)
{
  m_dmFactory.SetTypeId (dmClass);
  if (attr1 != "")
      m_dmFactory.Set (attr1, StringValue (value1));
  if (attr2 != "")
      m_dmFactory.Set (attr2, StringValue (value2));
  if (attr3 != "")
      m_dmFactory.Set (attr3, StringValue (value3));
  if (attr4 != "")
      m_dmFactory.Set (attr4, StringValue (value4));
}

void
DmStackHelper::SetFib (const std::string &fibClass,
                     const std::string &attr1, const std::string &value1,
                     const std::string &attr2, const std::string &value2,
                     const std::string &attr3, const std::string &value3,
                     const std::string &attr4, const std::string &value4)
{
  m_fibFactory.SetTypeId (fibClass);
  if (attr1 != "")
      m_fibFactory.Set (attr1, StringValue (value1));
  if (attr2 != "")
      m_fibFactory.Set (attr2, StringValue (value2));
  if (attr3 != "")
      m_fibFactory.Set (attr3, StringValue (value3));
  if (attr4 != "")
      m_fibFactory.Set (attr4, StringValue (value4));
}

void
DmStackHelper::SetDefaultRoutes (bool needSet)
{
  NS_LOG_FUNCTION (this << needSet);
  m_needSetDefaultRoutes = needSet;
}

void
DmStackHelper::EnableLimits (bool enable/* = true*/,
                           Time avgRtt/*=Seconds(0.1)*/,
                           uint32_t avgData/*=1100*/,
                           uint32_t avgInterest/*=40*/)
{
  NS_LOG_INFO ("EnableLimits: " << enable);
  m_limitsEnabled = enable;
  m_avgRtt = avgRtt;
  m_avgDataSize = avgData;
  m_avgInterestSize = avgInterest;
}

Ptr<FaceContainer>
DmStackHelper::Install (const NodeContainer &c) const
{
  Ptr<FaceContainer> faces = Create<FaceContainer> ();
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      faces->AddAll (Install (*i));
    }
  return faces;
}

Ptr<FaceContainer>
DmStackHelper::InstallAll () const
{
  return Install (NodeContainer::GetGlobal ());
}

Ptr<FaceContainer>
DmStackHelper::Install (Ptr<Node> node) const
{
  // NS_ASSERT_MSG (m_forwarding, "SetForwardingHelper() should be set prior calling Install() method");
  Ptr<FaceContainer> faces = Create<FaceContainer> ();

  if (node->GetObject<L3Protocol> () != 0)
    {
      NS_FATAL_ERROR ("DmStackHelper::Install (): Installing "
                      "a NdnStack to a node with an existing Ndn object");
      return 0;
    }

  Ptr<L3Protocol> ndn = m_ndnFactory.Create<L3Protocol> ();

  Ptr<Fib> fib = m_fibFactory.Create<Fib> ();
  ndn->AggregateObject (fib);
  ndn->AggregateObject (m_pitFactory.Create<Pit> ());
  ndn->AggregateObject (m_strategyFactory.Create<ForwardingStrategy> ());
  ndn->AggregateObject (m_contentStoreFactory.Create<ContentStore> ());
  ndn->AggregateObject (m_dmFactory.Create<Dm> ());
  node->AggregateObject (ndn);

  for (uint32_t index=0; index < node->GetNDevices (); index++)
    {
      Ptr<NetDevice> device = node->GetDevice (index);
      // This check does not make sense: LoopbackNetDevice is installed only if IP stack is installed,
      // Normally, ndnSIM works without IP stack, so no reason to check
      // if (DynamicCast<LoopbackNetDevice> (device) != 0)
      //   continue; // don't create face for a LoopbackNetDevice

      Ptr<NetDeviceFace> face;

      for (std::list< std::pair<TypeId, NetDeviceFaceCreateCallback> >::const_iterator item = m_netDeviceCallbacks.begin ();
           item != m_netDeviceCallbacks.end ();
           item++)
        {
          if (device->GetInstanceTypeId () == item->first ||
              device->GetInstanceTypeId ().IsChildOf (item->first))
            {
              face = item->second (node, ndn, device);
              if (face != 0)
                break;
            }
        }
      if (face == 0)
        {
          face = DefaultNetDeviceCallback (node, ndn, device);
        }

      if (m_needSetDefaultRoutes)
        {
          // default route with lowest priority possible
          AddRoute (node, "/", StaticCast<Face> (face), std::numeric_limits<int32_t>::max ());
        }

      face->SetUp ();
      faces->Add (face);
    }

  return faces;
}

void
DmStackHelper::AddNetDeviceFaceCreateCallback (TypeId netDeviceType, DmStackHelper::NetDeviceFaceCreateCallback callback)
{
  m_netDeviceCallbacks.push_back (std::make_pair (netDeviceType, callback));
}

void
DmStackHelper::UpdateNetDeviceFaceCreateCallback (TypeId netDeviceType, NetDeviceFaceCreateCallback callback)
{
  for (NetDeviceCallbackList::iterator i = m_netDeviceCallbacks.begin (); i != m_netDeviceCallbacks.end (); i++)
    {
      if (i->first == netDeviceType)
        {
          i->second = callback;
          return;
        }
    }
}

void
DmStackHelper::RemoveNetDeviceFaceCreateCallback (TypeId netDeviceType, NetDeviceFaceCreateCallback callback)
{
  for (NetDeviceCallbackList::iterator i = m_netDeviceCallbacks.begin (); i != m_netDeviceCallbacks.end (); i++)
    {
      if (i->first == netDeviceType)
        {
          m_netDeviceCallbacks.erase (i);
          return;
        }
    }
}

Ptr<NetDeviceFace>
DmStackHelper::DefaultNetDeviceCallback (Ptr<Node> node, Ptr<L3Protocol> ndn, Ptr<NetDevice> netDevice) const
{
  NS_LOG_DEBUG ("Creating default NetDeviceFace on node " << node->GetId ());

  Ptr<NetDeviceFace> face = CreateObject<NetDeviceFace> (node, netDevice);

  ndn->AddFace (face);
  NS_LOG_LOGIC ("Node " << node->GetId () << ": added NetDeviceFace as face #" << *face);

  return face;
}

Ptr<NetDeviceFace>
DmStackHelper::PointToPointNetDeviceCallback (Ptr<Node> node, Ptr<L3Protocol> ndn, Ptr<NetDevice> device) const
{
  NS_LOG_DEBUG ("Creating point-to-point NetDeviceFace on node " << node->GetId ());

  Ptr<NetDeviceFace> face = CreateObject<NetDeviceFace> (node, device);

  ndn->AddFace (face);
  NS_LOG_LOGIC ("Node " << node->GetId () << ": added NetDeviceFace as face #" << *face);

  if (m_limitsEnabled)
    {
      Ptr<Limits> limits = face->GetObject<Limits> ();
      if (limits == 0)
        {
          NS_FATAL_ERROR ("Limits are enabled, but the selected forwarding strategy does not support limits. Please revise your scenario");
          exit (1);
        }

      NS_LOG_INFO ("Limits are enabled");
      Ptr<PointToPointNetDevice> p2p = DynamicCast<PointToPointNetDevice> (device);
      if (p2p != 0)
        {
          // Setup bucket filtering
          // Assume that we know average data packet size, and this size is equal default size
          // Set maximum buckets (averaging over 1 second)

          DataRateValue dataRate; device->GetAttribute ("DataRate", dataRate);
          TimeValue linkDelay;   device->GetChannel ()->GetAttribute ("Delay", linkDelay);

          NS_LOG_INFO("DataRate for this link is " << dataRate.Get());

          double maxInterestPackets = 1.0  * dataRate.Get ().GetBitRate () / 8.0 / (m_avgDataSize + m_avgInterestSize);
          // NS_LOG_INFO ("Max packets per second: " << maxInterestPackets);
          // NS_LOG_INFO ("Max burst: " << m_avgRtt.ToDouble (Time::S) * maxInterestPackets);
          NS_LOG_INFO ("MaxLimit: " << (int)(m_avgRtt.ToDouble (Time::S) * maxInterestPackets));

          // Set max to BDP
          limits->SetLimits (maxInterestPackets, m_avgRtt.ToDouble (Time::S));
          limits->SetLinkDelay (linkDelay.Get ().ToDouble (Time::S));
        }
    }

  return face;
}


Ptr<FaceContainer>
DmStackHelper::Install (const std::string &nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return Install (node);
}


void
DmStackHelper::AddRoute (Ptr<Node> node, const std::string &prefix, Ptr<Face> face, int32_t metric)
{
  NS_LOG_LOGIC ("[" << node->GetId () << "]$ route add " << prefix << " via " << *face << " metric " << metric);

  Ptr<Fib>  fib  = node->GetObject<Fib> ();

  NameValue prefixValue;
  prefixValue.DeserializeFromString (prefix, MakeNameChecker ());
  fib->Add (prefixValue.Get (), face, metric);
}

void
DmStackHelper::AddRoute (Ptr<Node> node, const std::string &prefix, uint32_t faceId, int32_t metric)
{
  Ptr<L3Protocol>     ndn = node->GetObject<L3Protocol> ();
  NS_ASSERT_MSG (ndn != 0, "Ndn stack should be installed on the node");

  Ptr<Face> face = ndn->GetFace (faceId);
  NS_ASSERT_MSG (face != 0, "Face with ID [" << faceId << "] does not exist on node [" << node->GetId () << "]");

  AddRoute (node, prefix, face, metric);
}

void
DmStackHelper::AddRoute (const std::string &nodeName, const std::string &prefix, uint32_t faceId, int32_t metric)
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  NS_ASSERT_MSG (node != 0, "Node [" << nodeName << "] does not exist");

  Ptr<L3Protocol>     ndn = node->GetObject<L3Protocol> ();
  NS_ASSERT_MSG (ndn != 0, "Ndn stack should be installed on the node");

  Ptr<Face> face = ndn->GetFace (faceId);
  NS_ASSERT_MSG (face != 0, "Face with ID [" << faceId << "] does not exist on node [" << nodeName << "]");

  AddRoute (node, prefix, face, metric);
}

void
DmStackHelper::AddRoute (Ptr<Node> node, const std::string &prefix, Ptr<Node> otherNode, int32_t metric)
{
  for (uint32_t deviceId = 0; deviceId < node->GetNDevices (); deviceId ++)
    {
      Ptr<PointToPointNetDevice> netDevice = DynamicCast<PointToPointNetDevice> (node->GetDevice (deviceId));
      if (netDevice == 0)
        continue;

      Ptr<Channel> channel = netDevice->GetChannel ();
      if (channel == 0)
        continue;

      if (channel->GetDevice (0)->GetNode () == otherNode ||
          channel->GetDevice (1)->GetNode () == otherNode)
        {
          Ptr<L3Protocol> ndn = node->GetObject<L3Protocol> ();
          NS_ASSERT_MSG (ndn != 0, "Ndn stack should be installed on the node");

          Ptr<Face> face = ndn->GetFaceByNetDevice (netDevice);
          NS_ASSERT_MSG (face != 0, "There is no face associated with the p2p link");

          AddRoute (node, prefix, face, metric);

          return;
        }
    }

  NS_FATAL_ERROR ("Cannot add route: Node# " << node->GetId () << " and Node# " << otherNode->GetId () << " are not connected");
}

void
DmStackHelper::AddRoute (const std::string &nodeName, const std::string &prefix, const std::string &otherNodeName, int32_t metric)
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  NS_ASSERT_MSG (node != 0, "Node [" << nodeName << "] does not exist");

  Ptr<Node> otherNode = Names::Find<Node> (otherNodeName);
  NS_ASSERT_MSG (otherNode != 0, "Node [" << otherNodeName << "] does not exist");

  AddRoute (node, prefix, otherNode, metric);
}

} // namespace ndn
} // namespace ns3
