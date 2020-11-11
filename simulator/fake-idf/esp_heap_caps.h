/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#define MALLOC_CAP_INTERNAL 0
#define MALLOC_CAP_8BIT 0
#define MALLOC_CAP_SPIRAM 0
#define MALLOC_CAP_DEFAULT 0

#ifdef __cplusplus
extern "C"
{
#endif
void *heap_caps_malloc(unsigned int , unsigned int  );
void heap_caps_free(void *);
void *heap_caps_calloc(unsigned int , unsigned int , unsigned int );
void *heap_caps_realloc(void *, unsigned int , unsigned int );
int heap_caps_get_free_size(unsigned int);
int heap_caps_get_largest_free_block(unsigned int);
#ifdef __cplusplus
}
#endif
