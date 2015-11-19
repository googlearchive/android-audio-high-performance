---
layout: page
title: Pro Audio Hardware Flag
permalink: /faqs/pro_audio_hardware_flag.html
site_nav_category_order: 200
is_site_nav_category2: true
site_nav_category: faqs
---

## What is a hardware feature flag?
A hardware feature flag is a method for Android device manufacturers (OEMs) to report that the device supports certain features.

Examples:

- Camera
- Pro audio

Hardware feature flags can be used to determine which devices your app will run on by specifying <feature required> in your app manifest.

They can also be used at runtime to determine how your app should perform. For example, if the device is reporting [android.hardware.audio.pro](http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_AUDIO_PRO) you could reduce your audio output buffer size to ensure the lowest possible latency.

## Can the requirements for a feature flag change with each release of Android?
Yes. The latest set of requirements can always be found in the latest [Android Compatibility Definition Document](http://static.googleusercontent.com/media/source.android.com/en//compatibility/android-cdd.pdf)

## How can I ensure that my app *only* runs on Pro Audio devices?

In your manifest include the following line: TODO

## Which Nexus devices are Pro Audio devices?

- Nexus 9 (running Android Marshmallow)
- Nexus 5X
- Nexus 6P
