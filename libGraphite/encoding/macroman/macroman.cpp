// Copyright (c) 2020 Tom Hancocks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include "libGraphite/encoding/macroman/macroman.hpp"

// MARK: - Encoding Tables

static uint16_t cp_table[0x100] =
{
    // Standard ASCII
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,

    // MacRoman Specific
    0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1,
    0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8,
    0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3,
    0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC,
    0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF,
    0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8,
    0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211,
    0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x03A9, 0x00E6, 0x00F8,
    0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB,
    0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
    0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA,
    0x00FF, 0x0178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02,
    0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1,
    0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4,
    0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC,
    0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7
};

// MARK: - Unicode Support

// TODO: Extract out into own file at somepoint.
namespace unicode
{

    struct hint
    {
    public:
        uint8_t m_mask;
        uint8_t m_lead;
        uint32_t m_beg;
        uint32_t m_end;
        size_t m_bits;

        hint(uint8_t mask, uint8_t lead, uint32_t beg, uint32_t end, size_t bits)
                : m_mask(mask), m_lead(lead), m_beg(beg), m_end(end), m_bits(bits)
        {

        }

        static auto utf8() -> std::vector<unicode::hint>
        {
            static std::vector<unicode::hint> utf8;
            if (utf8.empty()) {
                utf8.emplace_back(unicode::hint(0b00111111, 0b10000000, 0, 0, 6));
                utf8.emplace_back(unicode::hint(0b01111111, 0b00000000, 0000, 0177, 7));
                utf8.emplace_back(unicode::hint(0b00011111, 0b11000000, 0200, 03777, 5));
                utf8.emplace_back(unicode::hint(0b00001111, 0b11100000, 04000, 0177777, 4));
                utf8.emplace_back(unicode::hint(0b00000111, 0b11110000, 0200000, 04177777, 3));
            }
            return utf8;
        }

    };
};

// MARK: - Conversion Functions

auto graphite::encoding::mac_roman::from_utf8(const std::string &str) -> std::vector<uint8_t>
{
    std::vector<uint8_t> mac_roman_bytes;
    
    // Convert back to a C-String for easier processing here...
    const char *s = str.c_str();
    size_t bytes = strlen(s);
    
    auto utf8 = unicode::hint::utf8();
    for (auto i = 0; i < bytes;) {
        
        // Determine the length of the current character, and then condense it down
        // into a single codepoint, which can then be mapped to a MacRoman value.
        size_t n = 0;
        auto ch = static_cast<uint8_t>(s[i]);
        for (auto hint : utf8) {
            if ((ch & ~hint.m_mask) == hint.m_lead) {
                break;
            }
            ++n;
        }
        
        // Validate the size of the UTF8 Scalar
        if (n > 4) {
            throw std::runtime_error("Invalid UTF8 Scalar size (more than 4 bytes).");
        }
        
        auto shift = utf8[0].m_bits * (n - 1);
        uint32_t codepoint = (s[i++] & utf8[n].m_mask) << shift;
        for (auto j = 1; j < n; ++i, ++j) {
            shift -= utf8[0].m_bits;
            codepoint |= ((char)s[i] & utf8[0].m_mask) << shift;
        }
        
        
        // Look up the MacRoman byte for the codepoint and then add it to the vector.
        for (auto j = 0; j < 0x100; ++j) {
            if (cp_table[j] == codepoint) {
                mac_roman_bytes.emplace_back(j);
            }
        }
    }
    
    return mac_roman_bytes;
}

auto graphite::encoding::mac_roman::to_utf8(std::vector<uint8_t> bytes) -> std::string
{
    std::string result;
    
    auto utf8 = unicode::hint::utf8();
    for (auto c : bytes) {
        // Get the codepoint and determine the length of the UTF8 scalar.
        auto cp = cp_table[c];
        
        size_t n = 0;
        for (auto hint : utf8) {
            if ((cp >= hint.m_beg) && (cp <= hint.m_end)) {
                break;
            }
            ++n;
        }
        
        // Validate the size of the UTF8 scalar
        if (n > 4) {
            throw std::logic_error("Fatal error occurred. More than 4 bytes in a UTF8 scalar.\n");
        }
        
        // Build the UTF8 scalar from the unicode codepoint.
        int shift = static_cast<int>(utf8[0].m_bits * (n - 1));
        result += ((cp >> shift & utf8[n].m_mask) | utf8[n].m_lead);
        shift -= utf8[0].m_bits;
        for (auto j = 1; j < n; ++j) {
            result += ((cp >> shift & utf8[0].m_mask) | utf8[0].m_lead);
            shift -= utf8[0].m_bits;
        }
    }
    
    return result;
}
