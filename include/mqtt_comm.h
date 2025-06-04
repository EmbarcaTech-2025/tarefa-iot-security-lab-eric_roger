#ifndef MQTT_COMM_H
#define MQTT_COMM_H

#include <stdint.h>
#include <stddef.h>

// Define um tipo de função callback para tratar mensagens recebidas via MQTT
// Parâmetros:
//   - topic: nome do tópico recebido
//   - payload: ponteiro para os dados recebidos
//   - len: tamanho dos dados recebidos
typedef void (*mqtt_message_cb_t)(const char *topic, const uint8_t *payload, size_t len);

// Inicializa e conecta o cliente MQTT ao broker
// Parâmetros:
//   - client_id: identificador do cliente MQTT
//   - broker_ip: endereço IP do broker (ex: "192.168.1.1")
//   - user: nome de usuário para autenticação (pode ser NULL)
//   - pass: senha para autenticação (pode ser NULL)
void mqtt_setup(const char *client_id, const char *broker_ip, const char *user, const char *pass);

// Publica dados em um tópico MQTT
// Parâmetros:
//   - topic: nome do tópico (ex: "sensor/temperatura")
//   - data: ponteiro para os dados a serem enviados
//   - len: tamanho dos dados
void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len);

// Inscreve-se em um tópico MQTT e registra uma função callback para tratar mensagens recebidas
// Parâmetros:
//   - topic: nome do tópico para inscrição
//   - cb: função callback a ser chamada ao receber mensagem
void mqtt_comm_subscribe(const char *topic, mqtt_message_cb_t cb);

#endif