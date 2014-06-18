/*
Author: Minsheng Zhang
*/

#include "ndn-mc-impl.h"

//#include "../../utils/trie/random-policy.h"
#include "../../utils/trie/lru-policy-apt.h"
#include "../../utils/trie/lru-policy.h"
//#include "../../utils/trie/fifo-policy.h"
//#include "../../utils/trie/lfu-policy.h"
//#include "../../utils/trie/multi-policy.h"
//#include "../../utils/trie/aggregate-stats-policy.h"

#define NS_OBJECT_ENSURE_REGISTERED_TEMPL(type, templ)  \
  static struct X ## type ## templ ## RegistrationClass \
  {                                                     \
    X ## type ## templ ## RegistrationClass () {        \
      ns3::TypeId tid = type<templ>::GetTypeId ();      \
      tid.GetParent ();                                 \
    }                                                   \
  } x_ ## type ## templ ## RegistrationVariable

namespace ns3 {
namespace ndn {

using namespace ndnSIM;

namespace mc {

template class McImpl<lru_policy_traits>;
template class McImpl<lru_policy_traits_apt>;

NS_OBJECT_ENSURE_REGISTERED_TEMPL(McImpl, lru_policy_traits);
NS_OBJECT_ENSURE_REGISTERED_TEMPL(McImpl, lru_policy_traits_apt);

/*#ifdef DOXYGEN

class Lru : public McImpl<lru_policy_traits> { };

class Fifo : public McImpl<fifo_policy_traits> { };

class Random : public McImpl<random_policy_traits> { };

class Lfu : public McImpl<lfu_policy_traits> { };

#endif*/


} // namespace mc
} // namespace ndn
} // namespace ns3
