/*
* iSamsungTV: Remote Command Line Interface
* Copyright (c) 2013-2016 Tristan (@monkeycat.nl)
* 
* v1.03 Push The Button, Sweet (github publish release)
*
* Thanks!
* Modified base64 (Base64EncodeDecode.c) from Sam Ernest Kumar 
* Part code/research from various places/people on the samygo forum
*
* Missing:
* Network Error Handling (mostly missing), it really hangs with a wrong IP ;-)
* Key &| Text Input Checking, some chars will result in disabling the TV's remote part
*

Copyright (c) 2016, Tristan Crispijn
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. You may not use source, binary forms or derivative work, with or without modification, for commercial purposes. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. ENJOY!
               
*/
 
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
 
 
int samsungtv_base64encodeblock(char *input, char *output, int oplen){
  int rc = 0, iplen = 0;
  char encodedstr[5] = "";
  char encodingtabe[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  iplen = strlen(input);
  encodedstr[0] = encodingtabe[ input[0] >> 2 ];
  encodedstr[1] = encodingtabe[ ((input[0] & 0x03) << 4) | ((input[1] & 0xf0) >> 4) ];
  encodedstr[2] = (iplen > 1 ? encodingtabe[ ((input[1] & 0x0f) << 2) | ((input[2] & 0xc0) >> 6) ] : '=');
  encodedstr[3] = (iplen > 2 ? encodingtabe[ input[2] & 0x3f ] : '=');
  strncat(output, encodedstr, oplen-strlen(output));  
  return rc;
}
 
int samsungtv_base64encode(char *input, char *output, int oplen){
  int rc = 0;
  int index = 0, ipindex = 0, iplen = 0;
  char encoderinput[4] = "";
  iplen = strlen(input);
  while(ipindex < iplen){
    for(index = 0; index < 3; index++){
      if(ipindex < iplen){
        encoderinput[index] = input[ipindex];
      }else{
        encoderinput[index] = 0;
      }
      ipindex++;
    }
    rc = samsungtv_base64encodeblock(encoderinput, output, oplen);
  }
  return rc;
}
 
int samsungtv_response(int net_socket) {
 unsigned char message[256];
 memset (message,0x00,256);
 int i = 3;
 recv(net_socket,message,3,0);
 recv(net_socket,message+i,message[1],0);
 i += message[1];
 recv(net_socket,message+i,2,0);
 i += 2;
 unsigned char response = message[i-2];
 recv(net_socket,message+i,response,0);
 i += response;
 if (message[i-4] == 0x00 && message[i-3] == 0x00 && message[i-2] == 0x00 && message[i-1] == 0x00)
   if (message[0] == 0x01) return 4;           // success repeat keystroke...
   else return 0;                     // success
 if (message[i-response] == 0x65) return 3;         // timeoutt....
 if (message[i-4] == 0x64 && message[i-2] == 0x00) return 1; // access denied...
 if (message[i-4] == 0x64 && message[i-2] == 0x01) return 0; // success
 if (message[i-response] == 0x0A) return 2;         // waiting for user...
 return -1;                         // bug!
}
 
int samsungtv_setlength(unsigned char message[], unsigned int length) {
  message[0] = (unsigned char) (length & 0xFF);
  message[1] = (unsigned char) ((length >> 8) & 0xFF);
}
 
int samsungtv_setstring(unsigned char message[],unsigned char string[],int base64) {
  unsigned char s[512];
  memset (s,0x00,512);
  if (base64 == 1) samsungtv_base64encode(string,s,strlen(string)*2);
  else strncpy(s,string,strlen(string));
  samsungtv_setlength(message,strlen(s));
  strncpy(message+2,s,strlen(s));
  return strlen(s)+2;
}
 
enum modes {eKey, eText, eSMS, eCall, eSchedule, eAuth, eUnknown};
 
int samsungtv_message(unsigned char string[], int net_socket,int type) {
  unsigned char remote[] = "SamsungTVRemote";
  unsigned char message[1024];
  memset (message,0x00,1024);
  unsigned int s = samsungtv_setstring(message+1,"iphone.iapp.samsung",0) + 1;
  unsigned int i = s + 4 + (type==eKey?1:0);
  i += samsungtv_setstring(message+i,string,1);
  if (type == eAuth) {
    message[s+2] = 0x64;
    i += samsungtv_setstring(message+i,remote,1);
    i += samsungtv_setstring(message+i,remote,1);
  }
  if (type == eText) {
   message[0] = 0x01;
   message[s+2] = 0x01;
  }
  samsungtv_setlength(message+s,i-s-2);
  send(net_socket,message, i, 0);
  return (type==eText?0:samsungtv_response(net_socket));
}
 
int samsungtv_authenticate(unsigned char ip[], int net_socket) { return samsungtv_message(ip,net_socket,eAuth); }
int samsungtv_key(unsigned char key[], int net_socket) { return samsungtv_message(key,net_socket,eKey); }
int samsungtv_text(unsigned char text[], int net_socket) { return samsungtv_message(text,net_socket,eText); }
 
int samsungtv_sms(char ip[], int net_socket, char date[], char time[], char from[], char fromnumber[], char to[],
         char tonumber[], char message[]) {
 char request[3072];
  sprintf( request,"&lt;Category&gt;SMS&lt;/Category&gt;"
     "&lt;DisplayType&gt;Maximum&lt;/DisplayType&gt;"
     "&lt;ReceiveTime&gt;"
     "&lt;Date&gt;%s&lt;/Date&gt;"
     "&lt;Time&gt;%s&lt;/Time&gt;"
     "&lt;/ReceiveTime&gt;"
     "&lt;Receiver&gt;"
     "&lt;Number&gt;%s&lt;/Number&gt;"
     "&lt;Name&gt;%s&lt;/Name&gt;"
     "&lt;/Receiver&gt;"
     "&lt;Sender&gt;"
     "&lt;Number&gt;%s&lt;/Number&gt;"
     "&lt;Name&gt;%s&lt;/Name&gt;"
     "&lt;/Sender&gt;"
     "&lt;Body&gt;%s&lt;/Body&gt;",date,time,tonumber,to,fromnumber,from,message);
  return samsungtv_soap(ip,net_socket,request);
}
 
int samsungtv_schedule(char ip[], int net_socket, char subject[], char startdate[], char starttime[], char enddate[], char endtime[], char location[], char owner[], char number[], char message[]) {
char request[3072];
sprintf( request,"&lt;Category&gt;Schedule Reminder&lt;/Category&gt;"
"&lt;DisplayType&gt;Maximum&lt;/DisplayType&gt;"
"&lt;StartTime&gt;"
"&lt;Date&gt;%s&lt;/Date&gt;"
"&lt;Time&gt;%s&lt;/Time&gt;"
"&lt;/StartTime&gt;"
"&lt;Owner&gt;"
"&lt;Number&gt;%s&lt;/Number&gt;"
"&lt;Name&gt;%s&lt;/Name&gt;"
"&lt;/Owner&gt;"
"&lt;Subject&gt;%s&lt;/Subject&gt;"
"&lt;EndTime&gt;"
"&lt;Date&gt;%s&lt;/Date&gt;"
"&lt;Time&gt;%s&lt;/Time&gt;"
"&lt;/EndTime&gt;"
"&lt;Location&gt;%s&lt;/Location&gt;"
"&lt;Body&gt;%s&lt;/Body&gt;",startdate,starttime,number,owner,subject,enddate,endtime,location,message);
 return samsungtv_soap(ip,net_socket,request);
 
}
 
int samsungtv_call(char ip[], int net_socket, char date[], char time[], char from[], char fromnumber[], char to[] , char tonumber[]) {
 
  char request[3072];
  sprintf( request,"&lt;Category&gt;Incoming Call&lt;/Category&gt;"
      "&lt;DisplayType&gt;Maximum&lt;/DisplayType&gt;"
      "&lt;CallTime&gt;"
      "&lt;Date&gt;%s&lt;/Date&gt;"
      "&lt;Time&gt;%s&lt;/Time&gt;"
      "&lt;/CallTime&gt;"
      "&lt;Callee&gt;"
      "&lt;Number&gt;%s&lt;/Number&gt;"
      "&lt;Name&gt;%s&lt;/Name&gt;"
      "&lt;/Callee&gt;"
      "&lt;Caller&gt;"
      "&lt;Number&gt;%s&lt;/Number&gt;"
      "&lt;Name&gt;%s&lt;/Name&gt;"
      "&lt;/Caller&gt;",date,time,tonumber,to,fromnumber,from);
 
  return samsungtv_soap(ip,net_socket,request);
}
 
int samsungtv_soap(unsigned char ip[], int net_socket,char requestbody[]) {
 
  char request[3072];
  char buffer[4096];
 
  strcpy( request,
     "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
     "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
     "<s:Body>"
     "   <u:AddMessage xmlns:u=\"urn:samsung.com:service:MessageBoxService:1\">"
     "    <MessageType>text/xml</MessageType>"
     "    <MessageID>anything</MessageID>"
     "<Message>");
 
  strcat(request,requestbody);
  strcat(request,
     "</Message>"
     "   </u:AddMessage>"
     " </s:Body>"
     "</s:Envelope>" );
 
 sprintf( buffer,
    "POST /PMR/control/MessageBoxService HTTP/1.0\r\n"
    "Content-Type: text/xml; charset=\"utf-8\"\r\n"
    "HOST: %s\r\n"
    "Content-Length: %d\r\n"
    "SOAPACTION: \"uuid:samsung.com:service:MessageBoxService:1#AddMessage\"\r\n"
    "Connection: close\r\n"
    "\r\n", ip, strlen( request));
  strcat( buffer, request );
 
  int numbytes;
  if((numbytes = send(net_socket, buffer, strlen( buffer ), 0)) == -1) { }  
  if((numbytes = recv(net_socket, buffer, 10000, 0)) == -1) { }
}
 
int main(int argc, char *argv[]) {
 
  struct addrinfo hints, *res, *p;
  int net_status, net_socket;
   enum modes mode = eUnknown;
 
  if (argc > 2) {
    if (strcmp(argv[2],"-TEXT") == 0) mode = eText;
    if (strcmp(argv[2],"-KEY") == 0) mode = eKey;
    if (strcmp(argv[2],"-SMS") == 0) mode = eSMS;
    if (strcmp(argv[2],"-CALL") == 0) mode = eCall;
    if (strcmp(argv[2],"-SCHEDULE") == 0) mode = eSchedule;
 
  }
 
  if (argc < 4 || mode == eUnknown ||
 
    !(mode == eText && argc == 4) &&
    !(mode == eKey && argc == 4) &&
    !(mode == eSMS && argc == 10) &&
    !(mode == eCall && argc == 9) &&
    !(mode == eSchedule && argc == 12) 
 
    ) {
    printf("iSamsungTV: Push The Button, Sweet Remote Command Line Interface v0.03\nCopyright (c) 2013-2016 Tristan (@monkeycat.nl)\n\n");
    printf("Usage: iSamsungTV IP -KEY      KEY\n");
    printf("       iSamsungTV IP -TEXT     TEXT\n");
    printf("       iSamsungTV IP -SMS      DATE TIME FROM NUMBER TO NUMBER MESSAGE\n");
    printf("       iSamsungTV IP -CALL     DATE TIME FROM NUMBER TO NUMBER\n");
    printf("       iSamsungTV IP -SCHEDULE SUBJECT STARTDATE STARTTIME ENDDATE ENDTIME LOCATION OWNER NUMBER MESSAGE\n\n");
    printf("Examples: iSamsungTV 192.168.1.11 -KEY KEY_VOLUP\n          (Simulates press button Volume Up)\n\n");
    printf("          iSamsungTV 192.168.1.11 -TEXT \"Colour Haze\"\n          (Sends text to YouTube...)\n\n");
    printf("          iSamsungTV 192.168.1.11 -SMS 2013-6-24 \"7:01:01 PM\" Cris +555-4323 Me +555-2343 \"Get Off The Couch!?\"\n          (Show incomming SMS)\n\n");
    printf("          iSamsungTV 192.168.1.11 -CALL 23:06:01 Cris +555-4323 \"\" \"\"\n          (Show incomming call, skips input with empty strings)\n\n\n");
    printf("iSamsungTVPopup.sh bash script for notification messages on the big screen\n\n#!/bin/bash\niSamsungTV $1 -SMS \"\" \"\" \"\" \"\" \"\" \"\" $2\nSLEEP 0.3\niSamsungTV $1 -KEY KEY_ENTER\nSLEEP 3\niSamsungTV $1 -KEY KEY_ENTER\n\nUsage: iSamsungTVPopup.sh 10.0.0.2 \"Pop Says the message on the silver screen\"\n\n");
   return 1;
  }
 
  char port[] = "55000";
  if (mode != eKey && mode != eText) strcpy(port,"52235");
 
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
  hints.ai_socktype = SOCK_STREAM;
    if ((net_status = getaddrinfo(argv[1],port, &hints, &res)) != 0) {
    fprintf(stderr, "iSamsungTV: Connection Failure: (%s)\n", gai_strerror(net_status));
    return 2;
  }
  net_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if ((net_status = connect(net_socket, res->ai_addr, res->ai_addrlen)) != 0) {
    fprintf(stderr, "iSamsungTV: Connected Failure (%s)\n", gai_strerror(net_status));
    return 2;
  }
 
  if (mode == eKey || mode == eText) {
   int response,auth_status = samsungtv_authenticate(argv[1],net_socket);
   if (auth_status == 0) {
    if (mode == eText) response = samsungtv_text(argv[3],net_socket);
    else response = samsungtv_key(argv[3],net_socket);
    if (response != 0 && response != 4) fprintf(stderr, "iSamsungTV: Bug!\n");
   }
   else
   { if (auth_status == -1) fprintf(stderr, "iSamsungTV: Bug!\n");
    if (auth_status == 1) fprintf(stderr, "iSamsungTV: Access Denied\n");
    if (auth_status == 2) fprintf(stderr, "iSamsungTV: Waiting On User\n");
    if (auth_status == 3) fprintf(stderr, "iSamsungTV: Time Out\n");
   }
  }
  else {
   if (mode == eSMS) samsungtv_sms(argv[1],net_socket,argv[3],argv[4],argv[5],argv[6],argv[7],argv[8],argv[9]);
   if (mode == eCall) samsungtv_call(argv[1],net_socket,argv[3],argv[4],argv[5],argv[6],argv[7],argv[8]);
   if (mode == eSchedule) samsungtv_schedule(argv[1],net_socket,argv[3],argv[4],argv[5],argv[6],argv[7],argv[8],argv[9],argv[10],argv[11]);
  }
  close(net_socket);
  freeaddrinfo(res);
  return 0;
}


/* 

Key Codes

KEY_0
KEY_1
KEY_2
KEY_3
KEY_4
KEY_5
KEY_6
KEY_7
KEY_8
KEY_9
KEY_11
KEY_12
KEY_3SPEED
KEY_4_3
KEY_16_9
KEY_AD
KEY_ADDDEL
KEY_ALT_MHP
KEY_ANGLE
KEY_ANTENA
KEY_ANYNET
KEY_ANYVIEW
KEY_APP_LIST
KEY_ASPECT
KEY_AUTO_ARC_ANTENNA_AIR
KEY_AUTO_ARC_ANTENNA_CABLE
KEY_AUTO_ARC_ANTENNA_SATELLITE
KEY_AUTO_ARC_ANYNET_AUTO_START
KEY_AUTO_ARC_ANYNET_MODE_OK
KEY_AUTO_ARC_AUTOCOLOR_FAIL
KEY_AUTO_ARC_AUTOCOLOR_SUCCESS
KEY_AUTO_ARC_CAPTION_ENG
KEY_AUTO_ARC_CAPTION_KOR
KEY_AUTO_ARC_CAPTION_OFF
KEY_AUTO_ARC_CAPTION_ON
KEY_AUTO_ARC_C_FORCE_AGING
KEY_AUTO_ARC_JACK_IDENT
KEY_AUTO_ARC_LNA_OFF
KEY_AUTO_ARC_LNA_ON
KEY_AUTO_ARC_PIP_CH_CHANGE
KEY_AUTO_ARC_PIP_DOUBLE
KEY_AUTO_ARC_PIP_LARGE
KEY_AUTO_ARC_PIP_LEFT_BOTTOM
KEY_AUTO_ARC_PIP_LEFT_TOP
KEY_AUTO_ARC_PIP_RIGHT_BOTTOM
KEY_AUTO_ARC_PIP_RIGHT_TOP
KEY_AUTO_ARC_PIP_SMALL
KEY_AUTO_ARC_PIP_SOURCE_CHANGE
KEY_AUTO_ARC_PIP_WIDE
KEY_AUTO_ARC_RESET
KEY_AUTO_ARC_USBJACK_INSPECT
KEY_AUTO_FORMAT
KEY_AUTO_PROGRAM
KEY_AV1
KEY_AV2
KEY_AV3
KEY_BACK_MHP
KEY_BOOKMARK
KEY_CALLER_ID
KEY_CAPTION
KEY_CATV_MODE
KEY_CHDOWN
KEY_CHUP
KEY_CH_LIST
KEY_CLEAR
KEY_CLOCK_DISPLAY
KEY_COMPONENT1
KEY_COMPONENT2
KEY_CONTENTS
KEY_CONVERGENCE
KEY_CONVERT_AUDIO_MAINSUB
KEY_CUSTOM
KEY_CYAN
KEY_DEVICE_CONNECT
KEY_DISC_MENU
KEY_DMA
KEY_DNET
KEY_DNIe
KEY_DNSe
KEY_DOOR
KEY_DOWN
KEY_DSS_MODE
KEY_DTV
KEY_DTV_LINK
KEY_DTV_SIGNAL
KEY_DVD_MODE
KEY_DVI
KEY_DVR
KEY_DVR_MENU
KEY_DYNAMIC
KEY_ENTER
KEY_ENTERTAINMENT
KEY_ESAVING
KEY_EXIT
KEY_EXT1
KEY_EXT2
KEY_EXT3
KEY_EXT4
KEY_EXT5
KEY_EXT6
KEY_EXT7
KEY_EXT8
KEY_EXT9
KEY_EXT10
KEY_EXT11
KEY_EXT12
KEY_EXT13
KEY_EXT14
KEY_EXT15
KEY_EXT16
KEY_EXT17
KEY_EXT18
KEY_EXT19
KEY_EXT20
KEY_EXT21
KEY_EXT22
KEY_EXT23
KEY_EXT24
KEY_EXT25
KEY_EXT26
KEY_EXT27
KEY_EXT28
KEY_EXT29
KEY_EXT30
KEY_EXT31
KEY_EXT32
KEY_EXT33
KEY_EXT34
KEY_EXT35
KEY_EXT36
KEY_EXT37
KEY_EXT38
KEY_EXT39
KEY_EXT40
KEY_EXT41
KEY_FACTORY
KEY_FAVCH
KEY_FF
KEY_FF_
KEY_FM_RADIO
KEY_GAME
KEY_GREEN
KEY_GUIDE
KEY_HDMI
KEY_HDMI1
KEY_HDMI2
KEY_HDMI3
KEY_HDMI4
KEY_HELP
KEY_HOME
KEY_ID_INPUT
KEY_ID_SETUP
KEY_INFO
KEY_INSTANT_REPLAY
KEY_LEFT
KEY_LINK
KEY_LIVE
KEY_MAGIC_BRIGHT
KEY_MAGIC_CHANNEL
KEY_MDC
KEY_MENU
KEY_MIC
KEY_MORE
KEY_MOVIE1
KEY_MS
KEY_MTS
KEY_MUTE
KEY_NINE_SEPERATE
KEY_OPEN
KEY_PANNEL_CHDOWN
KEY_PANNEL_CHUP
KEY_PANNEL_ENTER
KEY_PANNEL_MENU
KEY_PANNEL_POWER
KEY_PANNEL_SOURCE
KEY_PANNEL_VOLDOW
KEY_PANNEL_VOLUP
KEY_PANORAMA
KEY_PAUSE
KEY_PCMODE
KEY_PERPECT_FOCUS
KEY_PICTURE_SIZE
KEY_PIP_CHDOWN
KEY_PIP_CHUP
KEY_PIP_ONOFF
KEY_PIP_SCAN
KEY_PIP_SIZE
KEY_PIP_SWAP
KEY_PLAY
KEY_PLUS100
KEY_PMODE
KEY_POWER
KEY_POWEROFF
KEY_POWERON
KEY_PRECH
KEY_PRINT
KEY_PROGRAM
KEY_QUICK_REPLAY
KEY_REC
KEY_RED
KEY_REPEAT
KEY_RESERVED1
KEY_RETURN
KEY_REWIND
KEY_REWIND_
KEY_RIGHT
KEY_RSS
KEY_RSURF
KEY_SCALE
KEY_SEFFECT
KEY_SETUP_CLOCK_TIMER
KEY_SLEEP
KEY_SOUND_MODE
KEY_SOURCE
KEY_SRS
KEY_STANDARD
KEY_STB_MODE
KEY_STILL_PICTURE
KEY_STOP
KEY_SUB_TITLE
KEY_SVIDEO1
KEY_SVIDEO2
KEY_SVIDEO3
KEY_TOOLS
KEY_TOPMENU
KEY_TTX_MIX
KEY_TTX_SUBFACE
KEY_TURBO
KEY_TV
KEY_TV_MODE
KEY_UP
KEY_VCHIP
KEY_VCR_MODE
KEY_VOLDOWN
KEY_VOLUP
KEY_WHEEL_LEFT
KEY_WHEEL_RIGHT
KEY_W_LINK
KEY_YELLOW
KEY_ZOOM1
KEY_ZOOmessage
KEY_ZOOM_IN
KEY_ZOOM_MOVE
KEY_ZOOM_OUT

*/

