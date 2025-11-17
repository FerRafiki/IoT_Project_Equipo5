#ifndef MQTT_FRAME_H
#define MQTT_FRAME_H

#include <stdint.h>

typedef struct
{
    uint8_t bFrameType;
    uint16_t wMsgLen;
    uint16_t wProtlNameLen;
    char sProtName[4];
    uint8_t bVersion;
    uint8_t bConnectFlags;
    uint16_t bKeepAlive;
    uint16_t wClientIdLen;
    char sClientID[100];
} sConnect;

typedef struct {
    uint8_t bFrameType;  // 0x82 SUBSCRIBE
    uint8_t bMsgLen;     // Remaining length (1 byte para pequeÃ±os paquetes)
    uint16_t wPacketID;  // Packet Identifier (big-endian)
    uint16_t wTopicLen;  // Longitud del topic
    char sTopic[100];    // Nombre del topic
    uint8_t bQoS;        // QoS (0,1,2)
} sSubscribe;

// Estructura para mensaje de conexiÃ³n ACK
typedef struct
{
    uint8_t bFrameType;    // 0x20
    uint8_t bMsgLen;       // 0x02
    uint8_t bFlags;        // 0x00
    uint8_t bReturnStatus; // 0x00: Aceptado
                           // 0x01: LÃ­mite de clientes alcanzado
                           // 0x02: Frame malformado
} sConnAck;

// Estructura para mensaje Ping Request/Response
typedef struct
{
    uint8_t bFrameType; // 0xC0: Request, 0xD0: Response
    uint8_t bMsgLen;    // 0x00
} sPingRE;

#endif // MQTT_FRAME_H
