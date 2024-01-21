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


#if defined(CONFIG_TBD_PLATFORM_STR)
string const s("{\"HWV\":\"" + TBD_HW_VERSION + "\",\"FWV\":\"" + TBD_FW_VERSION + "\",\"p\":\"str\",\"t\":[\"TRIG0\", \"TRIG1\"], \"cv\":[\"CV1\",\"CV2\",\"CV3\",\"CV4\",\"CV5\",\"CV6\",\"CV7\",\"CV8\"]}");
#elif defined(CONFIG_TBD_PLATFORM_MK2)
string const s("{\"HWV\":\"" + TBD_HW_VERSION + "\",\"FWV\":\"" + TBD_FW_VERSION + "\",\"p\":\"mk2\",\"t\":[\"TRIG0\",\"TRIG1\",\"TRIG2\",\"TRIG3\",\"TRIG4\",\"TRIG5\",\"M0NOTE\",\"M1NOTE\",\"M0VEL\",\"M1VEL\",\"MOD0\",\"MOD1\"],\"cv\":[\"UCVPOT0\",\"UCVPOT1\",\"UCVPOT2\",\"UCVPOT3\",\"POT0\",\"POT1\",\"POT2\",\"POT3\",\"PCV0\",\"PCV1\",\"BPCV0\",\"BPCV1\",\"BPCV2\",\"BPCV3\",\"M0NOTE\",\"M1NOTE\",\"M0VEL\",\"M1VEL\",\"M0PB\",\"M1PB\",\"M0MOD\",\"M1MOD\"]}");
#elif defined(CONFIG_TBD_PLATFORM_BBA)
string const s("{\"HWV\":\"" + TBD_HW_VERSION + "\",\"FWV\":\"" + TBD_FW_VERSION + "\",\"p\":\"bba\",\"t\":[\"A_NOTE\",\"A_VELO\",\"A_P_PROG\",\"A_P_AT\",\"B_NOTE\",\"B_VELO\",\"B_PROG\",\"B_AT\",\"C_NOTE\",\"C_VELO\",\"C_PROG\",\"C_AT\",\"D_NOTE\",\"D_VELO\",\"D_PROG\",\"D_AT\",\"A_75_P_C1\",\"A_76_P_C#1\",\"A_77_P_D1\",\"A_78_P_D#1\",\"B_75_P_E1\",\"B_76_P_F1\",\"B_77_P_F#1\",\"B_78_P_G1\",\"C_75_P_G#1\",\"C_76_P_A1\",\"C_77_P_A#1\",\"C_78_P_B1\",\"D_75_P_C2\",\"D_76_P_C#2\",\"D_77_P_D2\",\"D_78_P_D#2\",\"G_AT\",\"G_FX1_12\",\"G_FX2_13\",\"G_SUST_64\",\"G_PORT_65\",\"G_SSTN_66\",\"G_SOFT_67\",\"G_HOLD_69\"], \"cv\":[\"A_NOTE\",\"A_VELO\",\"A_P_BANK\",\"A_P_SBNK\",\"A_P_PRG\",\"A_P_PB\",\"A_P_PB_LG\",\"A_P_AT\",\"A_P_MW_1\",\"A_P_BC_2\",\"B_NOTE\",\"B_VELO\",\"B_BANK\",\"B_SBNK\",\"B_PRG\",\"B_PB\",\"B_PB_LG\",\"B_AT\",\"B_MW_1\",\"B_BC_2\",\"C_NOTE\",\"C_VELO\",\"C_BANK\",\"C_SBNK\",\"C_PRG\",\"C_PB\",\"C_PB_LG\",\"C_AT\",\"C_MW_1\",\"C_BC_2\",\"D_NOTE\",\"D_VELO\",\"D_BANK\",\"D_SBNK\",\"D_PRG\",\"D_PB\",\"D_PB_LG\",\"D_AT\",\"D_MW_1\",\"D_BC_2\",\"A_P_RES_71\",\"A_P_REL_72\",\"A_P_ATK_73\",\"A_P_CUT_74\",\"A_75_P_C1\",\"A_76_P_C#1\",\"A_77_P_D1\",\"A_78_P_D#1\",\"B_RES_71\",\"B_REL_72\",\"B_ATK_73\",\"B_CUT_74\",\"B_75_P_E1\",\"B_76_P_F1\",\"B_77_P_F#1\",\"B_78_P_G1\",\"C_RES_71\",\"C_REL_72\",\"C_ATK_73\",\"C_CUT_74\",\"C_75_P_G#1\",\"C_76_P_A1\",\"C_77_P_A#1\",\"C_78_P_B1\",\"D_RES_71\",\"D_REL_72\",\"D_ATK_73\",\"D_CUT_74\",\"D_75_P_C2\",\"D_76_P_C#2\",\"D_77_P_D2\",\"D_78_P_D#2\",\"G_PB\",\"G_PB_LG\",\"G_AT\",\"G_MW_1\",\"G_BC_2\",\"G_FOOT_4\",\"G_DAT_6\",\"G_VOL_7\",\"G_BAL_8\",\"G_PAN_10\",\"G_XPR_11\",\"G_FX1_12\",\"G_FX2_13\",\"G_SUST_64\",\"G_PORT_65\",\"G_SOST_66\",\"G_SOFT_67\",\"G_HOLD_69\"]}");
// ### alternative version with raw string-literals ### string const triggers_list = R"("A_NOTE","A_VELO","A_PROG","A_AT","B_NOTE","B_VELO","B_PROG","B_AT","C_NOTE","C_VELO","C_PROG","C_AT","D_NOTE","D_VELO","D_PROG","D_AT","A_75","A_76","A_77","A_78","B_75","B_76","B_77","B_78","C_75","C_76","C_77","C_78","D_75","D_76","D_77","D_78","G_AT","G_FX1_12","G_FX2_13","G_SUST_64","G_PORT_65","G_SSTN_66","G_SOFT_67","G_HOLD_69")";
// ### alternative version with raw string-literals ### string const cvs_list = R"("A_NOTE","A_VELO","A_BANK","A_SBNK","A_PRG","A_PB","A_PB_LG","A_AT","A_MW_1","A_BC_2","B_NOTE","B_VELO","B_BANK","B_SBNK","B_PRG","B_PB","B_PB_LG","B_AT","B_MW_1","B_BC_2","C_NOTE","C_VELO","C_BANK","C_SBNK","C_PRG","C_PB","C_PB_LG","C_AT","C_MW_1","C_BC_2","D_NOTE","D_VELO","D_BANK","D_SBNK","D_PRG","D_PB","D_PB_LG","D_AT","D_MW_1","D_BC_2","A_RES_71","A_REL_72","A_ATK_73","A_CUT_74","A_75","A_76","A_77","A_78","B_RES_71","B_REL_72","B_ATK_73","B_CUT_74","B_75","B_76","B_77","B_78","C_RES_71","C_REL_72","C_ATK_73","C_CUT_74","C_75","C_76","C_77","C_78","D_RES_71","D_REL_72","D_ATK_73","D_CUT_74","D_75","D_76","D_77","D_78","G_PB","G_PB_LG","G_AT","G_MW_1","G_BC_2","G_FOOT_4","G_DAT_6","G_VOL_7","G_BAL_8","G_PAN_10","G_XPR_11","G_FX1_12","G_FX2_13","G_SUST_64","G_PORT_65","G_SOST_66","G_SOFT_67","G_HOLD_69")";
// ### alternative version with raw string-literals ### string const s("{ \"HWV\":\"" + TBD_HW_VERSION + "\",\"FWV\":\"" + TBD_FW_VERSION + "\",\"p\":\"bba\",\"t\":[" + triggers_list + "], \"cv\":[" + cvs_list + "]}");
#else
string const s("{\"HWV\":\"" + TBD_HW_VERSION + "\",\"FWV\":\"" + TBD_FW_VERSION + "\",\"p\":\"mk1\",\"t\":[\"TRIG0\", \"TRIG1\"], \"cv\":[\"CV0\",\"CV1\",\"POT0\",\"POT1\"]}");
#endif