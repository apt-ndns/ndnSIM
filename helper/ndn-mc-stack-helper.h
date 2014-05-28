/*
Author: Minsheng Zhang
*/

#ifndef NDN_MC_STACK_HELPER_H
#define NDN_MC_STACK_HELPER_H

#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/object-factory.h"
#include "ns3/nstime.h"

namespace ns3 {

class Node;

namespace ndn {

class FaceContainer;
class Face;
class NetDeviceFace;
class L3Protocol;

class McStackHelper
{
public:
  
  McStackHelper();

  virtual ~McStackHelper ();

  void
  SetStackAttributes (const std::string &attr1 = "", const std::string &value1 = "",
                      const std::string &attr2 = "", const std::string &value2 = "",
                      const std::string &attr3 = "", const std::string &value3 = "",
                      const std::string &attr4 = "", const std::string &value4 = "");

  void
  SetForwardingStrategy (const std::string &forwardingStrategyClass,
                         const std::string &attr1 = "", const std::string &value1 = "",
                         const std::string &attr2 = "", const std::string &value2 = "",
                         const std::string &attr3 = "", const std::string &value3 = "",
                         const std::string &attr4 = "", const std::string &value4 = "");

  void
  SetMc (const std::string &mcClass,
         const std::string &attr1 = "", const std::string &value1 = "",
         const std::string &attr2 = "", const std::string &value2 = "",
         const std::string &attr3 = "", const std::string &value3 = "",
         const std::string &attr4 = "", const std::string &value4 = "");

  void
  SetContentStore (const std::string &contentStoreClass,
                   const std::string &attr1 = "", const std::string &value1 = "",
                   const std::string &attr2 = "", const std::string &value2 = "",
                   const std::string &attr3 = "", const std::string &value3 = "",
                   const std::string &attr4 = "", const std::string &value4 = "");

  void
  SetPit (const std::string &pitClass,
          const std::string &attr1 = "", const std::string &value1 = "",
          const std::string &attr2 = "", const std::string &value2 = "",
          const std::string &attr3 = "", const std::string &value3 = "",
          const std::string &attr4 = "", const std::string &value4 = "");

  void
  SetFib (const std::string &fibClass,
          const std::string &attr1 = "", const std::string &value1 = "",
          const std::string &attr2 = "", const std::string &value2 = "",
          const std::string &attr3 = "", const std::string &value3 = "",
          const std::string &attr4 = "", const std::string &value4 = "");

  typedef Callback< Ptr<NetDeviceFace>, Ptr<Node>, Ptr<L3Protocol>, Ptr<NetDevice> > NetDeviceFaceCreateCallback;

  void
  AddNetDeviceFaceCreateCallback (TypeId netDeviceType, NetDeviceFaceCreateCallback callback);

  void
  UpdateNetDeviceFaceCreateCallback (TypeId netDeviceType, NetDeviceFaceCreateCallback callback);

  void
  RemoveNetDeviceFaceCreateCallback (TypeId netDeviceType, NetDeviceFaceCreateCallback callback);

  void
  EnableLimits (bool enable = true, Time avgRtt=Seconds(0.1), uint32_t avgData=1100, uint32_t avgInterest=40);

  Ptr<FaceContainer>
  Install (const std::string &nodeName) const;

  Ptr<FaceContainer>
  Install (Ptr<Node> node) const;

  Ptr<FaceContainer>
  Install (const NodeContainer &c) const;

  Ptr<FaceContainer>
  InstallAll () const;

  static void
  AddRoute (const std::string &nodeName, const std::string &prefix, uint32_t faceId, int32_t metric);

  static void
  AddRoute (Ptr<Node> node, const std::string &prefix, uint32_t faceId, int32_t metric);

  static void
  AddRoute (Ptr<Node> node, const std::string &prefix, Ptr<Face> face, int32_t metric);

  static void
  AddRoute (Ptr<Node> node, const std::string &prefix, Ptr<Node> otherNode, int32_t metric);

  static void
  AddRoute (const std::string &nodeName, const std::string &prefix, const std::string &otherNodeName, int32_t metric);

  void
  SetDefaultRoutes (bool needSet);

private:
  Ptr<NetDeviceFace>
  DefaultNetDeviceCallback (Ptr<Node> node, Ptr<L3Protocol> ndn, Ptr<NetDevice> netDevice) const;

  Ptr<NetDeviceFace>
  PointToPointNetDeviceCallback (Ptr<Node> node, Ptr<L3Protocol> ndn, Ptr<NetDevice> netDevice) const;

private:
  McStackHelper (const McStackHelper &);
  McStackHelper &operator = (const McStackHelper &o);

private:
  ObjectFactory m_ndnFactory;
  ObjectFactory m_strategyFactory;
  ObjectFactory m_contentStoreFactory;
  ObjectFactory m_pitFactory;
  ObjectFactory m_fibFactory;
  ObjectFactory m_mcFactory;

  bool     m_limitsEnabled;
  Time     m_avgRtt;
  uint32_t m_avgDataSize;
  uint32_t m_avgInterestSize;
  bool     m_needSetDefaultRoutes;

  typedef std::list< std::pair<TypeId, NetDeviceFaceCreateCallback> > NetDeviceCallbackList;
  NetDeviceCallbackList m_netDeviceCallbacks;
};

} // namespace ndn
} // namespace ns3

#endif /* NDN_STACK_HELPER_H */