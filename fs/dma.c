/*****************************************************************************************
 Copyright (c) 2002 The UbixOS Project
 All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions, the following disclaimer and the list of authors.  Redistributions in binary
form must reproduce the above copyright notice, this list of conditions, the following
disclaimer and the list of authors in the documentation and/or other materials provided
with the distribution. Neither the name of the UbixOS Project nor the names of its
contributors may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 $Id: dma.c,v 1.4 2004/07/27 07:57:42 reddawg Exp $

*****************************************************************************************/

#define lowByte(x)  (x & 0x00FF)
#define highByte(x) ((x & 0xFF00) >> 8)
   
static unsigned char maskReg[8]   = { 0x0A, 0x0A, 0x0A, 0x0A, 0xD4, 0xD4, 0xD4, 0xD4 };
static unsigned char clearReg[8]  = { 0x0C, 0x0C, 0x0C, 0x0C, 0xD8, 0xD8, 0xD8, 0xD8 };
static unsigned char modeReg[8]   = { 0x0B, 0x0B, 0x0B, 0x0B, 0xD6, 0xD6, 0xD6, 0xD6 };
static unsigned char addrPort[8]  = { 0x00, 0x02, 0x04, 0x06, 0xC0, 0xC4, 0xC8, 0xCC };
static unsigned char pagePort[8]  = { 0x87, 0x83, 0x81, 0x82, 0x8F, 0x8B, 0x89, 0x8A };
static unsigned char countPort[8] = { 0x01, 0x03, 0x05, 0x07, 0xC2, 0xC6, 0xCA, 0xCE };

void _dmaXfer(unsigned char dmaChannel,unsigned char page,unsigned int offset,unsigned int length,unsigned char mode) {
  //asm("cli");
  outportb(maskReg[dmaChannel], 0x04 | dmaChannel);
  outportb(clearReg[dmaChannel], 0x00);
  outportb(modeReg[dmaChannel], mode);
  outportb(addrPort[dmaChannel], lowByte(offset));
  outportb(addrPort[dmaChannel], highByte(offset));
  outportb(pagePort[dmaChannel], page);
  outportb(countPort[dmaChannel], lowByte(length));
  outportb(countPort[dmaChannel], highByte(length));
  outportb(maskReg[dmaChannel], dmaChannel);
  //asm("sti");
  }

void dmaXfer(unsigned char channel,unsigned int address,unsigned int length,unsigned char read) {
  unsigned char page=0, mode=0;
  unsigned int offset = 0;
  if (read) {
    mode = 0x48 + channel;
    }
  else {
    mode = 0x44 + channel;
    }
  page = address >> 16;
  offset = address & 0xFFFF;
  length--;
  _dmaXfer(channel, page, offset, length, mode);
  }


/***
 $Log: dma.c,v $
 Revision 1.4  2004/07/27 07:57:42  reddawg
 chg: turned on ints during dma do we still boot?

 Revision 1.3  2004/07/21 17:42:22  reddawg
 Fixed dma code maybe it will work in 3.3.3 now

 Revision 1.2  2004/04/30 14:16:04  reddawg
 Fixed all the datatypes to be consistant unsigned char,uInt16,uInt32,Int8,Int16,Int32

 Revision 1.1.1.1  2004/04/15 12:07:17  reddawg
 UbixOS v1.0

 Revision 1.2  2004/04/13 16:36:34  reddawg
 Changed our copyright, it is all now under a BSD-Style license

 END
 ***/
