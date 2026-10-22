# Publication validation / 发布验证记录

Snapshot date: **2026-10-22 (Asia/Shanghai)**.

## Scope / 范围

The author reports that the original project already connected to NLECloud and
displayed values. That is user-provided context, not a new hardware measurement
performed while preparing this package. The publication copy retains the original
acquisition/display/upload behavior and changes only credential integration,
reproducible packaging and documentation.

作者已说明原工程完成上云显示。本次整理将这一点作为背景，不冒充重新实测。
重新构建使用的是模板占位配置，没有向原设备烧录、重启或发送云端数据。

## Verified environment / 实际构建环境

| Item | Observed value |
|---|---|
| Host | Windows, PowerShell |
| SDK command | `idf.py --version` → `ESP-IDF v6.1` |
| Target | `esp32s3` |
| MQTT component | `espressif/mqtt 1.1.0` |
| Type utility component | `k0i05/esp_type_utils 1.2.7` |
| Configuration | Placeholder-only `main/app_secrets.h`, generated from the public example |
| Build defaults | DIO / Flash 80 MHz / 2 MB / CPU 160 MHz / FreeRTOS 100 Hz |

## Build evidence / 编译证据

Commands executed from the publication project root, in the activated v6.1 shell:

```powershell
Copy-Item main/app_secrets.example.h main/app_secrets.h
idf.py set-target esp32s3
idf.py build
```

Observed result: **exit code 0**, `Project build complete`.
Selected actual build output:

```text
Bootloader binary size 0x5280 bytes. 0x2d80 bytes (36%) free.
nlecloud_environment.bin binary size 0xe46f0 bytes.
Smallest app partition is 0x100000 bytes. 0x1b910 bytes (11%) free.
Project build complete.
```

This establishes toolchain/link/partition feasibility for the placeholder build.
Real credential lengths and later code changes can change binary size. Build
artifacts are ignored and excluded from GitHub because real builds embed secrets.
The small remaining application space should be reconsidered before adding large
features. No performance, accuracy or long-term stability measurements are claimed.

这证明发布源码可以在本机配置下编译链接，不能证明真实 Wi-Fi、云端登录、
传感器、OLED 或云页面已经由本次操作再次验收。

## Package checks / 发布检查

- Original source/configuration snapshot: SHA-256 comparison of 44 original files
  confirmed no changes to the existing project.
- Publication files were compared privately against the original Wi-Fi name,
  Wi-Fi password, MQTT username/client ID/SecretKey. No original private values
  were found; neither those values nor their hashes are published.
- `python tools/check_package.py` checks the Git publication boundary: required
  files, known credential patterns, excluded private/build paths, local Markdown
  links, heading anchors, and placeholder-only example configuration.
- Eight PNG illustrations and eight editable SVG files were generated and
  visually reviewed for both languages. Figures are explanatory, not measured
  hardware/cloud evidence.
- Third-party MIT and Apache-2.0 license copies were retained where supplied.
  Missing AHT20 upstream licensing is explicitly recorded in `THIRD_PARTY.md`.

## Not verified / 未验证

- A fresh machine installing ESP-IDF from scratch; downloads depend on network
  and external registry availability.
- Flashing the publication firmware or reading a serial device.
- Actual board SKU, physical Flash size, module pull-ups or power specifications.
- Live Wi-Fi/MQTT authentication with a reader's own device credentials.
- Physical sensor/display output or a newly captured cloud screenshot.
- Cloud dashboard layout/export, account feature permissions, TLS/token support.
- Reconnection reliability over an extended run or any production security claim.

Follow the staged acceptance procedure in either README to verify your own board
and cloud account. Do not infer these outcomes from a successful build.
