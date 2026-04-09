#include <LoRa.h>
#include <SPI.h>
#include "mbedtls/sha256.h"

#define ss 5
#define rst 4
#define dio0 2

#define K 3
#define N 16
#define L 3

#define MSG_TAU 0x01
#define MSG_SYNC_DONE 0x02
#define MSG_DATA 0x03

enum Mode
{
    SYNC_MODE,
    SECURE_MODE
};

Mode currentMode = SYNC_MODE;

typedef struct
{
    int8_t w[K][N];
    int8_t sigma[K];
    int8_t tau;
} TPM;

TPM tpm;

int8_t X[K][N];
uint16_t round_id = 0;
int match_counter = 0;

int8_t sign(int v)
{
    if (v >= 0)
        return 1;
    return -1;
}

void initTPM(TPM *tpm)
{
    for (int i = 0; i < K; i++)
    {
        for (int j = 0; j < N; j++)
        {
            tpm->w[i][j] = random(-L, L + 1);
        }
    }
}

void generateInput(uint16_t round)
{
    randomSeed(round);

    for (int i = 0; i < K; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (random(0, 2) == 0)
                X[i][j] = -1;
            else
                X[i][j] = 1;
        }
    }
}

int8_t computeTPM(TPM *tpm)
{
    for (int i = 0; i < K; i++)
    {
        int sum = 0;

        for (int j = 0; j < N; j++)
            sum += tpm->w[i][j] * X[i][j];

        tpm->sigma[i] = sign(sum);
    }

    tpm->tau = 1;

    for (int i = 0; i < K; i++)
        tpm->tau *= tpm->sigma[i];

    return tpm->tau;
}

void updateWeights(TPM *tpm)
{
    for (int i = 0; i < K; i++)
    {
        if (tpm->sigma[i] == tpm->tau)
        {
            for (int j = 0; j < N; j++)
            {
                tpm->w[i][j] += X[i][j];

                if (tpm->w[i][j] > L)
                    tpm->w[i][j] = L;
                if (tpm->w[i][j] < -L)
                    tpm->w[i][j] = -L;
            }
        }
    }
}

void sendTau(uint16_t round, int8_t tau)
{
    LoRa.beginPacket();
    LoRa.write(MSG_TAU);
    LoRa.write((round >> 8) & 0xFF);
    LoRa.write(round & 0xFF);
    LoRa.write((uint8_t)tau);
    LoRa.endPacket();
}

bool receivePacket(uint8_t *type, uint16_t *round, int8_t *tau)
{
    int packetSize = LoRa.parsePacket();
    if (!packetSize)
        return false;

    *type = LoRa.read();

    if (*type == MSG_TAU)
    {
        uint8_t r1 = LoRa.read();
        uint8_t r2 = LoRa.read();
        *round = (r1 << 8) | r2;
        *tau = (int8_t)LoRa.read();
    }

    return true;
}
void printWeights(TPM *tpm)
{
    Serial.println("Final TPM Weights:");

    for (int i = 0; i < K; i++)
    {
        Serial.print("Neuron ");
        Serial.print(i);
        Serial.print(": ");

        for (int j = 0; j < N; j++)
        {
            Serial.print(tpm->w[i][j]);
            Serial.print(" ");
        }

        Serial.println();
    }

    Serial.println("------");
}
void printKey(TPM *tpm)
{
    uint8_t hash[32];

    mbedtls_sha256((uint8_t *)tpm->w, sizeof(tpm->w), hash, 0);

    Serial.print("KEY: ");
    for (int i = 0; i < 32; i++)
    {
        Serial.printf("%02x", hash[i]);
    }
    Serial.println();
}
void setup()
{
    Serial.begin(115200);

    randomSeed(esp_random());

    LoRa.setPins(ss, rst, dio0);

    while (!LoRa.begin(433E6))
    {
        Serial.println("LoRa init failed");
        delay(500);
    }

    LoRa.setSyncWord(0xA5);

    Serial.println("LoRa ready");

    initTPM(&tpm);

    Serial.println("TPM initialized");
}

void loop()
{
    uint8_t type;
    uint16_t round_remote;
    int8_t tau_remote;

    if (receivePacket(&type, &round_remote, &tau_remote))
    {
        // ===== SYNC DONE MESSAGE =====
        if (type == MSG_SYNC_DONE)
        {
            Serial.println("SYNC CONFIRMED → SECURE MODE");

            printWeights(&tpm);
            printKey(&tpm);

            currentMode = SECURE_MODE;
            return;
        }

        // ===== NORMAL SYNC =====
        if (type == MSG_TAU && currentMode == SYNC_MODE)
        {
            round_id = round_remote;

            generateInput(round_id);

            int8_t tau_local = computeTPM(&tpm);

            Serial.print("Round ");
            Serial.print(round_id);
            Serial.print(" remote=");
            Serial.print(tau_remote);
            Serial.print(" local=");
            Serial.println(tau_local);

            if (tau_local == tau_remote)
            {
                updateWeights(&tpm);
                match_counter++;
            }
            else
            {
                match_counter = 0;
            }

            if (match_counter > 50)
            {
                Serial.println("Sync condition met (waiting confirmation)");
            }

            sendTau(round_id, tau_local);
        }

        // ===== SECURE MODE =====
        else if (type == MSG_DATA && currentMode == SECURE_MODE)
        {
            Serial.println("Encrypted data received (future step)");
        }
    }
}