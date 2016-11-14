# iSamsungTV: Remote Command Line Interface


## Introduction
iBrew is an interface to the Samsung TV C and D Series
It uses the network capabilities of the TV to mimic the remote.
 
### It features!
 * Used for bridging and scripting in smarthome controller software ! 
 * Command Line 
 * Popup messages! 
 * Youtube Search (? don't think we need it, but ok :-)

### What it can't do?
 * Read the current TV source
 * Read the current Volume
 * Get the TV out of Stand-by
 
## Usage 

### Key Codes

The Key Codes can be found [here!]()
 
### iSamsungTVPopup.sh

It also lets you send popup messages to your screen. 
 
```
iSamsungTVPopup.sh 10.0.0.2 "Pop says the message on the big screen!" 
```

_Using the sms function with a little bit of extra scripting_


## Download

You can download a precompiled binary for [macOS]() or the [Raspberry Pi]()

### From source

```
git clone ...
cd iSamsungTV
make
```

## Future Update

Found interesting links about SOAP and DLNA... Reading the TV settings!?

[1](http://sc0ty.pl/tag/rendering-control/)
[2](https://wiki.samygo.tv/index.php5/Media_Play_and_DLNA)
[3](http://upnp.org/specs/av/UPnP-av-RenderingControl-v1-Service.pdf)


### SOAP

The SOAP ports are 52235 & 55000.

I found some other SOAP stuff which I have not tried yet... and there is probably others where you can read the Source input and the Volume Level... its up to you to expand this!

```
/* Future!!!! GetVolume/GetMute... && Trying to Play a avi.... 
POST /upnp/control/RenderingControl1 HTTP/1.1
Host: 192.168.0.10:52235
SOAPAction: "urn:schemas-upnp-org:service:RenderingControl:1#SetMute"
Accept-Language: LC-ctypes=en-us;q=1, LC-ctype=en;q=0.5
Content-Type: text/xml; charset=utf-8
Content-Length: 335
 
<?xml version="1.0"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
<s:Body><u:SetMute xmlns:u="urn:schemas-upnp-org:service:RenderingControl:1">
<InstanceID>0</InstanceID>
<Channel>Master</Channel>
<DesiredMute>0</DesiredMute></u:SetMute>
</s:Body>
</s:Envelope>
 
POST /upnp/control/AVTransport1 HTTP/1.1
SOAPACTION: urn:schemas-upnp-org:service:AVTransport:1#SetAVTransportURI
Connection: close
Content-Length: 619
Content-Type: text/xml
Host: 192.168.1.51:52235
User-Agent: HttpSamyGO/1.1
 
<?xml version='1.0' encoding='UTF-8' standalone='no' ?><s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"><s:Body><u:SetAVTransportURI xmlns:u="urn:schemas-upnp-org:service:AVTransport:1"><InstanceID>0</InstanceID><CurrentURI>http://75.101.165.227:8080/app/iLJy+VD9xyYqv5jtERGBijAeiqUmYWqCFzy4Li6gM0uMzI8pYoRWTxqp+UxEy14ibHGOrLpqJTkjI+WE6Q6lbQ6e2+1X96ToH8lGCv0f4f88M0jxU6S6z4SwC8KOCoMhscRxjOiy4CJVzNNeCGQxpw==.mp4</CurrentURI><CurrentURIMetaData>&lt;DIDL-Lite&gt;&lt;/DIDL-Lite&gt;</CurrentURIMetaData></u:SetAVTransportURI></s:Body></s:Envelope>
 
*/
```

### Versions
* v0.00 Brainstorm!
* v0.01 Samygo & [Remote](https://forum.samygo.tv/viewtopic.php?t=5794)
* v0.02 Domoticz & [SOAP](https://www.domoticz.com/wiki/Samsung_TV)
* v1.03 Push The Button, Sweet ([github]() publish release)

## License

Copyright (c) 2016, Tristan Crispijn
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. You may not use source, binary forms or derivative work, with or without modification, for commercial purposes. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. ENJOY!