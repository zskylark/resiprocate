#if defined(USE_SSL)

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/pkcs7.h>
#include <openssl/x509v3.h>

#include "sip2/sipstack/SipStack.hxx"
#include "sip2/sipstack/Security.hxx"
#include "sip2/sipstack/Contents.hxx"
#include "sip2/sipstack/Pkcs7Contents.hxx"
#include "sip2/sipstack/PlainContents.hxx"
#include "sip2/util/Logger.hxx"
#include "sip2/util/Random.hxx"
#include "sip2/util/DataStream.hxx"


using namespace Vocal2;

#define VOCAL_SUBSYSTEM Subsystem::SIP



Security::Security()
{
   privateKey = NULL;
   publicCert = NULL;
   certAuthorities = NULL;
   
   static bool initDone=false;
   if ( !initDone )
   {
      initDone = true;
      
      OpenSSL_add_all_algorithms();
      ERR_load_crypto_strings();

      Random::initialize();
   }
}


Security::~Security()
{
}
  

Data 
Security::getPath( const Data& dirPath, const Data& file )
{
   Data path = dirPath;
   
   if ( path.empty() )
   {
#ifdef WIN32
      assert(0);
#else
      char* v = getenv("SIP");
      if (v)
      {
         path = Data(v);
      }
      else
      {  
          v = getenv("HOME");
          if ( v )
          {
             path = Data(v);
             path += Data("/.sip");
          }
          else
          {
             ErrLog( << "Environment variobal HOME is not set" );
             path = "/etc/sip";
          }
      }
#endif
   }
   
#ifdef WIN32
   path += Data("\\");
#else
   path += Data("/");
#endif

   assert( !file.empty() );
   
   path += file;

   DebugLog( << "Using file path " << path );
   
   return path;
}


bool 
Security::loadAllCerts( const Data& password, const Data&  dirPath )
{
   bool ok = true;
   ok = loadRootCerts( getPath( dirPath, Data("root.pem")) ) ? ok : false;
   ok = loadMyPublicCert( getPath( dirPath, Data("id.pem")) ) ? ok : false;
   ok = loadMyPrivateKey( password, getPath(dirPath,Data("id_key.pem") )) ? ok : false;

   return ok;
}
     

bool 
Security::loadMyPublicCert( const Data&  filePath )
{
   assert( !filePath.empty() );
   
   FILE* fp = fopen(filePath.c_str(),"r");
   if ( !fp )
   {
      ErrLog( << "Could not read public cert from " << filePath );
      return false;
   }
   
   publicCert = PEM_read_X509(fp,NULL,NULL,NULL);
   if (!publicCert)
   {
      ErrLog( << "Error reading contents of public cert file " << filePath );
      return false;
   }
   
   InfoLog( << "Loaded public cert from " << filePath );
   
   return true;
}


bool 
Security::loadRootCerts(  const Data& filePath )
{ 
   assert( !filePath.empty() );
   
   certAuthorities = X509_STORE_new();
   assert( certAuthorities );
   
   if ( X509_STORE_load_locations(certAuthorities,filePath.c_str(),NULL) != 1 )
   {  
      ErrLog( << "Error reading contents of root cert file " << filePath );
      return false;
   }
   
   InfoLog( << "Loaded public CAs from " << filePath );

   return true;
}


bool 
Security::loadMyPrivateKey( const Data& password, const Data&  filePath )
{
   assert( !filePath.empty() );
   
   FILE* fp = fopen(filePath.c_str(),"r");
   if ( !fp )
   {
      ErrLog( << "Could not read private key from " << filePath );
      return false;
   }
   
   //DebugLog( "password is " << password );
   
   privateKey = PEM_read_PrivateKey(fp,NULL,NULL,(void*)password.c_str());
   if (!privateKey)
   {
      ErrLog( << "Error reading contents of private key file " << filePath );

      while (1)
      {
         const char* file;
         int line;
         
         unsigned long code = ERR_get_error_line(&file,&line);
         if ( code == 0 )
         {
            break;
         }

         char buf[256];
         ERR_error_string_n(code,buf,sizeof(buf));
         ErrLog( << buf  );
         DebugLog( << "Error code = " << code << " file=" << file << " line=" << line );
      }
      
      return false;
   }
   
   InfoLog( << "Loaded private key from << " << filePath );
   
   return true;
}



Pkcs7Contents* 
Security::sign( Contents* bodyIn )
{
   assert( bodyIn );

   int flags;
   flags |= PKCS7_BINARY;
   
   Data bodyData;
   oDataStream strm(bodyData);
#if 1
   strm << "Content-Type: " << bodyIn->getType() << Symbols::CRLF;
   strm << Symbols::CRLF;
#endif
   bodyIn->encode( strm );
   strm.flush();
   
   DebugLog( << "body data to sign is <" << bodyData << ">" );
      
   const char* p = bodyData.data();
   int s = bodyData.size();
   
   BIO* in;
   in = BIO_new_mem_buf( (void*)p,s);
   assert(in);
   DebugLog( << "ceated in BIO");
    
   BIO* out;
   out = BIO_new(BIO_s_mem());
   assert(out);
   DebugLog( << "created out BIO" );
     
   STACK_OF(X509)* chain=NULL;
   chain = sk_X509_new_null();
   assert(chain);

   DebugLog( << "checking" );
   assert( publicCert );
   assert( privateKey );
   
   int i = X509_check_private_key(publicCert, privateKey);
   DebugLog( << "checked cert and key ret=" << i  );
   
   PKCS7* pkcs7 = PKCS7_sign( publicCert, privateKey, chain, in, flags);
   if ( !pkcs7 )
   {
       ErrLog( << "Error creating PKCS7 signing object" );
      return NULL;
   }
    DebugLog( << "created PKCS7 sign object " );

#if 0
   if ( SMIME_write_PKCS7(out,pkcs7,in,0) != 1 )
   {
      ErrLog( << "Error doind S/MIME write of signed object" );
      return NULL;
   }
   DebugLog( << "created SMIME write object" );
#else
   i2d_PKCS7_bio(out,pkcs7);
#endif
   BIO_flush(out);
   
   char* outBuf=NULL;
   long size = BIO_get_mem_data(out,&outBuf);
   assert( size > 0 );
   
   Data outData(outBuf,size);
  
   InfoLog( << "Signed body size is <" << outData.size() << ">" );
   //InfoLog( << "Signed body is <" << outData.escaped() << ">" );

   Pkcs7Contents* outBody = new Pkcs7Contents( outData );
   assert( outBody );

   return outBody;
}


Pkcs7Contents* 
Security::encrypt( Contents* bodyIn, const Data& recipCertName )
{
   assert( bodyIn );
   
   int flags = 0 ;  
   flags |= PKCS7_BINARY;
   
   Data bodyData;
   oDataStream strm(bodyData);
#if 1
   strm << "Content-Type: " << bodyIn->getType() << Symbols::CRLF;
   strm << Symbols::CRLF;
#endif
   bodyIn->encode( strm );
   strm.flush();
   
   DebugLog( << "body data to encrypt is <" << bodyData << ">" );
      
   const char* p = bodyData.data();
   int s = bodyData.size();
   
   BIO* in;
   in = BIO_new_mem_buf( (void*)p,s);
   assert(in);
   DebugLog( << "ceated in BIO");
    
   BIO* out;
   out = BIO_new(BIO_s_mem());
   assert(out);
   DebugLog( << "created out BIO" );

   InfoLog( << "target cert name is " << recipCertName );
   X509* cert = publicCert; // !cj! this is the wrong one - need to load right one
   
   STACK_OF(X509) *certs;
   certs = sk_X509_new_null();
   assert(certs);
   assert( cert );
   sk_X509_push(certs, cert);
   
   EVP_CIPHER* cipher = EVP_des_ede3_cbc();
   assert( cipher );
   
   PKCS7* pkcs7 = PKCS7_encrypt( certs, in, cipher, flags);
   if ( !pkcs7 )
   {
       ErrLog( << "Error creating PKCS7 encrypt object" );
      return NULL;
   }
    DebugLog( << "created PKCS7 encrypt object " );

#if 0
   if ( SMIME_write_PKCS7(out,pkcs7,in,0) != 1 )
   {
      ErrLog( << "Error doind S/MIME write of signed object" );
      return NULL;
   }
   DebugLog( << "created SMIME write object" );
#else
   i2d_PKCS7_bio(out,pkcs7);
#endif
   BIO_flush(out);
   
   char* outBuf=NULL;
   long size = BIO_get_mem_data(out,&outBuf);
   assert( size > 0 );
   
   Data outData(outBuf,size);
  
   InfoLog( << "Encrypted body size is <" << outData.size() << ">" );
   //InfoLog( << "Encrypted body is <" << outData.escaped() << ">" );

   Pkcs7Contents* outBody = new Pkcs7Contents( outData );
   assert( outBody );

   return outBody;
   
}


Security::OperationNeeded 
Security::operationNeeded( Contents* inBody )
{ 
   assert( inBody );

   
   Pkcs7Contents* body = dynamic_cast<Pkcs7Contents*>( inBody );
   if ( !body )
   {
      return isPlain;
   }
 
   const char* p = body->text().data();
   int   s = body->text().size();
   
   BIO* in;
   in = BIO_new_mem_buf( (void*)p,s);
   assert(in);
   DebugLog( << "ceated in BIO");
    
#if 0
   BIO* pkcs7Bio=NULL;
   PKCS7* pkcs7 = SMIME_read_PKCS7(in,&pkcs7Bio);
   if ( !pkcs7 )
   {
      ErrLog( << "Problems doing SMIME_read_PKCS7" );
      return NULL;
   }
   if ( pkcs7Bio )
   {
      ErrLog( << "Can not deal with mutlipart mime version stuff " );
      return NULL;
   }  
#else
   PKCS7* pkcs7 = d2i_PKCS7_bio(in, NULL);
   if ( !pkcs7 )
   {
      ErrLog( << "Problems doing decode of PKCS7 object" );
      return isBad;
   }
   BIO_flush(in);
#endif
   
   int type=OBJ_obj2nid(pkcs7->type);
   switch (type)
   {
      case NID_pkcs7_signed:
      {
         return isSigned;
      }
      
      case NID_pkcs7_enveloped:
      {
         return isEncrypted;
      }
      
      default:
         return isUnknown;
   }

   assert(0);
   return isUnknown;
}


Contents* 
Security::uncode( Pkcs7Contents* sBody, Data* signedBy )
{
   Contents* outBody = uncodeSingle( sBody, signedBy );
   
   Pkcs7Contents* recuriveBody = dynamic_cast<Pkcs7Contents*>( outBody );
   if ( recuriveBody )
   {
      InfoLog( << "Recursively calling Security::uncode");
      Contents* ret = uncodeSingle( recuriveBody, signedBy );
      return ret;
   }
   
   return outBody;
}

Contents* 
Security::uncodeSingle( Pkcs7Contents* sBody, Data* signedBy )
{
   int flags=0;
   flags |= PKCS7_BINARY;
   
   // for now, assume that this is only a singed message
   assert( sBody );
   
   //const char* p = sBody->text().data();
   const char* p = sBody->text().c_str();
   int   s = sBody->text().size();
   //InfoLog( << "uncode body = " << sBody->text().escaped() );
   InfoLog( << "uncode body size = " << s );
   
   BIO* in;
   in = BIO_new_mem_buf( (void*)p,s);
   assert(in);
   InfoLog( << "ceated in BIO");
    
   BIO* out;
   out = BIO_new(BIO_s_mem());
   assert(out);
   InfoLog( << "created out BIO" );

#if 0
   BIO* pkcs7Bio=NULL;
   PKCS7* pkcs7 = SMIME_read_PKCS7(in,&pkcs7Bio);
   if ( !pkcs7 )
   {
      ErrLog( << "Problems doing SMIME_read_PKCS7" );
      return NULL;
   }
   if ( pkcs7Bio )
   {
      ErrLog( << "Can not deal with mutlipart mime version stuff " );
      return NULL;
   }  
#else
   BIO* pkcs7Bio=NULL;
   PKCS7* pkcs7 = d2i_PKCS7_bio(in, NULL);
   if ( !pkcs7 )
   {
      ErrLog( << "Problems doing decode of PKCS7 object" );

      while (1)
           {
              const char* file;
              int line;
              
              unsigned long code = ERR_get_error_line(&file,&line);
              if ( code == 0 )
              {
                 break;
              }
              
              char buf[256];
              ERR_error_string_n(code,buf,sizeof(buf));
              ErrLog( << buf  );
              InfoLog( << "Error code = " << code << " file=" << file << " line=" << line );
           }
           
      return NULL;
   }
   BIO_flush(in);
#endif
   
   int type=OBJ_obj2nid(pkcs7->type);
   switch (type)
   {
      case NID_pkcs7_signed:
         InfoLog( << "data is pkcs7 signed" );
         break;
      case NID_pkcs7_signedAndEnveloped:
         InfoLog( << "data is pkcs7 signed and enveloped" );
         break;
      case NID_pkcs7_enveloped:
         InfoLog( << "data is pkcs7 enveloped" );
         break;
      case NID_pkcs7_data:
         InfoLog( << "data i pkcs7 data" );
         break;
      case NID_pkcs7_encrypted:
         InfoLog( << "data is pkcs7 encrypted " );
         break;
      case NID_pkcs7_digest:
         InfoLog( << "data is pkcs7 digest" );
         break;
      default:
         InfoLog( << "Unkown pkcs7 type" );
         break;
   }

   STACK_OF(X509)* signers = PKCS7_get0_signers(pkcs7, NULL/*certs*/, 0/*flags*/ );
   for (int i=0; i<sk_X509_num(signers); i++)
   {
      X509* x = sk_X509_value(signers,i);

      STACK* emails = X509_get1_email(x);

      for ( int j=0; j<sk_num(emails); j++)
      {
         char* e = sk_value(emails,j);
         InfoLog("email field of signing cert is <" << e << ">" );
         if ( signedBy)
         {
            *signedBy = Data(e);
         }
      }
   }

   STACK_OF(X509)* certs;
   certs = sk_X509_new_null();
   assert( certs );
   //sk_X509_push(certs, publicCert);
   
   //flags |=  PKCS7_NOVERIFY;
   
   assert( certAuthorities );
   
   switch (type)
   {
     case NID_pkcs7_signedAndEnveloped:
     {
        assert(0);
     }
     break;
     
     case NID_pkcs7_enveloped:
     {
        if ( PKCS7_decrypt(pkcs7, privateKey, publicCert, out, flags ) != 1 )
        {
           ErrLog( << "Problems doing PKCS7_decrypt" );
           while (1)
           {
              const char* file;
              int line;
              
              unsigned long code = ERR_get_error_line(&file,&line);
              if ( code == 0 )
              {
                 break;
              }
              
              char buf[256];
              ERR_error_string_n(code,buf,sizeof(buf));
              ErrLog( << buf  );
              InfoLog( << "Error code = " << code << " file=" << file << " line=" << line );
           }
           
           return NULL;
        }
     }
     break;
      
      case NID_pkcs7_signed:
      {
         if ( PKCS7_verify(pkcs7, certs, certAuthorities, pkcs7Bio, out, flags ) != 1 )
         {
            ErrLog( << "Problems doing PKCS7_verify" );
            while (1)
            {
               const char* file;
               int line;
               
               unsigned long code = ERR_get_error_line(&file,&line);
               if ( code == 0 )
               {
                  break;
               }
               
               char buf[256];
               ERR_error_string_n(code,buf,sizeof(buf));
               ErrLog( << buf  );
               InfoLog( << "Error code = " << code << " file=" << file << " line=" << line );
            }
            
            return NULL;
         }
      }
      break;
      
      default:
         ErrLog("Got PKCS7 data that could not be handled type=" << type );
         return NULL;
   }
      
   BIO_flush(out);
   char* outBuf=NULL;
   long size = BIO_get_mem_data(out,&outBuf);
   assert( size >= 0 );
   
   Data outData(outBuf,size);
   
   //InfoLog( << "uncodec body is <" << outData << ">" );

#if 1
   // parse out the header information and form new body.
   // !cj! this is a really crappy parser - shoudl do proper mime stuff
   ParseBuffer pb( outData.data(), outData.size() );
   pb.skipToChar(Symbols::COLON[0]);
   pb.skipChar();
   const char* anchor1 = pb.skipWhitespace();
   const char* anchor2 = pb.skipToChar('/');
   Data mimeType( anchor1, anchor2-anchor1 );
   anchor2++;
   const char* anchor3 = pb.skipNonWhitespace();
   Data mimeSubtype( anchor2, anchor3-anchor2 );
   
   InfoLog( << "got type as <" << mimeType << "> and subtype as <" << mimeSubtype << ">" );

   char* ss = (char*)pb.position();
   while (true)
   {
      if (!strncmp(ss,Symbols::CRLFCRLF,4))
      {
         ss += 4;
         break;
      }
      if ( pb.eof() )
      {
         ErrLog( << "Bad data in encoded PKCS7 body");
         return NULL;
      }

      ss=(char*)pb.skipChar();
  }
      
   Data newBody( ss , pb.skipToEnd()-ss );
   //InfoLog( << "body data <" << newBody.escaped() << ">" );

   Mime mime(mimeType,mimeSubtype);
   
   if ( Contents::getFactoryMap().find(mime) == Contents::getFactoryMap().end())
   {
      ErrLog( << "Don't know how to deal with MIME type " << mime );
      return NULL;
   }
   
   Contents* outBody = Contents::getFactoryMap()[mime]->create(newBody,mime);
#else
   Contents* outBody = new Pkcs7Contents( outData );
#endif

   return outBody;  
}

     

#endif

/* ====================================================================
 * The Vovida Software License, Version 1.0  *  * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved. *  * Redistribution and use in source and binary forms, with or without * modification, are permitted provided that the following conditions * are met: *  * 1. Redistributions of source code must retain the above copyright *    notice, this list of conditions and the following disclaimer. *  * 2. Redistributions in binary form must reproduce the above copyright *    notice, this list of conditions and the following disclaimer in *    the documentation and/or other materials provided with the *    distribution. *  * 3. The names "VOCAL", "Vovida Open Communication Application Library", *    and "Vovida Open Communication Application Library (VOCAL)" must *    not be used to endorse or promote products derived from this *    software without prior written permission. For written *    permission, please contact vocal@vovida.org. * * 4. Products derived from this software may not be called "VOCAL", nor *    may "VOCAL" appear in their name, without prior written *    permission of Vovida Networks, Inc. *  * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL, * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH * DAMAGE. *  * ==================================================================== *  * This software consists of voluntary contributions made by Vovida * Networks, Inc. and many individuals on behalf of Vovida Networks, * Inc.  For more information on Vovida Networks, Inc., please see * <http://www.vovida.org/>. *
 */
