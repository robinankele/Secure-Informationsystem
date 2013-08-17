#include "registry/registry.h"
#include "server/database.h"
#include "server/server.h"
#include "communication/crypto/sha1.h"
#include "communication/crypto/sha1_impl.h"
#include "communication/crypto/hmac.h"
#include "communication/channel-hmac.h"
#include "communication/channel-with-server.h"
#include "communication/channel.h"
#include "communication/bpack.h"
#include "communication/simple-memory-buffer.h"
#include "communication/datastore.h"
#include "errors.h"
#include "datastructure.h"
#include "memory.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>


/* ************************************************************************** */
int myassert(int value, int line);
void resetTests();

void NullChecks();
void RegistryOpen();
void RegistryClose();
void RegistryGetInt64();
void RegistrySetInt64();
void RegistryGetDouble();
void RegistrySetDouble();
void RegistryGetString();
void RegistrySetString();
void RegistryGetBlob();
void RegistrySetBlob();
void RegistryEnumKeys();
void RegistryKeyGetValueType();
void RegistryGetChannel();
void DatabaseChecks();
void SHA1Checks();
void HMACChecks();
void HMACChannelChecks();
void ChannelChecks();
void ServerInit();
void ServerShutdown();
void ServerProcess();
void HardcoreEncryptionTests();
void TrickyHacks();


#define NUMBEROFTESTS 24
const char* testname[NUMBEROFTESTS] = {"NullChecks", "RegistryOpen", "RegistryClose", "RegistryGetInt64", "RegistrySetInt64", 
                                       "RegistryGetDouble", "RegistrySetDouble", "RegistryGetString", "RegistrySetString",
                                       "RegistryGetBlob", "RegistrySetBlob", "RegistryEnumKeys", "RegistryKeyGetValueType",
                                       "RegistryGetChannel","DatabaseChecks", "SHA1Checks", "HMACChecks", "HMACChannelChecks",
                                       "ChannelChecks", "ServerInit", "ServerShutdown", "ServerProcess", "HardcoreEncryptionTests",
                                       "TrickyHacks"};


int tests[NUMBEROFTESTS] = {0};
int failedasserts[NUMBEROFTESTS][100] = {{0}};
int currentTest = 0;
int currentAssert = 0;
/* ************************************************************************** */
int main(void)
{
  NullChecks();
  resetTests();
  RegistryOpen();
  resetTests();
  RegistryClose(); 
  resetTests();
  RegistryGetInt64();
  resetTests();
  RegistrySetInt64();
  resetTests();
  RegistryGetDouble();
  resetTests();
  RegistrySetDouble();
  resetTests();
  RegistryGetString();
  resetTests();
  RegistrySetString();
  resetTests();
  RegistryGetBlob();
  resetTests();
  RegistrySetBlob();
  resetTests();
  RegistryEnumKeys();
  resetTests();
  RegistryKeyGetValueType();
  resetTests();
  RegistryGetChannel();
  resetTests();
  DatabaseChecks();
  resetTests();
  SHA1Checks();
  resetTests();
  HMACChecks();
  resetTests();
  HMACChannelChecks();
  resetTests();
  ChannelChecks();
  resetTests();
  ServerInit();
  resetTests();
  ServerShutdown();
  resetTests();
  ServerProcess();
  resetTests();
  HardcoreEncryptionTests();
  resetTests();


  printf("********************Testcases********************** *\n");
  int i = 0;
  for(; i < NUMBEROFTESTS; i++){
    if(tests[i] == 0)
      printf("TEST[%s] PASSED\n", testname[i]);
    else{
      printf("TEST[%s] FAILED\n", testname[i]);
      int j = 0;
      for(; j < 100; j++){
        int line = failedasserts[i][j];
        if(line)
          printf("ASSERT FAILED in line %d\n", line);
      }
    }
  }
}

/* ************************************************************************** */
int myassert(int value, int line)
{
  if(value == 0){
    tests[currentTest] = 1;
    failedasserts[currentTest][currentAssert] = line;
    currentAssert++;
    //printf("ASSERT FAILED in line %d\n", line);
    //exit(-1);
    return 1;
  }
  if(value != 0)
    return 0;
  return -1;
}

/* ************************************************************************** */
void resetTests()
{
  currentTest++;
  currentAssert = 0;
}

/* ************************************************************************** */
void NullChecks()
{
  registry_t* registry = NULL;
  myassert(registry_open(NULL, "file://mydb.sqlite", "domain") == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_open(&registry, NULL, "domain") == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite", NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_close(NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  char key = 'a';
  int64_t ivalue = 42;
  double dvalue = 42.0;
  char cvalue[42] = "fortytwo\0";
  char *svalue = NULL;
  unsigned char *sbvalue = NULL;
  unsigned char bvalue[] = {0x42, 0x21, 0x13, 0x23};
  myassert(registry_get_int64(NULL, &key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_int64(registry, NULL, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_int64(registry, &key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_int64(NULL, &key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_int64(registry, NULL, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(NULL, &key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(registry, NULL, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(registry, &key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_double(NULL, &key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_double(registry, NULL, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(NULL, &key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(registry, NULL, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(registry, &key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(NULL, &key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, NULL, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, &key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  size_t size = 42;
  myassert(registry_get_blob(NULL, &key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, NULL, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, &key, NULL, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, &key, &sbvalue, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(NULL, &key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, NULL, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, &key, NULL, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  char *pattern = NULL;
  size_t count = 4;
  char *keys = NULL;
  myassert(registry_enum_keys(NULL, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, NULL, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, pattern, NULL, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, pattern, &count, NULL, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, pattern, &count, &size, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  int type = 42; 
  myassert(registry_key_get_value_type(NULL, &key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_key_get_value_type(registry, NULL, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_key_get_value_type(registry, &key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_channel(NULL) == NULL, __LINE__);
}

/* ************************************************************************** */
void RegistryOpen()
{
  registry_t* registry = NULL;
  myassert(registry_open(NULL, "file://mydb.sqlite", "domain") == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry == NULL, __LINE__);
  myassert(registry_open(&registry, NULL, "domain") == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry == NULL, __LINE__);
  myassert(registry_open(&registry, "", "domain") == ERROR_REGISTRY_UNKNOWN_IDENTIFIER, __LINE__);
  myassert(registry == NULL, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite", NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry == NULL, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite", "") == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry == NULL, __LINE__);

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key", "domain") == ERROR_OK, __LINE__);
  myassert(registry_close(registry) == ERROR_OK, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key|hmac://key", "domain") == ERROR_OK, __LINE__);
  myassert(registry_close(registry) == ERROR_OK, __LINE__);
  myassert(registry_open(&registry, "filee://mydb.sqlite", "domain") == ERROR_REGISTRY_UNKNOWN_IDENTIFIER, __LINE__);
  myassert(registry == NULL, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite|hmeac://key", "domain") == ERROR_REGISTRY_UNKNOWN_IDENTIFIER, __LINE__);
  myassert(registry == NULL, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key|hmrac://key", "domain") == ERROR_REGISTRY_UNKNOWN_IDENTIFIER, __LINE__);
  myassert(registry == NULL, __LINE__);
  myassert(registry_open(&registry, "fil", "domain") == ERROR_REGISTRY_UNKNOWN_IDENTIFIER, __LINE__);
  myassert(registry == NULL, __LINE__);
  myassert(registry_open(&registry, " file://mydb.sqlite", "domain") == ERROR_REGISTRY_UNKNOWN_IDENTIFIER, __LINE__);
  myassert(registry == NULL, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite|", "domain") == ERROR_REGISTRY_UNKNOWN_IDENTIFIER, __LINE__);
  myassert(registry == NULL, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);
  myassert(registry_close(registry) == ERROR_OK, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://mykey|", "domain") == ERROR_REGISTRY_UNKNOWN_IDENTIFIER, __LINE__);
  myassert(registry == NULL, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);
  myassert(registry_close(registry) == ERROR_OK, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key", "domain") == ERROR_OK, __LINE__);
  myassert(registry_close(registry) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void RegistryClose()
{
  registry_t* registry = NULL;
  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);
  myassert(registry_close(registry) == ERROR_OK, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key", "domain") == ERROR_OK, __LINE__);
  myassert(registry_close(registry) == ERROR_OK, __LINE__);
  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);
  myassert(registry_close(NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  channel_t *temp = registry->channel;
  registry->channel = NULL;
  myassert(registry_close(registry) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = temp;
  myassert(registry_close(registry) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void RegistryGetInt64()
{
  registry_t* registry = NULL;
  char *key = "int2";
  int64_t ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;

  myassert(registry_get_int64(NULL, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_int64(registry, NULL, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_int64(registry, "", &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_int64(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  char *domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel_t *channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);
  key = "notexisting";
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* GET INT check via HMAC Channel without encryption */

  registry = NULL;
  key = "int2";
  ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;

  myassert(registry_get_int64(NULL, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_int64(registry, NULL, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_int64(registry, "", &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_int64(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);
  key = "notexisting";
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* GET INT check via HMAC Channel with encryption */

  registry = NULL;
  key = "int223";
  ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;

  myassert(registry_get_int64(NULL, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_int64(registry, NULL, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_int64(registry, "", &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_int64(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);
  key = "notexisting";
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void RegistrySetInt64()
{
  registry_t* registry = NULL;
  char *key = "int1";
  int64_t ivalue = 42;

  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_int64(NULL, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_int64(registry, NULL, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_int64(registry, "", ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  char *domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel_t *channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET INT check via HMAC Channel without encryption */

  registry = NULL;
  key = "int1";
  ivalue = 42;

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_int64(NULL, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_int64(registry, NULL, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_int64(registry, "", ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET INT check via HMAC Channel with encryption */

  registry = NULL;
  key = "int123";
  ivalue = 42;

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_int64(NULL, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_int64(registry, NULL, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_int64(registry, "", ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

}

/* ************************************************************************** */
void RegistryGetDouble()
{
  registry_t* registry = NULL;
  char *key = "double2";
  double dvalue = 42.0;

  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_double(registry, key, dvalue) == ERROR_OK, __LINE__);
  dvalue = 0.0;

  myassert(registry_get_double(NULL, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(registry, NULL, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(registry, "", &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  char *domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel_t *channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_OK, __LINE__);
  myassert(dvalue == 42.0, __LINE__);
  key = "notexisting";
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* GET DOUBLE check via HMAC Channel without encryption */

  registry = NULL;
  key = "double2";
  dvalue = 42.0;

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_double(registry, key, dvalue) == ERROR_OK, __LINE__);
  dvalue = 0.0;

  myassert(registry_get_double(NULL, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(registry, NULL, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(registry, "", &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_OK, __LINE__);
  myassert(dvalue == 42.0, __LINE__);
  key = "notexisting";
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* GET DOUBLE check via HMAC Channel with encryption */

  registry = NULL;
  key = "double223";
  dvalue = 42.0;

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://keyoida", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_double(registry, key, dvalue) == ERROR_OK, __LINE__);
  dvalue = 0.0;

  myassert(registry_get_double(NULL, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(registry, NULL, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(registry, "", &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_double(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_OK, __LINE__);
  myassert(dvalue == 42.0, __LINE__);
  key = "notexisting";
  myassert(registry_get_double(registry, key, &dvalue) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void RegistrySetDouble()
{
  registry_t* registry = NULL;
  char *key = "double1";
  double dvalue = 42.0;

  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_double(NULL, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_double(registry, NULL, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_double(registry, "", dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  char *domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_double(registry, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_double(registry, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel_t *channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_double(registry, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_double(registry, key, dvalue) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET DOUBLE check via HMAC Channel without encryption */

  registry = NULL;
  key = "double1";
  dvalue = 42.0;

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_double(NULL, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_double(registry, NULL, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_double(registry, "", dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_double(registry, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_double(registry, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_double(registry, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_double(registry, key, dvalue) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET DOUBLE check via HMAC Channel with encryption */

  registry = NULL;
  key = "double123";
  dvalue = 42.0;

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://rambaldi", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_double(NULL, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_double(registry, NULL, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_double(registry, "", dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_double(registry, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_double(registry, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_double(registry, key, dvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_double(registry, key, dvalue) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void RegistryGetString()
{
  registry_t* registry = NULL;
  char *key = "string2";
  char cvalue[9] = "fortytwo";
  char *svalue = NULL;
  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);

  myassert(registry_get_string(NULL, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(registry, NULL, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(registry, "", &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  char *domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_string(registry, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_string(registry, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel_t *channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_string(registry, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_string(registry, key, &svalue) == ERROR_OK, __LINE__);
  myassert(strlen(cvalue) == strlen(svalue), __LINE__);
  uint64_t i = 0;
  for(; i < strlen(cvalue); i++)
    if(cvalue[i] != svalue[i])
      myassert(0, __LINE__);
  freeMemory(svalue);

  key = "notexisting";
  myassert(registry_get_string(registry, key, &svalue) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET DOUBLE check via HMAC Channel without encryption */

  registry = NULL;
  key = "string2";
  svalue = NULL;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);

  myassert(registry_get_string(NULL, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(registry, NULL, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(registry, "", &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_string(registry, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_string(registry, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_string(registry, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_string(registry, key, &svalue) == ERROR_OK, __LINE__);
  myassert(strlen(cvalue) == strlen(svalue), __LINE__);
  i = 0;
  for(; i < strlen(cvalue); i++)
    if(cvalue[i] != svalue[i])
      myassert(0, __LINE__);
  freeMemory(svalue);

  key = "notexisting";
  myassert(registry_get_string(registry, key, &svalue) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET DOUBLE check via HMAC Channel with encryption */

  registry = NULL;
  key = "string223";
  svalue = NULL;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://yoyoyoy", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);

  myassert(registry_get_string(NULL, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(registry, NULL, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(registry, "", &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_string(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_string(registry, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_string(registry, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_string(registry, key, &svalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_string(registry, key, &svalue) == ERROR_OK, __LINE__);
  myassert(strlen(cvalue) == strlen(svalue), __LINE__);
  i = 0;
  for(; i < strlen(cvalue); i++)
    if(cvalue[i] != svalue[i])
      myassert(0, __LINE__);
  freeMemory(svalue);

  key = "notexisting";
  myassert(registry_get_string(registry, key, &svalue) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void RegistrySetString()
{
  registry_t* registry = NULL;
  char *key = "string1";
  char cvalue[9] = "fortytwo";
  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_string(NULL, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, NULL, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, "", cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, key, "") == ERROR_OK, __LINE__);
  char *domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_string(registry, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_string(registry, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel_t *channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_string(registry, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET STRING check via HMAC Channel without encryption */

  registry = NULL;
  key = "string1";
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_string(NULL, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, NULL, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, "", cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, key, "") == ERROR_OK, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_string(registry, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_string(registry, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_string(registry, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET STRING check via HMAC Channel with encryption */

  registry = NULL;
  key = "string123";
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://thisisthekeyandnoother", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_string(NULL, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, NULL, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, "", cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_string(registry, key, "") == ERROR_OK, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_string(registry, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_string(registry, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_string(registry, key, cvalue) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void RegistryGetBlob()
{
  registry_t* registry = NULL;
  char *key = "blob2";
  unsigned char *sbvalue = NULL;
  size_t size = 4;
  unsigned char bvalue[] = {0x42, 0x21, 0x13, 0x23};

  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_OK, __LINE__);

  myassert(registry_get_blob(NULL, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, NULL, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, "", &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, key, NULL, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, key, &sbvalue, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  char *domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel_t *channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_OK, __LINE__);
  myassert(memcmp(sbvalue, bvalue, 4) == 0, __LINE__);
  myassert(size == 4, __LINE__);
  freeMemory(sbvalue);

  key = "notexisting";
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET BLOB check via HMAC Channel without encryption */

  registry = NULL;
  key = "blob2";
  sbvalue = NULL;
  size = 4;
  
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_OK, __LINE__);

  myassert(registry_get_blob(NULL, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, NULL, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, "", &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, key, NULL, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, key, &sbvalue, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_OK, __LINE__);
  myassert(memcmp(sbvalue, bvalue, 4) == 0, __LINE__);
  myassert(size == 4, __LINE__);
  freeMemory(sbvalue);

  key = "notexisting";
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET BLOB check via HMAC Channel with encryption */

  registry = NULL;
  key = "blob223";
  sbvalue = NULL;
  size = 4;
  
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://actuallywecandothisaskey", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_OK, __LINE__);

  myassert(registry_get_blob(NULL, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, NULL, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, "", &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, key, NULL, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_get_blob(registry, key, &sbvalue, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_OK, __LINE__);
  myassert(memcmp(sbvalue, bvalue, 4) == 0, __LINE__);
  myassert(size == 4, __LINE__);
  freeMemory(sbvalue);

  key = "notexisting";
  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

}

/* ************************************************************************** */
void RegistrySetBlob()
{
  registry_t* registry = NULL;
  char *key = "blob1";
  unsigned char bvalue[] = {0x42, 0x21, 0x13, 0x23};
  size_t size = 4;
  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_blob(NULL, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, NULL, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, "", bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, key, NULL, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, key, bvalue, 0) == ERROR_OK, __LINE__);
  char *domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel_t *channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET BLOB check via HMAC Channel without encryption */

  registry = NULL;
  key = "blob1";
  size = 4;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_blob(NULL, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, NULL, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, "", bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, key, NULL, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, key, bvalue, 0) == ERROR_OK, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET BLOB check via HMAC Channel with encryption */

  registry = NULL;
  key = "blob123";
  size = 4;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://theREGISTRY", "domain") == ERROR_OK, __LINE__);

  myassert(registry_set_blob(NULL, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, NULL, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, "", bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, key, NULL, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_set_blob(registry, key, bvalue, 0) == ERROR_OK, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_OK, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

}

/* ************************************************************************** */
void RegistryEnumKeys()
{
  registry_t* registry = NULL;
  char *pattern = "pattern*";
  size_t count = 0;
  char *keys = NULL;
  char *key = "pattern1";
  char* cvalue = "teststring";
  size_t size = 0;

  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);
  key = "pattern3";
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);
  key = "pattern1";
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);
  key = "pattern2";
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);

  myassert(registry_enum_keys(NULL, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, NULL, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);

  myassert(registry_enum_keys(registry, "", &count, &size, &keys) == ERROR_OK, __LINE__);
  myassert(registry_enum_keys(registry, pattern, NULL, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, pattern, &count, NULL, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, pattern, &count, &size, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  char *domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel_t *channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_OK, __LINE__);
  myassert(count == 3, __LINE__);
  myassert(size == 27, __LINE__);
  myassert(strncmp("pattern1", keys, strlen(key)) == 0, __LINE__);
  myassert(strncmp("pattern2", keys + 9, strlen(key)) == 0, __LINE__);
  myassert(strncmp("pattern3", keys + 18, strlen(key)) == 0, __LINE__);
  freeMemory(keys);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET ENUM check via HMAC Channel without encryption */

  registry = NULL;
  pattern = "pattern*";
  count = 0;
  keys = NULL;
  key = "pattern1";
  cvalue = "teststring";
  size = 0;

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);

  myassert(registry_enum_keys(NULL, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, NULL, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, "", &count, &size, &keys) == ERROR_OK, __LINE__);
  myassert(registry_enum_keys(registry, pattern, NULL, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, pattern, &count, NULL, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, pattern, &count, &size, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_OK, __LINE__);
  myassert(count == 3, __LINE__);
  myassert(size == 27, __LINE__);
  myassert(strncmp("pattern1", keys, strlen(key)) == 0, __LINE__);
  myassert(strncmp("pattern2", keys + 9, strlen(key)) == 0, __LINE__);
  myassert(strncmp("pattern3", keys + 18, strlen(key)) == 0, __LINE__);
  freeMemory(keys);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET ENUM check via HMAC Channel with encryption */

  registry = NULL;
  pattern = "pattern*";
  count = 0;
  keys = NULL;
  key = "pattern1";
  cvalue = "teststring";
  size = 0;

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://shitongit", "domain") == ERROR_OK, __LINE__);

  myassert(registry_enum_keys(NULL, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, NULL, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, "", &count, &size, &keys) == ERROR_OK, __LINE__);
  myassert(registry_enum_keys(registry, pattern, NULL, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, pattern, &count, NULL, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_enum_keys(registry, pattern, &count, &size, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_OK, __LINE__);
  myassert(count == 3, __LINE__);
  myassert(size == 27, __LINE__);
  myassert(strncmp("pattern1", keys, strlen(key)) == 0, __LINE__);
  myassert(strncmp("pattern2", keys + 9, strlen(key)) == 0, __LINE__);
  myassert(strncmp("pattern3", keys + 18, strlen(key)) == 0, __LINE__);
  freeMemory(keys);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

}

/* ************************************************************************** */
void RegistryKeyGetValueType()
{
  registry_t* registry = NULL;
  char *key = "string1";
  char* cvalue = "teststring";
  int type = 0;

  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);

  myassert(registry_key_get_value_type(NULL, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_key_get_value_type(registry, NULL, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_key_get_value_type(registry, "", &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_key_get_value_type(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  char *domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel_t *channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_OK, __LINE__);
  myassert(type == DATABASE_TYPE_STRING, __LINE__);

  key = "notexisting";
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET ENUM check via HMAC Channel without encryption */

  key = "string1";
  cvalue = "teststring";
  type = 0;

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);

  myassert(registry_key_get_value_type(NULL, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_key_get_value_type(registry, NULL, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_key_get_value_type(registry, "", &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_key_get_value_type(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_OK, __LINE__);
  myassert(type == DATABASE_TYPE_STRING, __LINE__);

  key = "notexisting";
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET ENUM check via HMAC Channel with encryption */

  key = "string1";
  cvalue = "teststring";
  type = 0;

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://einringsiezuknechten", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);

  myassert(registry_key_get_value_type(NULL, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_key_get_value_type(registry, NULL, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_key_get_value_type(registry, "", &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(registry_key_get_value_type(registry, key, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  domain = NULL;
  domain = registry->domain;
  registry->domain = NULL;
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = "";
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->domain = domain;
  channel = registry->channel;
  registry->channel = NULL;
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_INVALID_ARGUMENTS, __LINE__);
  registry->channel = channel;
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_OK, __LINE__);
  myassert(type == DATABASE_TYPE_STRING, __LINE__);

  key = "notexisting";
  myassert(registry_key_get_value_type(registry, key, &type) == ERROR_REGISTRY_NO_SUCH_KEY, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void RegistryGetChannel()
{
  registry_t* registry = NULL;
  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);
  myassert(registry_get_channel(NULL) == NULL, __LINE__);
  myassert(registry_get_channel(registry) != NULL, __LINE__);
  channel_t *temp = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_channel(registry) == NULL, __LINE__);
  registry->channel = temp;
  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET ENUM check via HMAC Channel without encryption */

  registry = NULL;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);
  myassert(registry_get_channel(NULL) == NULL, __LINE__);
  myassert(registry_get_channel(registry) != NULL, __LINE__);
  temp = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_channel(registry) == NULL, __LINE__);
  registry->channel = temp;
  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* SET ENUM check via HMAC Channel with encryption */

  registry = NULL;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://mychannels", "domain") == ERROR_OK, __LINE__);
  myassert(registry_get_channel(NULL) == NULL, __LINE__);
  myassert(registry_get_channel(registry) != NULL, __LINE__);
  temp = registry->channel;
  registry->channel = NULL;
  myassert(registry_get_channel(registry) == NULL, __LINE__);
  registry->channel = temp;
  myassert(registry_close(registry) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void DatabaseChecks()
{
    /* Get and set a integer from the datbase */
  database_handle_t* database = NULL;

  myassert(database_open(NULL, "mydb.sqlite") == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(database == NULL, __LINE__);
  myassert(database_open(&database, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(database == NULL, __LINE__);
  myassert(database_open(&database, "../mydb.sqlite") == ERROR_DATABASE_OPEN, __LINE__);
  myassert(database == NULL, __LINE__);

  // no permissions 
   //myassert(database_open(&database, "errordb1.sqlite") == ERROR_DATABASE_OPEN, __LINE__);
  //myassert(database == NULL, __LINE__);
 // myassert(0, __LINE__);
  // wrong type 
  myassert(database_open(&database, "errordb2.sqlite") == ERROR_DATABASE_INVALID, __LINE__);
  myassert(database == NULL, __LINE__);
  // non existing table
  myassert(database_open(&database, "errordb3.sqlite") == ERROR_DATABASE_INVALID, __LINE__);
  myassert(database == NULL, __LINE__);
  // no existing blob path in db 
  myassert(database_open(&database, "errordb4.sqlite") == ERROR_DATABASE_INVALID, __LINE__);
  myassert(database == NULL, __LINE__);
  // wrong blob path 
  myassert(database_open(&database, "errordb5.sqlite") == ERROR_DATABASE_INVALID, __LINE__);
  myassert(database == NULL, __LINE__);

  myassert(database_open(&database, "../examples/mydb.sqlite") == ERROR_OK, __LINE__);
  myassert(database != NULL, __LINE__);
  database_close(database);

  myassert(database_open(&database, "mydb.sqlite") == ERROR_OK, __LINE__);
  myassert(database != NULL, __LINE__);

//NAN checks
  myassert(database_set_double(database, "another domain", "my integer value", NAN) == ERROR_INVALID_ARGUMENTS, __LINE__);


  /*******************
  /
  /  INTEGER TESTS   /
  /
  *******************/
  int64_t test_in = 7;
  int64_t test_out = 0;


  // Set integer value 
  myassert(database_set_int64(database, "another domain", "my integer value", test_in) == ERROR_OK, __LINE__);
  test_in = 5;
  myassert(database_set_int64(database, "another domain", "my integer value", test_in) == ERROR_OK, __LINE__);

  // Get integer value 

  myassert(database_get_int64(database, "another domain", "my integer value", &test_out) == ERROR_OK, __LINE__);
  myassert(test_out == test_in, __LINE__);
  myassert(database_get_int64(database, "another domain", "fuck sase value", &test_out) == ERROR_DATABASE_NO_SUCH_KEY, __LINE__);
  myassert(test_out == test_in, __LINE__);
  myassert(database_get_int64(database, "fuck sase domain", "my integer value", &test_out) == ERROR_DATABASE_NO_SUCH_KEY, __LINE__);
  myassert(test_out == test_in, __LINE__);
  myassert(database_get_int64(database, "fuck sase domain", "fuck sase value", &test_out) == ERROR_DATABASE_NO_SUCH_KEY, __LINE__);
  myassert(test_out == test_in, __LINE__);

  // Null pointer checks 
  myassert(database_get_int64(database, NULL, "my integer value", &test_out) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(test_out == test_in, __LINE__);
  myassert(database_get_int64(database, "another domain", NULL, &test_out) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(test_out == test_in, __LINE__);
  myassert(database_get_int64(database, NULL, NULL, &test_out) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(test_out == test_in, __LINE__);
  myassert(database_get_int64(database, "another domain", "my integer value", NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(test_out == test_in, __LINE__);

  /*******************
  /
  /   DOUBLE TESTS   /
  /
  *******************/
  double test_in_d = 42.5;
  double test_out_d = 0;

  // Set double value 
 myassert(database_set_double(database, "another domain", "my double value", test_in_d) == ERROR_OK, __LINE__);
  test_in_d = 45.5;
  myassert(database_set_double(database, "another domain", "my double value", test_in_d) == ERROR_OK, __LINE__);

// Get Double Values 
 myassert(database_get_double(database, "another domain", "my double value", &test_out_d) == ERROR_OK, __LINE__);
  myassert(test_out_d == test_in_d, __LINE__);
  myassert(database_get_double(database, "another domain", "my double value", &test_out_d) == ERROR_OK, __LINE__);
  myassert(test_out_d == test_in_d, __LINE__);
  myassert(database_get_double(database, "fuck sase", "my double value", &test_out_d) == ERROR_DATABASE_NO_SUCH_KEY, __LINE__);
  myassert(test_out_d == test_in_d, __LINE__);
  myassert(database_get_double(database, "another domain", "fuck sase", &test_out_d) == ERROR_DATABASE_NO_SUCH_KEY, __LINE__);
  myassert(test_out_d == test_in_d, __LINE__);
  myassert(database_get_double(database, "fuck", "sase", &test_out_d) == ERROR_DATABASE_NO_SUCH_KEY, __LINE__);
  myassert(test_out_d == test_in_d, __LINE__);

  // Null pointer checks 
  myassert(database_get_double(database, NULL, "my integer value", &test_out_d) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(test_out_d == test_in_d, __LINE__);
  myassert(database_get_double(database, "another domain", NULL, &test_out_d) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(test_out_d == test_in_d, __LINE__);
  myassert(database_get_double(database, NULL, NULL, &test_out_d) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(test_out_d == test_in_d, __LINE__);
  myassert(database_get_double(database, "another domain", "my integer value", NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(test_out_d == test_in_d, __LINE__);


  /*******************
  /
  /   STRINGS TESTS   /
  /
  *******************/
   char* test_in_str = "test";
  char* test_out_str = NULL;

  // Set string value 
 myassert(database_set_string(database, "another domain", "my string value", test_in_str) == ERROR_OK, __LINE__);
  test_in_str = "abcd";
  myassert(database_set_string(database, "another domain", "my string value", test_in_str) == ERROR_OK, __LINE__);

  // Get string value 
  myassert(database_get_string(database, "another domain", "my string value", &test_out_str) == ERROR_OK, __LINE__);
  myassert(!strcmp(test_out_str, test_in_str), __LINE__);
 free(test_out_str);
 test_out_str = NULL;
  myassert(database_get_string(database, "fuck sase", "my string value", &test_out_str) == ERROR_DATABASE_NO_SUCH_KEY, __LINE__);
  myassert(test_out_str == NULL, __LINE__);
 myassert(database_get_string(database, "test", "fuck sase", &test_out_str) == ERROR_DATABASE_NO_SUCH_KEY, __LINE__);
  myassert(test_out_str == NULL, __LINE__);
  myassert(database_get_string(database, "fuck", "sase", &test_out_str) == ERROR_DATABASE_NO_SUCH_KEY, __LINE__);
   myassert(test_out_str == NULL, __LINE__);


  // Null pointer checks 
  myassert(database_get_string(database, NULL, "my string value", &test_out_str) == ERROR_INVALID_ARGUMENTS, __LINE__);
   myassert(test_out_str == NULL, __LINE__);
  myassert(database_get_string(database, "another domain", NULL, &test_out_str) == ERROR_INVALID_ARGUMENTS, __LINE__);
    myassert(test_out_str == NULL, __LINE__);
  myassert(database_get_string(database, NULL, NULL, &test_out_str) == ERROR_INVALID_ARGUMENTS, __LINE__);
   myassert(test_out_str == NULL, __LINE__);
  myassert(database_get_string(database, "another domain", "my string value", NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(test_out_str == NULL, __LINE__);



  /*******************
  /
  /   BLOBS TESTS   /
  /
  *******************/

  // Set and get a blob 
  const unsigned char blob[] = { 0x00, 0x6E, 0x00, 0xA7, 0x00, 0x19, 0x00, 0x03 };
  myassert(database_set_blob(database, "another domain", "my blob value", blob, sizeof(blob)) == ERROR_OK, __LINE__);

  size_t blob_size = 0;
  unsigned char* theblob = NULL;
  myassert(database_get_blob(database, "another domain", "my blob value", &theblob, &blob_size) == ERROR_OK, __LINE__);

  myassert(theblob != NULL, __LINE__);

  myassert(blob_size == sizeof(blob), __LINE__);
  myassert(memcmp(blob, theblob, sizeof(blob)) == 0, __LINE__);
  free(theblob);


  /***************************
  /
  /   MIXED HARDCORE TESTS   /
  /
  ***************************/
  int64_t test_out_mixed_int = 0;
  double test_out_mixed_double = 0.0;
  myassert(database_set_int64(database, "mixedtests", "key", 10) == ERROR_OK, __LINE__);
  myassert(database_get_int64(database, "mixedtests", "key", &test_out_mixed_int) == ERROR_OK, __LINE__);
  myassert(test_out_mixed_int == 10, __LINE__);
  myassert(database_set_double(database, "mixedtests", "key", 1.1) == ERROR_OK, __LINE__);
  myassert(database_get_double(database, "mixedtests", "key", &test_out_mixed_double) == ERROR_OK, __LINE__);
  myassert(test_out_mixed_double == 1.100000, __LINE__);


  // Get and set a string from the database 
  assert(database_set_string(database, "strings", "are awesome", "'; --") == ERROR_OK);
  char* string_value = NULL;
  assert(database_get_string(database, "strings", "are awesome", &string_value) == ERROR_OK);
  assert(string_value != NULL);
  assert(strncmp(string_value, "'; --", 6) == 0);
  free(string_value);

  // Set and get a double 
  assert(database_set_double(database, "double", "1", INFINITY) == ERROR_OK);
  double d = 0;
  assert(database_get_double(database, "double", "1", &d) == ERROR_OK);
  assert(d == INFINITY);

  // Set an integer value 
  int64_t val = 10, nval;
  // Set integer value 
  assert(database_set_int64(database, "another domain", "my integer value", val) == ERROR_OK);

  // Get integer value 
  assert(database_get_int64(database, "another domain", "my integer value", &nval) == ERROR_OK);
  assert(nval == val);


  /***************************
  /
  /   GET TYPE               /
  /
  ***************************/

  database_value_type_t* type = NULL;
  type = malloc(sizeof(database_value_type_t));

 myassert(database_get_type(database, "another domain", "my integer value", type) == ERROR_OK, __LINE__);
// myassert(type == DATABASE_TYPE_INT64, __LINE__);
  myassert(database_get_type(database, "another domain", "my double value", type) == ERROR_OK, __LINE__);
  myassert(*type == DATABASE_TYPE_DOUBLE, __LINE__);

  free(type);


  /***************************
  /
  /   ENUMERATE KEYS         /
  /
  ***************************/



  // Enumerate some keys 
  assert(database_set_int64(database, "enum", "key1", 0) == ERROR_OK);
  assert(database_set_int64(database, "enum", "key2", 0) == ERROR_OK);
  assert(database_set_int64(database, "enum", "key3", 0) == ERROR_OK);
  assert(database_set_int64(database, "enum", "no match", 0) == ERROR_OK);

  size_t size = 0, count = 0;
  char* keys = NULL;
  assert(database_enum_keys(database, "enum", "", &count, &size, &keys) == ERROR_OK);
  //printf("size: %d, count: %d, keys: %d\n", size, count, keys);
  assert(database_enum_keys(database, "enum", "key*", &count, &size, &keys) == ERROR_OK);
  assert(count == 3); // 3 results 
  assert(size == 15); // each key is a string with 4 characters and a 0 byte 
  assert(keys != NULL);
  assert(strncmp(keys, "key1", 5) == 0); // keys are sorted 
  assert(strncmp(keys + 5, "key2", 5) == 0);
  assert(strncmp(keys + 10, "key3", 5) == 0);
  free(keys);





  database_close(database);
}

/* ************************************************************************** */
void  SHA1Checks()
{
  const unsigned char* test1 = (unsigned char*)"SASESUCKS";
  uint8_t comparet1[20] = {0x4e, 0xef, 0x47, 0x06, 0x43, 0xf8, 0x66,
                           0x5f, 0x59, 0x1e, 0x20, 0x5b, 0x86, 0xd0, 
                           0xe3, 0x80, 0xbc, 0x8a, 0xb7, 0x76};
  size_t len = 9;
  uint8_t res[20] = {0};

  myassert(sha1(NULL, len, res) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(sha1(test1, 0, res) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(sha1(test1, len, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  len = strlen((char*)test1);
  myassert(sha1(test1, len, res) == ERROR_OK, __LINE__);
  uint8_t i = 0;
  for(; i < 20; i++)
    if(res[i] != comparet1[i]){
      myassert(0, __LINE__);
      break;
    }
  const unsigned char* test2 = (unsigned char*)" ";
  uint8_t comparet2[20] = {0xb8, 0x58, 0xcb, 0x28, 0x26, 0x17, 0xfb,
                           0x09, 0x56, 0x1e, 0xd9, 0x60, 0x5c, 0x8e, 
                           0x84, 0xd1, 0xcc, 0xf9, 0x09, 0xc6};
  len = strlen((char*)test2);
  myassert(sha1(test2, len, res) == ERROR_OK, __LINE__);
  for(; i < 20; i++)
    if(res[i] != comparet2[i]){
      myassert(0, __LINE__);
      break;
    }
}

/* ************************************************************************** */
void HMACChecks()
{
  unsigned char* key = (unsigned char*)"thekey";
  size_t keysize = 6;
  unsigned char* message = (unsigned char*)"SHITONGIT";
  size_t messagesize = 9;
  unsigned char res[20] = {'\0'};
  
  myassert(hmac(NULL, keysize, message, messagesize, res) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(hmac(key, 0, message, messagesize, res) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(hmac(key, keysize, NULL, messagesize, res) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(hmac(key, keysize, message, 0, res) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(hmac(key, keysize, message, messagesize, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(hmac(key, keysize, message, messagesize, res) == ERROR_OK, __LINE__);

  myassert(hmac_verify(NULL, keysize, message, messagesize, res) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(hmac_verify(key, 0, message, messagesize, res) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(hmac_verify(key, keysize, NULL, messagesize, res) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(hmac_verify(key, keysize, message, 0, res) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(hmac_verify(key, keysize, message, messagesize, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(hmac_verify(key, keysize, message, messagesize, res) == ERROR_OK, __LINE__);

  uint8_t i = 0;
  unsigned char key1[20] = {0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 
                            0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b};
  keysize = 20;
  message = (unsigned char*)"Hi There";
  messagesize = 8;
  unsigned char digest[20] = {0xb6, 0x17, 0x31, 0x86, 0x55, 0x05, 0x72, 0x64, 0xe2, 0x8b,
                              0xc0, 0xb6, 0xfb, 0x37, 0x8c, 0x8e, 0xf1, 0x46, 0xbe, 0x00};
  myassert(hmac(key1, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  myassert(hmac_verify(key1, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  for(i = 0; i < 20; i++)
    if(res[i] != digest[i]){
      myassert(0, __LINE__);
      break;
    }

  key = (unsigned char*)"Jefe";
  keysize = 4;
  message = (unsigned char*)"what do ya want for nothing?";
  messagesize = 28;
  unsigned char digest1[20] = {0xef, 0xfc, 0xdf, 0x6a, 0xe5, 0xeb, 0x2f, 0xa2, 0xd2, 0x74,
                               0x16, 0xd5, 0xf1, 0x84, 0xdf, 0x9c, 0x25, 0x9a, 0x7c, 0x79};
  myassert(hmac(key, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  myassert(hmac_verify(key, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  for(i = 0; i < 20; i++)
    if(res[i] != digest1[i]){
      myassert(0, __LINE__);
      break;
    }


  unsigned char key2[20] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 
                            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
  keysize = 20;
  unsigned char message1[50] = {0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 
                                0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 
                                0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 
                                0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 
                                0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd};
  messagesize =  50;
  unsigned char digest2[20] = {0x12, 0x5d, 0x73, 0x42, 0xb9, 0xac, 0x11, 0xcd, 0x91, 0xa3, 
                               0x9a, 0xf4, 0x8a, 0xa1, 0x7b, 0x4f, 0x63, 0xf1, 0x75, 0xd3};
  myassert(hmac(key2, keysize, message1, messagesize, res) == ERROR_OK, __LINE__);
  myassert(hmac_verify(key2, keysize, message1, messagesize, res) == ERROR_OK, __LINE__);
  for(i = 0; i < 20; i++)
    if(res[i] != digest2[i]){
      myassert(0, __LINE__);
      break;
    }



  unsigned char key3[25] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
  keysize = 25;
  unsigned char message2[50] = {0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 
                                0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,  
                                0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 
                                0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 
                                0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd};
  messagesize =  50;
  unsigned char digest3[20] = {0x4c, 0x90, 0x07, 0xf4, 0x02, 0x62, 0x50, 0xc6, 0xbc, 0x84, 
                               0x14, 0xf9, 0xbf, 0x50, 0xc8, 0x6c, 0x2d, 0x72, 0x35, 0xda};
  myassert(hmac(key3, keysize, message2, messagesize, res) == ERROR_OK, __LINE__);
  myassert(hmac_verify(key3, keysize, message2, messagesize, res) == ERROR_OK, __LINE__);
  for(i = 0; i < 20; i++)
    if(res[i] != digest3[i]){
      myassert(0, __LINE__);
      break;
    }  


  unsigned char key4[20] = {0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 
                            0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c};
  keysize = 20;
  message = (unsigned char*)"Test With Truncation";
  messagesize = 20;
  unsigned char digest0[20] = {0x4c, 0x1a, 0x03, 0x42, 0x4b, 0x55, 0xe0, 0x7f, 0xe7, 0xf2, 
                               0x7b, 0xe1, 0xd5, 0x8b, 0xb9, 0x32, 0x4a, 0x9a, 0x5a, 0x04};
  //digest-96 = {0x4c, 0x1a, 0x03, 0x42, 0x4b, 0x55, 0xe0, 0x7f, 0xe7, 0xf2, 0x7b, 0xe1};
  myassert(hmac(key4, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  myassert(hmac_verify(key4, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  for(i = 0; i < 20; i++)
    if(res[i] != digest0[i]){
      myassert(0, __LINE__);
      break;
    }


  unsigned char key5[80] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 
                            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
  keysize = 80;
  message = (unsigned char*)"Test Using Larger Than Block-Size Key - Hash Key First";
  messagesize = 54;
  unsigned char digest4[20] = {0xaa, 0x4a, 0xe5, 0xe1, 0x52, 0x72, 0xd0, 0x0e, 0x95, 0x70, 0x56,
                               0x37, 0xce, 0x8a, 0x3b, 0x55, 0xed, 0x40, 0x21, 0x12};
  myassert(hmac(key5, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  myassert(hmac_verify(key5, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  for(i = 0; i < 20; i++)
    if(res[i] != digest4[i]){
      myassert(0, __LINE__);
      break;
    }


  message = (unsigned char*)"Test Using Larger Than Block-Size Key - Hash Key First";
  messagesize = 54;
  unsigned char digest6[20] = {0xaa, 0x4a, 0xe5, 0xe1, 0x52, 0x72, 0xd0, 0x0e, 0x95, 0x70, 
                               0x56, 0x37, 0xce, 0x8a, 0x3b, 0x55, 0xed, 0x40, 0x21, 0x12};
  myassert(hmac(key5, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  myassert(hmac_verify(key5, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  for(i = 0; i < 20; i++)
    if(res[i] != digest6[i]){
      myassert(0, __LINE__);
      break;
    }


  message = (unsigned char*)"Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data";
  messagesize = 73;
  unsigned char digest7[20] = {0xe8, 0xe9, 0x9d, 0x0f, 0x45, 0x23, 0x7d, 0x78, 0x6d, 0x6b, 
                               0xba, 0xa7, 0x96, 0x5c, 0x78, 0x08, 0xbb, 0xff, 0x1a, 0x91};
  myassert(hmac(key5, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  myassert(hmac_verify(key5, keysize, message, messagesize, res) == ERROR_OK, __LINE__);
  for(i = 0; i < 20; i++)
    if(res[i] != digest7[i]){
      myassert(0, __LINE__);
      break;
    }
}

/* ************************************************************************** */
void HMACChannelChecks()
{
  channel_t *channel = NULL;
  channel_t *child = NULL;
  const unsigned char* key = (unsigned char*)"thekey";
  size_t len = 6;
  const char* database = "mydb.sqlite";

  myassert(channel_hmac_new(NULL, child) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_hmac_new(&channel, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_with_server_new(&channel, database) == ERROR_OK, __LINE__);
  myassert(channel->data != NULL, __LINE__);
  myassert(channel_hmac_new(&channel, channel) == ERROR_OK, __LINE__);
  channel_hmac_t *channel_hmac = channel->data;
  myassert(channel_hmac->child != NULL, __LINE__);

  myassert(channel_hmac_set_key(NULL, key, len) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_hmac_set_key(channel, key, len) == ERROR_OK, __LINE__);
  myassert(strncmp((char*)channel_hmac->key, (char*)key, len) == 0, __LINE__);
  myassert(channel_hmac->keysize == len, __LINE__);
  myassert(channel_free(channel) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void ChannelChecks()
{
  channel_t *channel = NULL;
  char* database = "mydb.sqlite";
  size_t size = 0;
  unsigned char* bytes = NULL;

  myassert(channel_with_server_new(&channel, database) == ERROR_OK, __LINE__);

  /* client write bytes */
  int64_t integer = 0x0123456789abcdef;
  unsigned char *key = (unsigned char*)"int42";
  unsigned char* domain = (unsigned char*)"domain";
  data_store_t ds;
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_SET_INT) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ssl", domain, key, integer) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &bytes) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);
  myassert(channel_client_write_bytes(NULL, bytes, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_write_bytes(channel, NULL, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_write_bytes(channel, bytes, 0) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_write_bytes(channel, bytes, size) == ERROR_OK, __LINE__);  
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  /* server read bytes */
  unsigned char packettype = '\0';
  size = 0;
  bytes = NULL;
  myassert(channel_server_read_bytes(NULL, &bytes, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_read_bytes(channel, NULL, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_read_bytes(channel, &bytes, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_read_bytes(channel, &bytes, &size) == ERROR_INVALID_ARGUMENTS, __LINE__); //never can work
  myassert(size == 0, __LINE__);
  myassert(bytes == NULL, __LINE__);
  myassert(channel_client_read_bytes(channel, &bytes, &size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_new(&ds, bytes, size) == ERROR_OK, __LINE__);

  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  freeMemory(bytes);


  key = (unsigned char*)"int42";
  domain = (unsigned char*)"domain";
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_GET_INT) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ss", domain, key) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &bytes) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);
  myassert(channel_client_write_bytes(channel, bytes, size) == ERROR_OK, __LINE__); 
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);


  /* client read bytes */
  bytes = NULL;
  myassert(channel_client_read_bytes(NULL, &bytes, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, NULL, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, &bytes, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, &bytes, &size) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, bytes, size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_INT, __LINE__);
  int64_t compare_integer = 0;
  myassert(bunpack(&ds, "l", &compare_integer) == ERROR_OK, __LINE__);
  myassert(compare_integer == 0x0123456789abcdef, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  freeMemory(bytes);

  /* server write bytes */
  bytes = (unsigned char*)"message";
  size = 7;
  myassert(channel_server_write_bytes(NULL, bytes, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_write_bytes(channel, NULL, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_write_bytes(channel, bytes, 0) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_write_bytes(channel, bytes, size) == ERROR_OK, __LINE__);

  /* client read bytes */
  myassert(channel_client_read_bytes(NULL, &bytes, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, NULL, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, &bytes, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, &bytes, &size) == ERROR_OK, __LINE__);
  myassert(strncmp((char*)bytes , (char*)"message", 7) == 0, __LINE__);
  myassert(size == 7, __LINE__);
  freeMemory(bytes);

  myassert(channel_free(NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_free(channel) == ERROR_OK, __LINE__);

  /*                                        */
  /* Channel HMAC checks WITHOUT encryption */
  /*                                        */

  database = "mydb.sqlite";
  key = NULL;
  size_t len = 42;
  myassert(channel_with_server_new(&channel, database) == ERROR_OK, __LINE__);
  myassert(channel_hmac_new(&channel, channel) == ERROR_OK, __LINE__);
  myassert(channel_hmac_set_key(channel, key, len) == ERROR_OK, __LINE__);

  /* client write bytes */
  integer = 0x0123456789abcdef;
  key = (unsigned char*)"int42";
  domain = (unsigned char*)"domain";
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_SET_INT) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ssl", domain, key, integer) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &bytes) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);
  myassert(channel_client_write_bytes(NULL, bytes, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_write_bytes(channel, NULL, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_write_bytes(channel, bytes, 0) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_write_bytes(channel, bytes, size) == ERROR_OK, __LINE__);  
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  /* server read bytes */
  packettype = '\0';
  size = 0;
  bytes = NULL;
  myassert(channel_server_read_bytes(NULL, &bytes, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_read_bytes(channel, NULL, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_read_bytes(channel, &bytes, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_read_bytes(channel, &bytes, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(size == 0, __LINE__);
  myassert(bytes == NULL, __LINE__);
  myassert(channel_client_read_bytes(channel, &bytes, &size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_new(&ds, bytes, size) == ERROR_OK, __LINE__);

  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  freeMemory(bytes);


  key = (unsigned char*)"int42";
  domain = (unsigned char*)"domain";
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_GET_INT) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ss", domain, key) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &bytes) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);
  myassert(channel_client_write_bytes(channel, bytes, size) == ERROR_OK, __LINE__); 
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);


  /* client read bytes */
  bytes = NULL;
  myassert(channel_client_read_bytes(NULL, &bytes, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, NULL, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, &bytes, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, &bytes, &size) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, bytes, size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_INT, __LINE__);
  compare_integer = 0;
  myassert(bunpack(&ds, "l", &compare_integer) == ERROR_OK, __LINE__);
  myassert(compare_integer == 0x0123456789abcdef, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  freeMemory(bytes);

  /* server write bytes */
  bytes = (unsigned char*)"message";
  size = 7;
  myassert(channel_server_write_bytes(NULL, bytes, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_write_bytes(channel, NULL, size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_write_bytes(channel, bytes, 0) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_server_write_bytes(channel, bytes, size) == ERROR_OK, __LINE__);

  /* client read bytes */
  myassert(channel_client_read_bytes(NULL, &bytes, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, NULL, &size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, &bytes, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_client_read_bytes(channel, &bytes, &size) == ERROR_OK, __LINE__);
  myassert(strncmp((char*)bytes , (char*)"message", 7) == 0, __LINE__);
  myassert(size == 7, __LINE__);
  freeMemory(bytes);


  myassert(channel_free(NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_free(channel) == ERROR_OK, __LINE__);

  /*                                        */
  /* Channel HMAC checks WITH encryption    */
  /*                                        */

  database = "mydb.sqlite";
  key = (unsigned char*)"thekey";
  len = 6;
  myassert(channel_with_server_new(&channel, database) == ERROR_OK, __LINE__);
  myassert(channel_hmac_new(&channel, channel) == ERROR_OK, __LINE__);
  myassert(channel_hmac_set_key(channel, key, len) == ERROR_OK, __LINE__);

  // just doable with endpointconnector
  
  myassert(channel_free(NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(channel_free(channel) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void ServerInit()
{
  server_t *server = NULL;
  const char database[12] = "mydb.sqlite";
  myassert(server_init(NULL, database) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(server_init(&server, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(server_init(&server, "") == ERROR_DATABASE_OPEN, __LINE__);
  freeMemory(server);
  myassert(server_init(&server, "anywhere") == ERROR_DATABASE_OPEN, __LINE__);
  freeMemory(server);
  myassert(server_init(&server, database) == ERROR_OK, __LINE__);
  myassert(server_shutdown(server)== ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void ServerShutdown()
{
  server_t *server = NULL;
  const char database[12] = "mydb.sqlite";
  myassert(server_init(&server, database) == ERROR_OK, __LINE__);
  myassert(server_shutdown(NULL)== ERROR_INVALID_ARGUMENTS, __LINE__);
  database_handle_t* db = server->db;
  server->db = NULL;
  myassert(server_shutdown(server)== ERROR_INVALID_ARGUMENTS, __LINE__);
  server->db = db;
  myassert(server_shutdown(server)== ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void ServerProcess()
{
  server_t *server = NULL;
  const char database[12] = "mydb.sqlite";
  myassert(server_init(&server, database) == ERROR_OK, __LINE__);

  const char* domain = "domain";
  const unsigned char* key = (unsigned char*)"key1";
  unsigned char* data = NULL;
  size_t size = 0;
  unsigned char packettype = '\0';

  // PACKET_SET_INT 
  int64_t integer = 0x0123456789abcdef;
  key = (unsigned char*)"int1";
  data_store_t ds;
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_SET_INT) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ssl", domain, key, integer) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &data) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);

  unsigned char* response = NULL;
  size_t response_size = 0;
  myassert(server_process(NULL, data, size, &response, &response_size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  database_handle_t* db = server->db;
  server->db = NULL;
  myassert(server_process(server, data, size, &response, &response_size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  server->db = db;
  myassert(server_process(server, NULL, size, &response, &response_size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(server_process(server, data, 0, &response, &response_size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(server_process(server, data, size, NULL, &response_size) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(server_process(server, data, size, &response, NULL) == ERROR_INVALID_ARGUMENTS, __LINE__);
  myassert(server_process(server, data, size, &response, &response_size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, response, response_size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  freeMemory(response);

  // PACKET_GET_INT 
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_GET_INT) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ss", domain, key) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &data) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);

  response = NULL;
  response_size = 0;
  myassert(server_process(server, data, size, &response, &response_size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, response, response_size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_INT, __LINE__);
  int64_t response_int = 0;
  myassert(bunpack(&ds, "l", &response_int) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  myassert(integer == response_int, __LINE__);
  freeMemory(response);

  // PACKET_SET_DOUBLE 
  double dob = -0.0010;
  key = (unsigned char*)"double1";
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_SET_DOUBLE) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ssd", domain, key, dob) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &data) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);

  response = NULL;
  response_size = 0;
  myassert(server_process(server, data, size, &response, &response_size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, response, response_size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  freeMemory(response);

  // PACKET_GET_DOUBLE 
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_GET_DOUBLE) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ss", domain, key) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &data) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);

  response = NULL;
  response_size = 0;
  myassert(server_process(server, data, size, &response, &response_size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, response, response_size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_DOUBLE, __LINE__);
  double response_dob = 0.0;
  myassert(bunpack(&ds, "d", &response_dob) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  myassert(dob == response_dob, __LINE__);
  freeMemory(response);

  // PACKET_SET_STRING 
  char* string = "teststring";
  key = (unsigned char*)"string1";
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_SET_STRING) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "sss", domain, key, string) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &data) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);

  response = NULL;
  response_size = 0;
  myassert(server_process(server, data, size, &response, &response_size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, response, response_size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  freeMemory(response);

  // PACKET_GET_STRING 
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_GET_STRING) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ss", domain, key) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &data) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);

  response = NULL;
  response_size = 0;
  myassert(server_process(server, data, size, &response, &response_size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, response, response_size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_STRING, __LINE__);
  char* response_string = NULL;
  myassert(bunpack(&ds, "s", &response_string) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  myassert(strncmp(string, response_string, strlen(string)) == 0, __LINE__);
  myassert(strlen(string) == strlen(response_string), __LINE__);
  freeMemory(response);
  freeMemory(response_string);

  // PACKET_SET_BLOB 
  unsigned char blob[4] = {0x42, 0x42, 0x42, 0x42};
  key = (unsigned char*)"blob1";
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_SET_BLOB) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ssb", domain, key, 4, blob) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &data) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);

  response = NULL;
  response_size = 0;
  myassert(server_process(server, data, size, &response, &response_size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, response, response_size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  freeMemory(response);

  // PACKET_GET_BLOB 
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_GET_BLOB) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ss", domain, key) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &data) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);

  response = NULL;
  response_size = 0;
  myassert(server_process(server, data, size, &response, &response_size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, response, response_size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_BLOB, __LINE__);
  unsigned char* response_blob = NULL;
  size_t size_blob = 0;
  myassert(bunpack(&ds, "b", &size_blob, &response_blob) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  myassert(memcmp(response_blob, blob, size_blob) == 0, __LINE__);
  myassert(size_blob == 4, __LINE__);
  freeMemory(response);
  freeMemory(response_blob);

  // PACKET_GET_ENUM 
  key = (unsigned char*)"string1";
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_GET_ENUM) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ss", domain, key) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &data) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);

  response = NULL;
  response_size = 0;
  myassert(server_process(server, data, size, &response, &response_size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, response, response_size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_ENUM, __LINE__);
  uint64_t count = 0;
  size_t size_keys = 0;
  unsigned char* keys = NULL;
  myassert(bunpack(&ds, "lb", &count, &size_keys, &keys) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  myassert(count == 1, __LINE__);
  myassert(size_keys == strlen("string1") + 1, __LINE__);
  myassert(memcmp("string1", keys, strlen("string1")) == 0, __LINE__);
  freeMemory(response);
  freeMemory(keys);

  // PACKET_GET_TYPE 
  key = (unsigned char*)"string1";
  myassert(simple_memory_buffer_new(&ds, NULL, 0) == ERROR_OK, __LINE__);
  myassert(data_store_write_byte(&ds, PACKET_GET_VALUE_TYPE) == ERROR_OK, __LINE__);
  myassert(bpack(&ds, "ss", domain, key) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_data(&ds, &data) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_get_size(&ds, &size) == ERROR_OK, __LINE__);

  response = NULL;
  response_size = 0;
  myassert(server_process(server, data, size, &response, &response_size) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);

  myassert(simple_memory_buffer_new(&ds, response, response_size) == ERROR_OK, __LINE__);
  myassert(data_store_read_byte(&ds, &packettype) == ERROR_OK, __LINE__);
  myassert(packettype == PACKET_TYPE, __LINE__);
  int64_t datatype = 0;
  myassert(bunpack(&ds, "l", &datatype) == ERROR_OK, __LINE__);
  myassert(simple_memory_buffer_free(&ds) == ERROR_OK, __LINE__);
  myassert(datatype == DATABASE_TYPE_STRING, __LINE__);
  freeMemory(response);

  myassert(server_shutdown(server) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void HardcoreEncryptionTests()
{

  registry_t* registry = NULL;
  char *key = "int2";
  int64_t ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);
  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* GET INT check via HMAC Channel without encryption */

  registry = NULL;
  key = "int2";
  ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);
  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* GET INT check via HMAC Channel with encryption */

  registry = NULL;
  key = "int223";
  ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* encryption, encryption */

  registry = NULL;
  key = "int223";
  ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key|hmac://key1|hmac://key2|hmac://key3|hmac://42|hmac:// |hmac://000|hmac://key123", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* encryption, no encryption */

  registry = NULL;
  key = "int223";
  ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key|hmac://|hmac://yek", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);


  /* encryption, no encryption */

  registry = NULL;
  key = "int223";
  ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key|hmac://|hmac://key2|hmac://key3|hmac://|hmac:// |hmac://000|hmac://key123", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);


  /* encryption, no encryption */

  registry = NULL;
  key = "int223";
  ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key|hmac://|hmac://1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890|hmac://key3|hmac://|hmac:// |hmac://000|hmac://key123", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);



  /* try nice passcodes */

  registry = NULL;
  key = "int223";
  ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key|hmac://this is a test and should work|hmac://keyoida", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);


  /* try /0s */

  registry = NULL;
  key = "int223";
  ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key|hmac://this\0 34\0sdf|hmac://keyoida", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* try /0s */

  registry = NULL;
  key = "int223";
  ivalue = 42;
  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key|hmac://thisdfhma\0c://keyoidahma c://keyoida|hmac://keyoida", "domain") == ERROR_OK, __LINE__);
  myassert(registry_set_int64(registry, key, ivalue) == ERROR_OK, __LINE__);
  ivalue = 0;
  myassert(registry_get_int64(registry, key, &ivalue) == ERROR_OK, __LINE__);
  myassert(ivalue == 42, __LINE__);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);
}

/* ************************************************************************** */
void TrickyHacks()
{
  registry_t* registry = NULL;
  char *pattern = "NEVEREXISTING*";
  size_t count = 0;
  char *keys = NULL;
  char *key = "pattern1";
  char* cvalue = "teststring";
  size_t size = 0;

  myassert(registry_open(&registry, "file://mydb.sqlite|hmac://key|hmac://|hmac://abc¹²³²¸´³²½32535³²½³³{45{½[½]¼³½¬¼²³¼|hmac://23", "domain") == ERROR_OK, __LINE__);
  key = "pattern3";
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);
  key = "pattern1";
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);
  key = "pattern2";
  myassert(registry_set_string(registry, key, cvalue) == ERROR_OK, __LINE__);

  myassert(registry_enum_keys(registry, pattern, &count, &size, &keys) == ERROR_OK, __LINE__);
  myassert(count == 0, __LINE__);
  myassert(size == 0, __LINE__);
  myassert(keys == NULL, __LINE__);
  freeMemory(keys);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);

  /* do some nasty path traversals */

  registry = NULL;
  key = "../../../";
  unsigned char *sbvalue = NULL;
  size = 4;
  unsigned char bvalue[] = {0x42, 0x21, 0x13, 0x23};

  myassert(registry_open(&registry, "file://mydb.sqlite", "../../../") == ERROR_OK, __LINE__);
  myassert(registry_set_blob(registry, key, bvalue, size) == ERROR_OK, __LINE__);

  myassert(registry_get_blob(registry, key, &sbvalue, &size) == ERROR_OK, __LINE__);
  myassert(memcmp(sbvalue, bvalue, 4) == 0, __LINE__);
  myassert(size == 4, __LINE__);
  freeMemory(sbvalue);

  myassert(registry_close(registry) == ERROR_OK, __LINE__);
}



