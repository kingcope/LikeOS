/* lib.h - PaulOS embedded operating system
   Copyright (C) 2002, 2003  Paul Sheer

   Rights to copy and modify this program are restricted to strict copyright
   terms. These terms can be found in the file LICENSE distributed with this
   software.

   This software is provided "AS IS" and WITHOUT WARRANTY, either express or
   implied, including, without limitation, the warranties of NON-INFRINGEMENT,
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO
   THE QUALITY OF THIS SOFTWARE IS WITH YOU. Under no circumstances and under
   no legal theory, whether in tort (including negligence), contract, or
   otherwise, shall the copyright owners be liable for any direct, indirect,
   special, incidental, or consequential damages of any character arising as a
   result of the use of this software including, without limitation, damages
   for loss of goodwill, work stoppage, computer failure or malfunction, or any
   and all other commercial damages or losses. This limitation of liability
   shall not apply to liability for death or personal injury resulting from
   copyright owners' negligence to the extent applicable law prohibits such
   limitation. Some jurisdictions do not allow the exclusion or limitation of
   incidental or consequential damages, so this exclusion and limitation may
   not apply to You.
*/

#ifndef __ARCH_LIB_H__
#define __ARCH_LIB_H__

#if !defined(LWIP_STRING_H) && !defined (_STRING_H_) && !defined (_STRINGS_H_) && !defined (_STRING_H) && !defined (_STRINGS_H)
#define LWIP_STRING_H
int strlen(const char *str);
int strncmp(const char *str1, const char *str2, int len);
//void bcopy(const void *src, void *dest, int len);
//void bzero(void *data, int n);
#endif /* _STRING_H */

#endif /* __ARCH_LIB_H__ */
