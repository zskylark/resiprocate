#if !defined(UDPTRANSPORT_HXX)
#define UDPTRANSPORT_HXX

#include <vector>

#include "sip2/sipstack/Security.hxx"
#include "sip2/util/Timer.hxx"

namespace Vocal2
{

class Pidf;

class TuIM
{
   private:
      class Buddy;
      
public:
      class Callback
      {
         public:
            virtual void receivedPage(const Data& msg, 
                                      const Uri& from, 
                                      const Data& signedBy,  
                                      Security::SignatureStatus sigStatus,
                                      bool wasEncryped  ) = 0; 
            virtual void sendPageFailed(const Uri& dest, int respNumber ) =0;
            virtual void receivePageFailed(const Uri& sender) =0;
            virtual void registrationFailed(const Uri& dest, int respNumber ) =0;
            virtual void registrationWorked(const Uri& dest ) =0;
            virtual void presenseUpdate(const Uri& dest, bool open, const Data& status ) =0;
            virtual ~Callback();
      };
      
      TuIM(SipStack* stack, 
           const Uri& aor, 
           const Uri& contact,
           Callback* callback);

      ///
      void setOutboundProxy( const Uri& uri );
      void setUANameProxy( const Data& name );
      
      bool haveCerts( bool sign, const Data& encryptFor );
      void sendPage(const Data& text, const Uri& dest, bool sign, const Data& encryptFor );

      void process();

      // Registration management 
      void registerAor( const Uri& uri, const Data& password = Data::Empty );

      // Buddy List management
      int getNumBuddies() const;
      const Uri getBuddyUri(const int index);
      const Data getBuddyGroup(const int index);
      bool getBuddyStatus(const int index, Data* status=NULL);
      void addBuddy( const Uri& uri, const Data& group );
      void removeBuddy( const Uri& name);

      // Presense management
      void setMyPresense( const bool open, const Data& status = Data::Empty );

   private:
      void processRequest(SipMessage* msg);
      void processMessageRequest(SipMessage* msg);
      void processSubscribeRequest(SipMessage* msg);
      void processNotifyRequest(SipMessage* msg);

      void processResponse(SipMessage* msg);
      void processRegisterResponse(SipMessage* msg);
      void processSubscribeResponse(SipMessage* msg, Buddy& buddy );

      void sendNotify(Dialog* dialog);
      
      void setOutbound( SipMessage& msg );
      
      Callback* mCallback;
      SipStack* mStack;
      Uri mAor;
      Uri mContact;
      //Data mPassword;

      // people I subscibe too 
      class Buddy
      {
         public:
            Uri uri;
            Data group;
            Dialog* presDialog; 
            UInt64 mNextTimeToSubscribe;
            bool online;
            Data status;
      };
      vector<Buddy> mBuddy;
	  typedef vector<Buddy>::iterator BuddyIterator;

      // people who subscribe to me 
      vector<Dialog*> mSubscribers;
      Pidf* mPidf;
      
      // registration information
      Dialog mRegistrationDialog;
      UInt64 mNextTimeToRegister;
      Data   mRegistrationPassword;

      Uri mOutboundProxy;
      Data mUAName;
};

}

#endif


/* ====================================================================
 * The Vovida Software License, Version 1.0 
 */

