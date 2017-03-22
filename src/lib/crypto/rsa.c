/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * RSA utilities
 */

/* ========================================================================== */

#include <stdint.h>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

/* see rsa_topem */
#define PKCS_PEM

/* -------------------------------------------------------------------------- */

/*
 * Generate a private 1024-bits RSA key with e=65537.
 *
 * https://www.openssl.org/docs/man1.1.0/crypto/RSA_new.html
 */
void* rsa_keygen(void)
{
	int r;
	RSA* k;
	BIGNUM* bn;


	r = 0;
	k = 0;

	if (0 != (bn = BN_new())) {
		if (1 == BN_set_word(bn, 65537)) {
			if (0 != (k = RSA_new())) {
				r = RSA_generate_key_ex(k, 1024, bn, 0);
			}
		}
		BN_free(bn);
	}

	if ((1 != r) && (0 != k)) {
		RSA_free(k);
		k = 0;
	}

	return k;
}

/* NOTE not strictly important for cscoins. memo for later */
static int __savekey(void* key, const char* path, int pub)
{
	int r;
	BIO* f;

	r = 0;

	if (0 != (f = BIO_new_file(path, "w"))) {
		if (0 != pub) {
			r = PEM_write_bio_RSAPublicKey(f, key);
		} else {
			r = PEM_write_bio_RSAPrivateKey(f, key, 0, 0, 0, 0, 0);
		}
		BIO_free_all(f);
	}

	return (1 == r) ? 0 : -1;
}

int rsa_savekeys(void* key, const char* priv)
{
	/*
	 * We can use the private key to reconstruct the
	 * public key. Also, we do not need to send it to
	 * anyone. KISS
	 */
	return __savekey(key, priv, 0);
}

/*
 * https://www.openssl.org/docs/man1.1.0/crypto/PEM_read_RSAPrivateKey.html
 * https://www.openssl.org/docs/man1.1.0/crypto/PEM_read_RSA_PUBKEY.html
 */
void* rsa_loadkeys(const char* priv)
{
	RSA* k;
	RSA* t;
	FILE* f;


	if (0 == (f = fopen(priv, "rb"))) {
		return 0;
	}

	if (0 != (k = RSA_new())) {
		t = PEM_read_RSAPrivateKey(f, &k, 0, 0);
		if (0 == t) {
			RSA_free(k);
		}
		k = t;
	}

	fclose(f);

	return k;
}

/*
 * https://openssl.org/docs/man1.1.0/crypto/BIO_new_mem_buf.html
 */
void* rsa_loadmemkeys(const void* buf, size_t len)
{
	BIO* b;
	RSA* k;
	RSA* t;

	if (0 != (k = RSA_new())) {
		if (0 != (b = BIO_new_mem_buf(buf, len))) {
			t = PEM_read_bio_RSAPrivateKey(b, &k, 0, 0);
			if (0 == t) {
				RSA_free(k);
			}
			k = t;
			BIO_free(b);
		}
	}

	return k;
}

/* public key only */
char* rsa_topem(void* key)
{
	/*
	 * The CA does not accept PKCS#1 PEM FILES, but only X.509's
	 * ('BEGIN PUBLIC KEY' not 'BEGIN RSA PUBLIC KEY').
	 *
	 * To do so, we need to use OpenSSL's generic EVP and use
	 * PEM_..._PUBKEY() instead of PEM_..._RSAPublicKey().
	 */

	BIO* b;
	char* p;
	size_t l;

#ifdef PKCS_PEM
	EVP_PKEY* k;

	if (0 == (k = EVP_PKEY_new())) {
		return 0;
	}
	if (1 != EVP_PKEY_set1_RSA(k, key)) {
		EVP_PKEY_free(k);
		return 0;
	}
#endif /* PKCS_PEM */

	/*
	 * create bio membuf
	 * convert rsa to pem and store in bio
	 * get bio membuf size
	 * clone it
	 */

	p = 0;

	if (0 != (b = BIO_new(BIO_s_mem()))) {
#ifdef PKCS_PEM
		if (1 == PEM_write_bio_PUBKEY(b, k)) {
#else /* !PKGS_PEM */
		/* if we needed X.509s */
		if (1 == PEM_write_bio_RSAPublicKey(b, key)) {
#endif /* PKGS_PEM */
			l = BIO_pending(b);
			if (0 != (p = malloc(BIO_pending(b) + 1))) {
				BIO_read(b, p, l);
				p[l] = '\0';
			}
		}
		BIO_free_all(b);
	}

	EVP_PKEY_free(k);

	return p;
}

/*
 * https://www.openssl.org/docs/man1.1.0/crypto/i2d_RSA_PUBKEY.html
 */
uint8_t* rsa_toder(void* key, size_t* plen)
{
	unsigned char* p;

	p = 0;
	*plen = i2d_RSA_PUBKEY(key, &p);

	return (uint8_t*)p;
}

/*
 * https://www.openssl.org/docs/man1.1.0/crypto/RSA_private_encrypt.html
 * https://www.openssl.org/docs/man1.1.0/crypto/RSA_sign.html
 */
int rsa_sign(const void* msg, size_t len, void* key, void* dst)
{
	int r;
	unsigned int l;

	r = RSA_sign(NID_sha256, msg, len, dst, &l, key);

	return (1 == r) ? 0 : -1;
}

size_t rsa_signlen(void* key)
{
	return (size_t)RSA_size(key);
}

/* ========================================================================== */
