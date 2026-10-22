# Credentials and safe publication / 凭据与安全发布

## English

- Copy `main/app_secrets.example.h` to ignored `main/app_secrets.h`. Use credentials
  for your own test device. Never put real values into the example or documentation.
- Firmware embeds the configuration as strings. Do not distribute `.bin`, `.elf`,
  `build/`, flash dumps, or packet captures produced with real credentials.
- The inherited demo uses ordinary authentication over plain MQTT port 1883.
  A private header prevents Git disclosure; it does **not** encrypt network traffic
  or flash. Use an isolated teaching network and a disposable test device key.
- NLECloud documents MQTTS port 8883 and token authentication. This package does
  not implement certificate verification, clock setup or token generation. Merely
  changing `mqtt://` to `mqtts://` is not a validated security upgrade.
- Before publishing: inspect `git status --short`, run
  `git check-ignore main/app_secrets.h sdkconfig build/example.bin`, and run
  `python tools/check_package.py`. The checker is a guard, not a guarantee.
- Redact device IDs, tags, keys, Wi-Fi names, personal information and account
  details in screenshots. If a credential is exposed, revoke/rotate it first;
  deleting a file from the latest commit does not erase Git history.
- Original private firmware and configuration are outside this release folder.
  Do not add or push its parent workspace to publish this package.

## 中文

- 真实配置只写入被忽略的 `main/app_secrets.h`，不要修改模板填入密钥。
- 编译后的固件仍然包含凭据，不要分享用真实账号生成的固件、ELF、
  构建目录、Flash 导出和抓包文件。
- 保留原教学工程的 1883 明文 MQTT 与普通鉴权。Git 隔离不等于网络加密，
  建议使用隔离教学网络、专用测试设备和可撤销密钥。
- 官方文档提供 8883 MQTTS 与 token 鉴权；本工程没有实现证书校验、
  时间同步或 token 生成，不能只改协议前缀就声称已安全升级。
- 发布前检查 Git 暂存区、忽略规则，并运行 `python tools/check_package.py`。
  检查工具只覆盖已知规则，不能替代人工核查。
- 截图应遮盖账号、Wi-Fi、设备标识、设备 ID 和密钥。已经泄漏的凭据应先
  撤销/更换；删除最新文件不能清除历史提交中的秘密。
- 只在本独立工程目录执行 Git 操作，不要把含原凭据的上级工作区上传。

Reference: [NLECloud device access and authentication](https://www.nlecloud.com/doc/devicedev/).
