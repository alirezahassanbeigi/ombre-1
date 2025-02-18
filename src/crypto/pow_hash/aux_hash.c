// Copyright (c) 2018, Ombre Cryptocurrency Project
// Copyright (c) 2019, Ryo Currency Project
//
// Portions of this file are available under BSD-3 license. Please see ORIGINAL-LICENSE for details
// All rights reserved.
//
// Ombre changes to this code are in public domain. Please note, other licences may apply to the file.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// !! NB
// Hash functions in this file were optimised to handle only 200 bytes long input. As such they
// are not useable outside of PoW calculation.
//
// Optimisations made by https://github.com/BaseMax
//
/*
 * Based on followig implementations:
 * 
 * BLAKE reference C implementation
 *  Copyright (c) 2012 Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>
 *  To the extent possible under law, the author(s) have dedicated all copyright
 *  and related and neighboring rights to this software to the public domain
 *  worldwide. This software is distributed without any warranty.
 *
 * Groestl ANSI C code optimised for 32-bit machines
 * Author: Thomas Krinninger
 *
 *  This work is based on the implementation of
 *          Soeren S. Thomsen and Krystian Matusiewicz
 *
 * JH implementation by Wu Hongjun
 *
 *
 * Implementation of the Skein hash function.
 * Source code author: Doug Whiting, 2008.
 * This algorithm and source code is released to the public domain.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////
///// blake_hash
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	uint32_t h[8], s[4], t[2];
} blake_ctx;

#define U8TO32(p)                                              \
	(((uint32_t)((p)[0]) << 24) | ((uint32_t)((p)[1]) << 16) | \
	 ((uint32_t)((p)[2]) << 8) | ((uint32_t)((p)[3])))

#define U32TO8(p, v)               \
	(p)[0] = (uint8_t)((v) >> 24); \
	(p)[1] = (uint8_t)((v) >> 16); \
	(p)[2] = (uint8_t)((v) >> 8);  \
	(p)[3] = (uint8_t)((v));

const uint8_t sigma[][16] = {
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
	{11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
	{7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
	{9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
	{2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
	{12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
	{13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
	{6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
	{10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
	{11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
	{7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8}};

const uint32_t cst[16] = {
	0x243F6A88, 0x85A308D3, 0x13198A2E, 0x03707344,
	0xA4093822, 0x299F31D0, 0x082EFA98, 0xEC4E6C89,
	0x452821E6, 0x38D01377, 0xBE5466CF, 0x34E90C6C,
	0xC0AC29B7, 0xC97C50DD, 0x3F84D5B5, 0xB5470917};

void blake256_compress(blake_ctx* S, const uint8_t* block)
{
	uint32_t v[16], m[16], i;
#define ROT(x, n) (((x) << (32 - n)) | ((x) >> (n)))
#define G(a, b, c, d, e)                                    \
	v[a] += (m[sigma[i][e]] ^ cst[sigma[i][e + 1]]) + v[b]; \
	v[d] = ROT(v[d] ^ v[a], 16);                            \
	v[c] += v[d];                                           \
	v[b] = ROT(v[b] ^ v[c], 12);                            \
	v[a] += (m[sigma[i][e + 1]] ^ cst[sigma[i][e]]) + v[b]; \
	v[d] = ROT(v[d] ^ v[a], 8);                             \
	v[c] += v[d];                                           \
	v[b] = ROT(v[b] ^ v[c], 7);

	for(i = 0; i < 16; ++i)
		m[i] = U8TO32(block + i * 4);
	for(i = 0; i < 8; ++i)
		v[i] = S->h[i];
	v[8] = S->s[0] ^ 0x243F6A88;
	v[9] = S->s[1] ^ 0x85A308D3;
	v[10] = S->s[2] ^ 0x13198A2E;
	v[11] = S->s[3] ^ 0x03707344;
	v[12] = 0xA4093822;
	v[13] = 0x299F31D0;
	v[14] = 0x082EFA98;
	v[15] = 0xEC4E6C89;
	v[12] ^= S->t[0];
	v[13] ^= S->t[0];
	v[14] ^= S->t[1];
	v[15] ^= S->t[1];
	for(i = 0; i < 14; ++i)
	{
		G(0, 4, 8, 12, 0);
		G(1, 5, 9, 13, 2);
		G(2, 6, 10, 14, 4);
		G(3, 7, 11, 15, 6);
		G(3, 4, 9, 14, 14);
		G(2, 7, 8, 13, 12);
		G(0, 5, 10, 15, 8);
		G(1, 6, 11, 12, 10);
	}
	for(i = 0; i < 16; ++i)
		S->h[i % 8] ^= v[i];
	for(i = 0; i < 8; ++i)
		S->h[i] ^= S->s[i % 4];
}
void blake256_hash(const uint8_t* data, uint8_t* hashval)
{
	blake_ctx S;
	S.h[0] = 0x6A09E667;
	S.h[1] = 0xBB67AE85;
	S.h[2] = 0x3C6EF372;
	S.h[3] = 0xA54FF53A;
	S.h[4] = 0x510E527F;
	S.h[5] = 0x9B05688C;
	S.h[6] = 0x1F83D9AB;
	S.h[7] = 0x5BE0CD19;
	S.t[0] = S.t[1] = 0;
	S.s[0] = S.s[1] = S.s[2] = S.s[3] = 0;
	S.t[0] += 512;
	blake256_compress(&S, data);
	data += 64;
	S.t[0] += 512;
	blake256_compress(&S, data);
	data += 64;
	S.t[0] += 512;
	blake256_compress(&S, data);
	data += 64;
	uint8_t buf[64];
	memcpy(buf, data, 8);
	memset(buf + 8, 0, 48);
	buf[8] = 0x80;
	buf[55] = 0x01;
	S.t[0] += 64;
	U32TO8(buf + 56, S.t[1]);
	U32TO8(buf + 60, S.t[0]);
	blake256_compress(&S, buf);
	U32TO8(hashval + 0, S.h[0]);
	U32TO8(hashval + 4, S.h[1]);
	U32TO8(hashval + 8, S.h[2]);
	U32TO8(hashval + 12, S.h[3]);
	U32TO8(hashval + 16, S.h[4]);
	U32TO8(hashval + 20, S.h[5]);
	U32TO8(hashval + 24, S.h[6]);
	U32TO8(hashval + 28, S.h[7]);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///// groestl_hash
///////////////////////////////////////////////////////////////////////////////////////////////

#define ROTL32(v, n) ((((v) << (n)) | ((v) >> (32 - (n)))) & li_32(ffffffff))
#define li_32(h) 0x##h##u
#define u32BIG(a)                       \
	((ROTL32(a, 8) & li_32(00FF00FF)) | \
	 (ROTL32(a, 24) & li_32(FF00FF00)))

typedef struct
{
	uint32_t chaining[16];
} groestl_ctx;

const uint32_t T[512] = {0xa5f432c6, 0xc6a597f4, 0x84976ff8, 0xf884eb97, 0x99b05eee, 0xee99c7b0, 0x8d8c7af6, 0xf68df78c, 0xd17e8ff, 0xff0de517, 0xbddc0ad6, 0xd6bdb7dc, 0xb1c816de, 0xdeb1a7c8, 0x54fc6d91, 0x915439fc, 0x50f09060, 0x6050c0f0, 0x3050702, 0x2030405, 0xa9e02ece, 0xcea987e0, 0x7d87d156, 0x567dac87, 0x192bcce7, 0xe719d52b, 0x62a613b5, 0xb56271a6, 0xe6317c4d, 0x4de69a31, 0x9ab559ec, 0xec9ac3b5, 0x45cf408f, 0x8f4505cf, 0x9dbca31f, 0x1f9d3ebc, 0x40c04989, 0x894009c0, 0x879268fa, 0xfa87ef92, 0x153fd0ef, 0xef15c53f, 0xeb2694b2, 0xb2eb7f26, 0xc940ce8e, 0x8ec90740, 0xb1de6fb, 0xfb0bed1d, 0xec2f6e41, 0x41ec822f, 0x67a91ab3, 0xb3677da9, 0xfd1c435f, 0x5ffdbe1c, 0xea256045, 0x45ea8a25, 0xbfdaf923, 0x23bf46da, 0xf7025153, 0x53f7a602, 0x96a145e4, 0xe496d3a1, 0x5bed769b, 0x9b5b2ded, 0xc25d2875, 0x75c2ea5d, 0x1c24c5e1, 0xe11cd924, 0xaee9d43d, 0x3dae7ae9, 0x6abef24c, 0x4c6a98be, 0x5aee826c, 0x6c5ad8ee, 0x41c3bd7e, 0x7e41fcc3, 0x206f3f5, 0xf502f106, 0x4fd15283, 0x834f1dd1, 0x5ce48c68, 0x685cd0e4, 0xf4075651, 0x51f4a207, 0x345c8dd1, 0xd134b95c, 0x818e1f9, 0xf908e918, 0x93ae4ce2, 0xe293dfae, 0x73953eab, 0xab734d95, 0x53f59762, 0x6253c4f5, 0x3f416b2a, 0x2a3f5441, 0xc141c08, 0x80c1014, 0x52f66395, 0x955231f6, 0x65afe946, 0x46658caf, 0x5ee27f9d, 0x9d5e21e2, 0x28784830, 0x30286078, 0xa1f8cf37, 0x37a16ef8, 0xf111b0a, 0xa0f1411, 0xb5c4eb2f, 0x2fb55ec4, 0x91b150e, 0xe091c1b, 0x365a7e24, 0x2436485a, 0x9bb6ad1b, 0x1b9b36b6, 0x3d4798df, 0xdf3da547, 0x266aa7cd, 0xcd26816a, 0x69bbf54e, 0x4e699cbb, 0xcd4c337f, 0x7fcdfe4c, 0x9fba50ea, 0xea9fcfba, 0x1b2d3f12, 0x121b242d, 0x9eb9a41d, 0x1d9e3ab9, 0x749cc458, 0x5874b09c, 0x2e724634, 0x342e6872, 0x2d774136, 0x362d6c77, 0xb2cd11dc, 0xdcb2a3cd, 0xee299db4, 0xb4ee7329, 0xfb164d5b, 0x5bfbb616, 0xf601a5a4, 0xa4f65301, 0x4dd7a176, 0x764decd7, 0x61a314b7, 0xb76175a3, 0xce49347d, 0x7dcefa49, 0x7b8ddf52, 0x527ba48d, 0x3e429fdd, 0xdd3ea142, 0x7193cd5e, 0x5e71bc93, 0x97a2b113, 0x139726a2, 0xf504a2a6, 0xa6f55704, 0x68b801b9, 0xb96869b8, 0x0, 0x0, 0x2c74b5c1, 0xc12c9974, 0x60a0e040, 0x406080a0, 0x1f21c2e3, 0xe31fdd21, 0xc8433a79, 0x79c8f243, 0xed2c9ab6, 0xb6ed772c, 0xbed90dd4, 0xd4beb3d9, 0x46ca478d, 0x8d4601ca, 0xd9701767, 0x67d9ce70, 0x4bddaf72, 0x724be4dd, 0xde79ed94, 0x94de3379, 0xd467ff98, 0x98d42b67, 0xe82393b0, 0xb0e87b23, 0x4ade5b85, 0x854a11de, 0x6bbd06bb, 0xbb6b6dbd, 0x2a7ebbc5, 0xc52a917e, 0xe5347b4f, 0x4fe59e34, 0x163ad7ed, 0xed16c13a, 0xc554d286, 0x86c51754, 0xd762f89a, 0x9ad72f62, 0x55ff9966, 0x6655ccff, 0x94a7b611, 0x119422a7, 0xcf4ac08a, 0x8acf0f4a, 0x1030d9e9, 0xe910c930, 0x60a0e04, 0x406080a, 0x819866fe, 0xfe81e798, 0xf00baba0, 0xa0f05b0b, 0x44ccb478, 0x7844f0cc, 0xbad5f025, 0x25ba4ad5, 0xe33e754b, 0x4be3963e, 0xf30eaca2, 0xa2f35f0e, 0xfe19445d, 0x5dfeba19, 0xc05bdb80, 0x80c01b5b, 0x8a858005, 0x58a0a85, 0xadecd33f, 0x3fad7eec, 0xbcdffe21, 0x21bc42df, 0x48d8a870, 0x7048e0d8, 0x40cfdf1, 0xf104f90c, 0xdf7a1963, 0x63dfc67a, 0xc1582f77, 0x77c1ee58, 0x759f30af, 0xaf75459f, 0x63a5e742, 0x426384a5, 0x30507020, 0x20304050, 0x1a2ecbe5, 0xe51ad12e, 0xe12effd, 0xfd0ee112, 0x6db708bf, 0xbf6d65b7, 0x4cd45581, 0x814c19d4, 0x143c2418, 0x1814303c, 0x355f7926, 0x26354c5f, 0x2f71b2c3, 0xc32f9d71, 0xe13886be, 0xbee16738, 0xa2fdc835, 0x35a26afd, 0xcc4fc788, 0x88cc0b4f, 0x394b652e, 0x2e395c4b, 0x57f96a93, 0x93573df9, 0xf20d5855, 0x55f2aa0d, 0x829d61fc, 0xfc82e39d, 0x47c9b37a, 0x7a47f4c9, 0xacef27c8, 0xc8ac8bef, 0xe73288ba, 0xbae76f32, 0x2b7d4f32, 0x322b647d, 0x95a442e6, 0xe695d7a4, 0xa0fb3bc0, 0xc0a09bfb, 0x98b3aa19, 0x199832b3, 0xd168f69e, 0x9ed12768, 0x7f8122a3, 0xa37f5d81, 0x66aaee44, 0x446688aa, 0x7e82d654, 0x547ea882, 0xabe6dd3b, 0x3bab76e6, 0x839e950b, 0xb83169e, 0xca45c98c, 0x8cca0345, 0x297bbcc7, 0xc729957b, 0xd36e056b, 0x6bd3d66e, 0x3c446c28, 0x283c5044, 0x798b2ca7, 0xa779558b, 0xe23d81bc, 0xbce2633d, 0x1d273116, 0x161d2c27, 0x769a37ad, 0xad76419a, 0x3b4d96db, 0xdb3bad4d, 0x56fa9e64, 0x6456c8fa, 0x4ed2a674, 0x744ee8d2, 0x1e223614, 0x141e2822, 0xdb76e492, 0x92db3f76, 0xa1e120c, 0xc0a181e, 0x6cb4fc48, 0x486c90b4, 0xe4378fb8, 0xb8e46b37, 0x5de7789f, 0x9f5d25e7, 0x6eb20fbd, 0xbd6e61b2, 0xef2a6943, 0x43ef862a, 0xa6f135c4, 0xc4a693f1, 0xa8e3da39, 0x39a872e3, 0xa4f7c631, 0x31a462f7, 0x37598ad3, 0xd337bd59, 0x8b8674f2, 0xf28bff86, 0x325683d5, 0xd532b156, 0x43c54e8b, 0x8b430dc5, 0x59eb856e, 0x6e59dceb, 0xb7c218da, 0xdab7afc2, 0x8c8f8e01, 0x18c028f, 0x64ac1db1, 0xb16479ac, 0xd26df19c, 0x9cd2236d, 0xe03b7249, 0x49e0923b, 0xb4c71fd8, 0xd8b4abc7, 0xfa15b9ac, 0xacfa4315, 0x709faf3, 0xf307fd09, 0x256fa0cf, 0xcf25856f, 0xafea20ca, 0xcaaf8fea, 0x8e897df4, 0xf48ef389, 0xe9206747, 0x47e98e20, 0x18283810, 0x10182028, 0xd5640b6f, 0x6fd5de64, 0x888373f0, 0xf088fb83, 0x6fb1fb4a, 0x4a6f94b1, 0x7296ca5c, 0x5c72b896, 0x246c5438, 0x3824706c, 0xf1085f57, 0x57f1ae08, 0xc7522173, 0x73c7e652, 0x51f36497, 0x975135f3, 0x2365aecb, 0xcb238d65, 0x7c8425a1, 0xa17c5984, 0x9cbf57e8, 0xe89ccbbf, 0x21635d3e, 0x3e217c63, 0xdd7cea96, 0x96dd377c, 0xdc7f1e61, 0x61dcc27f, 0x86919c0d, 0xd861a91, 0x85949b0f, 0xf851e94, 0x90ab4be0, 0xe090dbab, 0x42c6ba7c, 0x7c42f8c6, 0xc4572671, 0x71c4e257, 0xaae529cc, 0xccaa83e5, 0xd873e390, 0x90d83b73, 0x50f0906, 0x6050c0f, 0x103f4f7, 0xf701f503, 0x12362a1c, 0x1c123836, 0xa3fe3cc2, 0xc2a39ffe, 0x5fe18b6a, 0x6a5fd4e1, 0xf910beae, 0xaef94710, 0xd06b0269, 0x69d0d26b, 0x91a8bf17, 0x17912ea8, 0x58e87199, 0x995829e8, 0x2769533a, 0x3a277469, 0xb9d0f727, 0x27b94ed0, 0x384891d9, 0xd938a948, 0x1335deeb, 0xeb13cd35, 0xb3cee52b, 0x2bb356ce, 0x33557722, 0x22334455, 0xbbd604d2, 0xd2bbbfd6, 0x709039a9, 0xa9704990, 0x89808707, 0x7890e80, 0xa7f2c133, 0x33a766f2, 0xb6c1ec2d, 0x2db65ac1, 0x22665a3c, 0x3c227866, 0x92adb815, 0x15922aad, 0x2060a9c9, 0xc9208960, 0x49db5c87, 0x874915db, 0xff1ab0aa, 0xaaff4f1a, 0x7888d850, 0x5078a088, 0x7a8e2ba5, 0xa57a518e, 0x8f8a8903, 0x38f068a, 0xf8134a59, 0x59f8b213, 0x809b9209, 0x980129b, 0x1739231a, 0x1a173439, 0xda751065, 0x65daca75, 0x315384d7, 0xd731b553, 0xc651d584, 0x84c61351, 0xb8d303d0, 0xd0b8bbd3, 0xc35edc82, 0x82c31f5e, 0xb0cbe229, 0x29b052cb, 0x7799c35a, 0x5a77b499, 0x11332d1e, 0x1e113c33, 0xcb463d7b, 0x7bcbf646, 0xfc1fb7a8, 0xa8fc4b1f, 0xd6610c6d, 0x6dd6da61, 0x3a4e622c, 0x2c3a584e};

#define ROTATE_COLUMN_DOWN(v1, v2, amount_bytes, temp_var)                    \
	temp_var = (v1 << (8 * amount_bytes)) | (v2 >> (8 * (4 - amount_bytes))); \
	v2 = (v2 << (8 * amount_bytes)) | (v1 >> (8 * (4 - amount_bytes)));       \
	v1 = temp_var;

#define COLUMN(x, y, i, c0, c1, c2, c3, c4, c5, c6, c7, tv1, tv2, tu, tl, t) \
	tu = T[2 * (uint32_t)x[4 * c0 + 0]];                                     \
	tl = T[2 * (uint32_t)x[4 * c0 + 0] + 1];                                 \
	tv1 = T[2 * (uint32_t)x[4 * c1 + 1]];                                    \
	tv2 = T[2 * (uint32_t)x[4 * c1 + 1] + 1];                                \
	ROTATE_COLUMN_DOWN(tv1, tv2, 1, t)                                       \
	tu ^= tv1;                                                               \
	tl ^= tv2;                                                               \
	tv1 = T[2 * (uint32_t)x[4 * c2 + 2]];                                    \
	tv2 = T[2 * (uint32_t)x[4 * c2 + 2] + 1];                                \
	ROTATE_COLUMN_DOWN(tv1, tv2, 2, t)                                       \
	tu ^= tv1;                                                               \
	tl ^= tv2;                                                               \
	tv1 = T[2 * (uint32_t)x[4 * c3 + 3]];                                    \
	tv2 = T[2 * (uint32_t)x[4 * c3 + 3] + 1];                                \
	ROTATE_COLUMN_DOWN(tv1, tv2, 3, t)                                       \
	tu ^= tv1;                                                               \
	tl ^= tv2;                                                               \
	tl ^= T[2 * (uint32_t)x[4 * c4 + 0]];                                    \
	tu ^= T[2 * (uint32_t)x[4 * c4 + 0] + 1];                                \
	tv1 = T[2 * (uint32_t)x[4 * c5 + 1]];                                    \
	tv2 = T[2 * (uint32_t)x[4 * c5 + 1] + 1];                                \
	ROTATE_COLUMN_DOWN(tv1, tv2, 1, t)                                       \
	tl ^= tv1;                                                               \
	tu ^= tv2;                                                               \
	tv1 = T[2 * (uint32_t)x[4 * c6 + 2]];                                    \
	tv2 = T[2 * (uint32_t)x[4 * c6 + 2] + 1];                                \
	ROTATE_COLUMN_DOWN(tv1, tv2, 2, t)                                       \
	tl ^= tv1;                                                               \
	tu ^= tv2;                                                               \
	tv1 = T[2 * (uint32_t)x[4 * c7 + 3]];                                    \
	tv2 = T[2 * (uint32_t)x[4 * c7 + 3] + 1];                                \
	ROTATE_COLUMN_DOWN(tv1, tv2, 3, t)                                       \
	tl ^= tv1;                                                               \
	tu ^= tv2;                                                               \
	y[i] = tu;                                                               \
	y[i + 1] = tl;

static void RND512P(uint8_t* x, uint32_t* y, uint32_t r)
{
	uint32_t temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp;
	uint32_t* x32 = (uint32_t*)x;
	x32[0] ^= 0x00000000 ^ r;
	x32[2] ^= 0x00000010 ^ r;
	x32[4] ^= 0x00000020 ^ r;
	x32[6] ^= 0x00000030 ^ r;
	x32[8] ^= 0x00000040 ^ r;
	x32[10] ^= 0x00000050 ^ r;
	x32[12] ^= 0x00000060 ^ r;
	x32[14] ^= 0x00000070 ^ r;
	COLUMN(x, y, 0, 0, 2, 4, 6, 9, 11, 13, 15, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 2, 2, 4, 6, 8, 11, 13, 15, 1, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 4, 4, 6, 8, 10, 13, 15, 1, 3, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 6, 6, 8, 10, 12, 15, 1, 3, 5, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 8, 8, 10, 12, 14, 1, 3, 5, 7, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 10, 10, 12, 14, 0, 3, 5, 7, 9, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 12, 12, 14, 0, 2, 5, 7, 9, 11, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 14, 14, 0, 2, 4, 7, 9, 11, 13, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
}

static void RND512Q(uint8_t* x, uint32_t* y, uint32_t r)
{
	uint32_t temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp;
	uint32_t* x32 = (uint32_t*)x;
	x32[0] = ~x32[0];
	x32[1] ^= 0xffffffff ^ r;
	x32[2] = ~x32[2];
	x32[3] ^= 0xefffffff ^ r;
	x32[4] = ~x32[4];
	x32[5] ^= 0xdfffffff ^ r;
	x32[6] = ~x32[6];
	x32[7] ^= 0xcfffffff ^ r;
	x32[8] = ~x32[8];
	x32[9] ^= 0xbfffffff ^ r;
	x32[10] = ~x32[10];
	x32[11] ^= 0xafffffff ^ r;
	x32[12] = ~x32[12];
	x32[13] ^= 0x9fffffff ^ r;
	x32[14] = ~x32[14];
	x32[15] ^= 0x8fffffff ^ r;
	COLUMN(x, y, 0, 2, 6, 10, 14, 1, 5, 9, 13, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 2, 4, 8, 12, 0, 3, 7, 11, 15, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 4, 6, 10, 14, 2, 5, 9, 13, 1, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 6, 8, 12, 0, 4, 7, 11, 15, 3, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 8, 10, 14, 2, 6, 9, 13, 1, 5, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 10, 12, 0, 4, 8, 11, 15, 3, 7, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 12, 14, 2, 6, 10, 13, 1, 5, 9, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
	COLUMN(x, y, 14, 0, 4, 8, 12, 15, 3, 7, 11, temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp);
}

static void Transform(groestl_ctx* ctx, const uint8_t* input, int msglen)
{
	for(; msglen >= 64; msglen -= 64, input += 64)
	{
		uint32_t Ptmp[16];
		uint32_t Qtmp[16];
		uint32_t y[16];
		uint32_t z[16];
		z[0] = ((uint32_t*)input)[0];
		Ptmp[0] = ctx->chaining[0] ^ ((uint32_t*)input)[0];
		z[1] = ((uint32_t*)input)[1];
		Ptmp[1] = ctx->chaining[1] ^ ((uint32_t*)input)[1];
		z[2] = ((uint32_t*)input)[2];
		Ptmp[2] = ctx->chaining[2] ^ ((uint32_t*)input)[2];
		z[3] = ((uint32_t*)input)[3];
		Ptmp[3] = ctx->chaining[3] ^ ((uint32_t*)input)[3];
		z[4] = ((uint32_t*)input)[4];
		Ptmp[4] = ctx->chaining[4] ^ ((uint32_t*)input)[4];
		z[5] = ((uint32_t*)input)[5];
		Ptmp[5] = ctx->chaining[5] ^ ((uint32_t*)input)[5];
		z[6] = ((uint32_t*)input)[6];
		Ptmp[6] = ctx->chaining[6] ^ ((uint32_t*)input)[6];
		z[7] = ((uint32_t*)input)[7];
		Ptmp[7] = ctx->chaining[7] ^ ((uint32_t*)input)[7];
		z[8] = ((uint32_t*)input)[8];
		Ptmp[8] = ctx->chaining[8] ^ ((uint32_t*)input)[8];
		z[9] = ((uint32_t*)input)[9];
		Ptmp[9] = ctx->chaining[9] ^ ((uint32_t*)input)[9];
		z[10] = ((uint32_t*)input)[10];
		Ptmp[10] = ctx->chaining[10] ^ ((uint32_t*)input)[10];
		z[11] = ((uint32_t*)input)[11];
		Ptmp[11] = ctx->chaining[11] ^ ((uint32_t*)input)[11];
		z[12] = ((uint32_t*)input)[12];
		Ptmp[12] = ctx->chaining[12] ^ ((uint32_t*)input)[12];
		z[13] = ((uint32_t*)input)[13];
		Ptmp[13] = ctx->chaining[13] ^ ((uint32_t*)input)[13];
		z[14] = ((uint32_t*)input)[14];
		Ptmp[14] = ctx->chaining[14] ^ ((uint32_t*)input)[14];
		z[15] = ((uint32_t*)input)[15];
		Ptmp[15] = ctx->chaining[15] ^ ((uint32_t*)input)[15];
		RND512Q((uint8_t*)z, y, 0x00000000);
		RND512Q((uint8_t*)y, z, 0x01000000);
		RND512Q((uint8_t*)z, y, 0x02000000);
		RND512Q((uint8_t*)y, z, 0x03000000);
		RND512Q((uint8_t*)z, y, 0x04000000);
		RND512Q((uint8_t*)y, z, 0x05000000);
		RND512Q((uint8_t*)z, y, 0x06000000);
		RND512Q((uint8_t*)y, z, 0x07000000);
		RND512Q((uint8_t*)z, y, 0x08000000);
		RND512Q((uint8_t*)y, Qtmp, 0x09000000);
		RND512P((uint8_t*)Ptmp, y, 0x00000000);
		RND512P((uint8_t*)y, z, 0x00000001);
		RND512P((uint8_t*)z, y, 0x00000002);
		RND512P((uint8_t*)y, z, 0x00000003);
		RND512P((uint8_t*)z, y, 0x00000004);
		RND512P((uint8_t*)y, z, 0x00000005);
		RND512P((uint8_t*)z, y, 0x00000006);
		RND512P((uint8_t*)y, z, 0x00000007);
		RND512P((uint8_t*)z, y, 0x00000008);
		RND512P((uint8_t*)y, Ptmp, 0x00000009);
		ctx->chaining[0] ^= Ptmp[0] ^ Qtmp[0];
		ctx->chaining[1] ^= Ptmp[1] ^ Qtmp[1];
		ctx->chaining[2] ^= Ptmp[2] ^ Qtmp[2];
		ctx->chaining[3] ^= Ptmp[3] ^ Qtmp[3];
		ctx->chaining[4] ^= Ptmp[4] ^ Qtmp[4];
		ctx->chaining[5] ^= Ptmp[5] ^ Qtmp[5];
		ctx->chaining[6] ^= Ptmp[6] ^ Qtmp[6];
		ctx->chaining[7] ^= Ptmp[7] ^ Qtmp[7];
		ctx->chaining[8] ^= Ptmp[8] ^ Qtmp[8];
		ctx->chaining[9] ^= Ptmp[9] ^ Qtmp[9];
		ctx->chaining[10] ^= Ptmp[10] ^ Qtmp[10];
		ctx->chaining[11] ^= Ptmp[11] ^ Qtmp[11];
		ctx->chaining[12] ^= Ptmp[12] ^ Qtmp[12];
		ctx->chaining[13] ^= Ptmp[13] ^ Qtmp[13];
		ctx->chaining[14] ^= Ptmp[14] ^ Qtmp[14];
		ctx->chaining[15] ^= Ptmp[15] ^ Qtmp[15];
	}
}

void groestl_hash(const uint8_t* data, uint8_t* hashval)
{
	groestl_ctx context;
	memset(context.chaining, 0, 60);
	context.chaining[15] = u32BIG((uint32_t)256);
	Transform(&context, data, 200);
	uint8_t buf[64];
	memcpy(buf, data + 192, 8);
	memset(buf + 8, 0, 56);
	buf[8] = 0x80;
	buf[63] = 4;
	Transform(&context, buf, 64);
	uint32_t temp[16];
	uint32_t y[16];
	uint32_t z[16];
	memcpy(temp, context.chaining, 64);
	RND512P((uint8_t*)temp, y, 0x00000000);
	RND512P((uint8_t*)y, z, 0x00000001);
	RND512P((uint8_t*)z, y, 0x00000002);
	RND512P((uint8_t*)y, z, 0x00000003);
	RND512P((uint8_t*)z, y, 0x00000004);
	RND512P((uint8_t*)y, z, 0x00000005);
	RND512P((uint8_t*)z, y, 0x00000006);
	RND512P((uint8_t*)y, z, 0x00000007);
	RND512P((uint8_t*)z, y, 0x00000008);
	RND512P((uint8_t*)y, temp, 0x00000009);
	context.chaining[8] ^= temp[8];
	context.chaining[9] ^= temp[9];
	context.chaining[10] ^= temp[10];
	context.chaining[11] ^= temp[11];
	context.chaining[12] ^= temp[12];
	context.chaining[13] ^= temp[13];
	context.chaining[14] ^= temp[14];
	context.chaining[15] ^= temp[15];
	memcpy(hashval, context.chaining + 8, 32);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///// jh_hash
///////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__GNUC__)
#define DATA_ALIGN16(x) x __attribute__((aligned(16)))
#else
#define DATA_ALIGN16(x) __declspec(align(16)) x
#endif

typedef struct
{
	DATA_ALIGN16(uint64_t x[8][2]);
} jh_ctx;

const unsigned char JH256_H0[128] = {0xeb, 0x98, 0xa3, 0x41, 0x2c, 0x20, 0xd3, 0xeb, 0x92, 0xcd, 0xbe, 0x7b, 0x9c, 0xb2, 0x45, 0xc1, 0x1c, 0x93, 0x51, 0x91, 0x60, 0xd4, 0xc7, 0xfa, 0x26, 0x0, 0x82, 0xd6, 0x7e, 0x50, 0x8a, 0x3, 0xa4, 0x23, 0x9e, 0x26, 0x77, 0x26, 0xb9, 0x45, 0xe0, 0xfb, 0x1a, 0x48, 0xd4, 0x1a, 0x94, 0x77, 0xcd, 0xb5, 0xab, 0x26, 0x2, 0x6b, 0x17, 0x7a, 0x56, 0xf0, 0x24, 0x42, 0xf, 0xff, 0x2f, 0xa8, 0x71, 0xa3, 0x96, 0x89, 0x7f, 0x2e, 0x4d, 0x75, 0x1d, 0x14, 0x49, 0x8, 0xf7, 0x7d, 0xe2, 0x62, 0x27, 0x76, 0x95, 0xf7, 0x76, 0x24, 0x8f, 0x94, 0x87, 0xd5, 0xb6, 0x57, 0x47, 0x80, 0x29, 0x6c, 0x5c, 0x5e, 0x27, 0x2d, 0xac, 0x8e, 0xd, 0x6c, 0x51, 0x84, 0x50, 0xc6, 0x57, 0x5, 0x7a, 0xf, 0x7b, 0xe4, 0xd3, 0x67, 0x70, 0x24, 0x12, 0xea, 0x89, 0xe3, 0xab, 0x13, 0xd3, 0x1c, 0xd7, 0x69};

const unsigned char E8_bitslice_roundconstant[42][32] = {
	{0x72, 0xd5, 0xde, 0xa2, 0xdf, 0x15, 0xf8, 0x67, 0x7b, 0x84, 0x15, 0xa, 0xb7, 0x23, 0x15, 0x57, 0x81, 0xab, 0xd6, 0x90, 0x4d, 0x5a, 0x87, 0xf6, 0x4e, 0x9f, 0x4f, 0xc5, 0xc3, 0xd1, 0x2b, 0x40},
	{0xea, 0x98, 0x3a, 0xe0, 0x5c, 0x45, 0xfa, 0x9c, 0x3, 0xc5, 0xd2, 0x99, 0x66, 0xb2, 0x99, 0x9a, 0x66, 0x2, 0x96, 0xb4, 0xf2, 0xbb, 0x53, 0x8a, 0xb5, 0x56, 0x14, 0x1a, 0x88, 0xdb, 0xa2, 0x31},
	{0x3, 0xa3, 0x5a, 0x5c, 0x9a, 0x19, 0xe, 0xdb, 0x40, 0x3f, 0xb2, 0xa, 0x87, 0xc1, 0x44, 0x10, 0x1c, 0x5, 0x19, 0x80, 0x84, 0x9e, 0x95, 0x1d, 0x6f, 0x33, 0xeb, 0xad, 0x5e, 0xe7, 0xcd, 0xdc},
	{0x10, 0xba, 0x13, 0x92, 0x2, 0xbf, 0x6b, 0x41, 0xdc, 0x78, 0x65, 0x15, 0xf7, 0xbb, 0x27, 0xd0, 0xa, 0x2c, 0x81, 0x39, 0x37, 0xaa, 0x78, 0x50, 0x3f, 0x1a, 0xbf, 0xd2, 0x41, 0x0, 0x91, 0xd3},
	{0x42, 0x2d, 0x5a, 0xd, 0xf6, 0xcc, 0x7e, 0x90, 0xdd, 0x62, 0x9f, 0x9c, 0x92, 0xc0, 0x97, 0xce, 0x18, 0x5c, 0xa7, 0xb, 0xc7, 0x2b, 0x44, 0xac, 0xd1, 0xdf, 0x65, 0xd6, 0x63, 0xc6, 0xfc, 0x23},
	{0x97, 0x6e, 0x6c, 0x3, 0x9e, 0xe0, 0xb8, 0x1a, 0x21, 0x5, 0x45, 0x7e, 0x44, 0x6c, 0xec, 0xa8, 0xee, 0xf1, 0x3, 0xbb, 0x5d, 0x8e, 0x61, 0xfa, 0xfd, 0x96, 0x97, 0xb2, 0x94, 0x83, 0x81, 0x97},
	{0x4a, 0x8e, 0x85, 0x37, 0xdb, 0x3, 0x30, 0x2f, 0x2a, 0x67, 0x8d, 0x2d, 0xfb, 0x9f, 0x6a, 0x95, 0x8a, 0xfe, 0x73, 0x81, 0xf8, 0xb8, 0x69, 0x6c, 0x8a, 0xc7, 0x72, 0x46, 0xc0, 0x7f, 0x42, 0x14},
	{0xc5, 0xf4, 0x15, 0x8f, 0xbd, 0xc7, 0x5e, 0xc4, 0x75, 0x44, 0x6f, 0xa7, 0x8f, 0x11, 0xbb, 0x80, 0x52, 0xde, 0x75, 0xb7, 0xae, 0xe4, 0x88, 0xbc, 0x82, 0xb8, 0x0, 0x1e, 0x98, 0xa6, 0xa3, 0xf4},
	{0x8e, 0xf4, 0x8f, 0x33, 0xa9, 0xa3, 0x63, 0x15, 0xaa, 0x5f, 0x56, 0x24, 0xd5, 0xb7, 0xf9, 0x89, 0xb6, 0xf1, 0xed, 0x20, 0x7c, 0x5a, 0xe0, 0xfd, 0x36, 0xca, 0xe9, 0x5a, 0x6, 0x42, 0x2c, 0x36},
	{0xce, 0x29, 0x35, 0x43, 0x4e, 0xfe, 0x98, 0x3d, 0x53, 0x3a, 0xf9, 0x74, 0x73, 0x9a, 0x4b, 0xa7, 0xd0, 0xf5, 0x1f, 0x59, 0x6f, 0x4e, 0x81, 0x86, 0xe, 0x9d, 0xad, 0x81, 0xaf, 0xd8, 0x5a, 0x9f},
	{0xa7, 0x5, 0x6, 0x67, 0xee, 0x34, 0x62, 0x6a, 0x8b, 0xb, 0x28, 0xbe, 0x6e, 0xb9, 0x17, 0x27, 0x47, 0x74, 0x7, 0x26, 0xc6, 0x80, 0x10, 0x3f, 0xe0, 0xa0, 0x7e, 0x6f, 0xc6, 0x7e, 0x48, 0x7b},
	{0xd, 0x55, 0xa, 0xa5, 0x4a, 0xf8, 0xa4, 0xc0, 0x91, 0xe3, 0xe7, 0x9f, 0x97, 0x8e, 0xf1, 0x9e, 0x86, 0x76, 0x72, 0x81, 0x50, 0x60, 0x8d, 0xd4, 0x7e, 0x9e, 0x5a, 0x41, 0xf3, 0xe5, 0xb0, 0x62},
	{0xfc, 0x9f, 0x1f, 0xec, 0x40, 0x54, 0x20, 0x7a, 0xe3, 0xe4, 0x1a, 0x0, 0xce, 0xf4, 0xc9, 0x84, 0x4f, 0xd7, 0x94, 0xf5, 0x9d, 0xfa, 0x95, 0xd8, 0x55, 0x2e, 0x7e, 0x11, 0x24, 0xc3, 0x54, 0xa5},
	{0x5b, 0xdf, 0x72, 0x28, 0xbd, 0xfe, 0x6e, 0x28, 0x78, 0xf5, 0x7f, 0xe2, 0xf, 0xa5, 0xc4, 0xb2, 0x5, 0x89, 0x7c, 0xef, 0xee, 0x49, 0xd3, 0x2e, 0x44, 0x7e, 0x93, 0x85, 0xeb, 0x28, 0x59, 0x7f},
	{0x70, 0x5f, 0x69, 0x37, 0xb3, 0x24, 0x31, 0x4a, 0x5e, 0x86, 0x28, 0xf1, 0x1d, 0xd6, 0xe4, 0x65, 0xc7, 0x1b, 0x77, 0x4, 0x51, 0xb9, 0x20, 0xe7, 0x74, 0xfe, 0x43, 0xe8, 0x23, 0xd4, 0x87, 0x8a},
	{0x7d, 0x29, 0xe8, 0xa3, 0x92, 0x76, 0x94, 0xf2, 0xdd, 0xcb, 0x7a, 0x9, 0x9b, 0x30, 0xd9, 0xc1, 0x1d, 0x1b, 0x30, 0xfb, 0x5b, 0xdc, 0x1b, 0xe0, 0xda, 0x24, 0x49, 0x4f, 0xf2, 0x9c, 0x82, 0xbf},
	{0xa4, 0xe7, 0xba, 0x31, 0xb4, 0x70, 0xbf, 0xff, 0xd, 0x32, 0x44, 0x5, 0xde, 0xf8, 0xbc, 0x48, 0x3b, 0xae, 0xfc, 0x32, 0x53, 0xbb, 0xd3, 0x39, 0x45, 0x9f, 0xc3, 0xc1, 0xe0, 0x29, 0x8b, 0xa0},
	{0xe5, 0xc9, 0x5, 0xfd, 0xf7, 0xae, 0x9, 0xf, 0x94, 0x70, 0x34, 0x12, 0x42, 0x90, 0xf1, 0x34, 0xa2, 0x71, 0xb7, 0x1, 0xe3, 0x44, 0xed, 0x95, 0xe9, 0x3b, 0x8e, 0x36, 0x4f, 0x2f, 0x98, 0x4a},
	{0x88, 0x40, 0x1d, 0x63, 0xa0, 0x6c, 0xf6, 0x15, 0x47, 0xc1, 0x44, 0x4b, 0x87, 0x52, 0xaf, 0xff, 0x7e, 0xbb, 0x4a, 0xf1, 0xe2, 0xa, 0xc6, 0x30, 0x46, 0x70, 0xb6, 0xc5, 0xcc, 0x6e, 0x8c, 0xe6},
	{0xa4, 0xd5, 0xa4, 0x56, 0xbd, 0x4f, 0xca, 0x0, 0xda, 0x9d, 0x84, 0x4b, 0xc8, 0x3e, 0x18, 0xae, 0x73, 0x57, 0xce, 0x45, 0x30, 0x64, 0xd1, 0xad, 0xe8, 0xa6, 0xce, 0x68, 0x14, 0x5c, 0x25, 0x67},
	{0xa3, 0xda, 0x8c, 0xf2, 0xcb, 0xe, 0xe1, 0x16, 0x33, 0xe9, 0x6, 0x58, 0x9a, 0x94, 0x99, 0x9a, 0x1f, 0x60, 0xb2, 0x20, 0xc2, 0x6f, 0x84, 0x7b, 0xd1, 0xce, 0xac, 0x7f, 0xa0, 0xd1, 0x85, 0x18},
	{0x32, 0x59, 0x5b, 0xa1, 0x8d, 0xdd, 0x19, 0xd3, 0x50, 0x9a, 0x1c, 0xc0, 0xaa, 0xa5, 0xb4, 0x46, 0x9f, 0x3d, 0x63, 0x67, 0xe4, 0x4, 0x6b, 0xba, 0xf6, 0xca, 0x19, 0xab, 0xb, 0x56, 0xee, 0x7e},
	{0x1f, 0xb1, 0x79, 0xea, 0xa9, 0x28, 0x21, 0x74, 0xe9, 0xbd, 0xf7, 0x35, 0x3b, 0x36, 0x51, 0xee, 0x1d, 0x57, 0xac, 0x5a, 0x75, 0x50, 0xd3, 0x76, 0x3a, 0x46, 0xc2, 0xfe, 0xa3, 0x7d, 0x70, 0x1},
	{0xf7, 0x35, 0xc1, 0xaf, 0x98, 0xa4, 0xd8, 0x42, 0x78, 0xed, 0xec, 0x20, 0x9e, 0x6b, 0x67, 0x79, 0x41, 0x83, 0x63, 0x15, 0xea, 0x3a, 0xdb, 0xa8, 0xfa, 0xc3, 0x3b, 0x4d, 0x32, 0x83, 0x2c, 0x83},
	{0xa7, 0x40, 0x3b, 0x1f, 0x1c, 0x27, 0x47, 0xf3, 0x59, 0x40, 0xf0, 0x34, 0xb7, 0x2d, 0x76, 0x9a, 0xe7, 0x3e, 0x4e, 0x6c, 0xd2, 0x21, 0x4f, 0xfd, 0xb8, 0xfd, 0x8d, 0x39, 0xdc, 0x57, 0x59, 0xef},
	{0x8d, 0x9b, 0xc, 0x49, 0x2b, 0x49, 0xeb, 0xda, 0x5b, 0xa2, 0xd7, 0x49, 0x68, 0xf3, 0x70, 0xd, 0x7d, 0x3b, 0xae, 0xd0, 0x7a, 0x8d, 0x55, 0x84, 0xf5, 0xa5, 0xe9, 0xf0, 0xe4, 0xf8, 0x8e, 0x65},
	{0xa0, 0xb8, 0xa2, 0xf4, 0x36, 0x10, 0x3b, 0x53, 0xc, 0xa8, 0x7, 0x9e, 0x75, 0x3e, 0xec, 0x5a, 0x91, 0x68, 0x94, 0x92, 0x56, 0xe8, 0x88, 0x4f, 0x5b, 0xb0, 0x5c, 0x55, 0xf8, 0xba, 0xbc, 0x4c},
	{0xe3, 0xbb, 0x3b, 0x99, 0xf3, 0x87, 0x94, 0x7b, 0x75, 0xda, 0xf4, 0xd6, 0x72, 0x6b, 0x1c, 0x5d, 0x64, 0xae, 0xac, 0x28, 0xdc, 0x34, 0xb3, 0x6d, 0x6c, 0x34, 0xa5, 0x50, 0xb8, 0x28, 0xdb, 0x71},
	{0xf8, 0x61, 0xe2, 0xf2, 0x10, 0x8d, 0x51, 0x2a, 0xe3, 0xdb, 0x64, 0x33, 0x59, 0xdd, 0x75, 0xfc, 0x1c, 0xac, 0xbc, 0xf1, 0x43, 0xce, 0x3f, 0xa2, 0x67, 0xbb, 0xd1, 0x3c, 0x2, 0xe8, 0x43, 0xb0},
	{0x33, 0xa, 0x5b, 0xca, 0x88, 0x29, 0xa1, 0x75, 0x7f, 0x34, 0x19, 0x4d, 0xb4, 0x16, 0x53, 0x5c, 0x92, 0x3b, 0x94, 0xc3, 0xe, 0x79, 0x4d, 0x1e, 0x79, 0x74, 0x75, 0xd7, 0xb6, 0xee, 0xaf, 0x3f},
	{0xea, 0xa8, 0xd4, 0xf7, 0xbe, 0x1a, 0x39, 0x21, 0x5c, 0xf4, 0x7e, 0x9, 0x4c, 0x23, 0x27, 0x51, 0x26, 0xa3, 0x24, 0x53, 0xba, 0x32, 0x3c, 0xd2, 0x44, 0xa3, 0x17, 0x4a, 0x6d, 0xa6, 0xd5, 0xad},
	{0xb5, 0x1d, 0x3e, 0xa6, 0xaf, 0xf2, 0xc9, 0x8, 0x83, 0x59, 0x3d, 0x98, 0x91, 0x6b, 0x3c, 0x56, 0x4c, 0xf8, 0x7c, 0xa1, 0x72, 0x86, 0x60, 0x4d, 0x46, 0xe2, 0x3e, 0xcc, 0x8, 0x6e, 0xc7, 0xf6},
	{0x2f, 0x98, 0x33, 0xb3, 0xb1, 0xbc, 0x76, 0x5e, 0x2b, 0xd6, 0x66, 0xa5, 0xef, 0xc4, 0xe6, 0x2a, 0x6, 0xf4, 0xb6, 0xe8, 0xbe, 0xc1, 0xd4, 0x36, 0x74, 0xee, 0x82, 0x15, 0xbc, 0xef, 0x21, 0x63},
	{0xfd, 0xc1, 0x4e, 0xd, 0xf4, 0x53, 0xc9, 0x69, 0xa7, 0x7d, 0x5a, 0xc4, 0x6, 0x58, 0x58, 0x26, 0x7e, 0xc1, 0x14, 0x16, 0x6, 0xe0, 0xfa, 0x16, 0x7e, 0x90, 0xaf, 0x3d, 0x28, 0x63, 0x9d, 0x3f},
	{0xd2, 0xc9, 0xf2, 0xe3, 0x0, 0x9b, 0xd2, 0xc, 0x5f, 0xaa, 0xce, 0x30, 0xb7, 0xd4, 0xc, 0x30, 0x74, 0x2a, 0x51, 0x16, 0xf2, 0xe0, 0x32, 0x98, 0xd, 0xeb, 0x30, 0xd8, 0xe3, 0xce, 0xf8, 0x9a},
	{0x4b, 0xc5, 0x9e, 0x7b, 0xb5, 0xf1, 0x79, 0x92, 0xff, 0x51, 0xe6, 0x6e, 0x4, 0x86, 0x68, 0xd3, 0x9b, 0x23, 0x4d, 0x57, 0xe6, 0x96, 0x67, 0x31, 0xcc, 0xe6, 0xa6, 0xf3, 0x17, 0xa, 0x75, 0x5},
	{0xb1, 0x76, 0x81, 0xd9, 0x13, 0x32, 0x6c, 0xce, 0x3c, 0x17, 0x52, 0x84, 0xf8, 0x5, 0xa2, 0x62, 0xf4, 0x2b, 0xcb, 0xb3, 0x78, 0x47, 0x15, 0x47, 0xff, 0x46, 0x54, 0x82, 0x23, 0x93, 0x6a, 0x48},
	{0x38, 0xdf, 0x58, 0x7, 0x4e, 0x5e, 0x65, 0x65, 0xf2, 0xfc, 0x7c, 0x89, 0xfc, 0x86, 0x50, 0x8e, 0x31, 0x70, 0x2e, 0x44, 0xd0, 0xb, 0xca, 0x86, 0xf0, 0x40, 0x9, 0xa2, 0x30, 0x78, 0x47, 0x4e},
	{0x65, 0xa0, 0xee, 0x39, 0xd1, 0xf7, 0x38, 0x83, 0xf7, 0x5e, 0xe9, 0x37, 0xe4, 0x2c, 0x3a, 0xbd, 0x21, 0x97, 0xb2, 0x26, 0x1, 0x13, 0xf8, 0x6f, 0xa3, 0x44, 0xed, 0xd1, 0xef, 0x9f, 0xde, 0xe7},
	{0x8b, 0xa0, 0xdf, 0x15, 0x76, 0x25, 0x92, 0xd9, 0x3c, 0x85, 0xf7, 0xf6, 0x12, 0xdc, 0x42, 0xbe, 0xd8, 0xa7, 0xec, 0x7c, 0xab, 0x27, 0xb0, 0x7e, 0x53, 0x8d, 0x7d, 0xda, 0xaa, 0x3e, 0xa8, 0xde},
	{0xaa, 0x25, 0xce, 0x93, 0xbd, 0x2, 0x69, 0xd8, 0x5a, 0xf6, 0x43, 0xfd, 0x1a, 0x73, 0x8, 0xf9, 0xc0, 0x5f, 0xef, 0xda, 0x17, 0x4a, 0x19, 0xa5, 0x97, 0x4d, 0x66, 0x33, 0x4c, 0xfd, 0x21, 0x6a},
	{0x35, 0xb4, 0x98, 0x31, 0xdb, 0x41, 0x15, 0x70, 0xea, 0x1e, 0xf, 0xbb, 0xed, 0xcd, 0x54, 0x9b, 0x9a, 0xd0, 0x63, 0xa1, 0x51, 0x97, 0x40, 0x72, 0xf6, 0x75, 0x9d, 0xbf, 0x91, 0x47, 0x6f, 0xe2}};

#define SWAP1(x) (x) = ((((x)&0x5555555555555555ULL) << 1) | (((x)&0xaaaaaaaaaaaaaaaaULL) >> 1));
#define SWAP2(x) (x) = ((((x)&0x3333333333333333ULL) << 2) | (((x)&0xccccccccccccccccULL) >> 2));
#define SWAP4(x) (x) = ((((x)&0x0f0f0f0f0f0f0f0fULL) << 4) | (((x)&0xf0f0f0f0f0f0f0f0ULL) >> 4));
#define SWAP8(x) (x) = ((((x)&0x00ff00ff00ff00ffULL) << 8) | (((x)&0xff00ff00ff00ff00ULL) >> 8));
#define SWAP16(x) (x) = ((((x)&0x0000ffff0000ffffULL) << 16) | (((x)&0xffff0000ffff0000ULL) >> 16));
#define SWAP32(x) (x) = (((x) << 32) | ((x) >> 32));
#define L(m0, m1, m2, m3, m4, m5, m6, m7) \
	(m4) ^= (m1);                         \
	(m5) ^= (m2);                         \
	(m6) ^= (m0) ^ (m3);                  \
	(m7) ^= (m0);                         \
	(m0) ^= (m5);                         \
	(m1) ^= (m6);                         \
	(m2) ^= (m4) ^ (m7);                  \
	(m3) ^= (m4);
#define SS(m0, m1, m2, m3, m4, m5, m6, m7, cc0, cc1) \
	m3 = ~(m3);                                      \
	m7 = ~(m7);                                      \
	m0 ^= ((~(m2)) & (cc0));                         \
	m4 ^= ((~(m6)) & (cc1));                         \
	temp0 = (cc0) ^ ((m0) & (m1));                   \
	temp1 = (cc1) ^ ((m4) & (m5));                   \
	m0 ^= ((m2) & (m3));                             \
	m4 ^= ((m6) & (m7));                             \
	m3 ^= ((~(m1)) & (m2));                          \
	m7 ^= ((~(m5)) & (m6));                          \
	m1 ^= ((m0) & (m2));                             \
	m5 ^= ((m4) & (m6));                             \
	m2 ^= ((m0) & (~(m3)));                          \
	m6 ^= ((m4) & (~(m7)));                          \
	m0 ^= ((m1) | (m3));                             \
	m4 ^= ((m5) | (m7));                             \
	m3 ^= ((m1) & (m2));                             \
	m7 ^= ((m5) & (m6));                             \
	m1 ^= (temp0 & (m0));                            \
	m5 ^= (temp1 & (m4));                            \
	m2 ^= temp0;                                     \
	m6 ^= temp1;

static void F8(jh_ctx* state, const uint8_t* buffer)
{
	uint64_t i;
	for(i = 0; i < 8; i++)
		state->x[i >> 1][i & 1] ^= ((uint64_t*)buffer)[i];
	uint64_t roundnumber, temp0, temp1;
	for(roundnumber = 0; roundnumber < 42; roundnumber = roundnumber + 7)
	{
		for(i = 0; i < 2; i++)
		{
			SS(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 0])[i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 0])[i + 2]);
			L(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i]);
			SWAP1(state->x[1][i]);
			SWAP1(state->x[3][i]);
			SWAP1(state->x[5][i]);
			SWAP1(state->x[7][i]);
		}
		for(i = 0; i < 2; i++)
		{
			SS(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 1])[i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 1])[i + 2]);
			L(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i]);
			SWAP2(state->x[1][i]);
			SWAP2(state->x[3][i]);
			SWAP2(state->x[5][i]);
			SWAP2(state->x[7][i]);
		}
		for(i = 0; i < 2; i++)
		{
			SS(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 2])[i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 2])[i + 2]);
			L(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i]);
			SWAP4(state->x[1][i]);
			SWAP4(state->x[3][i]);
			SWAP4(state->x[5][i]);
			SWAP4(state->x[7][i]);
		}
		for(i = 0; i < 2; i++)
		{
			SS(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 3])[i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 3])[i + 2]);
			L(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i]);
			SWAP8(state->x[1][i]);
			SWAP8(state->x[3][i]);
			SWAP8(state->x[5][i]);
			SWAP8(state->x[7][i]);
		}
		for(i = 0; i < 2; i++)
		{
			SS(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 4])[i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 4])[i + 2]);
			L(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i]);
			SWAP16(state->x[1][i]);
			SWAP16(state->x[3][i]);
			SWAP16(state->x[5][i]);
			SWAP16(state->x[7][i]);
		}
		for(i = 0; i < 2; i++)
		{
			SS(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 5])[i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 5])[i + 2]);
			L(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i]);
			SWAP32(state->x[1][i]);
			SWAP32(state->x[3][i]);
			SWAP32(state->x[5][i]);
			SWAP32(state->x[7][i]);
		}
		for(i = 0; i < 2; i++)
		{
			SS(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 6])[i], ((uint64_t*)E8_bitslice_roundconstant[roundnumber + 6])[i + 2]);
			L(state->x[0][i], state->x[2][i], state->x[4][i], state->x[6][i], state->x[1][i], state->x[3][i], state->x[5][i], state->x[7][i]);
		}
		for(i = 1; i < 8; i = i + 2)
		{
			temp0 = state->x[i][0];
			state->x[i][0] = state->x[i][1];
			state->x[i][1] = temp0;
		}
	}
	state->x[4][0] ^= ((uint64_t*)buffer)[0];
	state->x[4][1] ^= ((uint64_t*)buffer)[1];
	state->x[5][0] ^= ((uint64_t*)buffer)[2];
	state->x[5][1] ^= ((uint64_t*)buffer)[3];
	state->x[6][0] ^= ((uint64_t*)buffer)[4];
	state->x[6][1] ^= ((uint64_t*)buffer)[5];
	state->x[7][0] ^= ((uint64_t*)buffer)[6];
	state->x[7][1] ^= ((uint64_t*)buffer)[7];
}

void jh_hash(const uint8_t* data, uint8_t* hashval)
{
	jh_ctx state;
	memcpy(state.x, JH256_H0, 128);
	F8(&state, data);
	F8(&state, data + 64);
	F8(&state, data + 128);
	uint8_t buf[64];
	memcpy(buf, data + 192, 8);
	memset(buf + 8, 0, 56);
	buf[8] = 128;
	F8(&state, buf);
	memset(buf, 0, 12);
	buf[62] = 6;
	buf[63] = 64;
	F8(&state, buf);
	memcpy(hashval, (uint8_t*)state.x + 96, 32);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///// skein_hash
///////////////////////////////////////////////////////////////////////////////////////////////

#define RotL_64(x, N) (((x) << (N)) | ((x) >> (64 - (N))))

typedef struct
{
	uint64_t T[2];
	uint64_t X[8];
} skein_ctx;

enum
{
	R_512_0_0 = 46,
	R_512_0_1 = 36,
	R_512_0_2 = 19,
	R_512_0_3 = 37,
	R_512_1_0 = 33,
	R_512_1_1 = 27,
	R_512_1_2 = 14,
	R_512_1_3 = 42,
	R_512_2_0 = 17,
	R_512_2_1 = 49,
	R_512_2_2 = 36,
	R_512_2_3 = 39,
	R_512_3_0 = 44,
	R_512_3_1 = 9,
	R_512_3_2 = 54,
	R_512_3_3 = 56,
	R_512_4_0 = 39,
	R_512_4_1 = 30,
	R_512_4_2 = 34,
	R_512_4_3 = 24,
	R_512_5_0 = 13,
	R_512_5_1 = 50,
	R_512_5_2 = 10,
	R_512_5_3 = 17,
	R_512_6_0 = 25,
	R_512_6_1 = 29,
	R_512_6_2 = 39,
	R_512_6_3 = 43,
	R_512_7_0 = 8,
	R_512_7_1 = 35,
	R_512_7_2 = 56,
	R_512_7_3 = 22,
};

const uint64_t SKEIN_512_IV_256[] =
	{
		0xCCD044A12FDB3E13, 0xE83590301A79A9EB,
		0x55AEA0614F816E6F, 0x2A2767A4AE9B94DB,
		0xEC06025E74DD7683, 0xE7A436CDC4746251,
		0xC36FBAF9393AD185, 0x3EEDBA1833EDFC13};

#define ks (kw + 3)
#define ts (kw)
static void Skein_512_Process_Block(skein_ctx* ctx, const uint8_t* blkPtr, size_t blkCnt, size_t byteCntAdd)
{
	uint64_t kw[8 + 4];
	uint64_t X0, X1, X2, X3, X4, X5, X6, X7;
	uint64_t w[8];
	ts[0] = ctx->T[0];
	ts[1] = ctx->T[1];
	do
	{
		ts[0] += byteCntAdd;
		ks[0] = ctx->X[0];
		ks[1] = ctx->X[1];
		ks[2] = ctx->X[2];
		ks[3] = ctx->X[3];
		ks[4] = ctx->X[4];
		ks[5] = ctx->X[5];
		ks[6] = ctx->X[6];
		ks[7] = ctx->X[7];
		ks[8] = ks[0] ^ ks[1] ^ ks[2] ^ ks[3] ^ ks[4] ^ ks[5] ^ ks[6] ^ ks[7] ^ 2004413935125273122ull;
		ts[2] = ts[0] ^ ts[1];
		for(size_t n = 0; n < 64; n += 8)
			w[n / 8] = (((uint64_t)blkPtr[n])) +
					   (((uint64_t)blkPtr[n + 1]) << 8) +
					   (((uint64_t)blkPtr[n + 2]) << 16) +
					   (((uint64_t)blkPtr[n + 3]) << 24) +
					   (((uint64_t)blkPtr[n + 4]) << 32) +
					   (((uint64_t)blkPtr[n + 5]) << 40) +
					   (((uint64_t)blkPtr[n + 6]) << 48) +
					   (((uint64_t)blkPtr[n + 7]) << 56);
		ctx->T[0] = ts[0];
		ctx->T[1] = ts[1];
		X0 = w[0] + ks[0];
		X1 = w[1] + ks[1];
		X2 = w[2] + ks[2];
		X3 = w[3] + ks[3];
		X4 = w[4] + ks[4];
		X5 = w[5] + ks[5] + ts[0];
		X6 = w[6] + ks[6] + ts[1];
		X7 = w[7] + ks[7];
		blkPtr += 64;
#define Round512(p0, p1, p2, p3, p4, p5, p6, p7, ROT, rNum) \
	X##p0 += X##p1;                                         \
	X##p1 = RotL_64(X##p1, ROT##_0);                        \
	X##p1 ^= X##p0;                                         \
	X##p2 += X##p3;                                         \
	X##p3 = RotL_64(X##p3, ROT##_1);                        \
	X##p3 ^= X##p2;                                         \
	X##p4 += X##p5;                                         \
	X##p5 = RotL_64(X##p5, ROT##_2);                        \
	X##p5 ^= X##p4;                                         \
	X##p6 += X##p7;                                         \
	X##p7 = RotL_64(X##p7, ROT##_3);                        \
	X##p7 ^= X##p6;
#define R512(p0, p1, p2, p3, p4, p5, p6, p7, ROT, rNum) \
	Round512(p0, p1, p2, p3, p4, p5, p6, p7, ROT, rNum)
#define I512(R)                                  \
	X0 += ks[((R) + 1) % 9];                     \
	X1 += ks[((R) + 2) % 9];                     \
	X2 += ks[((R) + 3) % 9];                     \
	X3 += ks[((R) + 4) % 9];                     \
	X4 += ks[((R) + 5) % 9];                     \
	X5 += ks[((R) + 6) % 9] + ts[((R) + 1) % 3]; \
	X6 += ks[((R) + 7) % 9] + ts[((R) + 2) % 3]; \
	X7 += ks[((R) + 8) % 9] + (R) + 1;
		{
#define R512_8_rounds(R)                                \
	R512(0, 1, 2, 3, 4, 5, 6, 7, R_512_0, 8 * (R) + 1); \
	R512(2, 1, 4, 7, 6, 5, 0, 3, R_512_1, 8 * (R) + 2); \
	R512(4, 1, 6, 3, 0, 5, 2, 7, R_512_2, 8 * (R) + 3); \
	R512(6, 1, 0, 7, 2, 5, 4, 3, R_512_3, 8 * (R) + 4); \
	I512(2 * (R));                                      \
	R512(0, 1, 2, 3, 4, 5, 6, 7, R_512_4, 8 * (R) + 5); \
	R512(2, 1, 4, 7, 6, 5, 0, 3, R_512_5, 8 * (R) + 6); \
	R512(4, 1, 6, 3, 0, 5, 2, 7, R_512_6, 8 * (R) + 7); \
	R512(6, 1, 0, 7, 2, 5, 4, 3, R_512_7, 8 * (R) + 8); \
	I512(2 * (R) + 1);
			R512_8_rounds(0);
			R512_8_rounds(1);
			R512_8_rounds(2);
			R512_8_rounds(3);
			R512_8_rounds(4);
			R512_8_rounds(5);
			R512_8_rounds(6);
			R512_8_rounds(7);
			R512_8_rounds(8);
		}
		ctx->X[0] = X0 ^ w[0];
		ctx->X[1] = X1 ^ w[1];
		ctx->X[2] = X2 ^ w[2];
		ctx->X[3] = X3 ^ w[3];
		ctx->X[4] = X4 ^ w[4];
		ctx->X[5] = X5 ^ w[5];
		ctx->X[6] = X6 ^ w[6];
		ctx->X[7] = X7 ^ w[7];
		ts[1] &= ~4611686018427387904ull;
	} while(--blkCnt);
	ctx->T[0] = ts[0];
	ctx->T[1] = ts[1];
}

void skein_hash(const uint8_t* data, uint8_t* hashval)
{
	skein_ctx state;
	uint8_t b[64];
	memcpy(state.X, SKEIN_512_IV_256, sizeof(state.X));
	state.T[0] = 0;
	state.T[1] = 8070450532247928832ull;
	Skein_512_Process_Block(&state, data, 3, 64);
	memcpy(b, data + 192, 8);
	memset(b + 8, 0, 56);
	state.T[1] |= 9223372036854775808ull;
	Skein_512_Process_Block(&state, b, 1, 8);
	memset(b, 0, 8);
	state.T[0] = 0;
	state.T[1] = 18374686479671623680ull;
	Skein_512_Process_Block(&state, b, 1, sizeof(uint64_t));
	memcpy(hashval, state.X, 32);
}
