# Android High Performance Audio Samples
![Bugdroid DJing image](djdroid-light.png "Bugdroid DJing image")

**Note: Unless you specifically need to use OpenSL ES or AAudio consider using the [Oboe library](https://github.com/google/oboe) for development**

Welcome to the Android high-performance audio samples repository. These samples demonstrate how to use the [AAudio](https://developer.android.com/ndk/guides/audio/aaudio/aaudio.html) and [OpenSL ES](https://developer.android.com/ndk/guides/audio/opensl/index.html) audio APIs on Android to create real-time audio apps. 

If you haven't already it's worth reading the [high-performance audio guide](https://developer.android.com/ndk/guides/audio/index.html). There's also [plenty of other resources](RESOURCES.md) to help you create amazing audio apps on Android.

### Sample list
The repository is structured as follows: 

- [oboe](oboe): Samples for the Oboe library. [Moved to here](https://github.com/google/oboe/tree/master/samples).
- [aaudio](aaudio): This folder contains all the samples for the AAudio API
- [SimpleSynth](SimpleSynth): Shows how to achieve optimal output latency and avoid audio glitches using OpenSL ES 

Check the README in each of these subfolders for instructions on how to load the sample(s) into Android Studio. 

### Issues
If you've found an error in these samples, please [file an issue](https://github.com/googlesamples/android-audio-high-performance/issues/new).

### Contributing
Patches and new samples are encouraged, and may be submitted by [forking this project](https://github.com/googlesamples/android-audio-high-performance/fork) and
submitting a pull request through GitHub. Please see [CONTRIBUTING.md](CONTRIBUTING.md) for more details.

### License
Copyright 2017 The Android Open Source Project, Inc.

Licensed to the Apache Software Foundation (ASF) under one or more contributor
license agreements.  See the NOTICE file distributed with this work for
additional information regarding copyright ownership.  The ASF licenses this
file to you under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy of
the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
License for the specific language governing permissions and limitations under
the License.

[LICENSE](LICENSE)
