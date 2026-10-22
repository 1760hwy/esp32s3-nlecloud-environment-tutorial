# Third-party code and provenance

This is a public reproduction package of the author's existing firmware. Public
visibility is not a blanket license grant. No repository-wide license is inferred
for project-authored code or for code whose upstream license is unavailable.

| Path | Provenance | License information |
|---|---|---|
| `main/BSP/ss1306/` | Eric Gionet / K0I05, ESP-IDF SSD1306 component; existing source notices retained | MIT, copy in `LICENSES/SSD1306-MIT.txt` |
| `main/BSP/bh1750/` | Adapted existing project driver; supplied upstream `aZholtikov/zh_bh1750` version 2.0.0 | Apache-2.0, copy in `LICENSES/BH1750-Apache-2.0.txt` |
| `main/BSP/ath20/` | Existing project AHT20 port; supplied reference folder `esp-idf-aht20-main` | Supplied reference has no LICENSE file; no upstream license is claimed. Confirm permission before redistribution in a separately licensed product. |
| `espressif/mqtt` | Espressif Component Registry, version 1.1.0 | Downloaded by Component Manager; consult its own LICENSE |
| `k0i05/esp_type_utils` | Component Registry, version 1.2.7 | Downloaded by Component Manager; consult its own LICENSE |

The AHT20 and BH1750 sources are project adaptations, not untouched upstream
releases. The AHT20 implementation uses the current I2C master API, busy polling,
CRC checking and validated sample decoding. The BH1750 source uses the project's
shared bus handle. These adaptations predate this publication package. The
publication changes only credential integration, project packaging and documentation.

Original source links recorded by supplied metadata:

- [SSD1306 component](https://github.com/K0I05/ESP32-S3_ESP-IDF_COMPONENTS/tree/main/components/peripherals/i2c/esp_ssd1306)
- [BH1750 component](https://github.com/aZholtikov/zh_bh1750)

Unused reference libraries, manufacturer PDFs and upstream example artwork are
excluded. All tutorial diagrams are newly drawn explanatory figures; they are not
screenshots of an authenticated cloud session or photographs of tested hardware.
