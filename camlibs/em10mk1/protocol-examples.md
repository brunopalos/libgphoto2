# Olympus EM-10 mk1

## URL examples

Note that some of these were not tested and not used in this library. They are just listed for reference.

http://192.168.0.10/clear_resvflg.cgi
http://192.168.0.10/exec_pwoff.cgi
http://192.168.0.10/exec_shutter.cgi?com=1st2ndpush
http://192.168.0.10/exec_shutter.cgi?com=1stpush
http://192.168.0.10/exec_shutter.cgi?com=1strelease
http://192.168.0.10/exec_shutter.cgi?com=2nd1strelease
http://192.168.0.10/exec_shutter.cgi?com=2ndpush
http://192.168.0.10/exec_shutter.cgi?com=2ndrelease
http://192.168.0.10/exec_takemisc.cgi?com=ctrlzoom&move=
http://192.168.0.10/exec_takemisc.cgi?com=digitalzoomshift
http://192.168.0.10/exec_takemisc.cgi?com=getlastjpg
http://192.168.0.10/exec_takemisc.cgi?com=getrecview
http://192.168.0.10/exec_takemisc.cgi?com=startliveview&port=
http://192.168.0.10/exec_takemisc.cgi?com=stopliveview
http://192.168.0.10/exec_takemisc.cgi?com=supermacroaflock&func=lock
http://192.168.0.10/exec_takemisc.cgi?com=supermacroaflock&func=release
http://192.168.0.10/exec_takemisc.cgi?com=supermacromfinaflock&move=
http://192.168.0.10/exec_takemotion.cgi?com=assignafframe&point=
http://192.168.0.10/exec_takemotion.cgi?com=assignaflock&point=
http://192.168.0.10/exec_takemotion.cgi?com=releaseafframe
http://192.168.0.10/exec_takemotion.cgi?com=releaseaflock
http://192.168.0.10/exec_takemotion.cgi?com=starttake
http://192.168.0.10/exec_takemotion.cgi?com=starttake&exposuremin=
http://192.168.0.10/exec_takemotion.cgi?com=starttake&point=
http://192.168.0.10/exec_takemotion.cgi?com=starttake&upperlimit=200
http://192.168.0.10/exec_takemotion.cgi?com=starttake&upperlimit=200&point=
http://192.168.0.10/exec_takemotion.cgi?com=stoptake
http://192.168.0.10/exec_takemotion.cgi?com=takeready
http://192.168.0.10/exec_takemotion.cgi?com=takeready&point=
http://192.168.0.10/get_caminfo.cgi
http://192.168.0.10/get_camprop.cgi?com=desc&propname=desclist
http://192.168.0.10/get_camprop.cgi?com=get&propname=touchactiveframe
http://192.168.0.10/get_commandlist.cgi
http://192.168.0.10/get_connectmode.cgi
http://192.168.0.10/get_dcffilenum.cgi
http://192.168.0.10/get_gpsdivunit.cgi
http://192.168.0.10/get_imglist.cgi?DIR=
http://192.168.0.10/get_imglist.cgi?DIR=/DCIM
http://192.168.0.10/get_resizeimg.cgi?DIR=
http://192.168.0.10/get_rsvimglist.cgi
http://192.168.0.10/get_thumbnail.cgi?DIR=
http://192.168.0.10/get_unusedcapacity.cgi
http://192.168.0.10/req_attachexifgps.cgi
http://192.168.0.10/req_storegpsinfo.cgi?mode=
http://192.168.0.10/set_camprop.cgi?com=set&propname=artfilter
http://192.168.0.10/set_camprop.cgi?com=set&propname=drivemode
http://192.168.0.10/set_camprop.cgi?com=set&propname=expcomp
http://192.168.0.10/set_camprop.cgi?com=set&propname=focalvalue
http://192.168.0.10/set_camprop.cgi?com=set&propname=isospeedvalue
http://192.168.0.10/set_camprop.cgi?com=set&propname=shutspeedvalue
http://192.168.0.10/set_camprop.cgi?com=set&propname=supermacrosub
http://192.168.0.10/set_camprop.cgi?com=set&propname=takemode
http://192.168.0.10/set_camprop.cgi?com=set&propname=wbvalue
http://192.168.0.10/set_takemode.cgi?com=normal
http://192.168.0.10/set_takemode.cgi?com=selftimer
http://192.168.0.10/set_utctimediff.cgi?utctime=
http://192.168.0.10/switch_cammode.cgi?mode=play
http://192.168.0.10/switch_cammode.cgi?mode=rec&lvqty=
http://192.168.0.10/switch_cammode.cgi?mode=shutter

## Command list

curl http://192.168.0.10/get_commandlist.cgi

```xml
<?xml version="1.0"?>
<oishare>
 <version>2.20</version>
 <support func="web"/>
 <support func="remote"/>
 <support func="gps"/>
 <support func="release"/>
 <cgi name="get_connectmode">
  <http_method type="get"/>
 </cgi>
 <cgi name="switch_cammode">
  <http_method type="get">
   <cmd1 name="mode">
    <param1 name="rec">
     <cmd2 name="lvqty">
      <param2 name="0320x0240"/>
      <param2 name="0640x0480"/>
     </cmd2>
    </param1>
    <param1 name="play"/>
    <param1 name="shutter">
     <cmd2 name="lvqty">
      <param2 name="0320x0240"/>
      <param2 name="0640x0480"/>
     </cmd2>
    </param1>
   </cmd1>
  </http_method>
 </cgi>
 <cgi name="get_caminfo">
  <http_method type="get"/>
 </cgi>
 <cgi name="exec_pwoff">
  <http_method type="get"/>
 </cgi>
 <cgi name="get_resizeimg">
  <http_method type="get">
   <cmd1 name="DIR">
    <param1>
     <cmd2 name="size">
      <param2 name="1024"/>
      <param2 name="1600"/>
      <param2 name="1920"/>
      <param2 name="2048"/>
     </cmd2>
    </param1>
   </cmd1>
  </http_method>
 </cgi>
 <cgi name="get_movplaytime">
  <http_method type="get">
   <cmd1 name="DIR"/>
  </http_method>
 </cgi>
 <cgi name="clear_resvflg">
  <http_method type="get"/>
 </cgi>
 <cgi name="get_rsvimglist">
  <http_method type="get"/>
 </cgi>
 <cgi name="get_imglist">
  <http_method type="get">
   <cmd1 name="DIR"/>
  </http_method>
 </cgi>
 <cgi name="get_thumbnail">
  <http_method type="get">
   <cmd1 name="DIR"/>
  </http_method>
 </cgi>
 <cgi name="exec_takemotion">
  <http_method type="get">
   <cmd1 name="com">
    <param1 name="assignafframe">
     <cmd2 name="point"/>
    </param1>
    <param1 name="releaseafframe"/>
    <param1 name="takeready">
     <cmd2 name="point"/>
    </param1>
    <param1 name="starttake">
     <cmd2 name="point">
      <cmd3 name="exposuremin"/>
      <cmd3 name="upperlimit"/>
     </cmd2>
    </param1>
    <param1 name="stoptake"/>
   </cmd1>
  </http_method>
 </cgi>
 <cgi name="exec_takemisc">
  <http_method type="get">
   <cmd1 name="com">
    <param1 name="startliveview">
     <cmd2 name="port"/>
    </param1>
    <param1 name="stopliveview"/>
    <param1 name="getrecview"/>
    <param1 name="getlastjpg"/>
    <param1 name="ctrlzoom">
     <cmd2 name="move">
      <param2 name="widemove"/>
      <param2 name="telemove"/>
      <param2 name="off"/>
      <param2 name="wideterm"/>
      <param2 name="teleterm"/>
     </cmd2>
    </param1>
   </cmd1>
  </http_method>
 </cgi>
 <cgi name="get_camprop">
  <http_method type="get">
   <cmd1 name="com">
    <param1 name="desc">
     <cmd2 name="propname">
      <param2 name="touchactiveframe"/>
      <param2 name="takemode"/>
      <param2 name="drivemode"/>
      <param2 name="focalvalue"/>
      <param2 name="expcomp"/>
      <param2 name="shutspeedvalue"/>
      <param2 name="isospeedvalue"/>
      <param2 name="wbvalue"/>
      <param2 name="noisereduction"/>
      <param2 name="lowvibtime"/>
      <param2 name="bulbtimelimit"/>
      <param2 name="artfilter"/>
      <param2 name="digitaltelecon"/>
      <param2 name="desclist"/>
     </cmd2>
    </param1>
    <param1 name="get">
     <cmd2 name="propname">
      <param2 name="touchactiveframe"/>
      <param2 name="takemode"/>
      <param2 name="drivemode"/>
      <param2 name="focalvalue"/>
      <param2 name="expcomp"/>
      <param2 name="shutspeedvalue"/>
      <param2 name="isospeedvalue"/>
      <param2 name="wbvalue"/>
      <param2 name="noisereduction"/>
      <param2 name="lowvibtime"/>
      <param2 name="bulbtimelimit"/>
      <param2 name="artfilter"/>
      <param2 name="digitaltelecon"/>
      <param2 name="desclist"/>
     </cmd2>
    </param1>
   </cmd1>
  </http_method>
 </cgi>
 <cgi name="set_camprop">
  <http_method type="post">
   <cmd1 name="com">
    <param1 name="set">
     <cmd2 name="propname">
      <param2 name="takemode"/>
      <param2 name="drivemode"/>
      <param2 name="focalvalue"/>
      <param2 name="expcomp"/>
      <param2 name="shutspeedvalue"/>
      <param2 name="isospeedvalue"/>
      <param2 name="wbvalue"/>
      <param2 name="artfilter"/>
     </cmd2>
    </param1>
   </cmd1>
  </http_method>
 </cgi>
 <cgi name="get_activate">
  <http_method type="get"/>
 </cgi>
 <cgi name="set_utctimediff">
  <http_method type="get">
   <cmd1 name="utctime">
    <cmd2 name="diff"/>
   </cmd1>
  </http_method>
 </cgi>
 <cgi name="get_gpsdivunit">
  <http_method type="get"/>
 </cgi>
 <cgi name="get_unusedcapacity">
  <http_method type="get"/>
 </cgi>
 <cgi name="get_dcffilenum">
  <http_method type="get"/>
 </cgi>
 <cgi name="req_attachexifgps">
  <http_method type="post"/>
 </cgi>
 <cgi name="req_storegpsinfo">
  <http_method type="post">
   <cmd1 name="mode">
    <param1 name="new"/>
    <param1 name="append"/>
    <cmd2 name="date"/>
   </cmd1>
  </http_method>
 </cgi>
 <cgi name="exec_shutter">
  <http_method type="get">
   <cmd1 name="com">
    <param1 name="1stpush"/>
    <param1 name="2ndpush"/>
    <param1 name="1st2ndpush"/>
    <param1 name="2nd1strelease"/>
    <param1 name="2ndrelease"/>
    <param1 name="1strelease"/>
   </cmd1>
  </http_method>
 </cgi>
 <cgi name="exec_shuttermisc">
  <http_method type="get">
   <cmd1 name="com">
    <param1 name="startliveview">
     <cmd2 name="port" />
    </param1>
    <param1 name="stopliveview" />
   </cmd1>
  </http_method>
 </cgi>
</oishare>
```

## Camera properties

Camera mode must be Rec (see bellow)

curl http://192.168.0.10/get_camprop.cgi?com=desc&propname=desclist

```xml
<?xml version="1.0"?>
<desclist>
    <desc>
        <propname>touchactiveframe</propname>
        <attribute>get</attribute>
        <value>0048-0036_0224x0168</value>
    </desc>
    <desc>
        <propname>takemode</propname>
        <attribute>getset</attribute>
        <value>iAuto</value>
        <enum>iAuto P A S M ART</enum>
    </desc>
    <desc>
        <propname>noisereduction</propname>
        <attribute>get</attribute>
        <value>off</value>
        <enum>off on auto</enum>
    </desc>
    <desc>
        <propname>lowvibtime</propname>
        <attribute>get</attribute>
        <value>0</value>
        <enum>0 1 125 250 500 1000 2000 4000 8000 15000 30000</enum>
    </desc>
    <desc>
        <propname>bulbtimelimit</propname>
        <attribute>get</attribute>
        <value>8</value>
        <enum>1 2 4 8 15 20 25 30</enum>
    </desc>
    <desc>
        <propname>digitaltelecon</propname>
        <attribute>get</attribute>
        <value>off</value>
        <enum>on off</enum>
    </desc>
    <desc>
        <propname>drivemode</propname>
        <attribute>getset</attribute>
        <value>normal</value>
        <enum>normal continuous-H continuous-L selftimer customselftimer</enum>
    </desc>
    <desc>
        <propname>focalvalue</propname>
        <attribute>getset</attribute>
        <value>5.6</value>
        <enum>1.0 1.1 1.2 1.4 1.6 1.8 2.0 2.2 2.5 2.8 3.2 3.5 4.0 4.5 5.0 5.6 6.3 7.1 8.0 9.0 10 11 13 14 16 18 20 22 25 29 32 36 40 45 51 57 64 72 81 91</enum>
    </desc>
    <desc>
        <propname>expcomp</propname>
        <attribute>getset</attribute>
        <value>0.0</value>
        <enum>-5.0 -4.7 -4.3 -4.0 -3.7 -3.3 -3.0 -2.7 -2.3 -2.0 -1.7 -1.3 -1.0 -0.7 -0.3 0.0 +0.3 +0.7 +1.0 +1.3 +1.7 +2.0 +2.3 +2.7 +3.0 +3.3 +3.7 +4.0 +4.3 +4.7 +5.0</enum>
    </desc>
    <desc>
        <propname>shutspeedvalue</propname>
        <attribute>getset</attribute>
        <value>250</value>
        <enum>livetime livebulb 60" 50" 40" 30" 25" 20" 15" 13" 10" 8" 6" 5" 4" 3.2" 2.5" 2" 1.6" 1.3" 1" 1.3 1.6 2 2.5 3 4 5 6 8 10 13 15 20 25 30 40 50 60 80 100 125 160 200 250 320 400 500 640 800 1000 1250 1600 2000 2500 3200 4000 5000 6400 8000</enum>
    </desc>
    <desc>
        <propname>isospeedvalue</propname>
        <attribute>getset</attribute>
        <value>Auto</value>
        <enum>Auto Low 200 250 320 400 500 640 800 1000 1250 1600 2000 2500 3200 4000 5000 6400 8000 10000 12800 16000 20000 25600</enum>
    </desc>
    <desc>
        <propname>wbvalue</propname>
        <attribute>getset</attribute>
        <value>0</value>
        <enum>0 18 16 17 20 35 64 23 256 257 258 259 512</enum>
    </desc>
    <desc>
        <propname>artfilter</propname>
        <attribute>getset</attribute>
        <value>ART04</value>
        <enum>ART01 ART02 ART03 ART04 ART05 ART06 ART07 ART08 ART09 ART10 ART11 ART12 ARTBKT</enum>
    </desc>
</desclist>
```

## Getting single property example

Camera mode must be Rec (see bellow)

curl http://192.168.0.10/get_camprop.cgi?com=desc&propname=isospeedvalue

```xml
<?xml version="1.0"?>
<desc>
    <propname>isospeedvalue</propname>
    <attribute>getset</attribute>
    <value>1600</value>
    <enum>Low 200 250 320 400 500 640 800 1000 1250 1600</enum>
</desc>
```

## Setting a camera property example

curl -X POST "http://192.168.0.10/set_camprop.cgi?com=set&propname=artfilter" -d '<set><value>ART04</value></set>'

## List pictures

Camera modes must be Play or Rec (see bellow)

curl http://192.168.0.10/get_imglist.cgi?DIR=/DCIM/100OLYMP

```
VER_100
/DCIM/100OLYMP,P9280001.AVI,950625704,0,20284,16578
/DCIM/100OLYMP,P6130005.JPG,6137845,0,20685,36243
/DCIM/100OLYMP,P6130006.JPG,6311473,0,20685,36245
/DCIM/100OLYMP,P9280004.AVI,1807596552,0,20284,16937
/DCIM/100OLYMP,P6130007.JPG,4535041,0,20685,36248
/DCIM/100OLYMP,P6130008.JPG,6237337,0,20685,36262
/DCIM/100OLYMP,P6130009.JPG,6367587,0,20685,36264
/DCIM/100OLYMP,P6130010.JPG,6261690,0,20685,36273
/DCIM/100OLYMP,P6130011.JPG,6325992,0,20685,41364
/DCIM/100OLYMP,P6130012.JPG,6130712,0,20685,41382
/DCIM/100OLYMP,P6130013.JPG,6144925,0,20685,41398
```
 
## Download picture

Camera modes must be Play or Rec (see bellow)

curl http://192.168.0.10/DCIM/100OLYMP/PICTURE001.JPG

## Take pictures

Camera mode must be Shutter (see bellow)

curl http://192.168.0.10/exec_shutter.cgi?com=1st2ndpush
curl http://192.168.0.10/exec_shutter.cgi?com=1stpush
curl http://192.168.0.10/exec_shutter.cgi?com=1strelease
curl http://192.168.0.10/exec_shutter.cgi?com=2nd1strelease
curl http://192.168.0.10/exec_shutter.cgi?com=2ndpush
curl http://192.168.0.10/exec_shutter.cgi?com=2ndrelease

## Start & stop streaming

Camera mode must be Rec (see bellow)

Start streaming to UDP port 23333 using Olympus protocol

curl 'http://192.168.0.10/exec_takemisc.cgi?com=startliveview&port=23333'

Stop streaming

curl "http://192.168.0.10/exec_takemisc.cgi?com=stopliveview

## Setting camera modes

curl http://192.168.0.10/switch_cammode.cgi?mode=rec&lvqty=
curl http://192.168.0.10/switch_cammode.cgi?mode=play
curl http://192.168.0.10/switch_cammode.cgi?mode=shutter

### Sources

* https://gist.github.com/mangelajo/6fa005ff3544fecdecfa
* https://dl-support.olympus-imaging.com/opc/en/
