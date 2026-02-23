# QGroundControl -- Airmobis Custom Build

This repository contains QGroundControl along with Airmobis' `custom` directory for integration with [GeoWork](https://www.mobis1.com/geowork).

We've also implemented a few changes, here's a non-exhaustive list:

| MAVLink                                                                 | Airmobis                                                 | Reaason                                                       |
|-------------------------------------------------------------------------|----------------------------------------------------------|---------------------------------------------------------------|
| Custom plugin has own CMakeLists.txt; included via `add_subdirectory()` | Custom plugin has config.cmake; included via `include()` | Tooling doesn't see QGC APIs otherwise                        |
| GStreamer found via various platform-specific hacks                     | GStreamer found via pkg-config                           | Enables platform-independent way of finding GStreamer         |
| Unmodified `PlanMasterController`                                       | Additional `uploadToGeoWork()` method                    | Enables usage of said method in Plan View                     |

Installation requirements:
- Qt 6
    - Ideally 6.9.2, since this version works both on Linux and macOS
    - Version 6.8.3 should work on Linux as well
    - CMake might not find it, depending on its installation directory. In that case:
        - Add it to `CMAKE_PREFIX_PATH` (to configure)
        - Add it to `LD_LIBRARY_PATH` (to find the libraries upon launching the app)
        - For example, if you installed Qt 6.9.2 under `/opt` on an x86-64 machine, set
          `CMAKE_PREFIX_PATH=/opt/Qt/6.9.2/gcc_64` and `LD_LIBRARY_PATH=/opt/Qt/6.9.2/gcc_64/lib`
- GStreamer
    - Version 1.24 has been tested to work
        - 1.26 and 1.28 fail due to missing GLib symbols (at least on macOS), though you could probably hack your way through that
        - Previous versions might work, not sure
    - Our fork of QGroundControl has been modified to find GStreamer via pkg-config, so it should work on both Linux and macOS

Android requirements:
- CLI tools
    - `android-commandlinetools` on Homebrew, package name might be similar elsewhere
    - `sdkmanager "build-tools;35.0.0" "ndk;25.1.8937393" "platform-tools" "platforms;android-34"`
    - Don't forget to accept licenses via `yes | sdkmanager --licenses`

Below is the README of the original QGroundControl repository.

<p align="center">
  <img src="https://raw.githubusercontent.com/Dronecode/UX-Design/35d8148a8a0559cd4bcf50bfa2c94614983cce91/QGC/Branding/Deliverables/QGC_RGB_Logo_Horizontal_Positive_PREFERRED/QGC_RGB_Logo_Horizontal_Positive_PREFERRED.svg" alt="QGroundControl Logo" width="500">
</p>

<p align="center">
  <a href="https://github.com/mavlink/QGroundControl/releases">
    <img src="https://img.shields.io/github/release/mavlink/QGroundControl.svg" alt="Latest Release">
  </a>
</p>

*QGroundControl* (QGC) is a highly intuitive and powerful Ground Control Station (GCS) designed for UAVs. Whether you're a first-time pilot or an experienced professional, QGC provides a seamless user experience for flight control and mission planning, making it the go-to solution for any *MAVLink-enabled drone*.

---

### 🌟 *Why Choose QGroundControl?*

- *🚀 Ease of Use*: A beginner-friendly interface designed for smooth operation without sacrificing advanced features for pros.
- *✈️ Comprehensive Flight Control*: Full flight control and mission management for *PX4* and *ArduPilot* powered UAVs.
- *🛠️ Mission Planning*: Easily plan complex missions with a simple drag-and-drop interface.

🔍 For a deeper dive into using QGC, check out the [User Manual](https://docs.qgroundcontrol.com/en/) – although, thanks to QGC's intuitive UI, you may not even need it!


---

### 🚁 *Key Features*

- 🕹️ *Full Flight Control*: Supports all *MAVLink drones*.
- ⚙️ *Vehicle Setup*: Tailored configuration for *PX4* and *ArduPilot* platforms.
- 🔧 *Fully Open Source*: Customize and extend the software to suit your needs.

🎯 Check out the latest updates in our [New Features and Release Notes](https://github.com/mavlink/qgroundcontrol/blob/master/ChangeLog.md).

---

### 💻 *Get Involved!*

QGroundControl is *open-source*, meaning you have the power to shape it! Whether you're fixing bugs, adding features, or customizing for your specific needs, QGC welcomes contributions from the community.

🛠️ Start building today with our [Developer Guide](https://dev.qgroundcontrol.com/en/) and [build instructions](https://dev.qgroundcontrol.com/en/getting_started/).

---

### 🔗 *Useful Links*

- 🌐 [Official Website](http://qgroundcontrol.com)
- 📘 [User Manual](https://docs.qgroundcontrol.com/en/)
- 🛠️ [Developer Guide](https://dev.qgroundcontrol.com/en/)
- 💬 [Discussion & Support](https://docs.qgroundcontrol.com/en/Support/Support.html)
- 🤝 [Contributing](https://dev.qgroundcontrol.com/en/contribute/)
- 📜 [License Information](https://github.com/mavlink/qgroundcontrol/blob/master/.github/COPYING.md)

---

With QGroundControl, you're in full command of your UAV, ready to take your missions to the next level.
