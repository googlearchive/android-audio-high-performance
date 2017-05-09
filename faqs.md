---
layout: page
title: Frequently Asked Questions
permalink: /faqs.html
is_site_nav_category: false
site_nav_category: faqs
site_nav_category_order: 4
---

<!--
    Copyright 2015 The Android Open Source Project

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
-->
{::options toc_levels="2"/}

* TOC
{:toc}

## Audio files

#### How do I parse a WAV file?

The <a href="https://en.wikipedia.org/wiki/WAV">WAV</a>
<a href="https://en.wikipedia.org/wiki/Audio_file_format">audio file format</a>
is known for being easy to parse in restricted environments with your
own content, but it can be tricky to correctly implement for unusual
variations, and for arbitrary files found in the wild.

If you are only reading your own files, go ahead and write your own parser
based on the WAV specification.  It will be an interesting exercise.
But if you are going to process a wide assortment of files or files
that you did not create, then consider an open source library.
Be sure to confirm that the license is compatible with your needs.

#### What audio file libraries are there in Android?

The Android platform has several internal implementations of audio file parsers
that aren't exposed as public APIs.  As Android is an open
source platform, you are free to copy such code and import it into your
application, but be aware that the code was not intended for application
use and so may need some modification before use.

One internal Android implementation that is relatively easy to
adapt for application use is
<a href="https://android.googlesource.com/platform/system/media/+/master/audio_utils/tinysndfile.c">tinysndfile.c</a>.
This supports WAV files only and has an Apache 2.0 license.

#### What other open source audio file libraries are there?

See the table below:

<table>

<tr>
<th>Home</th>
<th>Wikipedia</th>
<th>License</th>
</tr>

<tr>
<td><a href="http://www.mega-nerd.com/libsndfile/">libsndfile</a></td>
<td><a href="https://en.wikipedia.org/wiki/Libsndfile">libsndfile</a></td>
<td>LGPL</td>
</tr>

<tr>
<td><a href="http://www.68k.org/~michael/audiofile/">audiofile</a></td>
<td></td>
<td>LGPL</td>
</tr>

<tr>
<td><a href="http://sox.sourceforge.net/libsox.html">libsox</a></td>
<td></td>
<td>LGPL</td>
</tr>

</table>


## Compatibility and CDD

#### What is the CDD?
The Android Compatibility Definition Document (CDD) enumerates the
hardware and software requirements of a compatible Android device. See
<a href="https://source.android.com/compatibility/">Android Compatibility</a>
for more information on the overall compatibility program, and
<a href="https://source.android.com/compatibility/android-cdd.pdf">CDD</a>
for the actual CDD document.

#### What is an Android feature?
An Android <em>feature</em> declares a single hardware or software
characteristic that can be implemented by an Android device, and used by
an app.  Most features are optional, or required only for certain device categories.
An app can declare statically (at build time) that it needs
a particular feature, or check dynamically at runtime for the presence of
a feature and adjust behavior accordingly.
For more information about declaring the use of a feature in your app, see
<a href="http://developer.android.com/guide/topics/manifest/uses-feature-element.html">&lt;uses-feature&gt;</a>.
The complete list of all defined features is at
<a href="http://developer.android.com/reference/android/content/pm/PackageManager.html">PackageManager</a>.


#### What are the features related to audio?
The audio-related features include:

<table>
<tr>
  <th>Symbol</th>
  <th>String</th>
  <th>CDD section</th>
  <th>Notes</th>
</tr>

<tr>
  <td><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_AUDIO_LOW_LATENCY">FEATURE_AUDIO_LOW_LATENCY</a></td>
  <td>android.hardware.audio.low_latency</td>
  <td>5.6. Audio Latency</td>
  <td>original one-way latency</td>
</tr>

<tr>
  <td><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_AUDIO_OUTPUT">FEATURE_AUDIO_OUTPUT</a></td>
  <td>android.hardware.audio.output</td>
  <td>5.5. Audio Playback</td>
  <td></td>
</tr>

<tr>
  <td><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_AUDIO_PRO">FEATURE_AUDIO_PRO</a></td>
  <td>android.hardware.audio.pro</td>
  <td>5.10. Professional Audio</td>
  <td>specifies round-trip latency,<br />among other requirements</td>
</tr>

<tr>
  <td><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_BLUETOOTH_LE">FEATURE_BLUETOOTH_LE</a></td>
  <td>android.hardware.bluetooth_le</td>
  <td>7.4.3. Bluetooth</td>
  <td>prerequisite for MIDI over BLE</td>
</tr>

<tr>
  <td><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_MICROPHONE">FEATURE_MICROPHONE</a></td>
  <td>android.hardware.microphone</td>
  <td>7.8.1. Microphone</td>
  <td></td>
</tr>

<tr>
  <td><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_MIDI">FEATURE_MIDI</a></td>
  <td>android.software.midi</td>
  <td>5.9. Musical Instrument Digital Interface (MIDI)</td>
  <td>prerequisite for android.hardware.audio.pro</td>
</tr>

<tr>
  <td><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_USB_HOST">FEATURE_USB_HOST</a></td>
  <td>android.hardware.usb.host</td>
  <td>7.7. USB</td>
  <td>prerequisite for USB host mode audio and MIDI</td>
</tr>

</table>


Section numbers are for the October 16th, 2015 edition of Android 6.0 CDD.
The requirements specified in the CDD supersede the informal summaries above.


#### Which sections of the CDD pertain to audio?
In addition to the feature-related sections listed above, see:

<ul>
<li>5.1.1. Audio Codecs</li>
<li>5.4. Audio Recording</li>
<li>5.5. Audio Playback</li>
<li>5.6. Audio Latency</li>
<li>5.9. Musical Instrument Digital Interface (MIDI)</li>
<li>5.10. Professional Audio</li>
<li>7.7. USB</li>
<li>7.8. Audio</li>
</ul>

#### What is "pro audio"? Do I have to be a professional musician or audio engineer to care?

"Pro audio" indicates the additional functional and performance
requirements that are commonly associated with audio content creation,
performance, mixing, post production, etc. applications as opposed
to content consumption applications. There are many users besides
professionals who need such functionality and performance, including
amateurs, hobbyists, and semi-pros. The "pro audio" moniker is simply a
convenient label to describe a platform capable of supporting demanding
creative audio apps.


#### Why is round-trip latency specified as only 20 ms or lower? Everyone knows that musicians need 10 ms.

There are important use cases that are enabled by 20 ms. We anticipate
decreasing the requirement in the future, as indicated by the <em>SHOULD</em>
for 10 ms.

#### Why does section 5.6 Audio Latency specify a maximum one-way output latency of 45 ms,
yet section 5.10 Professional Audio specifies a maximum round-trip latency
of 20 ms? That doesn't make sense.

Section 5.6 was written when device latencies were much higher, and
audio output was the focus (for example for games). At that time even
the 45 ms figure was a stretch goal. We still encourage device
OEMs to meet section 5.6 requirements, but for professional audio the
requirements of section 5.10 are more relevant.

#### What is the relationship between the <code>android.hardware.audio.low_latency</code> and <code>android.hardware.audio.pro</code> flags?
Feature <code>android.hardware.audio.low_latency</code> is a prerequisite
for <code>android.hardware.audio.pro</code>. A device can implement
<code>android.hardware.audio.low_latency</code> and not
<code>android.hardware.audio.pro</code>, but not vice-versa.

#### Why is MIDI over BLE transport only a <em>SHOULD</em>?
As of the publication of the CDD, the MIDI over BLE was in draft status. We
anticipate strengthening the requirement after the transport is
standardized, is more widely implemented, and initial interoperability
bugs are worked out.

#### Why is my favorite performance metric omitted from the requirements?
We recognize that there are many other metrics that are relevant,
such as MIDI latency and jitter, touch screen latency and jitter, and
others. Please let us know what you think is missing! By working with
you to establish relevant metrics and automated testing tools for those
metrics, we can move the ecosystem forward.

#### Will the CDD be revised?  Can I offer suggestions?
Yes, the CDD is revised at each major Android release.
We welcome your suggestions by using the <strong>Send Feedback</strong>
button located at the bottom of the
<a href="https://source.android.com/compatibility/">Android Compatibility</a> page.

## Digital audio workstations

#### Is there a standard DAW plug-in interface for Android pro audio?
No.

#### Is there a standard inter-app audio interface for Android pro audio?
No.

## Decoding audio

#### How do I decode audio?
From Java, see the SDK API
<a href="http://developer.android.com/reference/android/media/MediaCodec.html">android.media.MediaCodec</a>.
<br />
From C or C++, see the corresponding NDK API &lt;media/NdkMediaCodec.h&gt;.

#### Where is the documentation for the NDK version of the <code>MediaCodec</code> API?
There is no documentation currently; in the mean time see the
sample code at <a href="https://github.com/googlesamples/android-ndk/tree/master/native-codec">native-codec</a>.

## High-resolution audio

#### How do I use floating-point data with OpenSL ES for Android?
See section <em>Floating-point data</em> of <a href="{{ site.baseurl }}/guides/opensl_es.html">OpenSL ES for Android</a>.

## Latency

#### What is latency?

Latency is the time taken for a signal to travel through a system. Common examples related to audio apps are:

- **Output latency** - Time between an audio sample being generated by an app and the sample being played through the headphone jack or built-in speaker
- **Input latency** - Time between an audio signal being received by a device's audio input, such as the microphone and that same audio data being available to an app
- **Round trip latency** - Input latency + App processing + Output latency
- **MIDI-USB latency** - Time between a MIDI signal being sent by a MIDI controller connected via USB and it arriving to an app
- **MIDI-BT latency** - Time between a MIDI signal being sent by a MIDI controller connected via Bluetooth and it arriving to an app
- **Touch latency** - Time between a user touching the screen and that touch event being received by an app

#### How can I measure round-trip audio latency?
You can measure round-trip audio latency by creating an app which generates an audio signal, listens for that signal and measures the time between sending it and receiving it.

Alternatively, you can install this [latency testing app](https://play.google.com/store/apps/details?id=org.drrickorang.loopback). This performs a round-trip latency test using the [Larsen test](https://source.android.com/devices/audio/latency_measure.html#larsenTest).

Since the lowest latency is achieved over audio paths with minimal signal processing you may also want to use an [Audio Loopback Dongle](https://source.android.com/devices/audio/latency_measure.html#loopback) which will allow the test to be run over the headset connector.

<!-- TODO: Where can these be purchased? -->

#### Why does using the headset result in lower latency?
The speakers and microphones used in mobile devices generally have poor acoustics due to their small size. For this reason, signal processing is added to improve the sound quality. This signal processing introduces latency.

If your app relies on low latency audio you should advise users to use the headset. For example, by displaying a "Best with headphones" screen on first run.

Note that just using the headset doesn't guarantee the lowest possible latency. You may need to perform other steps to remove any unwanted signal processing from the audio path, such as by using the VOICE_RECOGNITION preset when recording.

<!-- TODO: Add link to VOICE_RECOGNITION in OpenSL ES guide and link to recording tips -->

#### What is the lowest possible round-trip audio latency?
The lowest possible round-trip audio latency varies greatly depending on device model and Android build.

To find it out you can use the following techniques:

- Measure it yourself using a latency testing app and loopback dongle
- *For Nexus devices* use the [published measurements](https://source.android.com/devices/audio/latency_measurements.html)

You can also get a rough idea of audio performance by testing whether the device reports support for the [low_latency](http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_AUDIO_LOW_LATENCY) and [pro](http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_AUDIO_PRO) hardware features.

#### What is the lowest possible audio output latency?
It is difficult to test audio output latency in isolation since it requires knowing exactly when the first sample is sent into the audio path ([although this can be done using a light testing circuit and an oscilloscope](https://source.android.com/devices/audio/testing_circuit.html)).

If you know the round-trip audio latency you can use the rough rule of thumb: **Audio output latency is half the round trip audio latency over paths without signal processing**.

#### What is the lowest possible audio input latency?
As with output latency, input latency is difficult to measure in isolation. The best solution at this time is to measure round-trip audio and divide by two.

<!-- TODO: Really? -->

#### How can I find the audio latency of a device at runtime?
There is currently no API to determine audio latency over any path on an Android device at runtime. You can, however, use the following hardware feature flags to find out whether the device makes any guarantees for latency.

- [android.hardware.audio.low_latency](http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_AUDIO_LOW_LATENCY) - indicates a continuous *output* latency of 45 milliseconds or less
- [android.hardware.audio.pro](http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_AUDIO_PRO) - indicates a continuous *round-trip* latency of 20 milliseconds or less

The criteria for reporting these flags is defined in the [Android Compatibility Definition Document](http://static.googleusercontent.com/media/source.android.com/en//compatibility/android-cdd.pdf) under section 5.6 Audio Latency and 5.10 Pro Audio.

Here's how to check for these features in Java:

    boolean claimsLowLatencyFeature = getPackageManager().hasSystemFeature(PackageManager.FEATURE_AUDIO_LOW_LATENCY);
    boolean claimsProFeature = getPackageManager().hasSystemFeature(PackageManager.FEATURE_AUDIO_PRO);

#### Can I use different channel configurations for input and output (e.g. mono input and stereo output)?
Yes.

## Middleware

#### What is middleware?
<a href="https://en.wikipedia.org/wiki/Middleware">Middleware</a>
is application-level software that extends or abstracts the native operating system services.
If it solely abstracts and doesn't add significant functionality, it is called
<a href="https://en.wikipedia.org/wiki/Cross-platform_support_middleware">cross-platform support middleware</a>.

#### What are the tradeoffs involved in deciding whether or not to use middleware?
The major benefits include increased portability and added functionality.
Risks include cost, performance loss due to additional software layers,
and the inability to take advantage of platform-specific features
if these are hidden by the middleware.

#### What audio-related middleware is available for Android?
As of this writing, a partial list of audio middleware with Android support includes:

<ul>
<li><a href="http://www.fmod.org/">FMOD</a></li>
<li><a href="http://www.juce.com/">JUCE</a></li>
<li><a href="https://github.com/google/pindrop">Pindrop</a></li>
<li><a href="http://www.libsdl.org/">SDL</a></li>
<li><a href="http://superpowered.com/">Superpowered</a></li>
</ul>

## MIDI

#### How do I know if my device supports MIDI APIs?
Check the feature flag <code>android.software.midi</code>.

#### I have an earlier version of Android without built-in MIDI APIs. Can I still access USB MIDI peripherals?

Yes, there are several open source implementations, including:
<ul>
<li><a href="https://github.com/kshoji/USB-MIDI-Driver">USB-MIDI-Driver</a></li>
<li><a href="https://github.com/google/music-synthesizer-for-android">music-synthesizer-for-android</a></li>
<li><a href="https://github.com/nettoyeurny/btmidi">btmidi</a></li>
</ul>

#### How do I access MIDI from C or C++?
From an attached thread, use
<a href="http://developer.android.com/training/articles/perf-jni.html">JNI</a>
to call the
<a href="https://developer.android.com/reference/android/media/midi/package-summary.html">android.media.midi</a>
APIs.

#### What MIDI peripherals work with Nexus devices?

*Disclaimer:* The table below lists select MIDI peripherals that we have tested
with Nexus Android devices.  The presence of a peripheral in this
table with OK status does not constitute a recommendation to purchase, nor does
the absence of a peripheral or a non-OK status indicate a recommendation
against purchase.  We note any compatibility problems we found, but such
problems are not necessarily with the peripheral: the root cause
may be in the Android software or Nexus hardware.  Your test results
may vary from ours; please let us know your experience at the
<a href="https://groups.google.com/forum/#!forum/android-midi">android.midi group</a>.

<table>

<tr>
<th>Manufacturer</th>
<th>Model</th>
<th>Form factor</th>
<th>Status</th>
</tr>

<tr>
<td>Akai</td>
<td>LPK25</td>
<td>keyboard</td>
<td>OK</td>
</tr>
<tr>
<td>Akai</td>
<td>MPK mini</td>
<td>keyboard</td>
<td>OK with most Nexus devices<br />but incompatible with<br />Nexus 7 2013</td>
</tr>

<tr>
<td>Arturia</td>
<td>MicroBrute</td>
<td>keyboard+synth</td>
<td>OK</td>
</tr>
<tr>
<td>Arturia</td>
<td>MINI LAB</td>
<td>keyboard+synth</td>
<td>OK</td>
</tr>

<tr>
<td>Creative E-MU</td>
<td>Xmidi 1x1 Tab</td>
<td>MIDI 1.0 interface</td>
<td>OK</td>
</tr>

<tr>
<td>Focusrite</td>
<td>Scarlett 8i16</td>
<td>audio interface with<br />MIDI 1.0 interface</td>
<td>OK for MIDI portion</td>
</tr>

<tr>
<td>Keith McMillen</td>
<td>QuNexus</td>
<td>pad keyboard</td>
<td>OK</td>
</tr>

<tr>
<td>Korg</td>
<td>M50-61</td>
<td>keyboard+synth</td>
<td>OK</td>
</tr>
<tr>
<td>Korg</td>
<td>nanoKEY</td>
<td>keyboard</td>
<td>OK</td>
</tr>
<tr>
<td>Korg</td>
<td>nanoKEY2</td>
<td>keyboard</td>
<td>OK</td>
</tr>
<tr>
<td>Korg</td>
<td>nanoKONTROL2</td>
<td>controller</td>
<td>OK</td>
</tr>
<tr>
<td>Korg</td>
<td>TRITON taktile</td>
<td>keyboard</td>
<td>OK for Nexus 5X<br />but incompatible with<br />other Nexus devices</td>
</tr>

<tr>
<td>M-Audio</td>
<td>Keystation Mini-32</td>
<td>keyboard</td>
<td>OK</td>
</tr>
<tr>
<td>M-Audio</td>
<td>MIDISPORT 1x1</td>
<td>MIDI 1.0 interface</td>
<td>OK</td>
</tr>

<tr>
<td>Miselu</td>
<td>C.24</td>
<td>BLE keyboard</td>
<td>OK</td>
</tr>

<tr>
<td>Nektar</td>
<td>Impact LX25</td>
<td>keyboard</td>
<td>OK</td>
</tr>

<tr>
<td>Novation</td>
<td>Launchkey Mini</td>
<td>keyboard</td>
<td>OK</td>
</tr>
<tr>
<td>Novation</td>
<td>Launchpad S</td>
<td>button grid</td>
<td>OK</td>
</tr>
<tr>
<td>Novation</td>
<td>Nocturn 25</td>
<td>keyboard</td>
<td>incompatible</td>
</tr>

<tr>
<td>Quicco</td>
<td>mi.1</td>
<td>MIDI 1.0 to BLE adapter</td>
<td>no timestamps</td>
</tr>

<tr>
<td>ROLI</td>
<td>Seaboard RISE</td>
<td>keyboard</td>
<td>OK</td>
</tr>

</table>

## Pro audio hardware flag

#### What is a hardware feature flag?
A hardware feature flag is a method for Android device manufacturers (OEMs) to report that the device supports certain features.

Examples:

- Camera
- Pro audio

Hardware feature flags can be used to determine which devices your app will run on by specifying <feature required> in your app manifest.

They can also be used at runtime to determine how your app should perform. For example, if the device is reporting [android.hardware.audio.pro](http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_AUDIO_PRO) you could reduce your audio output buffer size to ensure the lowest possible latency.

#### Can the requirements for a feature flag change with each release of Android?
Yes. The latest set of requirements can always be found in the latest [Android Compatibility Definition Document](http://static.googleusercontent.com/media/source.android.com/en//compatibility/android-cdd.pdf)

#### How can I ensure that my app *only* runs on Pro Audio devices?
In your manifest include the following line: TODO

#### Which Nexus devices are Pro Audio devices?

- Nexus 9 (running Android Marshmallow)
- Nexus 5X
- Nexus 6P
