# Xclipse-940 Extreme Collector (Phase-1)

This project builds an extreme logging Vulkan ICD wrapper for Eden emulator.
It collects device features, extensions, formats, queue families, SPIR-V modules, and traces.

Logs are written to:
- /sdcard/eden_wrapper_logs/api/
- /sdcard/eden_wrapper_logs/device/
- /sdcard/eden_wrapper_logs/shader/
- /sdcard/eden_wrapper_logs/events/

After running games, pull logs via adb and upload for Phase-2 analysis.

Build & packaging run automatically in GitHub Actions and produce `your_wrapper.zip`.
