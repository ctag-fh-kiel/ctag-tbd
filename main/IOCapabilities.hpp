/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2023 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#pragma once
#include "version.hpp"

// the s string is used to describe the TBD hardware and firmware version, platform and available triggers and cv inputs
// it is used in the GetIOCapabilities request and response
// it is a JSON string that is sent to the host to inform about the capabilities of the TBD
// the fields are:
// HWV: Hardware Version
// FWV: Firmware Version
// p: Platform (e.g. "str", "mk2", "bba", "dada")
// t: Triggers available on the TBD
// cv: CV inputs available on the TBD
// the values are defined in the respective platform configuration files
// the string is constructed at compile time and is used to inform the host about the capabilities of the TBD

string const s("{\"HWV\":\"" + TBD_HW_VERSION + "\",\"FWV\":\"" + TBD_FW_VERSION + "\",\"p\":\"dada\",\"t\":[\"A_NOTE\",\"A_VELO\",\"A_P_PROG\",\"A_P_AT\",\"B_NOTE\",\"B_VELO\",\"B_P_PROG\",\"B_P_AT\",\"C_NOTE\",\"C_VELO\",\"C_P_PROG\",\"C_P_AT\",\"D_NOTE\",\"D_VELO\",\"D_P_PROG\",\"D_P_AT\",\"A_75_P_C1\",\"A_76_P_C#1\",\"A_77_P_D1\",\"A_78_P_D#1\",\"B_75_P_E1\",\"B_76_P_F1\",\"B_77_P_F#1\",\"B_78_P_G1\",\"C_75_P_G#1\",\"C_76_P_A1\",\"C_77_P_A#1\",\"C_78_P_B1\",\"D_75_P_C2\",\"D_76_P_C#2\",\"D_77_P_D2\",\"D_78_P_D#2\",\"G_AT\",\"G_FX1_12\",\"G_FX2_13\",\"G_SUST_64\",\"G_PORT_65\",\"G_SSTN_66\",\"G_SOFT_67\",\"G_HOLD_69\",\"ET_41\",\"ET_42\",\"ET_43\",\"ET_44\",\"ET_45\",\"ET_46\",\"ET_47\",\"ET_48\",\"ET_49\",\"ET_50\",\"ET_51\",\"ET_52\",\"ET_53\",\"ET_54\",\"ET_55\",\"ET_56\",\"ET_57\",\"ET_58\",\"ET_59\",\"ET_60\"], \"cv\":[\"A_NOTE\",\"A_VELO\",\"A_P_BANK\",\"A_P_SBNK\",\"A_P_PRG\",\"A_P_PB\",\"A_P_PB_LG\",\"A_P_AT\",\"A_P_MW_1\",\"A_P_BC_2\",\"B_NOTE\",\"B_VELO\",\"B_P_BANK\",\"B_P_SBNK\",\"B_P_PRG\",\"B_P_PB\",\"B_P_PB_LG\",\"B_P_AT\",\"B_P_MW_1\",\"B_P_BC_2\",\"C_NOTE\",\"C_VELO\",\"C_P_BANK\",\"C_P_SBNK\",\"C_P_PRG\",\"C_P_PB\",\"C_P_PB_LG\",\"C_P_AT\",\"C_P_MW_1\",\"C_P_BC_2\",\"D_NOTE\",\"D_VELO\",\"D_P_BANK\",\"D_P_SBNK\",\"D_P_PRG\",\"D_P_PB\",\"D_P_PB_LG\",\"D_P_AT\",\"D_P_MW_1\",\"D_P_BC_2\",\"A_P_RES_71\",\"A_P_REL_72\",\"A_P_ATK_73\",\"A_P_CUT_74\",\"A_75_P_C1\",\"A_76_P_C#1\",\"A_77_P_D1\",\"A_78_P_D#1\",\"B_P_RES_71\",\"B_P_REL_72\",\"B_P_ATK_73\",\"B_P_CUT_74\",\"B_75_P_E1\",\"B_76_P_F1\",\"B_77_P_F#1\",\"B_78_P_G1\",\"C_P_RES_71\",\"C_P_REL_72\",\"C_P_ATK_73\",\"C_P_CUT_74\",\"C_75_P_G#1\",\"C_76_P_A1\",\"C_77_P_A#1\",\"C_78_P_B1\",\"D_P_RES_71\",\"D_P_REL_72\",\"D_P_ATK_73\",\"D_P_CUT_74\",\"D_75_P_C2\",\"D_76_P_C#2\",\"D_77_P_D2\",\"D_78_P_D#2\",\"G_PB\",\"G_PB_LG\",\"G_AT\",\"G_MW_1\",\"G_BC_2\",\"G_FOOT_4\",\"G_DAT_6\",\"G_VOL_7\",\"G_BAL_8\",\"G_PAN_10\",\"G_XPR_11\",\"G_FX1_12\",\"G_FX2_13\",\"G_SUST_64\",\"G_PORT_65\",\"G_SOST_66\",\"G_SOFT_67\",\"G_HOLD_69\",\"ECV_91\",\"ECV_92\",\"ECV_93\",\"ECV_94\",\"ECV_95\",\"ECV_96\",\"ECV_97\",\"ECV_98\",\"ECV_99\",\"ECV_100\",\"ECV_101\",\"ECV_102\",\"ECV_103\",\"ECV_104\",\"ECV_105\",\"ECV_106\",\"ECV_107\",\"ECV_108\",\"ECV_109\",\"ECV_110\",\"ECV_111\",\"ECV_112\",\"ECV_113\",\"ECV_114\",\"ECV_115\",\"ECV_116\",\"ECV_117\",\"ECV_118\",\"ECV_119\",\"ECV_120\",\"ECV_121\",\"ECV_122\",\"ECV_123\",\"ECV_124\",\"ECV_125\",\"ECV_126\",\"ECV_127\",\"ECV_128\",\"ECV_129\",\"ECV_130\",\"ECV_131\",\"ECV_132\",\"ECV_133\",\"ECV_134\",\"ECV_135\",\"ECV_136\",\"ECV_137\",\"ECV_138\",\"ECV_139\",\"ECV_140\",\"ECV_141\",\"ECV_142\",\"ECV_143\",\"ECV_144\",\"ECV_145\",\"ECV_146\",\"ECV_147\",\"ECV_148\",\"ECV_149\",\"ECV_150\",\"ECV_151\",\"ECV_152\",\"ECV_153\",\"ECV_154\",\"ECV_155\",\"ECV_156\",\"ECV_157\",\"ECV_158\",\"ECV_159\",\"ECV_160\",\"ECV_161\",\"ECV_162\",\"ECV_163\",\"ECV_164\",\"ECV_165\",\"ECV_166\",\"ECV_167\",\"ECV_168\",\"ECV_169\",\"ECV_170\",\"ECV_171\",\"ECV_172\",\"ECV_173\",\"ECV_174\",\"ECV_175\",\"ECV_176\",\"ECV_177\",\"ECV_178\",\"ECV_179\",\"ECV_180\",\"ECV_181\",\"ECV_182\",\"ECV_183\",\"ECV_184\",\"ECV_185\",\"ECV_186\",\"ECV_187\",\"ECV_188\",\"ECV_189\",\"ECV_190\",\"ECV_191\",\"ECV_192\",\"ECV_193\",\"ECV_194\",\"ECV_195\",\"ECV_196\",\"ECV_197\",\"ECV_198\",\"ECV_199\",\"ECV_200\",\"ECV_201\",\"ECV_202\",\"ECV_203\",\"ECV_204\",\"ECV_205\",\"ECV_206\",\"ECV_207\",\"ECV_208\",\"ECV_209\",\"ECV_210\",\"ECV_211\",\"ECV_212\",\"ECV_213\",\"ECV_214\",\"ECV_215\",\"ECV_216\",\"ECV_217\",\"ECV_218\",\"ECV_219\",\"ECV_220\",\"ECV_221\",\"ECV_222\",\"ECV_223\",\"ECV_224\",\"ECV_225\",\"ECV_226\",\"ECV_227\",\"ECV_228\",\"ECV_229\",\"ECV_230\",\"ECV_231\",\"ECV_232\",\"ECV_233\",\"ECV_234\",\"ECV_235\",\"ECV_236\",\"ECV_237\",\"ECV_238\",\"ECV_239\",\"ECV_240\"]}");