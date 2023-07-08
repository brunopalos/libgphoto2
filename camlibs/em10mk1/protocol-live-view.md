# Live View Protocol

## Start & stop streaming

Start streaming to UDP port 23333 using Olympus protocol

curl 'http://192.168.0.10/exec_takemisc.cgi?com=startliveview&port=23333'

Stop streaming  

curl "http://192.168.0.10/exec_takemisc.cgi?com=stopliveview

## Packet Format of Liveview

```
            _________________________________________________________________________           _______________________________________________
UDP Packet | H |      RTP Packet           | ...     | H |      RTP Packet           | ...     | H |      RTP Packet           | ...
           |___|___________________________|_________|___|___________________________|         |___|___________________________|_______________
RTP Packet     | H | E | JPEG subframe 1-1 | ...         | H | JPEG subframe 1-N     | ...         | H | E | JPEG subframe 2-1 | ...
               |___|___|___________________|_____________|___|_______________________|             |___|___|___________________|_______________
Live View              |                       JPEG Frame 1                          |                     |       JPEG Frame 2        | ...
                       |_____________________________________________________________|                     |___________________________|_______
```

H - Protocol Header
E - Extension Header only on first JPEG subframe

## RTP Packet
```                                                                                            
 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
|-----------------------------------------------------------------------------------------------| _
|V    |P |X | CC       |M  |PT (7 bits)         | Sequence number (16 bits)                     |  |
|-----------------------------------------------------------------------------------------------|  |
| Timestamp (32 bits)                                                                           |  |
|-----------------------------------------------------------------------------------------------|  |
| SSRC Identifier (32 bits)                                                                     |   > Header
|-----------------------------------------------------------------------------------------------|  |
| Extension Header, when X is 1 (32 bits)                                                       |  |
|-----------------------------------------------------------------------------------------------|  |
| ...                                                                                           | _|
|-----------------------------------------------------------------------------------------------| _
| JPEG subframe 1 (Starts with FFD8) (32 bits)                                                  |  |
|-----------------------------------------------------------------------------------------------|  |
| ....                                                                                          |   > Payload
|-----------------------------------------------------------------------------------------------|  |
| JPEG subframe N (Ends with FFD9) (32 bits)                                                    | _|
|-----------------------------------------------------------------------------------------------|
```

### Header

V (2 bits) - Version. Current version is 2.
P (1 bit) - Padding. Always 0.
X (1 bit) - Extension. Indicates presence of an extension header between the header and payload data with 1, 0 otherwise. First packet is 1.
CC (4 bits)- CSRC count. Always 0.
M (1 bit) - Marker. Last packet of JPEG payload is 1, 0 otherwise.
PT (1 bit)- Always 96.
Seq number (16 bits) - Sequence number (16 bits). Starts with 0 and increments 1 for each RTP frame.
Timestamp (32 bits) -
SSRC (32 bits) - Synchronization source identifier uniquely identifies the source of a stream. Must be unique in each RTP session.

### Payload

JPEG subframe (32 bits) - The first JPEG sub-frame starts with ‘FFD8’, and the last JPEG sub-frame ends with ‘FFD9’.

### Sources

* https://en.wikipedia.org/wiki/Real-time_Transport_Protocol
* https://dl-support.olympus-imaging.com/opc/en/
* https://github.com/misaka4e21/olympus-liveview
