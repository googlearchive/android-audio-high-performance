---
layout: page
title: Development Guides
permalink: /guides/index.html
site_nav_category: guides
is_site_nav_category: false
site_nav_category_order: 10
---

Whether you're writing a synthesizer, digital audio workstation or karaoke app, you'll want to make sure that you achieve the best audio quality with the minimum possible latency.

These guides are intended to help you get started with specific areas of audio development.

## Audio input latency
Understand how to use the Android NDK and OpenSL ES to achieve the lowest possible latency when recording from the inbuilt microphone and using an external microphone via the headset.

**Common use cases:**

- Digital audio workstations for monitoring recording external instruments and voices
- Karaoke apps for recording the singer


[Read more on audio input latency]({{ site.baseurl }}/guides/audio-input-latency.html)

## Audio output latency
Understand how to avoid warm-up latency, match the device's native sample rate and avoid costly signal processing interfaces for minimum output latency.

**Common use cases:**

- Synthesizers, drum machines and software instruments, especially those that are triggered via an external MIDI controller

[Read more on audio output latency]({{ site.baseurl }}/guides/audio-output-latency.html)

## Floating point audio
Should you use floating-point numbers to represent your audio data? Advantages include wider dynamic range and more headroom to avoid clipping.

[Read more on floating-point audio]({{ site.baseurl }}/guides/floating_point.html)

## OpenSL ES for Android
Android's audio APIs are based on OpenSL ES, a high-performance native audio library. To achieve the lowest possible latency and have the most control over your audio signals, you'll need to use OpenSL ES. This comprehensive guide explains how you can get the best out of OpenSL ES on Android.

[Read more on OpenSL ES for Android]({{ site.baseurl }}/guides/opensl_es.html)

## Sample rates
Learn about how to choose the optimal sample rate, how Android performs resampling and strategies for upsampling/downsampling on the fly.

[Read more on sample rates]({{ site.baseurl }}/guides/sample-rates.html)
