// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/lib/testing/test_framework/check.h"
#include "sw/device/lib/testing/test_framework/ottf_main.h"
#include "sw/device/silicon_creator/lib/base/sec_mmio.h"
#include "sw/device/silicon_creator/lib/sigverify/sigverify.h"

static const char kMessage[] = "test message";

// Digest of the test message above.
hmac_digest_t act_digest;

// See sw/device/silicon_creator/keys/README.md for more details on how to
// update the structs below.

static const sigverify_rsa_buffer_t kSignatureExp65537 = {
    .data = {
        0x725bfa2c, 0xdb359e00, 0x4dd50e25, 0x344ce68a, 0x2d49dc6b, 0x4a53a013,
        0x2abd4a7c, 0x762dd4aa, 0xe1935a41, 0xb807b2c2, 0xdf0222d7, 0x2dc12fdf,
        0xe432fb54, 0x2a12e15d, 0xf290eb01, 0x2529d6d4, 0x0813ab70, 0x78bd8229,
        0x63f3064e, 0x1cceba14, 0x4beff42b, 0xb9e98de4, 0x84a7f442, 0xb03649bc,
        0x7726af3d, 0xeaf2656d, 0xf82f963b, 0x31082a3d, 0x194ff701, 0x86588b75,
        0x5732f5de, 0x35d14195, 0x262c612d, 0x3f66ce59, 0xa2742c75, 0x276341fb,
        0x8cb84d0a, 0x1222f7f6, 0xbbd8ec56, 0x36e629b1, 0x891fd231, 0xfb351d0c,
        0x598dab98, 0x64534c32, 0xbcc39e4c, 0x256e4544, 0x3a3205ab, 0x02c5878c,
        0x99a7e70a, 0xc65c4d5d, 0xe5bedc24, 0x83de5d15, 0x16429111, 0x05d0b216,
        0xbf8d4dfe, 0x4be3707f, 0x004d6b75, 0xd64b4c66, 0x6e9e4375, 0xa5e1fc9f,
        0x4ca3c8f2, 0x544cf3d2, 0x34767ef2, 0xc361639c, 0x6062f836, 0x558ebb62,
        0xec7ee0af, 0x11033e71, 0x873742d3, 0x0ad49285, 0x6f163385, 0xd880305f,
        0x76e79003, 0x2bd4c955, 0x4a00fd2a, 0x7a045dd4, 0xdf671f3f, 0xd986e081,
        0x96cfc193, 0xd211ece5, 0x4486f7cb, 0x47be12f5, 0xe513619c, 0xe1a5f41c,
        0xbc4fbcb3, 0x78b903b7, 0xc8dcbff8, 0x5c088a19, 0x66301acc, 0x12b05bf9,
        0xa9c795a9, 0xe229e3ca, 0xe928d10b, 0x96eda9d9, 0x162f4a58, 0x069b950c,
    }};

static const sigverify_rsa_buffer_t kSignatureExp3 = {
    .data = {
        0x6a115691, 0xd07331f4, 0x9bfedfd8, 0x3004add3, 0x16a6abae, 0xfabe4be8,
        0xf2ab6423, 0x2a0cd35f, 0x7d35d309, 0xa7bbb201, 0xdc84059d, 0x1fb3e8e3,
        0x44311d7c, 0xc8827aa2, 0x1acc18ca, 0xe4c6fcbc, 0x103b5c35, 0x07d23ec1,
        0x8d086959, 0xa6af591c, 0x97fbea61, 0xb53fc23e, 0x2706405d, 0xde4bd032,
        0x4801e809, 0x6f5494a5, 0xa6c7ef11, 0xf8ac9ea1, 0x68f8c8a7, 0x56422d16,
        0x008659a1, 0x86f46918, 0x9704b7b2, 0xdbfa2abb, 0x1118894a, 0x25a6d273,
        0xe6eabcdd, 0x30c1f476, 0x84eb0986, 0xd271c618, 0xef168138, 0x2e02ec60,
        0xf8a6a633, 0xee9bb928, 0x371d447d, 0x5712ff98, 0xb52298f4, 0xd81d9860,
        0x3d1dda2e, 0x7394aeb9, 0x23c8a404, 0x59da1407, 0x4bce183a, 0x1dfca04d,
        0x6ee512e2, 0x6de88f40, 0x6fd447cd, 0xad202b15, 0xbafec85b, 0x1ccd55fc,
        0x6b4a1bef, 0x746128b6, 0x6e3a352b, 0xbf45d24c, 0xc9cd74e3, 0x9ac3fa48,
        0x04fdaacd, 0xa34d6290, 0xe4dda119, 0xaaffa422, 0x8ecb4296, 0xfbe34fb4,
        0x2435689d, 0x615a8633, 0x608c788c, 0x3487912e, 0xf9e1467c, 0x23a03c27,
        0x7142c168, 0xaae442df, 0xde84432a, 0x73bd51e0, 0xb0244c65, 0x77fbf67b,
        0xe34a4e12, 0x9ce334c5, 0xda73f20f, 0x1e821af7, 0x5c21d22d, 0x1d761f2f,
        0x55e2c441, 0xa6141c6f, 0x0b691a17, 0xbe151202, 0xb9f0e104, 0x5d411db9,
    }};

// sw/device/silicon_creator/rom/keys/fake/test_key_0_rsa_3072_exp_f4.public.der
static const sigverify_rsa_key_t kKeyExp65537 = {
    .n = {{
        0x5801a2bd, 0xeff64a46, 0xc8cf2251, 0xa7cd62cb, 0x634a39c2, 0x55c936d3,
        0x463d61fc, 0x762ebbaa, 0x01aadfb2, 0x23da15d1, 0x8475fdc6, 0x4ec67b7b,
        0xe9364570, 0xd23ec7c7, 0x98038d63, 0x5688a56b, 0x68037add, 0xb20ff289,
        0x9d96c1ce, 0xbac0b8cd, 0xead33d0b, 0x195f89c8, 0xd7dc110e, 0xf5bccc12,
        0x8dfa33dc, 0xedc404d2, 0x74ef8524, 0x9197c0c8, 0x79cc448e, 0x4c9c505d,
        0x4a586ad7, 0xe2d0f071, 0x589f28c2, 0x2ca7fc22, 0x0354b0e2, 0xefb63b44,
        0x33a75b04, 0x9e194454, 0x1b4b2cde, 0x8e3f78e0, 0x5260877c, 0x05685b72,
        0x4868ad4e, 0x10303ac9, 0x05ac2411, 0x5e797381, 0xd5407668, 0xe3522348,
        0xa33134f8, 0x38f7a953, 0xd926f672, 0x136f6753, 0xb186b0ab, 0x5ccab586,
        0x61e5bf2e, 0x9fc0eebb, 0x788ed0bd, 0x47b5fc70, 0xf971262a, 0x3b40d99b,
        0x5b9fd926, 0xce3c93bf, 0xd406005e, 0x72b9e555, 0xc9b9273e, 0xfcef747f,
        0xf0a35598, 0x2761e8f6, 0xec1799df, 0x462bc52d, 0x8e47218b, 0x429ccdae,
        0xe7e7d66c, 0x70c70b03, 0x0356c3d2, 0x3cb3e7d1, 0xd42d035d, 0x83c529a3,
        0x8df9930e, 0xb082e1f0, 0x07509c30, 0x5c33a350, 0x4f6884b9, 0x7b9d2de0,
        0x0f1d16b3, 0x38dbcf55, 0x168580ea, 0xc2f2aca4, 0x43f0ae60, 0x227dd2ed,
        0xd8dc61f4, 0x9404e8bc, 0x0db76fe3, 0x3491d3b0, 0x6ca44e27, 0xcda63719,
    }},
    .n0_inv =
        {
            0x9c9a176b,
            0x44d6fa52,
            0x71a63ec4,
            0xadc94595,
            0x3fd9bc73,
            0xa83cdc95,
            0xbe1bc819,
            0x2b421fae,
        },
};

static const sigverify_rsa_key_t kKeyExp3 = {
    .n = {{
        0xbd158913, 0xab75ea1a, 0xc04e5292, 0x68f5778a, 0xa71418c7, 0xddc4fc1c,
        0xcb09302d, 0xedf3142b, 0x656d7d85, 0xf761d32a, 0x2d334d1b, 0x26c91770,
        0x5b9ba5a0, 0x00ac6c05, 0xbabaf1bb, 0xa8299ecc, 0xb4223f99, 0x5b676ad3,
        0xcaa786c2, 0x3e2f1785, 0x204b6991, 0x21fa118f, 0x435573ab, 0xa3353ba1,
        0x1074c161, 0x2ad5e901, 0x7310247c, 0x1e21b8e9, 0x0cfc7762, 0x0a9139b1,
        0xfc655b33, 0x6990faaf, 0xbb88faec, 0x7c7bd6ef, 0x261e4555, 0x6bc3d813,
        0x5ce6e18b, 0xdd308629, 0x37d3d54d, 0x65acd84d, 0x97b7e0c3, 0xc0d35caa,
        0xb0be177a, 0x09473af3, 0x67f43155, 0x3b2f7661, 0xf9255df2, 0x1b42c84c,
        0x355cd607, 0x835e74ca, 0x1d011c4e, 0x46652555, 0x1566f96f, 0x6cffd2f9,
        0x204e783e, 0xa178a2eb, 0xe7297a95, 0xd7380039, 0x1a685545, 0x76ed97c9,
        0x6bc0b1b7, 0xd9b1338e, 0xa3b23005, 0x6fe7109f, 0x01c232e1, 0x851639c5,
        0xe81d338c, 0x25ebe0c4, 0x5b0202cd, 0x3690cb70, 0xad13b664, 0x8bf7833e,
        0x6017349c, 0xf6e90b08, 0x953ef3d8, 0x4bc11817, 0xd0f6e840, 0xfe01a954,
        0x9b866209, 0xb9653ff8, 0x0d654f5c, 0xff78177c, 0x3688833c, 0x57cc0c30,
        0x71965be7, 0xf61fb728, 0xaeac8ca2, 0xbdc9848b, 0x954c529f, 0x9917ac7f,
        0x4ba4c007, 0xce2dbf0b, 0xfc7d8504, 0x2712580b, 0xd0293151, 0xa4dbbff3,
    }},
    .n0_inv =
        {
            0x079056e5,
            0xe151dae1,
            0xd4f9deee,
            0xe18c4cab,
            0x868f9abe,
            0x8643ed1c,
            0x58022be6,
            0x8f8972c9,
        },
};

void compute_digest(void) {
  hmac_sha256_init();
  hmac_sha256_update(&kMessage, sizeof(kMessage) - 1);
  hmac_sha256_final(&act_digest);
}

rom_error_t sigverify_test_exp_3(void) {
  uint32_t flash_exec = 0;
  // Signature verification should fail when using exponent 3.
  if (sigverify_rsa_verify(&kSignatureExp3, &kKeyExp3, &act_digest, kLcStateRma,
                           &flash_exec) == kErrorOk) {
    return kErrorUnknown;
  }
  CHECK(flash_exec == UINT32_MAX);
  return kErrorOk;
}

rom_error_t sigverify_test_exp_65537(void) {
  uint32_t flash_exec = 0;
  rom_error_t result =
      sigverify_rsa_verify(&kSignatureExp65537, &kKeyExp65537, &act_digest,
                           kLcStateRma, &flash_exec);
  CHECK(flash_exec == kSigverifyRsaSuccess);
  return result;
}

rom_error_t sigverify_test_negative(void) {
  uint32_t flash_exec = 0;
  // Signature verification should fail when using the wrong signature.
  if (sigverify_rsa_verify(&kSignatureExp65537, &kKeyExp3, &act_digest,
                           kLcStateRma, &flash_exec) == kErrorOk) {
    return kErrorUnknown;
  }
  CHECK(flash_exec == UINT32_MAX);
  return kErrorOk;
}

OTTF_DEFINE_TEST_CONFIG();

bool test_main(void) {
  status_t result = OK_STATUS();

  compute_digest();

  EXECUTE_TEST(result, sigverify_test_exp_3);
  EXECUTE_TEST(result, sigverify_test_exp_65537);
  EXECUTE_TEST(result, sigverify_test_negative);
  return status_ok(result);
}
