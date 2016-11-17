/*

iSamsungTV: Square Eye
Remote command line interface for SamsungTV Series B, C, D, E and F
Version 1.04 copyright (c) 2013-2016 Tristan (@monkeycat.nl)
 
License:

Copyright (c) 2016, Tristan Crispijn
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. You may not use source, binary forms or derivative work, with or without modification, for commercial purposes. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. ENJOY!

 
Issues:
- Network Error Handling (mostly missing), it really hangs with a wrong IP ;-)
- Key &| Text Input Checking, some chars will result in disabling the TV's remote part
- Timeout on sockets?

*/


#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>


enum modes {eKey, eText, eSMS, eCall, eSchedule, eAuth, eUnknown};

//TV model series range from B to F, default C
char series = 'C';


/* 

iSamsungTV LAN Remote Interface

*/



//
// Base64 Encoder
//
// Modified base64 (Base64EncodeDecode.c) from Sam Ernest Kumar


int samsungtv_base64encodeblock(char *input, char *output, int oplen) {
    int rc = 0, iplen = 0;
    char encodedstr[5] = "";
    char encodingtabe[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    iplen = strlen(input);
    encodedstr[0] = encodingtabe[input[0]>>2];
    encodedstr[1] = encodingtabe[((input[0]&0x03)<<4)|((input[1]&0xf0)>>4)];
    encodedstr[2] = (iplen>1?encodingtabe[((input[1]&0x0f)<<2)|((input[2]&0xc0) >> 6)]:'=');
    encodedstr[3] = (iplen>2?encodingtabe[input[2]&0x3f ]:'=');
    strncat(output, encodedstr, oplen-strlen(output));
    return rc;
}



int samsungtv_base64encode(char *input, char *output, int oplen) {
    int rc = 0;
    int index = 0, ipindex = 0, iplen = 0;
    char encoderinput[4] = "";
    iplen = strlen(input);
    while (ipindex < iplen){
        for (index = 0; index < 3; index++) {
            if(ipindex < iplen) encoderinput[index] = input[ipindex];
            else encoderinput[index] = 0;
            ipindex++;
        }
        rc = samsungtv_base64encodeblock(encoderinput, output, oplen);
    }
    return rc;
}



//
// SamsungTV LAN Remote Protocol
//



int samsungtv_response(int net_socket) {
    // Returns
    //-1 bug
    // 0 success
    // 1 access denied
    // 2 witing for user
    // 3 timeout
    // 4 success keystroke
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
    
    // Communication status!
    if (message[i-4] == 0x00 && message[i-3] == 0x00 && message[i-2] == 0x00 && message[i-1] == 0x00)
        if (message[0] == 0x01) return 4;                       // success repeat keystroke...
        else return 0;                                          // success
    if (message[i-response] == 0x65) return 3;                  // timeoutt....
    if (message[i-4] == 0x64 && message[i-2] == 0x00) return 1; // access denied...
    if (message[i-4] == 0x64 && message[i-2] == 0x01) return 0; // success
    if (message[i-response] == 0x0A) return 2;                  // waiting for user...
    return -1;                                                  // bug!
}



int samsungtv_setlength(unsigned char message[], unsigned int length) {
    message[0] = (unsigned char)(length&0xFF);
    message[1] = (unsigned char)((length>>8)&0xFF);
}



int samsungtv_setstring(unsigned char message[],unsigned char string[],int base64) {
    unsigned char s[512];
    memset(s,0x00,512);
    if (base64 == 1) samsungtv_base64encode(string,s,strlen(string)*2);
    else strncpy(s,string,strlen(string));
    samsungtv_setlength(message,strlen(s));
    strncpy(message+2,s,strlen(s));
    return strlen(s) + 2;
}
 


int samsungtv_message(unsigned char string[], int net_socket,int type) {
    // Returns
    //-1 bug
    // 0 success
    // 1 access denied
    // 2 witing for user
    // 3 timeout
    // 4 success keystroke

    unsigned char remote[] = "SamsungTVRemote";
    unsigned char message[1024];
    memset (message,0x00,1024);
    unsigned int s = samsungtv_setstring(message+1,"iphone.iapp.samsung",0) + 1;
    unsigned int i = s+4+(type==eKey?1:0);
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
    send(net_socket,message,i,0);
    return (type==eText?0:samsungtv_response(net_socket));
}



// Authenticate SamsungTV LAN Remote
int samsungtv_authenticate(unsigned char ip[], int net_socket) {
    // Returns
    //-1 bug
    // 0 success
    // 1 access denied
    // 2 witing for user
    // 3 timeout
    // 4 success keystroke
    return samsungtv_message(ip,net_socket,eAuth);
}



// Send a key code
int samsungtv_key(unsigned char key[], int net_socket) {
    // Returns
    //-1 bug
    // 0 success
    // 1 access denied
    // 2 witing for user
    // 3 timeout
    // 4 success keystroke
    return samsungtv_message(key,net_socket,eKey);
}



// Send text (for youtube search, etc...)
int samsungtv_text(unsigned char text[], int net_socket) {
    // Returns
    //-1 bug
    // 0 success
    // 1 access denied
    // 2 witing for user
    // 3 timeout
    // 4 success keystroke
    return samsungtv_message(text,net_socket,eText);
}



/* 

iSamsungTV SOAP Interface

*/



//
// SamsungTV SOAP MessageBox Interface (SMS,Schedule,Call)
//
int samsungtv_soap(unsigned char ip[], int net_socket,char requestbody[]) {
 
    char request[3072];
    char buffer[4096];
 
    strcpy(request, "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
                    "<s:Body>"
                    "   <u:AddMessage xmlns:u=\"urn:samsung.com:service:MessageBoxService:1\">"
                    "    <MessageType>text/xml</MessageType>"
                    "    <MessageID>anything</MessageID>"
                    "<Message>");
    strcat(request,requestbody);
    strcat(request, "</Message>"
                    "   </u:AddMessage>"
                    " </s:Body>"
                    "</s:Envelope>");
    sprintf(buffer, "POST /PMR/control/MessageBoxService HTTP/1.0\r\n"
                    "Content-Type: text/xml; charset=\"utf-8\"\r\n"
                    "HOST: %s\r\n"
                    "Content-Length: %d\r\n"
                    "SOAPACTION: \"uuid:samsung.com:service:MessageBoxService:1#AddMessage\"\r\n"
                    "Connection: close\r\n"
                    "\r\n", ip, strlen(request));
    strcat(buffer,request);
    
    // Send and receive
    int numbytes;
    if ((numbytes = send(net_socket,buffer,strlen(buffer), 0)) == -1) { }
    if ((numbytes = recv(net_socket,buffer,10000,0)) == -1) { }
}



int samsungtv_sms(char ip[], int net_socket, char date[], char time[], char from[], char fromnumber[], char to[], char tonumber[], char message[]) {
    char request[3072];
    sprintf(request,"&lt;Category&gt;SMS&lt;/Category&gt;"
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
    sprintf(request,"&lt;Category&gt;Schedule Reminder&lt;/Category&gt;"
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
    sprintf(request,"&lt;Category&gt;Incoming Call&lt;/Category&gt;"
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




/* 

iSamsungTV Command Line Interface

v0.1 MessageBox Interface (SMS,Schedule,Call)
v0.4

*/

int intro() {
    printf("iSamsungTV 'Square Eye'\nVersion 1.04 copyright (c) 2013-2016 Tristan (@monkeycat.nl)\n");
    printf("Remote command line interface for Samsung Smart TV\nSamsung TV Series C, D, E, F and Blue Ray Disc Players with Smart Hub feature\n\n");
}

int keys() {
    printf("Key Codes\n\n");
    printf("KEY_0\n");
    printf("KEY_1\n");
    printf("KEY_2\n");
    printf("KEY_3\n");
    printf("KEY_4\n");
    printf("KEY_5\n");
    printf("KEY_6\n");
    printf("KEY_7\n");
    printf("KEY_8\n");
    printf("KEY_9\n");
    printf("KEY_11\n");
    printf("KEY_12\n");
    printf("KEY_3SPEED\n");
    printf("KEY_4_3\n");
    printf("KEY_16_9\n");
    printf("KEY_AD\n");
    printf("KEY_ADDDEL\n");
    printf("KEY_ALT_MHP\n");
    printf("KEY_ANGLE\n");
    printf("KEY_ANTENA\n");
    printf("KEY_ANYNET\n");
    printf("KEY_ANYVIEW\n");
    printf("KEY_APP_LIST\n");
    printf("KEY_ASPECT\n");
    printf("KEY_AUTO_ARC_ANTENNA_AIR\n");
    printf("KEY_AUTO_ARC_ANTENNA_CABLE\n");
    printf("KEY_AUTO_ARC_ANTENNA_SATELLITE\n");
    printf("KEY_AUTO_ARC_ANYNET_AUTO_START\n");
    printf("KEY_AUTO_ARC_ANYNET_MODE_OK\n");
    printf("KEY_AUTO_ARC_AUTOCOLOR_FAIL\n");
    printf("KEY_AUTO_ARC_AUTOCOLOR_SUCCESS\n");
    printf("KEY_AUTO_ARC_CAPTION_ENG\n");
    printf("KEY_AUTO_ARC_CAPTION_KOR\n");
    printf("KEY_AUTO_ARC_CAPTION_OFF\n");
    printf("KEY_AUTO_ARC_CAPTION_ON\n");
    printf("KEY_AUTO_ARC_C_FORCE_AGING\n");
    printf("KEY_AUTO_ARC_JACK_IDENT\n");
    printf("KEY_AUTO_ARC_LNA_OFF\n");
    printf("KEY_AUTO_ARC_LNA_ON\n");
    printf("KEY_AUTO_ARC_PIP_CH_CHANGE\n");
    printf("KEY_AUTO_ARC_PIP_DOUBLE\n");
    printf("KEY_AUTO_ARC_PIP_LARGE\n");
    printf("KEY_AUTO_ARC_PIP_LEFT_BOTTOM\n");
    printf("KEY_AUTO_ARC_PIP_LEFT_TOP\n");
    printf("KEY_AUTO_ARC_PIP_RIGHT_BOTTOM\n");
    printf("KEY_AUTO_ARC_PIP_RIGHT_TOP\n");
    printf("KEY_AUTO_ARC_PIP_SMALL\n");
    printf("KEY_AUTO_ARC_PIP_SOURCE_CHANGE\n");
    printf("KEY_AUTO_ARC_PIP_WIDE\n");
    printf("KEY_AUTO_ARC_RESET\n");
    printf("KEY_AUTO_ARC_USBJACK_INSPECT\n");
    printf("KEY_AUTO_FORMAT\n");
    printf("KEY_AUTO_PROGRAM\n");
    printf("KEY_AV1\n");
    printf("KEY_AV2\n");
    printf("KEY_AV3\n");
    printf("KEY_BACK_MHP\n");
    printf("KEY_BOOKMARK\n");
    printf("KEY_CALLER_ID\n");
    printf("KEY_CAPTION\n");
    printf("KEY_CATV_MODE\n");
    printf("KEY_CHDOWN\n");
    printf("KEY_CHUP\n");
    printf("KEY_CH_LIST\n");
    printf("KEY_CLEAR\n");
    printf("KEY_CLOCK_DISPLAY\n");
    printf("KEY_COMPONENT1\n");
    printf("KEY_COMPONENT2\n");
    printf("KEY_CONTENTS\n");
    printf("KEY_CONVERGENCE\n");
    printf("KEY_CONVERT_AUDIO_MAINSUB\n");
    printf("KEY_CUSTOM\n");
    printf("KEY_CYAN\n");
    printf("KEY_DEVICE_CONNECT\n");
    printf("KEY_DISC_MENU\n");
    printf("KEY_DMA\n");
    printf("KEY_DNET\n");
    printf("KEY_DNIe\n");
    printf("KEY_DNSe\n");
    printf("KEY_DOOR\n");
    printf("KEY_DOWN\n");
    printf("KEY_DSS_MODE\n");
    printf("KEY_DTV\n");
    printf("KEY_DTV_LINK\n");
    printf("KEY_DTV_SIGNAL\n");
    printf("KEY_DVD_MODE\n");
    printf("KEY_DVI\n");
    printf("KEY_DVR\n");
    printf("KEY_DVR_MENU\n");
    printf("KEY_DYNAMIC\n");
    printf("KEY_ENTER\n");
    printf("KEY_ENTERTAINMENT\n");
    printf("KEY_ESAVING\n");
    printf("KEY_EXIT\n");
    printf("KEY_EXT1\n");
    printf("KEY_EXT2\n");
    printf("KEY_EXT3\n");
    printf("KEY_EXT4\n");
    printf("KEY_EXT5\n");
    printf("KEY_EXT6\n");
    printf("KEY_EXT7\n");
    printf("KEY_EXT8\n");
    printf("KEY_EXT9\n");
    printf("KEY_EXT10\n");
    printf("KEY_EXT11\n");
    printf("KEY_EXT12\n");
    printf("KEY_EXT13\n");
    printf("KEY_EXT14\n");
    printf("KEY_EXT15\n");
    printf("KEY_EXT16\n");
    printf("KEY_EXT17\n");
    printf("KEY_EXT18\n");
    printf("KEY_EXT19\n");
    printf("KEY_EXT20\n");
    printf("KEY_EXT21\n");
    printf("KEY_EXT22\n");
    printf("KEY_EXT23\n");
    printf("KEY_EXT24\n");
    printf("KEY_EXT25\n");
    printf("KEY_EXT26\n");
    printf("KEY_EXT27\n");
    printf("KEY_EXT28\n");
    printf("KEY_EXT29\n");
    printf("KEY_EXT30\n");
    printf("KEY_EXT31\n");
    printf("KEY_EXT32\n");
    printf("KEY_EXT33\n");
    printf("KEY_EXT34\n");
    printf("KEY_EXT35\n");
    printf("KEY_EXT36\n");
    printf("KEY_EXT37\n");
    printf("KEY_EXT38\n");
    printf("KEY_EXT39\n");
    printf("KEY_EXT40\n");
    printf("KEY_EXT41\n");
    printf("KEY_FACTORY\n");
    printf("KEY_FAVCH\n");
    printf("KEY_FF\n");
    printf("KEY_FF_\n");
    printf("KEY_FM_RADIO\n");
    printf("KEY_GAME\n");
    printf("KEY_GREEN\n");
    printf("KEY_GUIDE\n");
    printf("KEY_HDMI\n");
    printf("KEY_HDMI1\n");
    printf("KEY_HDMI2\n");
    printf("KEY_HDMI3\n");
    printf("KEY_HDMI4\n");
    printf("KEY_HELP\n");
    printf("KEY_HOME\n");
    printf("KEY_ID_INPUT\n");
    printf("KEY_ID_SETUP\n");
    printf("KEY_INFO\n");
    printf("KEY_INSTANT_REPLAY\n");
    printf("KEY_LEFT\n");
    printf("KEY_LINK\n");
    printf("KEY_LIVE\n");
    printf("KEY_MAGIC_BRIGHT\n");
    printf("KEY_MAGIC_CHANNEL\n");
    printf("KEY_MDC\n");
    printf("KEY_MENU\n");
    printf("KEY_MIC\n");
    printf("KEY_MORE\n");
    printf("KEY_MOVIE1\n");
    printf("KEY_MS\n");
    printf("KEY_MTS\n");
    printf("KEY_MUTE\n");
    printf("KEY_NINE_SEPERATE\n");
    printf("KEY_OPEN\n");
    printf("KEY_PANNEL_CHDOWN\n");
    printf("KEY_PANNEL_CHUP\n");
    printf("KEY_PANNEL_ENTER\n");
    printf("KEY_PANNEL_MENU\n");
    printf("KEY_PANNEL_POWER\n");
    printf("KEY_PANNEL_SOURCE\n");
    printf("KEY_PANNEL_VOLDOW\n");
    printf("KEY_PANNEL_VOLUP\n");
    printf("KEY_PANORAMA\n");
    printf("KEY_PAUSE\n");
    printf("KEY_PCMODE\n");
    printf("KEY_PERPECT_FOCUS\n");
    printf("KEY_PICTURE_SIZE\n");
    printf("KEY_PIP_CHDOWN\n");
    printf("KEY_PIP_CHUP\n");
    printf("KEY_PIP_ONOFF\n");
    printf("KEY_PIP_SCAN\n");
    printf("KEY_PIP_SIZE\n");
    printf("KEY_PIP_SWAP\n");
    printf("KEY_PLAY\n");
    printf("KEY_PLUS100\n");
    printf("KEY_PMODE\n");
    printf("KEY_POWER\n");
    printf("KEY_POWEROFF\n");
    printf("KEY_POWERON\n");
    printf("KEY_PRECH\n");
    printf("KEY_PRINT\n");
    printf("KEY_PROGRAM\n");
    printf("KEY_QUICK_REPLAY\n");
    printf("KEY_REC\n");
    printf("KEY_RED\n");
    printf("KEY_REPEAT\n");
    printf("KEY_RESERVED1\n");
    printf("KEY_RETURN\n");
    printf("KEY_REWIND\n");
    printf("KEY_REWIND_\n");
    printf("KEY_RIGHT\n");
    printf("KEY_RSS\n");
    printf("KEY_RSURF\n");
    printf("KEY_SCALE\n");
    printf("KEY_SEFFECT\n");
    printf("KEY_SETUP_CLOCK_TIMER\n");
    printf("KEY_SLEEP\n");
    printf("KEY_SOUND_MODE\n");
    printf("KEY_SOURCE\n");
    printf("KEY_SRS\n");
    printf("KEY_STANDARD\n");
    printf("KEY_STB_MODE\n");
    printf("KEY_STILL_PICTURE\n");
    printf("KEY_STOP\n");
    printf("KEY_SUB_TITLE\n");
    printf("KEY_SVIDEO1\n");
    printf("KEY_SVIDEO2\n");
    printf("KEY_SVIDEO3\n");
    printf("KEY_TOOLS\n");
    printf("KEY_TOPMENU\n");
    printf("KEY_TTX_MIX\n");
    printf("KEY_TTX_SUBFACE\n");
    printf("KEY_TURBO\n");
    printf("KEY_TV\n");
    printf("KEY_TV_MODE\n");
    printf("KEY_UP\n");
    printf("KEY_VCHIP\n");
    printf("KEY_VCR_MODE\n");
    printf("KEY_VOLDOWN\n");
    printf("KEY_VOLUP\n");
    printf("KEY_WHEEL_LEFT\n");
    printf("KEY_WHEEL_RIGHT\n");
    printf("KEY_W_LINK\n");
    printf("KEY_YELLOW\n");
    printf("KEY_ZOOM1\n");
    printf("KEY_ZOOM2\n");
    printf("KEY_ZOOM_IN\n");
    printf("KEY_ZOOM_MOVE\n");
    printf("KEY_ZOOM_OUT\n");
}

int usage() {
    printf("Usage: iSamsungTV (SERIE) IP -COMMAND\n\n");
    printf("Argument: SERIE\n  The TV model series B, C, D, E or F are available.\n  If SERIES is ommited, it assumes a series C or D model TV or Blue Ray Disc Players\n\n");
    printf("Argument: COMMAND\n  The following commands are available KEY, TEXT, CALL, SMS or SCHEDULE\n\n");

    printf("COMMAND: KEY\n");
    printf("Usage:   iSamsungTV (SERIE) IP -KEY KEY\n");
    printf("Example: iSamsungTV E 10.0.0.13 -KEY KEY_VOLUP\n         (Simulates Volume Up remote button press on series E TV located on the network on ip 10.0.0.13)\n");
    printf("         For a list of keys run: iSamsungTV KEYS\n\n");

    printf("COMMAND: TEXT\n");
    printf("Usage:   iSamsungTV (SERIE) IP -TEXT TEXT\n");
    printf("Example: iSamsungTV D 10.0.0.11 -TEXT \"Colour Haze\"\n         (Sends text to YouTube... on a series D TV)\n\n");

    printf("COMMAND: CALL\n");
    printf("Usage:   iSamsungTV (SERIE) IP -CALL DATE TIME FROM NUMBER TO NUMBER\n");
    printf("Example: iSamsungTV 10.0.0.11 -CALL 23:06:01 Cris +555-4323 \"\" \"\"\n         (Show incomming call, skips input with empty strings)\n\n");
    
    printf("COMMAND: SMS\n");
    printf("Usage:   iSamsungTV (SERIE) IP -SMS DATE TIME FROM NUMBER TO NUMBER MESSAGE\n");
    printf("Example: iSamsungTV 10.0.0.13 -SMS 2013-6-24 \"7:01:01 PM\" Cris +555-4323 Me +555-2343 \"Get Off The Couch!?\"\n         (Show incomming SMS)\n\n");
    
    printf("COMMAND: SCHEDULE\n");
    printf("Usage:   iSamsungTV (SERIE) IP -SCHEDULE SUBJECT STARTDATE STARTTIME ENDDATE ENDTIME LOCATION OWNER NUMBER MESSAGE\n\n");
    
    printf("\niSamsungTVPopup.sh bash script for notification messages on the big screen\n\n#!/bin/bash\niSamsungTV $1 -SMS \"\" \"\" \"\" \"\" \"\" \"\" $2\nsleep 0.3\niSamsungTV $1 -KEY KEY_ENTER\nsleep 3\niSamsungTV $1 -KEY KEY_ENTER\n\nUsage: iSamsungTVPopup.sh 10.0.0.2 \"Pop Says the message on the silver screen\"\n\n");
}

int main(int argc, char *argv[]) {
 
    struct addrinfo hints, *res, *p;
    int net_status, net_socket;
    enum modes mode = eUnknown;
 
    // Display keys
    if (argc == 2) {
        if (strcasecmp(argv[1],"KEYS") == 0) {
            intro();
            keys();
            return 1;
        }
     }
    
    // Model series argument
    int sa = 0;
    if (argc > 2) {
        if (strcmp(argv[1],"B") == 0 || strcmp(argv[1],"C") == 0 || strcmp(argv[1],"D") == 0 || strcmp(argv[1],"E") == 0 || strcmp(argv[1],"F") == 0) {
            sa = 1;
            series = argv[1][0];
        }
    }

    // Which command to execute?
    // If this does not work on windows replace strcasecmp with stricmp, or in worst case strcmp
    if (argc > 2+sa) {
        if (strcasecmp(argv[2+sa],"-TEXT") == 0) mode = eText;
        if (strcasecmp(argv[2+sa],"-KEY") == 0) mode = eKey;
        if (strcasecmp(argv[2+sa],"-SMS") == 0) mode = eSMS;
        if (strcasecmp(argv[2+sa],"-CALL") == 0) mode = eCall;
        if (strcasecmp(argv[2+sa],"-SCHEDULE") == 0) mode = eSchedule;
    }
    
    // No commands... display help
    if (argc < 4+sa || mode == eUnknown || !(mode == eText && argc == 4+sa) && !(mode == eKey && argc == 4+sa) && !(mode == eSMS && argc == 10+sa) && !(mode == eCall && argc == 9+sa) && !(mode == eSchedule && argc == 12+sa)) {
        intro();
        usage();
        return 1;
    }
 

    // Choose right port for the right series
    // Port changes throughout the series increments

    char port_remote_cdef[] = "55000";
    char port_upnp_cd[]     = "52235";
    char port_upnp_ef[]     = "7676";
    char *port = port_remote_cdef;

    if (mode != eKey && mode != eText) {
        if (series == 'E' || series == 'F') port = port_upnp_ef;
        else port = port_upnp_cd;
    }
    
    
    // Connect!
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;
    
    // Need to set timeout here for network failure
    
    if ((net_status = getaddrinfo(argv[1+sa],port, &hints, &res)) != 0) {
        fprintf(stderr, "iSamsungTV: Connection Failure: (%s)\n", gai_strerror(net_status));
        return 2;
    }
    
    net_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    // ??? setsockopt(net_socket, SOL_SOCKET, SO_RCVTIMEO, 3 ,1);
    
    if ((net_status = connect(net_socket, res->ai_addr, res->ai_addrlen)) != 0) {
        fprintf(stderr, "iSamsungTV: Connected Failure (%s)\n", gai_strerror(net_status));
        return 2;
    }
   
    // Process protocol
    if (mode == eKey || mode == eText) {
        int auth_status = samsungtv_authenticate(argv[1+sa],net_socket);
        if (auth_status == 0) {
           int response;
           if (mode == eText) response = samsungtv_text(argv[3+sa],net_socket);
           else response = samsungtv_key(argv[3+sa],net_socket);
           if (response != 0 && response != 4) fprintf(stderr, "iSamsungTV: Bug!\n");
        }
        else {
            if (auth_status == -1) fprintf(stderr, "iSamsungTV: Bug!\n");
            if (auth_status == 1) fprintf(stderr, "iSamsungTV: Access Denied\n");
            if (auth_status == 2) fprintf(stderr, "iSamsungTV: Waiting On User\n");
            if (auth_status == 3) fprintf(stderr, "iSamsungTV: Time Out\n");
        }
    }
    else {
        if (mode == eSMS) samsungtv_sms(argv[1+sa],net_socket,argv[3+sa],argv[4+sa],argv[5+sa],argv[6+sa],argv[7+sa],argv[8+sa],argv[9+sa]);
        if (mode == eCall) samsungtv_call(argv[1+sa],net_socket,argv[3+sa],argv[4+sa],argv[5+sa],argv[6+sa],argv[7+sa],argv[8+sa]);
        if (mode == eSchedule) samsungtv_schedule(argv[1+sa],net_socket,argv[3+sa],argv[4+sa],argv[5+sa],argv[6+sa],argv[7+sa],argv[8+sa],argv[9+sa],argv[10+sa],argv[11+sa]);
    }
    
    // Finished
    close(net_socket);
    freeaddrinfo(res);
    return 0;
}