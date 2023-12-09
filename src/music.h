#include <stdint.h>

enum notes {
  C2, Cd2, D2, Dd2, E2, F2, Fd2, G2, Gd2, A2, Ad2, B2,
  C3, Cd3, D3, Dd3, E3, F3, Fd3, G3, Gd3, A3, Ad3, B3,
  C4, Cd4, D4, Dd4, E4, F4, Fd4, G4, Gd4, A4, Ad4, B4,
  C5, Cd5, D5, Dd5, E5, F5, Fd5, G5, Gd5, A5, Ad5, B5,
  C6, Cd6, D6, Dd6, E6, F6, Fd6, G6, Gd6, A6, Ad6, B6,
  SILENCE, END
};

const uint16_t frequencies[] = {
  1046, 1102, 1155, 1205, 1253, 1297, 1339, 1379, 1417, 1452, 1486, 1517,
  1546, 1575, 1602, 1627, 1650, 1673, 1694, 1714, 1732, 1750, 1767, 1783,
  1798, 1812, 1825, 1837, 1849, 1860, 1871, 1881, 1890, 1899, 1907, 1915,
  1923, 1930, 1936, 1943, 1949, 1954, 1959, 1964, 1969, 1974, 1978, 1982,
  1985, 1988, 1992, 1995, 1998, 2001, 2004, 2006, 2009, 2011, 2013, 2015,
  0, 65535
};

const uint8_t bass_intro_music[] = {
      // Intro
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x00 - 0x0F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x10 - 0x1F
      END
};

const uint8_t lead_intro_music[] = {
      // Intro
      SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0x00 - 0x0F
      SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0x10 - 0x1F
      END
};

const uint8_t bass_main_music[] = {
      // Main Theme
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x00 - 0x0F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x10 - 0x1F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x20 - 0x2F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x30 - 0x3F
      G2, G3, G2, G3, G2, G3, G2, G3,  // 0x40 - 0x4F
      G2, G3, G2, G3, G2, G3, G2, G3,  // 0x50 - 0x5F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x60 - 0x6F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x70 - 0x7F
      // Second Theme
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x00 - 0x0F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x10 - 0x1F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x20 - 0x2F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x30 - 0x3F
      G2, G3, G2, G3, G2, G3, G2, G3,  // 0x40 - 0x4F
      G2, G3, G2, G3, G2, G3, G2, G3,  // 0x50 - 0x5F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x60 - 0x6F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0x70 - 0x7F
      A2, A3, A2, A3, A2, A3, A2, A3,  // 0x80 - 0x8F
      A2, A3, A2, A3, A2, A3, A2, A3,  // 0x90 - 0x9F
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0xA0 - 0xAF
      D2, D3, D2, D3, D2, D3, D2, D3,  // 0xAB - 0xBF
      A2, A3, A2, A3, A2, A3, A2, A3,  // 0xC0 - 0xCF
      A2, A3, A2, A3, A2, A3, A2, A3,  // 0xD0 - 0xDF
      END
};

const uint8_t lead_main_music[] = {
      // Main Theme
      A4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, G4,  // 0x00 - 0x0F
      Fd4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0x10 - 0x1F
      A4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, G4,  // 0x20 - 0x2F
      Fd4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, E4,  // 0x30 - 0x3F
      D4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0x40 - 0x4F
      SILENCE, D4, E4, Fd4, SILENCE, E4, SILENCE, SILENCE,  // 0x50 - 0x5F
      D4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0x60 - 0x6F
      SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0x70 - 0x7F
      // Second Theme
      A4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, G4,  // 0x00 - 0x0F
      Fd4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0x10 - 0x1F
      A4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, G4,  // 0x20 - 0x2F
      Fd4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, E4,  // 0x30 - 0x3F
      D4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0x40 - 0x4F
      SILENCE, D4, E4, Fd4, SILENCE, E4, SILENCE, SILENCE,  // 0x50 - 0x5F
      D4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0x60 - 0x6F
      SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0x70 - 0x7F
      SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0x80 - 0x8F
      SILENCE, A3, B3, Cd4, D4, E4, Fd4, G4, // 0x90 - 0x9F
      Fd4, E4, D4, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0xA0 - 0xAF
      SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0xB0 - 0xBF
      SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE,  // 0xC0 - 0xCF
      SILENCE, E4, D4, Cd4, SILENCE, D4, SILENCE, SILENCE,  // 0xD0 - 0xDF
      END
};
