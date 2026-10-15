#pragma once
#include <cstring>
#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>
using mbedtls_sha256_context = CC_SHA256_CTX;
inline void mbedtls_sha256_init(mbedtls_sha256_context* p) { std::memset(p,0,sizeof(*p)); }
inline void mbedtls_sha256_free(mbedtls_sha256_context* p) { std::memset(p,0,sizeof(*p)); }
inline int mbedtls_sha256_starts(mbedtls_sha256_context* p,int is224) { return !is224 && CC_SHA256_Init(p)==1 ? 0 : -1; }
inline int mbedtls_sha256_update(mbedtls_sha256_context* p,const unsigned char* b,size_t n) { return CC_SHA256_Update(p,b,CC_LONG(n))==1 ? 0 : -1; }
inline int mbedtls_sha256_finish(mbedtls_sha256_context* p,unsigned char* b) { return CC_SHA256_Final(b,p)==1 ? 0 : -1; }
#else
#include <openssl/sha.h>
using mbedtls_sha256_context = SHA256_CTX;
inline void mbedtls_sha256_init(mbedtls_sha256_context* p) { std::memset(p,0,sizeof(*p)); }
inline void mbedtls_sha256_free(mbedtls_sha256_context* p) { std::memset(p,0,sizeof(*p)); }
inline int mbedtls_sha256_starts(mbedtls_sha256_context* p,int is224) { return !is224 && SHA256_Init(p)==1 ? 0 : -1; }
inline int mbedtls_sha256_update(mbedtls_sha256_context* p,const unsigned char* b,size_t n) { return SHA256_Update(p,b,n)==1 ? 0 : -1; }
inline int mbedtls_sha256_finish(mbedtls_sha256_context* p,unsigned char* b) { return SHA256_Final(b,p)==1 ? 0 : -1; }
#endif
