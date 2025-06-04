#ifndef XOR_CIPHER_H
#define XOR_CIPHER_H

#include <stdint.h>
#include <stddef.h>

// Função para criptografar dados usando cifra XOR
// Parâmetros:
//   - input: ponteiro para os dados de entrada (originais)
//   - output: ponteiro para onde os dados criptografados serão armazenados
//   - len: tamanho dos dados em bytes
//   - key: chave de criptografia (um byte)
void xor_encrypt(const uint8_t *input, uint8_t *output, size_t len, uint8_t key);

// Função para descriptografar dados usando cifra XOR
// Parâmetros:
//   - input: ponteiro para os dados criptografados
//   - output: ponteiro para onde os dados descriptografados serão armazenados
//   - len: tamanho dos dados em bytes
//   - key: chave de criptografia (um byte, deve ser igual à usada na criptografia)
void xor_decrypt(const uint8_t *input, uint8_t *output, size_t len, uint8_t key);

#endif