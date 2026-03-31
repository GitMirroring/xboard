/*
 * book.c -- code for probing Polyglot opening books
 *
 * This code was first released in the public domain by Michel Van den Bergh.
 * The array Random64 is taken from the Polyglot source code.
 * I am pretty sure that a table of random numbers is never protected
 * by copyright.
 *
 * It s adapted by H.G. Muller for working with xboard / Winboard
 *
 * The following terms apply to the enhanced version of XBoard distributed
 * by the Free Software Foundation:
 * ------------------------------------------------------------------------
 *
 * GNU XBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU XBoard is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 * ------------------------------------------------------------------------
 */

#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "moves.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define _(s) gettext(s)
# define N_(s) gettext_noop(s)
#else
# define _(s) (s)
# define N_(s) s
#endif

typedef struct {
    uint64_t key;
    uint16_t move;
    uint16_t weight;
    uint16_t learnPoints;
    uint16_t learnCount;
} entry_t;

entry_t entry_none = {0, 0, 0, 0};

char * promote_pieces = " nbrqac=+";

uint64_t Random64[781] = {
 UINT64_C(0x9d39247e33776d41),
 UINT64_C(0x2af7398005aaa5c7),
 UINT64_C(0x44db015024623547),
 UINT64_C(0x9c15f73e62a76ae2),
 UINT64_C(0x75834465489c0c89),
 UINT64_C(0x3290ac3a203001bf),
 UINT64_C(0x0fbbad1f61042279),
 UINT64_C(0xe83a908ff2fb60ca),
 UINT64_C(0x0d7e765d58755c10),
 UINT64_C(0x1a083822ceafe02d),
 UINT64_C(0x9605d5f0e25ec3b0),
 UINT64_C(0xd021ff5cd13a2ed5),
 UINT64_C(0x40bdf15d4a672e32),
 UINT64_C(0x011355146fd56395),
 UINT64_C(0x5db4832046f3d9e5),
 UINT64_C(0x239f8b2d7ff719cc),
 UINT64_C(0x05d1a1ae85b49aa1),
 UINT64_C(0x679f848f6e8fc971),
 UINT64_C(0x7449bbff801fed0b),
 UINT64_C(0x7d11cdb1c3b7adf0),
 UINT64_C(0x82c7709e781eb7cc),
 UINT64_C(0xf3218f1c9510786c),
 UINT64_C(0x331478f3af51bbe6),
 UINT64_C(0x4bb38de5e7219443),
 UINT64_C(0xaa649c6ebcfd50fc),
 UINT64_C(0x8dbd98a352afd40b),
 UINT64_C(0x87d2074b81d79217),
 UINT64_C(0x19f3c751d3e92ae1),
 UINT64_C(0xb4ab30f062b19abf),
 UINT64_C(0x7b0500ac42047ac4),
 UINT64_C(0xc9452ca81a09d85d),
 UINT64_C(0x24aa6c514da27500),
 UINT64_C(0x4c9f34427501b447),
 UINT64_C(0x14a68fd73c910841),
 UINT64_C(0xa71b9b83461cbd93),
 UINT64_C(0x03488b95b0f1850f),
 UINT64_C(0x637b2b34ff93c040),
 UINT64_C(0x09d1bc9a3dd90a94),
 UINT64_C(0x3575668334a1dd3b),
 UINT64_C(0x735e2b97a4c45a23),
 UINT64_C(0x18727070f1bd400b),
 UINT64_C(0x1fcbacd259bf02e7),
 UINT64_C(0xd310a7c2ce9b6555),
 UINT64_C(0xbf983fe0fe5d8244),
 UINT64_C(0x9f74d14f7454a824),
 UINT64_C(0x51ebdc4ab9ba3035),
 UINT64_C(0x5c82c505db9ab0fa),
 UINT64_C(0xfcf7fe8a3430b241),
 UINT64_C(0x3253a729b9ba3dde),
 UINT64_C(0x8c74c368081b3075),
 UINT64_C(0xb9bc6c87167c33e7),
 UINT64_C(0x7ef48f2b83024e20),
 UINT64_C(0x11d505d4c351bd7f),
 UINT64_C(0x6568fca92c76a243),
 UINT64_C(0x4de0b0f40f32a7b8),
 UINT64_C(0x96d693460cc37e5d),
 UINT64_C(0x42e240cb63689f2f),
 UINT64_C(0x6d2bdcdae2919661),
 UINT64_C(0x42880b0236e4d951),
 UINT64_C(0x5f0f4a5898171bb6),
 UINT64_C(0x39f890f579f92f88),
 UINT64_C(0x93c5b5f47356388b),
 UINT64_C(0x63dc359d8d231b78),
 UINT64_C(0xec16ca8aea98ad76),
 UINT64_C(0x5355f900c2a82dc7),
 UINT64_C(0x07fb9f855a997142),
 UINT64_C(0x5093417aa8a7ed5e),
 UINT64_C(0x7bcbc38da25a7f3c),
 UINT64_C(0x19fc8a768cf4b6d4),
 UINT64_C(0x637a7780decfc0d9),
 UINT64_C(0x8249a47aee0e41f7),
 UINT64_C(0x79ad695501e7d1e8),
 UINT64_C(0x14acbaf4777d5776),
 UINT64_C(0xf145b6beccdea195),
 UINT64_C(0xdabf2ac8201752fc),
 UINT64_C(0x24c3c94df9c8d3f6),
 UINT64_C(0xbb6e2924f03912ea),
 UINT64_C(0x0ce26c0b95c980d9),
 UINT64_C(0xa49cd132bfbf7cc4),
 UINT64_C(0xe99d662af4243939),
 UINT64_C(0x27e6ad7891165c3f),
 UINT64_C(0x8535f040b9744ff1),
 UINT64_C(0x54b3f4fa5f40d873),
 UINT64_C(0x72b12c32127fed2b),
 UINT64_C(0xee954d3c7b411f47),
 UINT64_C(0x9a85ac909a24eaa1),
 UINT64_C(0x70ac4cd9f04f21f5),
 UINT64_C(0xf9b89d3e99a075c2),
 UINT64_C(0x87b3e2b2b5c907b1),
 UINT64_C(0xa366e5b8c54f48b8),
 UINT64_C(0xae4a9346cc3f7cf2),
 UINT64_C(0x1920c04d47267bbd),
 UINT64_C(0x87bf02c6b49e2ae9),
 UINT64_C(0x092237ac237f3859),
 UINT64_C(0xff07f64ef8ed14d0),
 UINT64_C(0x8de8dca9f03cc54e),
 UINT64_C(0x9c1633264db49c89),
 UINT64_C(0xb3f22c3d0b0b38ed),
 UINT64_C(0x390e5fb44d01144b),
 UINT64_C(0x5bfea5b4712768e9),
 UINT64_C(0x1e1032911fa78984),
 UINT64_C(0x9a74acb964e78cb3),
 UINT64_C(0x4f80f7a035dafb04),
 UINT64_C(0x6304d09a0b3738c4),
 UINT64_C(0x2171e64683023a08),
 UINT64_C(0x5b9b63eb9ceff80c),
 UINT64_C(0x506aacf489889342),
 UINT64_C(0x1881afc9a3a701d6),
 UINT64_C(0x6503080440750644),
 UINT64_C(0xdfd395339cdbf4a7),
 UINT64_C(0xef927dbcf00c20f2),
 UINT64_C(0x7b32f7d1e03680ec),
 UINT64_C(0xb9fd7620e7316243),
 UINT64_C(0x05a7e8a57db91b77),
 UINT64_C(0xb5889c6e15630a75),
 UINT64_C(0x4a750a09ce9573f7),
 UINT64_C(0xcf464cec899a2f8a),
 UINT64_C(0xf538639ce705b824),
 UINT64_C(0x3c79a0ff5580ef7f),
 UINT64_C(0xede6c87f8477609d),
 UINT64_C(0x799e81f05bc93f31),
 UINT64_C(0x86536b8cf3428a8c),
 UINT64_C(0x97d7374c60087b73),
 UINT64_C(0xa246637cff328532),
 UINT64_C(0x043fcae60cc0eba0),
 UINT64_C(0x920e449535dd359e),
 UINT64_C(0x70eb093b15b290cc),
 UINT64_C(0x73a1921916591cbd),
 UINT64_C(0x56436c9fe1a1aa8d),
 UINT64_C(0xefac4b70633b8f81),
 UINT64_C(0xbb215798d45df7af),
 UINT64_C(0x45f20042f24f1768),
 UINT64_C(0x930f80f4e8eb7462),
 UINT64_C(0xff6712ffcfd75ea1),
 UINT64_C(0xae623fd67468aa70),
 UINT64_C(0xdd2c5bc84bc8d8fc),
 UINT64_C(0x7eed120d54cf2dd9),
 UINT64_C(0x22fe545401165f1c),
 UINT64_C(0xc91800e98fb99929),
 UINT64_C(0x808bd68e6ac10365),
 UINT64_C(0xdec468145b7605f6),
 UINT64_C(0x1bede3a3aef53302),
 UINT64_C(0x43539603d6c55602),
 UINT64_C(0xaa969b5c691ccb7a),
 UINT64_C(0xa87832d392efee56),
 UINT64_C(0x65942c7b3c7e11ae),
 UINT64_C(0xded2d633cad004f6),
 UINT64_C(0x21f08570f420e565),
 UINT64_C(0xb415938d7da94e3c),
 UINT64_C(0x91b859e59ecb6350),
 UINT64_C(0x10cff333e0ed804a),
 UINT64_C(0x28aed140be0bb7dd),
 UINT64_C(0xc5cc1d89724fa456),
 UINT64_C(0x5648f680f11a2741),
 UINT64_C(0x2d255069f0b7dab3),
 UINT64_C(0x9bc5a38ef729abd4),
 UINT64_C(0xef2f054308f6a2bc),
 UINT64_C(0xaf2042f5cc5c2858),
 UINT64_C(0x480412bab7f5be2a),
 UINT64_C(0xaef3af4a563dfe43),
 UINT64_C(0x19afe59ae451497f),
 UINT64_C(0x52593803dff1e840),
 UINT64_C(0xf4f076e65f2ce6f0),
 UINT64_C(0x11379625747d5af3),
 UINT64_C(0xbce5d2248682c115),
 UINT64_C(0x9da4243de836994f),
 UINT64_C(0x066f70b33fe09017),
 UINT64_C(0x4dc4de189b671a1c),
 UINT64_C(0x51039ab7712457c3),
 UINT64_C(0xc07a3f80c31fb4b4),
 UINT64_C(0xb46ee9c5e64a6e7c),
 UINT64_C(0xb3819a42abe61c87),
 UINT64_C(0x21a007933a522a20),
 UINT64_C(0x2df16f761598aa4f),
 UINT64_C(0x763c4a1371b368fd),
 UINT64_C(0xf793c46702e086a0),
 UINT64_C(0xd7288e012aeb8d31),
 UINT64_C(0xde336a2a4bc1c44b),
 UINT64_C(0x0bf692b38d079f23),
 UINT64_C(0x2c604a7a177326b3),
 UINT64_C(0x4850e73e03eb6064),
 UINT64_C(0xcfc447f1e53c8e1b),
 UINT64_C(0xb05ca3f564268d99),
 UINT64_C(0x9ae182c8bc9474e8),
 UINT64_C(0xa4fc4bd4fc5558ca),
 UINT64_C(0xe755178d58fc4e76),
 UINT64_C(0x69b97db1a4c03dfe),
 UINT64_C(0xf9b5b7c4acc67c96),
 UINT64_C(0xfc6a82d64b8655fb),
 UINT64_C(0x9c684cb6c4d24417),
 UINT64_C(0x8ec97d2917456ed0),
 UINT64_C(0x6703df9d2924e97e),
 UINT64_C(0xc547f57e42a7444e),
 UINT64_C(0x78e37644e7cad29e),
 UINT64_C(0xfe9a44e9362f05fa),
 UINT64_C(0x08bd35cc38336615),
 UINT64_C(0x9315e5eb3a129ace),
 UINT64_C(0x94061b871e04df75),
 UINT64_C(0xdf1d9f9d784ba010),
 UINT64_C(0x3bba57b68871b59d),
 UINT64_C(0xd2b7adeeded1f73f),
 UINT64_C(0xf7a255d83bc373f8),
 UINT64_C(0xd7f4f2448c0ceb81),
 UINT64_C(0xd95be88cd210ffa7),
 UINT64_C(0x336f52f8ff4728e7),
 UINT64_C(0xa74049dac312ac71),
 UINT64_C(0xa2f61bb6e437fdb5),
 UINT64_C(0x4f2a5cb07f6a35b3),
 UINT64_C(0x87d380bda5bf7859),
 UINT64_C(0x16b9f7e06c453a21),
 UINT64_C(0x7ba2484c8a0fd54e),
 UINT64_C(0xf3a678cad9a2e38c),
 UINT64_C(0x39b0bf7dde437ba2),
 UINT64_C(0xfcaf55c1bf8a4424),
 UINT64_C(0x18fcf680573fa594),
 UINT64_C(0x4c0563b89f495ac3),
 UINT64_C(0x40e087931a00930d),
 UINT64_C(0x8cffa9412eb642c1),
 UINT64_C(0x68ca39053261169f),
 UINT64_C(0x7a1ee967d27579e2),
 UINT64_C(0x9d1d60e5076f5b6f),
 UINT64_C(0x3810e399b6f65ba2),
 UINT64_C(0x32095b6d4ab5f9b1),
 UINT64_C(0x35cab62109dd038a),
 UINT64_C(0xa90b24499fcfafb1),
 UINT64_C(0x77a225a07cc2c6bd),
 UINT64_C(0x513e5e634c70e331),
 UINT64_C(0x4361c0ca3f692f12),
 UINT64_C(0xd941aca44b20a45b),
 UINT64_C(0x528f7c8602c5807b),
 UINT64_C(0x52ab92beb9613989),
 UINT64_C(0x9d1dfa2efc557f73),
 UINT64_C(0x722ff175f572c348),
 UINT64_C(0x1d1260a51107fe97),
 UINT64_C(0x7a249a57ec0c9ba2),
 UINT64_C(0x04208fe9e8f7f2d6),
 UINT64_C(0x5a110c6058b920a0),
 UINT64_C(0x0cd9a497658a5698),
 UINT64_C(0x56fd23c8f9715a4c),
 UINT64_C(0x284c847b9d887aae),
 UINT64_C(0x04feabfbbdb619cb),
 UINT64_C(0x742e1e651c60ba83),
 UINT64_C(0x9a9632e65904ad3c),
 UINT64_C(0x881b82a13b51b9e2),
 UINT64_C(0x506e6744cd974924),
 UINT64_C(0xb0183db56ffc6a79),
 UINT64_C(0x0ed9b915c66ed37e),
 UINT64_C(0x5e11e86d5873d484),
 UINT64_C(0xf678647e3519ac6e),
 UINT64_C(0x1b85d488d0f20cc5),
 UINT64_C(0xdab9fe6525d89021),
 UINT64_C(0x0d151d86adb73615),
 UINT64_C(0xa865a54edcc0f019),
 UINT64_C(0x93c42566aef98ffb),
 UINT64_C(0x99e7afeabe000731),
 UINT64_C(0x48cbff086ddf285a),
 UINT64_C(0x7f9b6af1ebf78baf),
 UINT64_C(0x58627e1a149bba21),
 UINT64_C(0x2cd16e2abd791e33),
 UINT64_C(0xd363eff5f0977996),
 UINT64_C(0x0ce2a38c344a6eed),
 UINT64_C(0x1a804aadb9cfa741),
 UINT64_C(0x907f30421d78c5de),
 UINT64_C(0x501f65edb3034d07),
 UINT64_C(0x37624ae5a48fa6e9),
 UINT64_C(0x957baf61700cff4e),
 UINT64_C(0x3a6c27934e31188a),
 UINT64_C(0xd49503536abca345),
 UINT64_C(0x088e049589c432e0),
 UINT64_C(0xf943aee7febf21b8),
 UINT64_C(0x6c3b8e3e336139d3),
 UINT64_C(0x364f6ffa464ee52e),
 UINT64_C(0xd60f6dcedc314222),
 UINT64_C(0x56963b0dca418fc0),
 UINT64_C(0x16f50edf91e513af),
 UINT64_C(0xef1955914b609f93),
 UINT64_C(0x565601c0364e3228),
 UINT64_C(0xecb53939887e8175),
 UINT64_C(0xbac7a9a18531294b),
 UINT64_C(0xb344c470397bba52),
 UINT64_C(0x65d34954daf3cebd),
 UINT64_C(0xb4b81b3fa97511e2),
 UINT64_C(0xb422061193d6f6a7),
 UINT64_C(0x071582401c38434d),
 UINT64_C(0x7a13f18bbedc4ff5),
 UINT64_C(0xbc4097b116c524d2),
 UINT64_C(0x59b97885e2f2ea28),
 UINT64_C(0x99170a5dc3115544),
 UINT64_C(0x6f423357e7c6a9f9),
 UINT64_C(0x325928ee6e6f8794),
 UINT64_C(0xd0e4366228b03343),
 UINT64_C(0x565c31f7de89ea27),
 UINT64_C(0x30f5611484119414),
 UINT64_C(0xd873db391292ed4f),
 UINT64_C(0x7bd94e1d8e17debc),
 UINT64_C(0xc7d9f16864a76e94),
 UINT64_C(0x947ae053ee56e63c),
 UINT64_C(0xc8c93882f9475f5f),
 UINT64_C(0x3a9bf55ba91f81ca),
 UINT64_C(0xd9a11fbb3d9808e4),
 UINT64_C(0x0fd22063edc29fca),
 UINT64_C(0xb3f256d8aca0b0b9),
 UINT64_C(0xb03031a8b4516e84),
 UINT64_C(0x35dd37d5871448af),
 UINT64_C(0xe9f6082b05542e4e),
 UINT64_C(0xebfafa33d7254b59),
 UINT64_C(0x9255abb50d532280),
 UINT64_C(0xb9ab4ce57f2d34f3),
 UINT64_C(0x693501d628297551),
 UINT64_C(0xc62c58f97dd949bf),
 UINT64_C(0xcd454f8f19c5126a),
 UINT64_C(0xbbe83f4ecc2bdecb),
 UINT64_C(0xdc842b7e2819e230),
 UINT64_C(0xba89142e007503b8),
 UINT64_C(0xa3bc941d0a5061cb),
 UINT64_C(0xe9f6760e32cd8021),
 UINT64_C(0x09c7e552bc76492f),
 UINT64_C(0x852f54934da55cc9),
 UINT64_C(0x8107fccf064fcf56),
 UINT64_C(0x098954d51fff6580),
 UINT64_C(0x23b70edb1955c4bf),
 UINT64_C(0xc330de426430f69d),
 UINT64_C(0x4715ed43e8a45c0a),
 UINT64_C(0xa8d7e4dab780a08d),
 UINT64_C(0x0572b974f03ce0bb),
 UINT64_C(0xb57d2e985e1419c7),
 UINT64_C(0xe8d9ecbe2cf3d73f),
 UINT64_C(0x2fe4b17170e59750),
 UINT64_C(0x11317ba87905e790),
 UINT64_C(0x7fbf21ec8a1f45ec),
 UINT64_C(0x1725cabfcb045b00),
 UINT64_C(0x964e915cd5e2b207),
 UINT64_C(0x3e2b8bcbf016d66d),
 UINT64_C(0xbe7444e39328a0ac),
 UINT64_C(0xf85b2b4fbcde44b7),
 UINT64_C(0x49353fea39ba63b1),
 UINT64_C(0x1dd01aafcd53486a),
 UINT64_C(0x1fca8a92fd719f85),
 UINT64_C(0xfc7c95d827357afa),
 UINT64_C(0x18a6a990c8b35ebd),
 UINT64_C(0xcccb7005c6b9c28d),
 UINT64_C(0x3bdbb92c43b17f26),
 UINT64_C(0xaa70b5b4f89695a2),
 UINT64_C(0xe94c39a54a98307f),
 UINT64_C(0xb7a0b174cff6f36e),
 UINT64_C(0xd4dba84729af48ad),
 UINT64_C(0x2e18bc1ad9704a68),
 UINT64_C(0x2de0966daf2f8b1c),
 UINT64_C(0xb9c11d5b1e43a07e),
 UINT64_C(0x64972d68dee33360),
 UINT64_C(0x94628d38d0c20584),
 UINT64_C(0xdbc0d2b6ab90a559),
 UINT64_C(0xd2733c4335c6a72f),
 UINT64_C(0x7e75d99d94a70f4d),
 UINT64_C(0x6ced1983376fa72b),
 UINT64_C(0x97fcaacbf030bc24),
 UINT64_C(0x7b77497b32503b12),
 UINT64_C(0x8547eddfb81ccb94),
 UINT64_C(0x79999cdff70902cb),
 UINT64_C(0xcffe1939438e9b24),
 UINT64_C(0x829626e3892d95d7),
 UINT64_C(0x92fae24291f2b3f1),
 UINT64_C(0x63e22c147b9c3403),
 UINT64_C(0xc678b6d860284a1c),
 UINT64_C(0x5873888850659ae7),
 UINT64_C(0x0981dcd296a8736d),
 UINT64_C(0x9f65789a6509a440),
 UINT64_C(0x9ff38fed72e9052f),
 UINT64_C(0xe479ee5b9930578c),
 UINT64_C(0xe7f28ecd2d49eecd),
 UINT64_C(0x56c074a581ea17fe),
 UINT64_C(0x5544f7d774b14aef),
 UINT64_C(0x7b3f0195fc6f290f),
 UINT64_C(0x12153635b2c0cf57),
 UINT64_C(0x7f5126dbba5e0ca7),
 UINT64_C(0x7a76956c3eafb413),
 UINT64_C(0x3d5774a11d31ab39),
 UINT64_C(0x8a1b083821f40cb4),
 UINT64_C(0x7b4a38e32537df62),
 UINT64_C(0x950113646d1d6e03),
 UINT64_C(0x4da8979a0041e8a9),
 UINT64_C(0x3bc36e078f7515d7),
 UINT64_C(0x5d0a12f27ad310d1),
 UINT64_C(0x7f9d1a2e1ebe1327),
 UINT64_C(0xda3a361b1c5157b1),
 UINT64_C(0xdcdd7d20903d0c25),
 UINT64_C(0x36833336d068f707),
 UINT64_C(0xce68341f79893389),
 UINT64_C(0xab9090168dd05f34),
 UINT64_C(0x43954b3252dc25e5),
 UINT64_C(0xb438c2b67f98e5e9),
 UINT64_C(0x10dcd78e3851a492),
 UINT64_C(0xdbc27ab5447822bf),
 UINT64_C(0x9b3cdb65f82ca382),
 UINT64_C(0xb67b7896167b4c84),
 UINT64_C(0xbfced1b0048eac50),
 UINT64_C(0xa9119b60369ffebd),
 UINT64_C(0x1fff7ac80904bf45),
 UINT64_C(0xac12fb171817eee7),
 UINT64_C(0xaf08da9177dda93d),
 UINT64_C(0x1b0cab936e65c744),
 UINT64_C(0xb559eb1d04e5e932),
 UINT64_C(0xc37b45b3f8d6f2ba),
 UINT64_C(0xc3a9dc228caac9e9),
 UINT64_C(0xf3b8b6675a6507ff),
 UINT64_C(0x9fc477de4ed681da),
 UINT64_C(0x67378d8eccef96cb),
 UINT64_C(0x6dd856d94d259236),
 UINT64_C(0xa319ce15b0b4db31),
 UINT64_C(0x073973751f12dd5e),
 UINT64_C(0x8a8e849eb32781a5),
 UINT64_C(0xe1925c71285279f5),
 UINT64_C(0x74c04bf1790c0efe),
 UINT64_C(0x4dda48153c94938a),
 UINT64_C(0x9d266d6a1cc0542c),
 UINT64_C(0x7440fb816508c4fe),
 UINT64_C(0x13328503df48229f),
 UINT64_C(0xd6bf7baee43cac40),
 UINT64_C(0x4838d65f6ef6748f),
 UINT64_C(0x1e152328f3318dea),
 UINT64_C(0x8f8419a348f296bf),
 UINT64_C(0x72c8834a5957b511),
 UINT64_C(0xd7a023a73260b45c),
 UINT64_C(0x94ebc8abcfb56dae),
 UINT64_C(0x9fc10d0f989993e0),
 UINT64_C(0xde68a2355b93cae6),
 UINT64_C(0xa44cfe79ae538bbe),
 UINT64_C(0x9d1d84fcce371425),
 UINT64_C(0x51d2b1ab2ddfb636),
 UINT64_C(0x2fd7e4b9e72cd38c),
 UINT64_C(0x65ca5b96b7552210),
 UINT64_C(0xdd69a0d8ab3b546d),
 UINT64_C(0x604d51b25fbf70e2),
 UINT64_C(0x73aa8a564fb7ac9e),
 UINT64_C(0x1a8c1e992b941148),
 UINT64_C(0xaac40a2703d9bea0),
 UINT64_C(0x764dbeae7fa4f3a6),
 UINT64_C(0x1e99b96e70a9be8b),
 UINT64_C(0x2c5e9deb57ef4743),
 UINT64_C(0x3a938fee32d29981),
 UINT64_C(0x26e6db8ffdf5adfe),
 UINT64_C(0x469356c504ec9f9d),
 UINT64_C(0xc8763c5b08d1908c),
 UINT64_C(0x3f6c6af859d80055),
 UINT64_C(0x7f7cc39420a3a545),
 UINT64_C(0x9bfb227ebdf4c5ce),
 UINT64_C(0x89039d79d6fc5c5c),
 UINT64_C(0x8fe88b57305e2ab6),
 UINT64_C(0xa09e8c8c35ab96de),
 UINT64_C(0xfa7e393983325753),
 UINT64_C(0xd6b6d0ecc617c699),
 UINT64_C(0xdfea21ea9e7557e3),
 UINT64_C(0xb67c1fa481680af8),
 UINT64_C(0xca1e3785a9e724e5),
 UINT64_C(0x1cfc8bed0d681639),
 UINT64_C(0xd18d8549d140caea),
 UINT64_C(0x4ed0fe7e9dc91335),
 UINT64_C(0xe4dbf0634473f5d2),
 UINT64_C(0x1761f93a44d5aefe),
 UINT64_C(0x53898e4c3910da55),
 UINT64_C(0x734de8181f6ec39a),
 UINT64_C(0x2680b122baa28d97),
 UINT64_C(0x298af231c85bafab),
 UINT64_C(0x7983eed3740847d5),
 UINT64_C(0x66c1a2a1a60cd889),
 UINT64_C(0x9e17e49642a3e4c1),
 UINT64_C(0xedb454e7badc0805),
 UINT64_C(0x50b704cab602c329),
 UINT64_C(0x4cc317fb9cddd023),
 UINT64_C(0x66b4835d9eafea22),
 UINT64_C(0x219b97e26ffc81bd),
 UINT64_C(0x261e4e4c0a333a9d),
 UINT64_C(0x1fe2cca76517db90),
 UINT64_C(0xd7504dfa8816edbb),
 UINT64_C(0xb9571fa04dc089c8),
 UINT64_C(0x1ddc0325259b27de),
 UINT64_C(0xcf3f4688801eb9aa),
 UINT64_C(0xf4f5d05c10cab243),
 UINT64_C(0x38b6525c21a42b0e),
 UINT64_C(0x36f60e2ba4fa6800),
 UINT64_C(0xeb3593803173e0ce),
 UINT64_C(0x9c4cd6257c5a3603),
 UINT64_C(0xaf0c317d32adaa8a),
 UINT64_C(0x258e5a80c7204c4b),
 UINT64_C(0x8b889d624d44885d),
 UINT64_C(0xf4d14597e660f855),
 UINT64_C(0xd4347f66ec8941c3),
 UINT64_C(0xe699ed85b0dfb40d),
 UINT64_C(0x2472f6207c2d0484),
 UINT64_C(0xc2a1e7b5b459aeb5),
 UINT64_C(0xab4f6451cc1d45ec),
 UINT64_C(0x63767572ae3d6174),
 UINT64_C(0xa59e0bd101731a28),
 UINT64_C(0x116d0016cb948f09),
 UINT64_C(0x2cf9c8ca052f6e9f),
 UINT64_C(0x0b090a7560a968e3),
 UINT64_C(0xabeeddb2dde06ff1),
 UINT64_C(0x58efc10b06a2068d),
 UINT64_C(0xc6e57a78fbd986e0),
 UINT64_C(0x2eab8ca63ce802d7),
 UINT64_C(0x14a195640116f336),
 UINT64_C(0x7c0828dd624ec390),
 UINT64_C(0xd74bbe77e6116ac7),
 UINT64_C(0x804456af10f5fb53),
 UINT64_C(0xebe9ea2adf4321c7),
 UINT64_C(0x03219a39ee587a30),
 UINT64_C(0x49787fef17af9924),
 UINT64_C(0xa1e9300cd8520548),
 UINT64_C(0x5b45e522e4b1b4ef),
 UINT64_C(0xb49c3b3995091a36),
 UINT64_C(0xd4490ad526f14431),
 UINT64_C(0x12a8f216af9418c2),
 UINT64_C(0x001f837cc7350524),
 UINT64_C(0x1877b51e57a764d5),
 UINT64_C(0xa2853b80f17f58ee),
 UINT64_C(0x993e1de72d36d310),
 UINT64_C(0xb3598080ce64a656),
 UINT64_C(0x252f59cf0d9f04bb),
 UINT64_C(0xd23c8e176d113600),
 UINT64_C(0x1bda0492e7e4586e),
 UINT64_C(0x21e0bd5026c619bf),
 UINT64_C(0x3b097adaf088f94e),
 UINT64_C(0x8d14dedb30be846e),
 UINT64_C(0xf95cffa23af5f6f4),
 UINT64_C(0x3871700761b3f743),
 UINT64_C(0xca672b91e9e4fa16),
 UINT64_C(0x64c8e531bff53b55),
 UINT64_C(0x241260ed4ad1e87d),
 UINT64_C(0x106c09b972d2e822),
 UINT64_C(0x7fba195410e5ca30),
 UINT64_C(0x7884d9bc6cb569d8),
 UINT64_C(0x0647dfedcd894a29),
 UINT64_C(0x63573ff03e224774),
 UINT64_C(0x4fc8e9560f91b123),
 UINT64_C(0x1db956e450275779),
 UINT64_C(0xb8d91274b9e9d4fb),
 UINT64_C(0xa2ebee47e2fbfce1),
 UINT64_C(0xd9f1f30ccd97fb09),
 UINT64_C(0xefed53d75fd64e6b),
 UINT64_C(0x2e6d02c36017f67f),
 UINT64_C(0xa9aa4d20db084e9b),
 UINT64_C(0xb64be8d8b25396c1),
 UINT64_C(0x70cb6af7c2d5bcf0),
 UINT64_C(0x98f076a4f7a2322e),
 UINT64_C(0xbf84470805e69b5f),
 UINT64_C(0x94c3251f06f90cf3),
 UINT64_C(0x3e003e616a6591e9),
 UINT64_C(0xb925a6cd0421aff3),
 UINT64_C(0x61bdd1307c66e300),
 UINT64_C(0xbf8d5108e27e0d48),
 UINT64_C(0x240ab57a8b888b20),
 UINT64_C(0xfc87614baf287e07),
 UINT64_C(0xef02cdd06ffdb432),
 UINT64_C(0xa1082c0466df6c0a),
 UINT64_C(0x8215e577001332c8),
 UINT64_C(0xd39bb9c3a48db6cf),
 UINT64_C(0x2738259634305c14),
 UINT64_C(0x61cf4f94c97df93d),
 UINT64_C(0x1b6baca2ae4e125b),
 UINT64_C(0x758f450c88572e0b),
 UINT64_C(0x959f587d507a8359),
 UINT64_C(0xb063e962e045f54d),
 UINT64_C(0x60e8ed72c0dff5d1),
 UINT64_C(0x7b64978555326f9f),
 UINT64_C(0xfd080d236da814ba),
 UINT64_C(0x8c90fd9b083f4558),
 UINT64_C(0x106f72fe81e2c590),
 UINT64_C(0x7976033a39f7d952),
 UINT64_C(0xa4ec0132764ca04b),
 UINT64_C(0x733ea705fae4fa77),
 UINT64_C(0xb4d8f77bc3e56167),
 UINT64_C(0x9e21f4f903b33fd9),
 UINT64_C(0x9d765e419fb69f6d),
 UINT64_C(0xd30c088ba61ea5ef),
 UINT64_C(0x5d94337fbfaf7f5b),
 UINT64_C(0x1a4e4822eb4d7a59),
 UINT64_C(0x6ffe73e81b637fb3),
 UINT64_C(0xddf957bc36d8b9ca),
 UINT64_C(0x64d0e29eea8838b3),
 UINT64_C(0x08dd9bdfd96b9f63),
 UINT64_C(0x087e79e5a57d1d13),
 UINT64_C(0xe328e230e3e2b3fb),
 UINT64_C(0x1c2559e30f0946be),
 UINT64_C(0x720bf5f26f4d2eaa),
 UINT64_C(0xb0774d261cc609db),
 UINT64_C(0x443f64ec5a371195),
 UINT64_C(0x4112cf68649a260e),
 UINT64_C(0xd813f2fab7f5c5ca),
 UINT64_C(0x660d3257380841ee),
 UINT64_C(0x59ac2c7873f910a3),
 UINT64_C(0xe846963877671a17),
 UINT64_C(0x93b633abfa3469f8),
 UINT64_C(0xc0c0f5a60ef4cdcf),
 UINT64_C(0xcaf21ecd4377b28c),
 UINT64_C(0x57277707199b8175),
 UINT64_C(0x506c11b9d90e8b1d),
 UINT64_C(0xd83cc2687a19255f),
 UINT64_C(0x4a29c6465a314cd1),
 UINT64_C(0xed2df21216235097),
 UINT64_C(0xb5635c95ff7296e2),
 UINT64_C(0x22af003ab672e811),
 UINT64_C(0x52e762596bf68235),
 UINT64_C(0x9aeba33ac6ecc6b0),
 UINT64_C(0x944f6de09134dfb6),
 UINT64_C(0x6c47bec883a7de39),
 UINT64_C(0x6ad047c430a12104),
 UINT64_C(0xa5b1cfdba0ab4067),
 UINT64_C(0x7c45d833aff07862),
 UINT64_C(0x5092ef950a16da0b),
 UINT64_C(0x9338e69c052b8e7b),
 UINT64_C(0x455a4b4cfe30e3f5),
 UINT64_C(0x6b02e63195ad0cf8),
 UINT64_C(0x6b17b224bad6bf27),
 UINT64_C(0xd1e0ccd25bb9c169),
 UINT64_C(0xde0c89a556b9ae70),
 UINT64_C(0x50065e535a213cf6),
 UINT64_C(0x9c1169fa2777b874),
 UINT64_C(0x78edefd694af1eed),
 UINT64_C(0x6dc93d9526a50e68),
 UINT64_C(0xee97f453f06791ed),
 UINT64_C(0x32ab0edb696703d3),
 UINT64_C(0x3a6853c7e70757a7),
 UINT64_C(0x31865ced6120f37d),
 UINT64_C(0x67fef95d92607890),
 UINT64_C(0x1f2b1d1f15f6dc9c),
 UINT64_C(0xb69e38a8965c6b65),
 UINT64_C(0xaa9119ff184cccf4),
 UINT64_C(0xf43c732873f24c13),
 UINT64_C(0xfb4a3d794a9a80d2),
 UINT64_C(0x3550c2321fd6109c),
 UINT64_C(0x371f77e76bb8417e),
 UINT64_C(0x6bfa9aae5ec05779),
 UINT64_C(0xcd04f3ff001a4778),
 UINT64_C(0xe3273522064480ca),
 UINT64_C(0x9f91508bffcfc14a),
 UINT64_C(0x049a7f41061a9e60),
 UINT64_C(0xfcb6be43a9f2fe9b),
 UINT64_C(0x08de8a1c7797da9b),
 UINT64_C(0x8f9887e6078735a1),
 UINT64_C(0xb5b4071dbfc73a66),
 UINT64_C(0x230e343dfba08d33),
 UINT64_C(0x43ed7f5a0fae657d),
 UINT64_C(0x3a88a0fbbcb05c63),
 UINT64_C(0x21874b8b4d2dbc4f),
 UINT64_C(0x1bdea12e35f6a8c9),
 UINT64_C(0x53c065c6c8e63528),
 UINT64_C(0xe34a1d250e7a8d6b),
 UINT64_C(0xd6b04d3b7651dd7e),
 UINT64_C(0x5e90277e7cb39e2d),
 UINT64_C(0x2c046f22062dc67d),
 UINT64_C(0xb10bb459132d0a26),
 UINT64_C(0x3fa9ddfb67e2f199),
 UINT64_C(0x0e09b88e1914f7af),
 UINT64_C(0x10e8b35af3eeab37),
 UINT64_C(0x9eedeca8e272b933),
 UINT64_C(0xd4c718bc4ae8ae5f),
 UINT64_C(0x81536d601170fc20),
 UINT64_C(0x91b534f885818a06),
 UINT64_C(0xec8177f83f900978),
 UINT64_C(0x190e714fada5156e),
 UINT64_C(0xb592bf39b0364963),
 UINT64_C(0x89c350c893ae7dc1),
 UINT64_C(0xac042e70f8b383f2),
 UINT64_C(0xb49b52e587a1ee60),
 UINT64_C(0xfb152fe3ff26da89),
 UINT64_C(0x3e666e6f69ae2c15),
 UINT64_C(0x3b544ebe544c19f9),
 UINT64_C(0xe805a1e290cf2456),
 UINT64_C(0x24b33c9d7ed25117),
 UINT64_C(0xe74733427b72f0c1),
 UINT64_C(0x0a804d18b7097475),
 UINT64_C(0x57e3306d881edb4f),
 UINT64_C(0x4ae7d6a36eb5dbcb),
 UINT64_C(0x2d8d5432157064c8),
 UINT64_C(0xd1e649de1e7f268b),
 UINT64_C(0x8a328a1cedfe552c),
 UINT64_C(0x07a3aec79624c7da),
 UINT64_C(0x84547ddc3e203c94),
 UINT64_C(0x990a98fd5071d263),
 UINT64_C(0x1a4ff12616eefc89),
 UINT64_C(0xf6f7fd1431714200),
 UINT64_C(0x30c05b1ba332f41c),
 UINT64_C(0x8d2636b81555a786),
 UINT64_C(0x46c9feb55d120902),
 UINT64_C(0xccec0a73b49c9921),
 UINT64_C(0x4e9d2827355fc492),
 UINT64_C(0x19ebb029435dcb0f),
 UINT64_C(0x4659d2b743848a2c),
 UINT64_C(0x963ef2c96b33be31),
 UINT64_C(0x74f85198b05a2e7d),
 UINT64_C(0x5a0f544dd2b1fb18),
 UINT64_C(0x03727073c2e134b1),
 UINT64_C(0xc7f6aa2de59aea61),
 UINT64_C(0x352787baa0d7c22f),
 UINT64_C(0x9853eab63b5e0b35),
 UINT64_C(0xabbdcdd7ed5c0860),
 UINT64_C(0xcf05daf5ac8d77b0),
 UINT64_C(0x49cad48cebf4a71e),
 UINT64_C(0x7a4c10ec2158c4a6),
 UINT64_C(0xd9e92aa246bf719e),
 UINT64_C(0x13ae978d09fe5557),
 UINT64_C(0x730499af921549ff),
 UINT64_C(0x4e4b705b92903ba4),
 UINT64_C(0xff577222c14f0a3a),
 UINT64_C(0x55b6344cf97aafae),
 UINT64_C(0xb862225b055b6960),
 UINT64_C(0xcac09afbddd2cdb4),
 UINT64_C(0xdaf8e9829fe96b5f),
 UINT64_C(0xb5fdfc5d3132c498),
 UINT64_C(0x310cb380db6f7503),
 UINT64_C(0xe87fbb46217a360e),
 UINT64_C(0x2102ae466ebb1148),
 UINT64_C(0xf8549e1a3aa5e00d),
 UINT64_C(0x07a69afdcc42261a),
 UINT64_C(0xc4c118bfe78feaae),
 UINT64_C(0xf9f4892ed96bd438),
 UINT64_C(0x1af3dbe25d8f45da),
 UINT64_C(0xf5b4b0b0d2deeeb4),
 UINT64_C(0x962aceefa82e1c84),
 UINT64_C(0x046e3ecaaf453ce9),
 UINT64_C(0xf05d129681949a4c),
 UINT64_C(0x964781ce734b3c84),
 UINT64_C(0x9c2ed44081ce5fbd),
 UINT64_C(0x522e23f3925e319e),
 UINT64_C(0x177e00f9fc32f791),
 UINT64_C(0x2bc60a63a6f3b3f2),
 UINT64_C(0x222bbfae61725606),
 UINT64_C(0x486289ddcc3d6780),
 UINT64_C(0x7dc7785b8efdfc80),
 UINT64_C(0x8af38731c02ba980),
 UINT64_C(0x1fab64ea29a2ddf7),
 UINT64_C(0xe4d9429322cd065a),
 UINT64_C(0x9da058c67844f20c),
 UINT64_C(0x24c0e332b70019b0),
 UINT64_C(0x233003b5a6cfe6ad),
 UINT64_C(0xd586bd01c5c217f6),
 UINT64_C(0x5e5637885f29bc2b),
 UINT64_C(0x7eba726d8c94094b),
 UINT64_C(0x0a56a5f0bfe39272),
 UINT64_C(0xd79476a84ee20d06),
 UINT64_C(0x9e4c1269baa4bf37),
 UINT64_C(0x17efee45b0dee640),
 UINT64_C(0x1d95b0a5fcf90bc6),
 UINT64_C(0x93cbe0b699c2585d),
 UINT64_C(0x65fa4f227a2b6d79),
 UINT64_C(0xd5f9e858292504d5),
 UINT64_C(0xc2b5a03f71471a6f),
 UINT64_C(0x59300222b4561e00),
 UINT64_C(0xce2f8642ca0712dc),
 UINT64_C(0x7ca9723fbb2e8988),
 UINT64_C(0x2785338347f2ba08),
 UINT64_C(0xc61bb3a141e50e8c),
 UINT64_C(0x150f361dab9dec26),
 UINT64_C(0x9f6a419d382595f4),
 UINT64_C(0x64a53dc924fe7ac9),
 UINT64_C(0x142de49fff7a7c3d),
 UINT64_C(0x0c335248857fa9e7),
 UINT64_C(0x0a9c32d5eae45305),
 UINT64_C(0xe6c42178c4bbb92e),
 UINT64_C(0x71f1ce2490d20b07),
 UINT64_C(0xf1bcc3d275afe51a),
 UINT64_C(0xe728e8c83c334074),
 UINT64_C(0x96fbf83a12884624),
 UINT64_C(0x81a1549fd6573da5),
 UINT64_C(0x5fa7867caf35e149),
 UINT64_C(0x56986e2ef3ed091b),
 UINT64_C(0x917f1dd5f8886c61),
 UINT64_C(0xd20d8c88c8ffe65f),
 UINT64_C(0x31d71dce64b2c310),
 UINT64_C(0xf165b587df898190),
 UINT64_C(0xa57e6339dd2cf3a0),
 UINT64_C(0x1ef6e6dbb1961ec9),
 UINT64_C(0x70cc73d90bc26e24),
 UINT64_C(0xe21a6b35df0c3ad7),
 UINT64_C(0x003a93d8b2806962),
 UINT64_C(0x1c99ded33cb890a1),
 UINT64_C(0xcf3145de0add4289),
 UINT64_C(0xd0e4427a5514fb72),
 UINT64_C(0x77c621cc9fb3a483),
 UINT64_C(0x67a34dac4356550b),
 UINT64_C(0xf8d626aaaf278509),
};

uint64_t * RandomPiece = Random64;
uint64_t * RandomCastle = Random64 + 768;
uint64_t * RandomEnPassant = Random64 + 772;
uint64_t * RandomTurn = Random64 + 780;

static char * sym;

int MustFlip(int moveNr) {
    int r, f, w = BOARD_WIDTH / 2;
    for (r = 0; r < BOARD_HEIGHT; r++) {
        for (f = 0; f < w; f++) {
            ChessSquare a = boards[moveNr][r][f], b = boards[moveNr][r][BOARD_WIDTH - 1 - f];
            if (a != b) {
                return (a < b);
            }
        }
    }
    return 0;
}

int FlipMove(int move) {
    int f, fr, ff, t, tr, tf, p;
    int width = BOARD_RGHT - BOARD_LEFT, size;

    size = width * BOARD_HEIGHT;
    p = move / (size * size);
    move = move % (size * size);
    f = move / size;
    fr = f / width;
    ff = f % width;
    t = move % size;
    tr = t / width;
    tf = t % width;

    return p * size * size + (fr * width + width - 1 - ff) * size + (tr * width + width - 1 - tf);
}

uint64_t hash(int moveNr, int flip, int old) {
    int r, f, p_enc, squareNr, pieceGroup;
    uint64_t key = 0, holdingsKey = 0, Zobrist;
    VariantClass v = gameInfo.variant;

    switch (v) {
    case VariantNormal:
    case VariantFischeRandom:  // compatible with normal
    case VariantNoCastle:
    case VariantXiangqi:  // for historic reasons; does never collide anyway because of other King type
        break;
    case VariantGiveaway:  // in opening same as suicide
        key += VariantSuicide;
        break;
    case VariantGothic:  // these are special cases of CRC, and can share book
    case VariantCapablanca:
        v = VariantCapaRandom;
    default:
        key += v;  // variant type incorporated in key to allow mixed books without collisions
    }

    for (f = 0; f < BOARD_WIDTH; f++) {
        for (r = 0; r < BOARD_HEIGHT; r++) {
            ChessSquare p = boards[moveNr][r][flip ? BOARD_WIDTH - 1 - f : f];
            if (f == BOARD_LEFT - 1 || f == BOARD_RGHT) {
                continue;  // between board and holdings
            }
            if (p != EmptySquare) {
                int j = (int)p, promoted = 0;
                j -= (j >= (int)BlackPawn) ? (int)BlackPawn : (int)WhitePawn;
                if (j >= WhitePBishop && j != WhiteKing) {
                    promoted++, j -= WhiteTokin;
                }
                if (j > (int)WhiteQueen) {
                    j++;  // make space for King
                }
                if (j > (int)WhiteKing) {
                    j = (int)WhiteQueen + 1;
                }
                p_enc = 2 * j + ((int)p < (int)BlackPawn);
                // holdings squares get nmbers immediately after board; first left, then right holdings
                if (f == BOARD_LEFT - 2) {
                    squareNr = (BOARD_RGHT - BOARD_LEFT) * BOARD_HEIGHT + r;
                } else if (f == BOARD_RGHT + 1) {
                    squareNr = (BOARD_RGHT - BOARD_LEFT + 1) * BOARD_HEIGHT + r;
                } else {
                    squareNr = (BOARD_RGHT - BOARD_LEFT) * r + (f - BOARD_LEFT);
                }
                // note that in normal Chess squareNr < 64 and p_enc < 12. The following code
                // maps other pieces and squares in this range, and then modify the corresponding
                // Zobrist random by rotating its bitpattern according to what the piece really was.
                pieceGroup = p_enc / 12;
                p_enc = p_enc % 12;
                Zobrist = RandomPiece[64 * p_enc + (squareNr & 63)];
                if (pieceGroup & 4) {
                    Zobrist *= 987654321;
                }
                switch (pieceGroup & 3) {
                case 1:  // pieces 5-10 (FEACWM)
                    Zobrist = (Zobrist << 16) ^ (Zobrist >> 48);
                    break;
                case 2:  // pieces 11-16 (OHIJGD)
                    Zobrist = (Zobrist << 32) ^ (Zobrist >> 32);
                    break;
                case 3:  // pieces 17-20 (VLSU)
                    Zobrist = (Zobrist << 48) ^ (Zobrist >> 16);
                    break;
                }
                if (promoted) {
                    Zobrist ^= 123456789 * RandomPiece[squareNr & 63];
                }
                if (squareNr & 64) {
                    Zobrist = (Zobrist << 8) ^ (Zobrist >> 56);
                }
                if (squareNr & 128) {
                    Zobrist = (Zobrist << 4) ^ (Zobrist >> 60);
                }
                // holdings have separate (additive) key, to encode presence of multiple pieces on same square
                if (f == BOARD_LEFT - 2) {
                    holdingsKey += Zobrist * boards[moveNr][r][f + 1];
                } else if (f == BOARD_RGHT + 1) {
                    holdingsKey += Zobrist * boards[moveNr][r][f - 1];
                } else {
                    key ^= Zobrist;
                }
            }
        }
    }

    if (boards[moveNr][CASTLING][2] != NoRights) {
        if (boards[moveNr][CASTLING][0] != NoRights) {
            key ^= RandomCastle[0];
        }
        if (boards[moveNr][CASTLING][1] != NoRights) {
            key ^= RandomCastle[1];
        }
    }
    if (boards[moveNr][CASTLING][5] != NoRights) {
        if (boards[moveNr][CASTLING][3] != NoRights) {
            key ^= RandomCastle[2];
        }
        if (boards[moveNr][CASTLING][4] != NoRights) {
            key ^= RandomCastle[3];
        }
    }

    f = boards[moveNr][EP_STATUS];
    if (f >= 0 && f < 8) {
        if (!WhiteOnMove(moveNr)) {
            // the test for neighboring Pawns might not be needed,
            // as epStatus already kept track of it, but better safe than sorry.
            if ((f > 0 && boards[moveNr][3][f - 1] == BlackPawn) || (f < 7 && boards[moveNr][3][f + 1] == BlackPawn)) {
                key ^= RandomEnPassant[f];
            }
        } else {
            if ((f > 0 && boards[moveNr][4][f - 1] == WhitePawn) || (f < 7 && boards[moveNr][4][f + 1] == WhitePawn)) {
                key ^= RandomEnPassant[f];
            }
        }
    }

    if (WhiteOnMove(moveNr)) {
        key ^= RandomTurn[0];
    }
    return key + holdingsKey;
}

#define MOVE_BUF 100

// fs routines read from memory buffer if no file specified

static unsigned char *memBuf, *memPtr;
static int bufSize;
Boolean mcMode;

int fsseek(FILE * f, int n, int mode) {
    if (f) {
        return fseek(f, n, mode);
    }
    if (mode == SEEK_SET) {
        memPtr = memBuf + n;
    } else if (mode == SEEK_END) {
        memPtr = memBuf + 16 * bufSize + n;
    }
    return memPtr < memBuf || memPtr > memBuf + 16 * bufSize;
}

int fstell(FILE * f) {
    if (f) {
        return ftell(f);
    }
    return memPtr - memBuf;
}

int fsgetc(FILE * f) {
    if (f) {
        return fgetc(f);
    }
    if (memPtr >= memBuf + 16 * bufSize) {
        return EOF;
    }
    return *memPtr++;
}

int int_from_file(FILE * f, int l, uint64_t * r) {
    int i, c;
    for (i = 0; i < l; i++) {
        c = fsgetc(f);
        if (c == EOF) {
            return 1;
        }
        (*r) = ((*r) << 8) + c;
    }
    return 0;
}

int entry_from_file(FILE * f, entry_t * entry) {
    int ret;
    uint64_t r;
    if (!f) {
        *entry = *(entry_t *)memPtr;
        memPtr += 16;
        return 0;
    }
    ret = int_from_file(f, 8, &r);
    if (ret) {
        return 1;
    }
    entry->key = r;
    ret = int_from_file(f, 2, &r);
    if (ret) {
        return 1;
    }
    entry->move = r;
    ret = int_from_file(f, 2, &r);
    if (ret) {
        return 1;
    }
    entry->weight = r;
    ret = int_from_file(f, 2, &r);
    if (ret) {
        return 1;
    }
    entry->learnCount = r;
    ret = int_from_file(f, 2, &r);
    if (ret) {
        return 1;
    }
    entry->learnPoints = r;
    return 0;
}

int find_key(FILE * f, uint64_t key, entry_t * entry) {
    int first, last, middle;
    entry_t last_entry, middle_entry;
    first = -1;
    if (fsseek(f, -16, SEEK_END)) {
        *entry = entry_none;
        entry->key = key + 1;  // hack
        return -1;
    }
    last = fstell(f) / 16;
    entry_from_file(f, &last_entry);
    while (1) {
        if (last - first == 1) {
            *entry = last_entry;
            return last;
        }
        middle = (first + last) / 2;
        fsseek(f, 16 * middle, SEEK_SET);
        entry_from_file(f, &middle_entry);
        if (key <= middle_entry.key) {
            last = middle;
            last_entry = middle_entry;
        } else {
            first = middle;
        }
    }
}

static int xStep[] = {0, 1, 1, 1, 0, -1, -1, -1};
static int yStep[] = {1, 1, 0, -1, -1, -1, 0, 1};

void move_to_string(char move_s[20], uint16_t move) {
    int f, fr, ff, t, tr, tf, p;
    int width = BOARD_RGHT - BOARD_LEFT, size;  // allow for alternative board formats

    size = width * BOARD_HEIGHT;
    p = move / (size * size);
    move = move % (size * size);
    f = move / size;
    fr = f / width;
    ff = f % width;
    t = move % size;
    tr = t / width;
    tf = t % width;
    snprintf(move_s, 9, "%c%d%c%d", ff + 'a', fr + 1 - (BOARD_HEIGHT == 10), tf + 'a', tr + 1 - (BOARD_HEIGHT == 10));

    if (IS_SHOGI(gameInfo.variant) && p) {
        if (p == 2) {
            p = 10;  // Lion moves, for boards so big that 10 is out of range
        } else if (p != 7) {
            p = 8;  // use '+' for all others that do not explicitly defer
        }
    }

    // kludge: encode drops as special promotion code
    if (gameInfo.holdingsSize && p == 9) {
        move_s[0] = f + '@';  // from square encodes piece type
        move_s[1] = '@';  // drop symbol
        p = 0;
    } else if (p == 10) {  // decode Lion move
        int i = t & 7, j = t >> 3 & 7;
        tf = ff + xStep[i] + xStep[j];
        tr = fr + yStep[i] + yStep[j];  // calculate true to-square
        snprintf(move_s, 20, "%c%d%c%d,%c%d%c%d", ff + 'a', fr + 1 - (BOARD_HEIGHT == 10), ff + xStep[i] + 'a',
         fr + yStep[i] + 1 - (BOARD_HEIGHT == 10), ff + xStep[i] + 'a', fr + yStep[i] + 1 - (BOARD_HEIGHT == 10), tf + 'a',
         tr + 1 - (BOARD_HEIGHT == 10));
        p = 0;
    }

    // add promotion piece, if any
    if (p) {
        int len = strlen(move_s);
        move_s[len] = promote_pieces[p];
        move_s[len + 1] = '\0';
    }

    if (gameInfo.variant != VariantNormal) {
        return;
    }

    // correct FRC-style castlings in variant normal.
    // [HGM] This is buggy code! e1h1 could very well be a normal R or Q move.
    if (!strcmp(move_s, "e1h1")) {
        safeStrCpy(move_s, "e1g1", 6);
    } else if (!strcmp(move_s, "e1a1")) {
        safeStrCpy(move_s, "e1c1", 6);
    } else if (!strcmp(move_s, "e8h8")) {
        safeStrCpy(move_s, "e8g8", 6);
    } else if (!strcmp(move_s, "e8a8")) {
        safeStrCpy(move_s, "e8c8", 6);
    }
}

int GetBookMoves(FILE * f, int moveNr, entry_t entries[],
 int max) {  // retrieve all entries for given position from book in 'entries', return number.
    int flip = sym && MustFlip(moveNr);
    entry_t entry;
    int offset;
    uint64_t key;
    int count;
    int ret;

    key = hash(moveNr, flip, 0);
    if (appData.debugMode) {
        fprintf(debugFP, "book key = %08x%08x\n", (unsigned int)(key >> 32), (unsigned int)key);
    }

    offset = find_key(f, key, &entry);
    if (entry.key != key) {
        return FALSE;
    }
    if (flip) {
        entry.move = FlipMove(entry.move);
    }
    entries[0] = entry;
    count = 1;
    fsseek(f, 16 * (offset + 1), SEEK_SET);
    while (1) {
        ret = entry_from_file(f, &entry);
        if (ret) {
            break;
        }
        if (entry.key != key) {
            break;
        }
        if (flip) {
            entry.move = FlipMove(entry.move);
        }
        if (count == max) {
            break;
        }
        entries[count++] = entry;
    }
    return count;
}

static int dirty;

int ReadFromBookFile(
 int moveNr, char * book, entry_t entries[]) {  // retrieve all entries for given position from book in 'entries', return number.
    static FILE * f = NULL;
    static char curBook[MSG_SIZ];

    if (book == NULL) {
        return -1;
    }
    if (dirty) {
        if (f) {
            fclose(f);
        }
        dirty = 0;
        f = NULL;
    }
    if (!f || strcmp(book, curBook)) {  // keep book file open until book changed
        strncpy(curBook, book, MSG_SIZ);
        if (f) {
            fclose(f);
        }
        f = fopen(book, "rb");
        sym = strstr(appData.polyglotBook, "-sym.");
    }
    if (!f) {
        DisplayError(_("Polyglot book not valid"), 0);
        appData.usePolyglotBook = FALSE;
        return -1;
    }

    return GetBookMoves(f, moveNr, entries, MOVE_BUF);
}

// next three made into subroutines to facilitate future changes in storage scheme (e.g. 2 x 3 bytes)

static int wins(entry_t * e) { return e->learnPoints; }

static int losses(entry_t * e) { return e->learnCount; }

static void CountMove(entry_t * e, int result) {
    switch (result) {
    case 0:
        e->learnCount++;
        break;
    case 1:
        e->learnCount++;  // count draw as win + loss
    case 2:
        e->learnPoints++;
        break;
    }
}

#define MERGESIZE 2048
#define HASHSIZE 1024 * 1024 * 4

entry_t *memBook, *hashTab, *mergeBuf;
int bookSize = 1, mergeSize = 1, mask = HASHSIZE - 1;

void InitMemBook(void) {
    static int initDone = FALSE;
    if (initDone) {
        return;
    }
    memBook = (entry_t *)calloc(1024 * 1024, sizeof(entry_t));
    hashTab = (entry_t *)calloc(HASHSIZE, sizeof(entry_t));
    mergeBuf = (entry_t *)calloc(MERGESIZE + 5, sizeof(entry_t));
    memBook[0].key = -1ll;
    mergeBuf[0].key = -1ll;
    initDone = TRUE;
}

char * MCprobe(int moveNr) {
    int count, count2, games, i, choice = 0;
    entry_t entries[MOVE_BUF];
    float nominal[MOVE_BUF], tot, deficit, max, min;
    static char move_s[6];

    InitMemBook();
    memBuf = (unsigned char *)memBook;
    bufSize = bookSize;  // in MC mode book resides in memory
    count = GetBookMoves(NULL, moveNr, entries, MOVE_BUF);
    if (count < 0) {
        count = 0;  // don't care about miss yet
    }
    memBuf = (unsigned char *)mergeBuf;
    bufSize = mergeSize;  // there could be moves still waiting to be merged
    count2 = count + GetBookMoves(NULL, moveNr, entries + count, MOVE_BUF - count);
    if (appData.debugMode) {
        fprintf(debugFP, "MC probe: %d/%d (%d+%d)\n", count, count2, bookSize, mergeSize);
    }
    if (!count2) {
        return NULL;
    }
    tot = games = 0;
    for (i = 0; i < count2; i++) {
        float w = wins(entries + i) + 10., l = losses(entries + i) + 10.;
        float h = (w * w * w * w + 22500. * w * w) / (l * l * l * l + 22500. * l * l);
        tot += nominal[i] = h;
        games += wins(entries + i) + losses(entries + i);
    }
    tot = games / tot;
    max = min = 0;
    for (i = 0; i < count2; i++) {
        nominal[i] *= tot;  // normalize so they sum to games
        deficit = nominal[i] - (wins(entries + i) + losses(entries + i));
        if (deficit > max) {
            max = deficit, choice = i;
        }
        if (deficit < min) {
            min = deficit;
        }
    }  // note that a single move will never be underplayed
    if (max - min > 0.5 * sqrt(nominal[choice])) {  // if one of the listed moves is significantly under-played, play it now.
        move_to_string(move_s, entries[choice].move);
        if (appData.debugMode) {
            fprintf(debugFP, "book move field = %d\n", entries[choice].move);
        }
        return move_s;
    }
    return NULL;  // otherwise fake book miss to force engine think, hoping for hitherto unplayed move.
}

char * ProbeBook(int moveNr, char * book) {  //
    entry_t entries[MOVE_BUF];
    int count;
    int i, j;
    static char move_s[6];
    int total_weight;

    if (moveNr >= 2 * appData.bookDepth) {
        return NULL;
    }
    if (mcMode) {
        return MCprobe(moveNr);
    }

    if ((count = ReadFromBookFile(moveNr, book, entries)) <= 0) {
        return NULL;  // no book, or no hit
    }

    if (appData.bookStrength != 50) {  // transform weights
        double power = 0, maxWeight = 0.0;
        if (appData.bookStrength) {
            power = (100. - appData.bookStrength) / appData.bookStrength;
        }
        for (i = 0; i < count; i++) {
            if (entries[i].weight > maxWeight) {
                maxWeight = entries[i].weight;
            }
        }
        for (i = 0; i < count; i++) {
            double weight = entries[i].weight / maxWeight;
            if (weight > 0) {
                entries[i].weight = appData.bookStrength || weight == 1.0 ? 1e4 * exp(power * log(weight)) + 0.5 : 0.0;
            }
        }
    }
    total_weight = 0;
    for (i = 0; i < count; i++) {
        total_weight += entries[i].weight;
    }
    if (total_weight == 0) {
        return NULL;  // force book miss rather than playing moves with weight 0.
    }
    j = (random() & 0xfff) * total_weight >> 12;  // create random < total_weight
    total_weight = 0;
    for (i = 0; i < count; i++) {
        total_weight += entries[i].weight;
        if (total_weight > j) {
            break;
        }
    }
    if (i >= count) {
        DisplayFatalError(_("Book Fault"), 0, 1);  // safety catch, cannot happen
    }
    move_to_string(move_s, entries[i].move);
    if (appData.debugMode) {
        fprintf(debugFP, "book move field = %d\n", entries[i].move);
    }

    return move_s;
}

extern char yy_textstr[];
entry_t lastEntries[MOVE_BUF];

char * MovesToText(int count, entry_t * entries) {
    int i, totalWeight = 0;
    char algMove[12];
    char * p = (char *)malloc(40 * count + 1);
    for (i = 0; i < count; i++) {
        totalWeight += entries[i].weight;
    }
    *p = 0;
    for (i = 0; i < count; i++) {
        char buf[MSG_SIZ], c1, c2, c3;
        int i1, i2, i3;
        move_to_string(algMove, entries[i].move);
        if (sscanf(algMove, "%c%d%*c%*d,%c%d%c%d", &c1, &i1, &c2, &i2, &c3, &i3) == 6) {
            snprintf(
             algMove, 12, "%c%dx%c%d-%c%d", c1, i1, c2, i2, c3, i3);  // cast double-moves in format SAN parser will understand
        } else if (sscanf(algMove, "%c%d%c%d%c", &c1, &i1, &c2, &i2, &c3) >= 4) {
            CoordsToAlgebraic(
             boards[currentMove], PosFlags(currentMove), i1 - ONE + '0', c1 - AAA, i2 - ONE + '0', c2 - AAA, c3, algMove);
        }
        buf[0] = NULLCHAR;
        if (entries[i].learnCount || entries[i].learnPoints) {
            snprintf(buf, MSG_SIZ, " {%d/%d}", entries[i].learnPoints, entries[i].learnCount);
        }
        snprintf(
         p + strlen(p), 40, "%5.1f%% %5d %s%s\n", 100 * entries[i].weight / (totalWeight + 0.001), entries[i].weight, algMove, buf);
        // lastEntries[i] = entries[i];
    }
    return p;
}

static int CoordsToMove(int fromX, int fromY, int toX, int toY, char promoChar) {
    int i, width = BOARD_RGHT - BOARD_LEFT;
    int to = toX - BOARD_LEFT + toY * width;
    int from = fromX - BOARD_LEFT + fromY * width;
    for (i = 0; promote_pieces[i]; i++) {
        if (promote_pieces[i] == promoChar) {
            break;
        }
    }
    if (!promote_pieces[i]) {
        i = 0;
    } else if (i == 9 && gameInfo.variant == VariantChu) {
        i = 1;  // on 12x12 only 3 promotion codes available, so use 1 to indicate promotion
    }
    if (fromY == DROP_RANK) {
        i = 9, from = ToUpper(PieceToChar(fromX)) - '@';
    }
    if (killX >= 0) {  // multi-leg move
        int dx = killX - fromX, dy = killY - fromY;
        for (i = 0; i < 8; i++) {
            if (dx == xStep[i] && dy == yStep[i]) {
                int j;
                dx = toX - killX;
                dy = toY - killY;
                for (j = 0; j < 8; j++) {
                    if (dx == xStep[j] && dy == yStep[j]) {
                        // special encoding in to-square, with promoType = 2. Assumes board >= 64 squares!
                        return i + 8 * j + (2 * width * BOARD_HEIGHT + from) * width * BOARD_HEIGHT;
                    }
                }
            }
        }
        i = 0;  // if not a valid Lion move, ignore kill-square and promoChar
    }
    return to + (i * width * BOARD_HEIGHT + from) * width * BOARD_HEIGHT;
}

int TextToMoves(char * text, int moveNum, entry_t * entries) {
    int flip = sym && MustFlip(moveNum);
    int i, w, move, count = 0;
    uint64_t hashKey = hash(moveNum, flip, 0);
    int fromX, fromY, toX, toY;
    ChessMove moveType;
    char promoChar, valid;
    float dummy;

    entries[0].key = hashKey;  // make sure key is returned even if no moves
    while ((i = sscanf(text, "%f%%%d", &dummy, &w)) == 2 || (i = sscanf(text, "%d", &w)) == 1) {
        if (i == 2) {
            text = strchr(text, '%') + 1;  // skip percentage
        }
        if (w == 1) {
            text = strstr(text, "1 ") + 2;  // skip weight that could be recognized as move number one
        }
        valid = ParseOneMove(text, moveNum, &moveType, &fromX, &fromY, &toX, &toY, &promoChar);
        text = strstr(text, yy_textstr) + strlen(yy_textstr);  // skip what we parsed
        if (!valid ||
         moveType != NormalMove && moveType != WhiteDrop && moveType != BlackDrop && moveType != FirstLeg &&
          moveType != WhitePromotion && moveType != BlackPromotion && moveType != WhiteCapturesEnPassant &&
          moveType != BlackCapturesEnPassant && moveType != WhiteKingSideCastle && moveType != BlackKingSideCastle &&
          moveType != WhiteQueenSideCastle && moveType != BlackQueenSideCastle && moveType != WhiteNonPromotion &&
          moveType != BlackNonPromotion) {
            continue;
        }
        if (*text == ' ' && sscanf(text + 1, "{%hd/%hd}", &entries[count].learnPoints, &entries[count].learnCount) == 2) {
            text = strchr(text + 1, '}') + 1;
        } else {
            entries[count].learnPoints = 0;
            entries[count].learnCount = 0;
        }
        move = CoordsToMove(fromX, fromY, toX, toY, promoChar);
        killX = killY = -1;
        if (flip) {
            move = FlipMove(move);
        }
        entries[count].move = move;
        entries[count].key = hashKey;
        entries[count].weight = w;
        count++;
    }
    return count;
}

Boolean bookUp;
int currentCount;

Boolean DisplayBook(int moveNr) {
    entry_t entries[MOVE_BUF];
    int count;
    char * p;
    if (!bookUp) {
        return FALSE;
    }
    count = currentCount = ReadFromBookFile(moveNr, appData.polyglotBook, entries);
    if (count < 0) {
        return FALSE;
    }
    p = MovesToText(count, entries);
    EditTagsPopUp(p, NULL);
    free(p);
    addToBookFlag = FALSE;
    return TRUE;
}

void EditBookEvent(void) {
    bookUp = TRUE;
    bookUp = DisplayBook(currentMove);
}

void int_to_file(FILE * f, int l, uint64_t r) {
    int i;
    for (i = l - 1; i >= 0; i--) {
        fputc(r >> 8 * i & 255, f);
    }
}

void entry_to_file(FILE * f, entry_t * entry) {
    int_to_file(f, 8, entry->key);
    int_to_file(f, 2, entry->move);
    int_to_file(f, 2, entry->weight);
    int_to_file(f, 2, entry->learnCount);
    int_to_file(f, 2, entry->learnPoints);
}

char buf1[4096], buf2[4096];

void SaveToBook(char * text) {
    entry_t entries[MOVE_BUF], entry;
    int count = TextToMoves(text, currentMove, entries);
    int offset, i, len1 = 0, len2, readpos = 0, writepos = 0;
    FILE * f;
    if (!count && !currentCount) {
        return;
    }
    f = fopen(appData.polyglotBook, "rb+");
    sym = strstr(appData.polyglotBook, "-sym.");
    if (!f) {
        DisplayError(_("Polyglot book not valid"), 0);
        return;
    }
    offset = find_key(f, entries[0].key, &entry);
    if (entries[0].key != entry.key && currentCount) {
        DisplayError(_("Hash keys are different"), 0);
        fclose(f);
        return;
    }
    if (count != currentCount) {
        readpos = 16 * (offset + currentCount);
        writepos = 16 * (offset + count);
        fsseek(f, readpos, SEEK_SET);
        readpos += len1 = fread(buf1, 1, 4096 - 16 * currentCount, f);  // salvage some entries immediately behind change
    }
    fsseek(f, 16 * (offset), SEEK_SET);
    for (i = 0; i < count; i++) {
        entry_to_file(f, entries + i);  // save the change
    }
    if (count != currentCount) {
        do {
            for (i = 0; i < len1; i++) {
                buf2[i] = buf1[i];
            }
            len2 = len1;
            if (readpos > writepos) {
                fsseek(f, readpos, SEEK_SET);
                readpos += len1 = fread(buf1, 1, 4096, f);
            } else {
                len1 = 0;  // wrote already past old EOF
            }
            fsseek(f, writepos, SEEK_SET);
            fwrite(buf2, 1, len2, f);
            writepos += len2;
        } while (len1);
    }
    dirty = 1;
    fclose(f);
}

void NewEntry(entry_t * e, uint64_t key, int move, int result) {
    e->key = key;
    e->move = move;
    e->learnPoints = 0;
    e->learnCount = 0;
    CountMove(e, result);
}

void Merge(void) {
    int i;

    if (appData.debugMode) {
        fprintf(debugFP, "book merge %d moves (old size %d)\n", mergeSize, bookSize);
    }

    bookSize += --mergeSize;
    for (i = bookSize - 1; mergeSize; i--) {
        while (mergeSize && (i < mergeSize || mergeBuf[mergeSize - 1].key >= memBook[i - mergeSize].key)) {
            memBook[i--] = mergeBuf[--mergeSize];
        }
        if (i < 0) {
            break;
        }
        memBook[i] = memBook[i - mergeSize];
    }
    if (mergeSize) {
        DisplayFatalError("merge error", 0, 0);  // impossible
    }
    mergeSize = 1;
    mergeBuf[0].key = -1ll;
}

void AddToBook(int moveNr, int result) {
    int flip = sym && MustFlip(moveNr);
    entry_t entry;
    int offset, start, move;
    uint64_t key;
    int i, j, fromY, toY;
    char fromX, toX, promo;
    extern char moveList[][MOVE_LEN];

    if (!moveList[moveNr][0] || moveList[moveNr][0] == '\n') {
        return;  // could be terminal position
    }

    if (appData.debugMode) {
        fprintf(debugFP, "add move %d to book %s", moveNr, moveList[moveNr]);
    }

    // calculate key and book representation of move
    key = hash(moveNr, flip, 0);
    if (moveList[moveNr][1] == '@') {
        sscanf(moveList[moveNr], "%c@%c%d", &promo, &toX, &toY);
        fromX = CharToPiece(WhiteOnMove(moveNr) ? ToUpper(promo) : ToLower(promo));
        fromY = DROP_RANK;
        promo = NULLCHAR;
    } else {
        sscanf(moveList[moveNr], "%c%d%c%d%c", &fromX, &fromY, &toX, &toY, &promo), fromX -= AAA, fromY -= ONE - '0';
    }
    move = CoordsToMove(fromX, fromY, toX - AAA, toY - ONE + '0', promo);
    if (flip) {
        move = FlipMove(move);
    }

    // if move already in book, just add count
    memBuf = (unsigned char *)memBook;
    bufSize = bookSize;  // in MC mode book resides in memory
    offset = find_key(NULL, key, &entry);
    while (memBook[offset].key == key) {
        if (memBook[offset].move == move) {
            CountMove(memBook + offset, result);
            return;
        } else {
            offset++;
        }
    }
    // move did not occur in the main book
    memBuf = (unsigned char *)mergeBuf;
    bufSize = mergeSize;  // it could be amongst moves still waiting to be merged
    start = offset = find_key(NULL, key, &entry);
    while (mergeBuf[offset].key == key) {
        if (mergeBuf[offset].move == move) {
            if (appData.debugMode) {
                fprintf(debugFP, "found in book merge buf @ %d\n", offset);
            }
            CountMove(mergeBuf + offset, result);
            return;
        } else {
            offset++;
        }
    }
    if (start != offset) {  // position was in mergeBuf, but move is new
        if (appData.debugMode) {
            fprintf(debugFP, "add in book merge buf @ %d\n", offset);
        }
        for (i = mergeSize++; i > offset; i--) {
            mergeBuf[i] = mergeBuf[i - 1];  // make room
        }
        NewEntry(mergeBuf + offset, key, move, result);
        return;
    }
    // position was not in mergeBuf; look in hash table
    i = (key & mask);
    offset = -1;
    while (hashTab[i].key) {  // search in hash table (necessary because sought item could be re-hashed)
        if (hashTab[i].key == 1 && offset < 0) {
            offset = i;  // remember first invalidated entry we pass
        }
        if (!((hashTab[i].key - key) & ~1)) {  // hit
            if (hashTab[i].move == move) {
                CountMove(hashTab + i, result);
                for (j = mergeSize++; j > start; j--) {
                    mergeBuf[j] = mergeBuf[j - 1];
                }
            } else {
                // position already in hash now occurs with different move; move both moves to mergeBuf
                for (j = mergeSize + 1; j > start + 1; j--) {
                    mergeBuf[j] = mergeBuf[j - 2];
                }
                NewEntry(mergeBuf + start + 1, key, move, result);
                mergeSize += 2;
            }
            hashTab[i].key = 1;  // kludge to invalidate hash entry
            mergeBuf[start] = hashTab[i];
            mergeBuf[start].key = key;
            if (mergeSize >= MERGESIZE) {
                Merge();
            }
            return;
        }
        i = i + 1 & mask;  // wrap!
    }
    // position did not yet occur in hash table. Put it there
    if (offset < 0) {
        offset = i;
    }
    NewEntry(hashTab + offset, key, move, result);
    if (appData.debugMode) {
        fprintf(debugFP, "book hash @ %d (%d-%d)\n", offset, hashTab[offset].learnPoints, hashTab[offset].learnCount);
    }
}

void AddGameToBook(int always) {
    int i, result;

    if (!mcMode && !always) {
        return;
    }

    InitMemBook();
    switch (gameInfo.result) {
    case GameIsDrawn:
        result = 1;
        break;
    case WhiteWins:
        result = 2;
        break;
    case BlackWins:
        result = 0;
        break;
    default:
        return;  // don't treat games with unknown result
    }

    if (appData.debugMode) {
        fprintf(debugFP, "add game to book (%d-%d)\n", backwardMostMove, forwardMostMove);
    }
    for (i = backwardMostMove; i < forwardMostMove && i < 2 * appData.bookDepth; i++) {
        AddToBook(i, WhiteOnMove(i) ? result : 2 - result);  // flip result when black moves
    }
}

void PlayBookMove(char * text, int index) {
    char *start = text + index, *end = start;
    while (start > text && start[-1] != ' ' && start[-1] != '\t') {
        start--;
    }
    while (*end && *++end != ' ' && *end != '\n')
        ;
    *end = NULLCHAR;  // find clicked word
    if (start != end) {
        TypeInDoneEvent(start);  // fake it was typed in move type-in
    }
}

void FlushBook(void) {
    FILE * f;
    int i;

    InitMemBook();
    Merge();  // flush merge buffer to memBook

    if (f = fopen(appData.polyglotBook, "wb")) {
        sym = strstr(appData.polyglotBook, "-sym.");
        for (i = 0; i < bookSize; i++) {
            entry_t entry = memBook[i];
            entry.weight = entry.learnPoints;
            //	    entry.learnPoints = 0;
            //	    entry.learnCount  = 0;
            entry_to_file(f, &entry);
        }
        fclose(f);
    } else {
        DisplayError(_("Could not create book"), 0);
    }
}
