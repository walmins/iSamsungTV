# iSamsungTV: Remote Command Line Interface

## Introduction
iSamsungTV is an interface to the Samsung TV series C, D, E and F and Blue Ray Disc Players with Smart Hub feature. It uses the network capabilities of the TV to mimic the remote and send notification messages to the big screen!?

__Please post any working model in the issues__

Contact <tristan@monkeycat.nl>!

### It features!
 * Used for bridging and scripting in smarthome controller software ! 
 * Command Line 
 * Popup messages! (C & D only)
 * Youtube Search (? don't think we need it, but ok :-)

### What it can't do?
 * Read the current TV source
 * Read the current Volume
 * Get the TV out of Stand-by (possible with CEC)

### Other nice stuff!!!

 * [iBrew](https://github.com/Tristan79/iBrew) iKettle, iKettle 2.0 and Smarter Coffee Interface
 * [Medisana Scale](https://github.com/keptenkurk/BS440) with [Domoticz](http://domoticz.com/) bridge to Medisana BS440, BS430,... weight scales.
 * [Xiaomi Mi Plant Sensor](https://github.com/Tristan79/miflora) with [Domoticz](http://domoticz.com/) bridge
 * [HUE Hack](https://github.com/Tristan79/HUEHack) Soft Link Button for HUE Bridge (& rooting the bridge & enable WiFi!)
 * [Vento](https://github.com/Tristan79/Vento)  The itho, duco, orcon, zehnder, storkair: arduino [mysensors 2.0](https://www.mysensors.org) controller!!!
 
## Download

You can download a precompiled binary for [macOS](https://github.com/Tristan79/iSamsungTV/raw/master/macOS/iSamsungTV) or the [Raspberry Pi](https://github.com/Tristan79/iSamsungTV/raw/master/pi/iSamsungTV) using a terminal

### MacOS

```
curl -L https://github.com/Tristan79/iSamsungTV/raw/master/macOS/iSamsungTV > iSamsungTV
chmod +x iSamsungTV
mv iSamsungTV /usr/local/bin/
```

### Raspberry Pi

```
wget https://github.com/Tristan79/iSamsungTV/raw/master/pi/iSamsungTV
chmod +x iSamsungTV
mv iSamsungTV /usr/local/bin/
```

## Source

You can download and unpack the [source](https://github.com/Tristan79/iSamsungTV/archive/master.zip) or download it from github using [Github Desktop](https://desktop.github.com) or manually with ```git clone https://github.com/Tristan79/iSamsungTV.git``` and run ```make``` inside the iSamsungTV folder to compile it and 
```mv iSamsungTV /usr/local/bin/``` it to the right location.

## Usage 

```
Usage: iSamsungTV (SERIE) IP -COMMAND
```

#### Argument: SERIE
```
  The TV model series C, D, E or F are available.
  If SERIES is ommited, it assumes a series C or D model TV or Blue Ray Disc Players
```

#### Argument: COMMAND
```
  The following commands are available KEY, TEXT, CALL, SMS or SCHEDULE
```

#### COMMAND: KEY
```
Usage:   iSamsungTV (SERIE) IP -KEY KEY
Example: iSamsungTV E 10.0.0.13 -KEY KEY_VOLUP
         (Simulates Volume Up remote button press on series E TV located on the network on ip 10.0.0.13)
```

__Keys__

To get a list of usable keys, run ```iSamsungTV keys```

#### COMMAND: TEXT
```
Usage:   iSamsungTV (SERIE) IP -TEXT TEXT
Example: iSamsungTV D 10.0.0.11 -TEXT "Colour Haze"
         (Sends text to YouTube... on a series D TV)
```

#### COMMAND: CALL
```
Usage:   iSamsungTV (SERIE) IP -CALL DATE TIME FROM NUMBER TO NUMBER
Example: iSamsungTV 10.0.0.11 -CALL 2013-6-24 23:06:01 Cris +555-4323 "" ""
         (Show incomming call, skips input with empty strings)
```

#### COMMAND: SMS
```
Usage:   iSamsungTV (SERIE) IP -SMS DATE TIME FROM NUMBER TO NUMBER MESSAGE
Example: iSamsungTV 10.0.0.13 -SMS 2013-6-24 "7:01:01 PM" Cris +555-4323 Me +555-2343 "Get Off The Couch!?"
         (Show incomming SMS)
```

#### COMMAND: SCHEDULE
```
Usage:   iSamsungTV (SERIE) IP -SCHEDULE SUBJECT STARTDATE STARTTIME ENDDATE ENDTIME LOCATION OWNER NUMBER MESSAGE
```


### [iSamsungTVPopup.sh](https://github.com/Tristan79/iSamsungTV/raw/master/iSamsungTVPopup.sh)

It lets you send notification popup messages to your screen (D & E only). 
 
```
iSamsungTVPopup.sh 10.0.0.2 "Pop says the message on the big screen!" 
```

_...using the sms function with a little bit of extra scripting..._


## Development

Found interesting links about SOAP and DLNA... Reading the TV settings!? [Link](http://sc0ty.pl/tag/rendering-control/) [Link](https://wiki.samygo.tv/index.php5/Media_Play_and_DLNA) [Link](http://upnp.org/specs/av/UPnP-av-RenderingControl-v1-Service.pdf)


### SOAP

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
* v1.03 Push The Button, Sweet ([github](https://github.com/Tristan79/iSamsungTV) publish release)
* v1.04 Added support E & F series

## License

Copyright (c) 2016, Tristan Crispijn
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. You may not use source, binary forms or derivative work, with or without modification, for commercial purposes. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. ENJOY!
