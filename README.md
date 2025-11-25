This repo contains two implementations described in ITU G.711.

* A uLaw CODEC
* A packet loss concealment (PLC) utility

CODEC Overview
==============

This is a bare-bones implementation of an ITU G.711 uLaw 
encoder/decoder. This can be used in embedded implementations.

The encode function converts a 16-bit PCM value to an 8-bit value. The
decode function goes the other way. 

The A-Law CODEC has not been implemented yet.

## References

* [Summary of the CODEC](https://en.wikipedia.org/wiki/G.711)
* [Official ITU standard document](https://www.itu.int/rec/dologin_pub.asp?lang=e&id=T-REC-G.711-198811-I!!PDF-E&type=items)
* [Online file converter](https://g711.org/) that understands
G.711 files.

Packet Loss Concealment Utility (PLC)
=====================================

This is more complicated than the CODEC. Packet Loss Concealment
systems are intended to
improve the quality of digital/packetized voice applications by
"smoothing over" playout gaps created by missing/late/out-of-order
network packets. The G.711 standard is a fairly simple
time-domain algorithm.

The implementation is hard-coded to work on 16-bit PCM audio 
sampled at 8 kHz with a 10ms (80 sample) frame size. This is
easily used in 20ms frame systems by calling the goodFrame()
and badFrame() functions twice for each 20ms audio frame.

The picture below illustrates the basic idea. The red trace is the 
input signal with a 20ms gap in the middle. The blue trace is
the interpolated output. Notice that the blue trace matches
the frequency and phase of the red trace even during the drop-out.
Obviously this is a contrived example and things are more 
complicated using real voice.

The blue trace lags the red trace by about 4ms. This is the
inherent delay of the G.711 PLC algorithm.

![PLC1](plc1.jpg)

* [Official ITU Standard Document](https://www.itu.int/rec/dologin_pub.asp?lang=e&id=T-REC-G.711-199909-I!AppI!PDF-E&type=items)


