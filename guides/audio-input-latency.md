---
layout: page
title: Audio input latency
permalink: /guides/audio-input-latency.html
site_nav_category_order: 110
is_site_nav_category2: true
site_nav_category: guides
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
{::options toc_levels="1,2,3"/}

* TOC
{:toc}

## Checklist

- You'll need to use Android native audio based on OpenSL ES
- If you have not already done so, install the NDK
- Many of the same requirements for lower latency audio output also apply to lower latency input. So first read the requirements for lower latency output in the [OpenSL ES performance section]({{site.baseurl}}/guides/opensl_es.html#performance)

<!--- Next read the requirements for lower latency input at XYZ-->

## Dos and Don'ts

<ul><li>
Do be prepared to handle nominal sample rates of 44100 and 48000 Hz as reported by
<a href="http://developer.android.com/reference/android/media/AudioManager.html#getProperty(java.lang.String)">getProperty(String)</a>
for
<a href="http://developer.android.com/reference/android/media/AudioManager.html#PROPERTY_OUTPUT_SAMPLE_RATE">PROPERTY_OUTPUT_SAMPLE_RATE</a>.
Other sample rates are possible but rare.
</li>

<li>
Do be prepared to handle the buffer size reported by
<a href="http://developer.android.com/reference/android/media/AudioManager.html#getProperty(java.lang.String)">getProperty(String)</a>
for
<a href="http://developer.android.com/reference/android/media/AudioManager.html#PROPERTY_OUTPUT_FRAMES_PER_BUFFER">PROPERTY_OUTPUT_FRAMES_PER_BUFFER</a>.
Typical buffer sizes include 96, 128, 160, 192, 240, 256, or 512 frames,
but other values are possible.
</li>

<li>
Don't assume that your input and output callbacks are synchronized.
For simultaneous input and output, separate buffer queue completion
handlers are used for each side.  Your callback handlers run in the
context of system-supplied thread(s).  The order of execution for input
and output callbacks is not specified, and may even be concurrent on
separate CPU cores.  There is no guarantee of the relative order of
these callbacks, or the synchronization of the audio clocks, even when
both sides use the same sample rate.  Your application should buffer
the data with proper buffer synchronization.
</li>

<li>
Don't assume that the actual sample rate exactly matches the nominal
sample rate.  For example, if the nominal sample rate is 48 kHz, it is
normal for the audio clock to advance at a slightly different rate than
the operating system CLOCK_MONOTONIC.  This is because the audio and
system clocks may derive from different crystals.
</li>

<li>
Don't assume that the actual playback sample rate exactly matches the
actual capture sample rate, especially if the endpoints are on separate
paths.  For example, if you are capturing from the on-device microphone
at 48 kHz nominal sample rate, and playing on USB audio at 48 kHz nominal
sample rate, the actual sample rates are likely to be slightly different
from each other.
<br />
A consequence of potentially independent audio clocks is the need for
asynchronous sample rate conversion. A simple (though not ideal for
audio quality) technique for asynchronous sample rate conversion is to
duplicate or drop samples as needed near a zero-crossing point. More
sophisticated conversions are possible.
</li>

</ul>

Be sure to check the [latency section of the FAQs]({{site.baseurl}}/faqs.html#latency) for more information.
