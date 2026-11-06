#ifndef HW1_H
#define HW1_H

#include <stddef.h>
#include <stdint.h>

/* Разбирает MAC-адрес формата AA:BB:CC:DD:EE:FF.
 * Допустимы разделители ':' и '-', регистр любой.
 * Возвращает 0 при успехе (и заполняет out[6]), -1 при любой ошибке:
 * неверная длина, нешестнадцатеричный символ, NULL-указатель. */
int parse_mac(const char *str, uint8_t out[6]);

/* Пишет MAC в виде AA:BB:CC:DD:EE:FF (заглавные) + '\0'. out >= 18 байт. */
void format_mac(const uint8_t mac[6], char *out /* [18] */);

/* Контрольная сумма заголовка IPv4 по RFC 1071: дополнение до единицы
 * суммы дополнений 16-битных слов. Поле checksum в hdr считается
 * равным нулю; len чётное (IPv4-заголовок всегда кратен 4 байтам). */
uint16_t ipv4_checksum(const uint8_t *hdr, size_t len);

#endif /* HW1_H */
