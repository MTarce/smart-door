#include "HT_MQTT_Api.h"

/*===========================================================
Inicio - Configuração de rede 
=============================================================*/
static void HT_SetConnectioParameters(void)
{
    uint8_t cid = 0;
    PsAPNSetting apnSetting;
    int32_t ret;
    uint8_t networkMode = 0; // nb-iot network mode
    uint8_t bandNum = 1;
    uint8_t band = 28;

    ret = appSetBandModeSync(networkMode, bandNum, &band);
    if (ret == CMS_RET_SUCC)
    {
        printf("SetBand Result: %d\n", ret);
    }

    apnSetting.cid = 0;
    apnSetting.apnLength = strlen("iot.datatem.com.br");
    strcpy((char *)apnSetting.apnStr, "iot.datatem.com.br");
    apnSetting.pdnType = CMI_PS_PDN_TYPE_IP_V4V6;
    ret = appSetAPNSettingSync(&apnSetting, &cid);
}
/*===========================================================
Fim - Configuração de rede 
=============================================================*/

/*===========================================================
Inicio - Funções para uso do MQTT
=============================================================*/

extern volatile uint8_t subscribe_callback;

static MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

#if MQTT_TLS_ENABLE == 1
static MqttClientContext mqtt_client_ctx;
#endif

uint8_t HT_MQTT_Connect(MQTTClient *mqtt_client, Network *mqtt_network, char *addr, int32_t port, uint32_t send_timeout, uint32_t rcv_timeout, char *clientID,
                        char *username, char *password, uint8_t mqtt_version, uint32_t keep_alive_interval, uint8_t *sendbuf,
                        uint32_t sendbuf_size, uint8_t *readbuf, uint32_t readbuf_size)
{

#if MQTT_TLS_ENABLE == 1
    mqtt_client_ctx.caCertLen = 0;
    mqtt_client_ctx.port = port;
    mqtt_client_ctx.host = addr;
    mqtt_client_ctx.timeout_ms = MQTT_GENERAL_TIMEOUT;
    mqtt_client_ctx.isMqtt = true;
    mqtt_client_ctx.timeout_r = MQTT_GENERAL_TIMEOUT;
    mqtt_client_ctx.timeout_s = MQTT_GENERAL_TIMEOUT;
#endif

    connectData.MQTTVersion = mqtt_version;
    connectData.clientID.cstring = clientID;
    connectData.username.cstring = username;
    connectData.password.cstring = password;
    connectData.keepAliveInterval = keep_alive_interval;
    connectData.will.qos = QOS0;
    connectData.cleansession = false;

#if MQTT_TLS_ENABLE == 1

    printf("Starting TLS handshake...\n");

    if (HT_MQTT_TLSConnect(&mqtt_client_ctx, mqtt_network) != 0)
    {
        printf("TLS Connection Error!\n");
        return 1;
    }

    MQTTClientInit(mqtt_client, mqtt_network, MQTT_GENERAL_TIMEOUT, (unsigned char *)sendbuf, sendbuf_size, (unsigned char *)readbuf, readbuf_size);

    if ((MQTTConnect(mqtt_client, &connectData)) != 0)
    {
        mqtt_client->ping_outstanding = 1;
        return 1;
    }
    else
    {
        mqtt_client->ping_outstanding = 0;
    }

#else

    NetworkInit(mqtt_network);
    MQTTClientInit(mqtt_client, mqtt_network, MQTT_GENERAL_TIMEOUT, (unsigned char *)sendbuf, sendbuf_size, (unsigned char *)readbuf, readbuf_size);

    if ((NetworkSetConnTimeout(mqtt_network, send_timeout, rcv_timeout)) != 0)
    {
        mqtt_client->keepAliveInterval = connectData.keepAliveInterval;
        mqtt_client->ping_outstanding = 1;
    }
    else
    {

        if ((NetworkConnect(mqtt_network, addr, port)) != 0)
        {
            mqtt_client->keepAliveInterval = connectData.keepAliveInterval;
            mqtt_client->ping_outstanding = 1;

            return 1;
        }
        else
        {
            if ((MQTTConnect(mqtt_client, &connectData)) != 0)
            {
                mqtt_client->ping_outstanding = 1;
                return 1;
            }
            else
            {
                mqtt_client->ping_outstanding = 0;
            }
        }

        if (mqtt_client->ping_outstanding == 0)
        {
            if ((MQTTStartRECVTask(mqtt_client)) != SUCCESS)
            {
                return 1;
            }
        }
    }

#endif

    return 0;
}


void HT_MQTT_Publish(MQTTClient *mqtt_client, char *topic, uint8_t *payload, uint32_t len, enum QoS qos, uint8_t retained, uint16_t id, uint8_t dup)
{
    MQTTMessage message;

    message.qos = qos;
    message.retained = retained;
    message.id = id;
    message.dup = dup;
    message.payload = payload;
    message.payloadlen = len;

    MQTTPublish(mqtt_client, topic, &message);
}



void HT_MQTT_SubscribeCallback(MessageData *msg) {
    printf("Subscribe received: %s\n", msg->message->payload);

    subscribe_callback = 1;
    HT_FSM_SetSubscribeBuff(msg);

    memset(msg->message->payload, 0, msg->message->payloadlen);
}

void HT_MQTT_Subscribe(MQTTClient *mqtt_client, char *topic, enum QoS qos) {
    MQTTSubscribe(mqtt_client, (const char *)topic, qos, HT_MQTT_SubscribeCallback);
}

