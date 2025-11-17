#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <netinet/in.h> // htons
#include "mqtt_frame.h"

#define BUF_SIZE 512
#define TOPIC_MAX 100
#define PAYLOAD_MAX 256


int sendConnect(int sockfd, sConnect *frame)
{
    unsigned char buffer[200];
    int offset = 0;
    uint16_t tmp;

    buffer[offset++] = frame->bFrameType; // 0x10 CONNECT

    buffer[offset++] = frame->wMsgLen; // Remaining Length codificado variable-length

    tmp = htons(frame->wProtlNameLen);
    memcpy(buffer + offset, &tmp, 2);
    offset += 2;

    memcpy(buffer + offset, frame->sProtName, 4);
    offset += 4;

    buffer[offset++] = frame->bVersion;
    buffer[offset++] = frame->bConnectFlags;

    tmp = htons(frame->bKeepAlive);
    memcpy(buffer + offset, &tmp, 2);
    offset += 2;

    tmp = htons(frame->wClientIdLen);
    memcpy(buffer + offset, &tmp, 2);
    offset += 2;

    memcpy(buffer + offset, frame->sClientID, frame->wClientIdLen);
    offset += frame->wClientIdLen;

    return send(sockfd, buffer, offset, 0);
}

// ----- Función para enviar PUBLISH QoS 0 -----
int sendPublish(int sockfd, const char *topic, const char *payload)
{
    unsigned char buffer[512];
    int offset = 0;
    uint16_t tmp;
    int topicLen = strlen(topic);
    int payloadLen = strlen(payload);
    int msgLen = 2 + topicLen + payloadLen; // topic length + topic + payload

    buffer[offset++] = 0x30; // PUBLISH QoS 0
    buffer[offset++] = msgLen;

    tmp = htons(topicLen);
    memcpy(buffer + offset, &tmp, 2);
    offset += 2;

    memcpy(buffer + offset, topic, topicLen);
    offset += topicLen;
    memcpy(buffer + offset, payload, payloadLen);
    offset += payloadLen;

    return send(sockfd, buffer, offset, 0);
}

int sendSubscribe(int sockfd, const char *topic, uint16_t packetID)
{
    unsigned char buffer[200];
    int offset = 0;
    uint16_t tmp;
    int topicLen = strlen(topic);
    int remainingLen = 2 + 2 + topicLen + 1; // packetID + topic length + topic + QoS

    buffer[offset++] = 0x82; // SUBSCRIBE
    buffer[offset++] = remainingLen;

    tmp = htons(packetID);
    memcpy(buffer + offset, &tmp, 2);
    offset += 2;

    tmp = htons(topicLen);
    memcpy(buffer + offset, &tmp, 2);
    offset += 2;

    memcpy(buffer + offset, topic, topicLen);
    offset += topicLen;

    buffer[offset++] = 0x00; // QoS 0

    return send(sockfd, buffer, offset, 0);
}

// Enviar PINGREQ
int sendPing(int sockfd)
{
    sPingRE ping;
    ping.bFrameType = 0xC0; // PINGREQ
    ping.bMsgLen = 0x00;
    return send(sockfd, &ping, sizeof(ping), 0);
}

// Función auxiliar para leer Remaining Length de MQTT (variable byte integer)
int readRemainingLength(int sockfd)
{
    int multiplier = 1;
    int value = 0;
    unsigned char encodedByte;
    do
    {
        if (recv(sockfd, &encodedByte, 1, 0) != 1)
            return -1;
        value += (encodedByte & 127) * multiplier;
        multiplier *= 128;
    } while (encodedByte & 128);
    return value;
}

int recvPublishWithPing(int sockfd, char *topicOut, char *payloadOut, int keepAlive)
{
    fd_set readfds;
    struct timeval tv;
    int remainingTime = keepAlive;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int n = select(sockfd + 1, &readfds, NULL, NULL, &tv);
        if (n < 0)
            return -1;
        if (n == 0)
        {
            remainingTime--;
            if (remainingTime <= 0)
            {
                sendPing(sockfd); // tu función de ping
                remainingTime = keepAlive;
            }
            continue;
        }

        unsigned char byte0;
        if (recv(sockfd, &byte0, 1, 0) != 1)
            return -1;

        if (byte0 == 0xD0)
            continue; // PINGRESP
        if ((byte0 & 0xF0) != 0x30)
            continue; // no es PUBLISH

        int remainingLen = readRemainingLength(sockfd);
        if (remainingLen < 0)
            return -1;

        uint16_t topicLen;
        if (recv(sockfd, &topicLen, 2, 0) != 2)
            return -1;
        topicLen = ntohs(topicLen);
        if (topicLen >= TOPIC_MAX)
            return -1;

        if (recv(sockfd, topicOut, topicLen, 0) != topicLen)
            return -1;
        topicOut[topicLen] = '\0';

        int payloadLen = remainingLen - 2 - topicLen;
        if (payloadLen >= PAYLOAD_MAX)
            return -1;

        if (recv(sockfd, payloadOut, payloadLen, 0) != payloadLen)
            return -1;
        payloadOut[payloadLen] = '\0';

        return payloadLen;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Uso: %s <ip_broker> <publish/receive> <topic> [mensaje]\n", argv[0]);
        return 1;
    }

    const char *ipBroker = argv[1];
    const char *action   = argv[2];
    const char *topic    = argv[3];
    const char *msg      = (argc >= 5) ? argv[4] : "";

    struct sockaddr_in serveraddr;
    int sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(1883);

    if (inet_pton(AF_INET, ipBroker, &serveraddr.sin_addr) <= 0)
    {
        perror("IP invalida");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("connect");
        return 1;
    }
    printf("Conectado a Mosquitto en %s!\n", ipBroker);

    // --- CONNECT ---
    sConnect frame;
    frame.bFrameType = 0x10;
    frame.wProtlNameLen = 4;
    memcpy(frame.sProtName, "MQTT", 4);
    frame.bVersion = 4;
    frame.bConnectFlags = 0x02; // Clean session
    frame.bKeepAlive = 60;
    frame.wClientIdLen = 5;
    memcpy(frame.sClientID, "C1D01", 5);
    frame.wMsgLen = 2 + 4 + 1 + 1 + 2 + 2 + frame.wClientIdLen;

    sendConnect(sockfd, &frame);
    printf("CONNECT enviado\n");

    sConnAck connack;
    int n = recv(sockfd, &connack, sizeof(connack), 0);
    if (n != sizeof(connack) || connack.bReturnStatus != 0)
    {
        printf("Error de conexion MQTT\n");
        close(sockfd);
        return 1;
    }
    printf("Conexion MQTT aceptada!\n");

    if (strcmp(action, "publish") == 0)
    {
        sendPublish(sockfd, topic, msg);
        printf("Mensaje PUBLISH enviado al topic '%s'\n", topic);
    }
    else if (strcmp(action, "receive") == 0)
    {
        uint16_t packetID = 1;
        sendSubscribe(sockfd, topic, packetID);
        printf("SUBSCRIBE enviado para topic '%s'\n", topic);
        printf("Escuchando mensajes del topic '%s'...\n", topic);
        char recvTopic[TOPIC_MAX];
        char recvPayload[PAYLOAD_MAX];

        while (1)
        {
            int len = recvPublishWithPing(sockfd, recvTopic, recvPayload, 60);
            if (len > 0)
                printf("Mensaje recibido: Topic='%s', Payload='%s'\n", recvTopic, recvPayload);
        }
    }
    else
    {
        printf("Accion desconocida: %s\n", action);
    }

    close(sockfd);
    printf("Socket cerrado.\n");
    return 0;
}
