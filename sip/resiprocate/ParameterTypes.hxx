#ifndef ParameterTypes_hxx
#define ParameterTypes_hxx

#include <sipstack/DataParameter.hxx>
#include <sipstack/IntegerParameter.hxx>
#include <sipstack/FloatParameter.hxx>
#include <sipstack/ExistsParameter.hxx>
#include <sipstack/ParameterTypeEnums.hxx>
#include <sipstack/Symbols.hxx>

#include <util/Data.hxx>

namespace Vocal2
{

template <typename ParameterTypes::Type T>
class ParameterType
{
};

class ParameterType<ParameterTypes::transport>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::transport] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::transport] = Symbols::transport;
      }
};
extern ParameterType<ParameterTypes::transport> p_transport;

class ParameterType<ParameterTypes::user>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::user] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::user] = Symbols::user;
      }
};
extern ParameterType<ParameterTypes::user> p_user;

class ParameterType<ParameterTypes::method>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::method] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::method] = Symbols::method;
      }
};
extern ParameterType<ParameterTypes::method> p_method;

class ParameterType<ParameterTypes::ttl>
{
   public:
      typedef IntegerParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::ttl] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::ttl] = Symbols::ttl;
      }
};
extern ParameterType<ParameterTypes::ttl> p_ttl;

class ParameterType<ParameterTypes::maddr>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::maddr] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::maddr] = Symbols::maddr;
      }
};
extern ParameterType<ParameterTypes::maddr> p_maddr;

class ParameterType<ParameterTypes::lr>
{
   public:
      typedef ExistsParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::lr] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::lr] = Symbols::lr;
      }
};
extern ParameterType<ParameterTypes::lr> p_lr;

class ParameterType<ParameterTypes::q>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::q] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::q] = Symbols::q;
      }
};
extern ParameterType<ParameterTypes::q> p_q;

class ParameterType<ParameterTypes::purpose>
{
   public:
      typedef FloatParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::purpose] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::purpose] = Symbols::purpose;
      }
};
extern ParameterType<ParameterTypes::purpose> p_purpose;

class ParameterType<ParameterTypes::expires>
{
   public:
      typedef IntegerParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::expires] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::expires] = Symbols::expires;
      }
};
extern ParameterType<ParameterTypes::expires> p_expires;

class ParameterType<ParameterTypes::handling>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::handling] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::handling] = Symbols::handling;
      }
};
extern ParameterType<ParameterTypes::handling> p_handling;

class ParameterType<ParameterTypes::tag>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::tag] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::tag] = Symbols::tag;
      }
};
extern ParameterType<ParameterTypes::tag> p_tag;

class ParameterType<ParameterTypes::toTag>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::toTag] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::toTag] = Symbols::toTag;
      }
};
extern ParameterType<ParameterTypes::toTag> p_toTag;

class ParameterType<ParameterTypes::fromTag>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::fromTag] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::fromTag] = Symbols::fromTag;
      }
};
extern ParameterType<ParameterTypes::fromTag> p_fromTag;

class ParameterType<ParameterTypes::duration>
{
   public:
      typedef IntegerParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::duration] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::duration] = Symbols::duration;
      }
};
extern ParameterType<ParameterTypes::duration> p_duration;

class ParameterType<ParameterTypes::branch>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::branch] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::branch] = Symbols::branch;
      }
};
extern ParameterType<ParameterTypes::branch> p_branch;

class ParameterType<ParameterTypes::received>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::received] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::received] = Symbols::received;
      }
};
extern ParameterType<ParameterTypes::received> p_received;

class ParameterType<ParameterTypes::comp>
{
   public:
      typedef DataParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::comp] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::comp] = Symbols::comp;
      }
};
extern ParameterType<ParameterTypes::comp> p_com;

class ParameterType<ParameterTypes::rport>
{
   public:
      typedef IntegerParameter Type;

      ParameterType()
      {
         ParameterTypes::ParameterFactories[ParameterTypes::rport] = Type::decode;
         ParameterTypes::ParameterNames[ParameterTypes::rport] = Symbols::rport;
      }
};
extern ParameterType<ParameterTypes::rport> p_rport;
 
}

#endif
