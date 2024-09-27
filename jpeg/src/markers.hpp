#pragma once

namespace jpeg {

enum Marker {
    DEFAULT_MARKER = 0xFF,
    JPG_MARKER = 0xD8,
    DHT_MARKER = 0xC4,
    DQT_MARKER = 0xDB,
    DATA_MARKER = 0xDA,
    END_MARKER = 0xD9,

    DNL_MARKER = 0xDC,
    DRI_MARKER = 0xDD,

    SOF0_MARKER = 0xC0,
    SOF1_MARKER = 0xC1,
    SOF2_MARKER = 0xC2,
    SOF3_MARKER = 0xC3,
    // SOF4_MARKER = 0xC4,  // DHT
    SOF5_MARKER = 0xC5,
    SOF6_MARKER = 0xC6,
    SOF7_MARKER = 0xC7,
    // SOF8_MARKER = 0xC8,  // JPG
    SOF9_MARKER = 0xC9,
    SOF10_MARKER = 0xCA,
    SOF11_MARKER = 0xCB,
    // SOF12_MARKER = 0xCC,  // DAC
    SOF13_MARKER = 0xCD,
    SOF14_MARKER = 0xCE,
    SOF15_MARKER = 0xCF
};

}  // namespace jpeg
