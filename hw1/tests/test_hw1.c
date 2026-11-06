#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hw1.h"

static int checks_run = 0;
static int checks_failed = 0;

#define CHECK(cond)                                                            \
  do {                                                                         \
    checks_run++;                                                              \
    if (cond) {                                                                \
      printf("  [ OK ] %s\n", #cond);                                          \
    } else {                                                                   \
      checks_failed++;                                                         \
      printf("  [FAIL] %s   (%s:%d)\n", #cond, __FILE__, __LINE__);            \
    }                                                                          \
  } while (0)

/* ---------- parse_mac: публичные сценарии из методички ------------- */

static void test_parse_mac_public(void) {
  uint8_t mac[6];
  printf("parse_mac: публичные сценарии\n");

  /* Канонический формат, заглавные буквы. */
  CHECK(parse_mac("AA:BB:CC:DD:EE:FF", mac) == 0);
  CHECK(mac[0] == 0xAA && mac[1] == 0xBB && mac[2] == 0xCC && mac[3] == 0xDD &&
        mac[4] == 0xEE && mac[5] == 0xFF);

  /* Нижний регистр. */
  CHECK(parse_mac("aa:bb:cc:dd:ee:ff", mac) == 0);
  CHECK(mac[0] == 0xAA && mac[5] == 0xFF);

  /* Смешанный регистр. */
  CHECK(parse_mac("Aa:bB:cC:Dd:eE:fF", mac) == 0);

  /* Разделитель '-'. */
  CHECK(parse_mac("aa-bb-cc-dd-ee-ff", mac) == 0);

  /* Смешанные разделители: в каждой позиции допустим любой из двух. */
  CHECK(parse_mac("aa:bb-cc:dd-ee:ff", mac) == 0);
  CHECK(parse_mac("Aa-bB:cC-dD:eE-fF", mac) == 0);

  /* Граничные значения байтов. */
  CHECK(parse_mac("00:00:00:00:00:00", mac) == 0 && mac[0] == 0x00 &&
        mac[5] == 0x00);
  CHECK(parse_mac("ff:ff:ff:ff:ff:ff", mac) == 0 && mac[0] == 0xFF &&
        mac[5] == 0xFF);
}

/* ---------- parse_mac: скрытые сценарии ----------------------------- */

static void test_parse_mac_hidden(void) {
  uint8_t mac[6];
  printf("parse_mac: скрытые сценарии (NULL, обрезка, мусор)\n");

  /* NULL-указатели. */
  CHECK(parse_mac(NULL, mac) == -1);
  CHECK(parse_mac("AA:BB:CC:DD:EE:FF", NULL) == -1);
  CHECK(parse_mac(NULL, NULL) == -1);

  /* Обрезанные строки. */
  CHECK(parse_mac("", mac) == -1);                 /* пусто            */
  CHECK(parse_mac("AA:BB:CC:DD:EE", mac) == -1);   /* нет 6-й группы   */
  CHECK(parse_mac("AA:BB:CC:DD:EE:F", mac) == -1); /* одна цифра       */
  CHECK(parse_mac("AABBCCDDEEFF", mac) == -1);     /* без разделителей */

  /* Лишние символы. */
  CHECK(parse_mac("AA:BB:CC:DD:EE:FF:", mac) == -1);   /* хвостовой ':'  */
  CHECK(parse_mac("AA:BB:CC:DD:EE:FF:00", mac) == -1); /* 7-я группа     */
  CHECK(parse_mac("AA:BB:CC:DD:EE:FF\n", mac) == -1);  /* перевод строки */
  CHECK(parse_mac(" AA:BB:CC:DD:EE:FF", mac) == -1);   /* пробел спереди */
  CHECK(parse_mac("AA:BB:CC:DD:EE:FF ", mac) == -1);   /* пробел сзади   */

  /* Нешестнадцатеричные символы и неверные разделители. */
  CHECK(parse_mac("AA:BB:CC:DD:EE:GG", mac) == -1);
  CHECK(parse_mac("AA.BB.CC.DD.EE.FF", mac) == -1);
  CHECK(parse_mac("AA:BB;CC:DD:EE:FF", mac) == -1);
}

/* ---------- format_mac ---------------------------------------------- */

static void test_format_mac(void) {
  char buf[18];
  uint8_t mac[6];
  printf("format_mac: канонический вывод\n");

  const uint8_t mac1[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
  format_mac(mac1, buf);
  CHECK(strcmp(buf, "00:11:22:33:44:55") == 0);

  const uint8_t mac2[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
  format_mac(mac2, buf);
  CHECK(strcmp(buf, "AA:BB:CC:DD:EE:FF") == 0);

  /* Буквы только заглавные, даже если исходные байты «строчные». */
  const uint8_t mac3[6] = {0x09, 0x0a, 0xAF, 0xfe, 0x5c, 0x0F};
  format_mac(mac3, buf);
  CHECK(strcmp(buf, "09:0A:AF:FE:5C:0F") == 0);

  /* Round-trip: parse -> format всегда даёт канонический вид. */
  CHECK(parse_mac("ab-cd-ef-01-23-45", mac) == 0);
  format_mac(mac, buf);
  CHECK(strcmp(buf, "AB:CD:EF:01:23:45") == 0);
}

/* ---------- ipv4_checksum ------------------------------------------- */

static void test_ipv4_checksum(void) {
  printf("ipv4_checksum: RFC 1071\n");

  /*
   * Эталонный пример из методички: заголовок IPv4 с обнулённым
   * полем checksum. Ожидаемое значение — 0xB861.
   */
  static const uint8_t hdr[20] = {0x45, 0x00, 0x00, 0x73, 0x00, 0x00, 0x40,
                                  0x00, 0x40, 0x11, 0x00, 0x00, 0xc0, 0xa8,
                                  0x00, 0x01, 0xc0, 0xa8, 0x00, 0xc7};
  CHECK(ipv4_checksum(hdr, sizeof hdr) == 0xB861);

  /* Тот же заголовок с уже вписанной верной суммой: результат 0.
   * Классическое свойство контроля целостности по RFC 1071. */
  static const uint8_t hdr_ok[20] = {0x45, 0x00, 0x00, 0x73, 0x00, 0x00, 0x40,
                                     0x00, 0x40, 0x11, 0xb8, 0x61, 0xc0, 0xa8,
                                     0x00, 0x01, 0xc0, 0xa8, 0x00, 0xc7};
  CHECK(ipv4_checksum(hdr_ok, sizeof hdr_ok) == 0x0000);

  /* «Заголовок без поля checksum»: первые 10 байт эталона. */
  CHECK(ipv4_checksum(hdr, 10) == 0x3A7B);

  /* Тривиальные случаи. */
  static const uint8_t zeros[4] = {0, 0, 0, 0};
  CHECK(ipv4_checksum(zeros, 4) == 0xFFFF);

  static const uint8_t ones[2] = {0xFF, 0xFF};
  CHECK(ipv4_checksum(ones, 2) == 0x0000);

  /* Пустой буфер: сумма ноль, дополнение — 0xFFFF. */
  CHECK(ipv4_checksum(zeros, 0) == 0xFFFF);

  /* Нечётный буфер: последний байт — старший байт слова,
   * младший дополняется нулём. */
  static const uint8_t odd[3] = {0x45, 0x00, 0x00};
  CHECK(ipv4_checksum(odd, 3) == 0xBAFF); /* 0x4500 + 0x0000 */

  static const uint8_t odd2[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
  CHECK(ipv4_checksum(odd2, 5) == 0xF6F9); /* 0x0102+0x0304+0x0500 */
}

/* ---------- main ----------------------------------------------------- */

int main(void) {
  test_parse_mac_public();
  test_parse_mac_hidden();
  test_format_mac();
  test_ipv4_checksum();

  printf("\nИтого: %d проверок, провалено: %d\n", checks_run, checks_failed);
  return checks_failed == 0 ? 0 : 1;
}
