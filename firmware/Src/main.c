/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2020 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "si5351.h"
#include "lcd.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define CH_CW   0
#define CH_CAL  1
#define CH_VFO  2

#define LOOP_DELAY 5

const int32_t CWFilterCenterFrequency = 8998300;
const int32_t SSBFilterLowFrequency  = 8997600;
const int32_t SSBFilterHighFrequency = 9000670;

const int32_t si5351_correction = 11014;
const si5351DriveStrength_t VFODriveStrength = SI5351_DRIVE_STRENGTH_4MA;

/* Adjust for used rotary encoders */
#define MAIN_DELTA_DIV 20
#define MAIN_DELTA_MULT 1
#define CLAR_DELTA_DIV 1
#define CLAR_DELTA_MULT -1

int32_t Fvfo = 1000000; // dummy value, will be changed below

const uint8_t eeprom_i2c_addr = (0x50 << 1);
#define EEPROM_PAGE_SIZE 32
#define KEYER_CONFIG_EEPROM_ADDR        0x0000
#define KEYER_MESSAGE_EEPROM_ADDR       0x0400

typedef enum {
    CLAR_MODE_DISABLED = 0,
    CLAR_MODE_RIT = 1,
    CLAR_MODE_XIT = 2,
} ClarMode_t;

bool lockMode = false;
bool fastMode = false;
ClarMode_t clarMode = CLAR_MODE_DISABLED;

int32_t clarOffset = 0;

typedef enum {
    CW_SEND_NONE = 0,
    CW_SEND_DIT = 1,
    CW_SEND_DAH = 2,
} CwSend_t;

CwSend_t lastSent = CW_SEND_NONE;
CwSend_t nextSend = CW_SEND_NONE;

typedef enum {
    KEYER_SETINGS_PAGE_SPEED = 0,
    KEYER_SETINGS_PAGE_SAVE = 1,
    KEYER_SETINGS_PAGE_PLAY = 2,
} KeyerSettingsPage_t;

typedef struct {
    uint16_t checksum;
    bool straightKey;
    int32_t speedWPM;
    int32_t ditTimeMs;
    KeyerSettingsPage_t settingsPage;
} KeyerConfig_t;

KeyerConfig_t keyerConfig = {
    .checksum = 0,
    .straightKey = false,
    .speedWPM = 19,

    // ditTimeMs = 60*1000/(50*WPM)
    // where 50 is the length in dits of "PARIS "
    // see https://morsecode.world/international/timing.html
    .ditTimeMs = 63,
    .settingsPage = KEYER_SETINGS_PAGE_SPEED,
};

#define MAX_KEYER_MESSAGE_LENGTH 500

typedef struct {
    uint16_t checksum;
    uint16_t length;
    char message[MAX_KEYER_MESSAGE_LENGTH];
} KeyerMessage_t;

KeyerMessage_t keyerMessage = {
    .checksum = 0,
    .length = 0,
};

bool inTransmitMode = false;
bool transmittingSSB = false;

#define BUTTON_DEBOUNCE_TIME_MS 200
typedef enum {
    BUTTON_STATUS_PRESSED = 0,
    BUTTON_STATUS_RELEASED = 1,
    BUTTON_STATUS_DEBOUNCE = 2,
} ButtonStatus_t;

typedef enum {
    USE_LPF_80 = 0,
    USE_LPF_40 = 1,
    USE_LPF_20_30 = 2,
    USE_LPF_15_17 = 3,
    USE_LPF_10_12 = 4,
} UseLPF_t;

typedef enum {
    USE_BPF_80 = 0,
    USE_BPF_40 = 1,
    USE_BPF_30 = 2,
    USE_BPF_20 = 3,
    USE_BPF_17 = 4,
    USE_BPF_15 = 5,
    USE_BPF_12 = 6,
    USE_BPF_10 = 7,
} UseBPF_t;

#define FAST_FREQUENCIES_NUMBER 5

typedef struct {
    int32_t minFreq;
    int32_t maxFreq;
    int32_t lastFreq;
    /* The array of FAST frequencies should be sorted */
    int32_t fastFreqs[FAST_FREQUENCIES_NUMBER];
    UseLPF_t lpf;
    UseBPF_t bpf;
    /*
     * Compensate for losses in RG-174, frequency response of the given
     * PA depending on the used transistor, imperfections of the LPFs, etc.
     * This works only for CW.
     */
    si5351DriveStrength_t txDriveStrength;
} BandInfo_t;

int32_t currentBand = 3; // default: 20m

BandInfo_t bands[] = {
    {
        .minFreq  = 3500000,
        .maxFreq  = 3800000,
        .lastFreq = 3560000,
        .fastFreqs = { 3500000, 3560000, 3570000, 3600000, 3800000 },
        .lpf = USE_LPF_80,
        .bpf = USE_BPF_80,
        .txDriveStrength = SI5351_DRIVE_STRENGTH_2MA,
    },
    {
        .minFreq  = 7000000,
        .maxFreq  = 7200000,
        .lastFreq = 7030000,
        .fastFreqs = { 7000000, 7030000, 7040000, 7050000, 7200000 },
        .lpf = USE_LPF_40,
        .bpf = USE_BPF_40,
        .txDriveStrength = SI5351_DRIVE_STRENGTH_2MA,
    },
    {
        .minFreq  = 10100000,
        .maxFreq  = 10150000,
        .lastFreq = 10116000,
        .fastFreqs = { 10100000, 10116000, 10130000, -1, -1 },
        .lpf = USE_LPF_20_30,
        .bpf = USE_BPF_30,
        .txDriveStrength = SI5351_DRIVE_STRENGTH_2MA,
    },
    {
        .minFreq  = 14000000,
        .maxFreq  = 14350000,
        .lastFreq = 14060000,
        .fastFreqs = { 14000000, 14060000, 14070000, 14101000, 14350000 },
        .lpf = USE_LPF_20_30,
        .bpf = USE_BPF_20,
        .txDriveStrength = SI5351_DRIVE_STRENGTH_2MA,
    },
    {
        .minFreq  = 18068000,
        .maxFreq  = 18168000,
        .lastFreq = 18086000,
        .fastFreqs = { 18068000, 18086000, 18095000, 18111000, 18168000 },
        .lpf = USE_LPF_15_17,
        .bpf = USE_BPF_17,
        .txDriveStrength = SI5351_DRIVE_STRENGTH_4MA,
    },
    {
        .minFreq  = 21000000,
        .maxFreq  = 21450000,
        .lastFreq = 21060000,
        .fastFreqs = { 21000000, 21060000, 21070000, 21151000, 21450000 },
        .lpf = USE_LPF_15_17,
        .bpf = USE_BPF_15,
        .txDriveStrength = SI5351_DRIVE_STRENGTH_8MA,
    },
    {
        .minFreq  = 24890000,
        .maxFreq  = 24990000,
        .lastFreq = 24906000,
        .fastFreqs = { 24890000, 24906000, 24915000, 24931000, 24990000 },
        .lpf = USE_LPF_10_12,
        .bpf = USE_BPF_12,
        .txDriveStrength = SI5351_DRIVE_STRENGTH_8MA,
    },
    {
        .minFreq  = 28000000,
        .maxFreq  = 29700000,
        .lastFreq = 28060000,
        .fastFreqs = { 28000000, 28060000, 28070000, 28225000, 29000000 },
        .lpf = USE_LPF_10_12,
        .bpf = USE_BPF_10,
        .txDriveStrength = SI5351_DRIVE_STRENGTH_8MA,
    },
};

void ADC_Select_Channel(uint32_t ch) {
    ADC_ChannelConfTypeDef conf = {
        .Channel = ch,
        .Rank = 1,
        .SamplingTime = ADC_SAMPLETIME_28CYCLES_5,
    };
    if (HAL_ADC_ConfigChannel(&hadc1, &conf) != HAL_OK) {
        Error_Handler();
    }

    /*
     * Without this delay the next call of ADC_ReadVoltage()
     * may get a value from the previously used channel.
     * As a result updateSWRMeter() will see equal v_fwd
     * and v_ref, which results in a false high SWR value.
     * So keep the delay!
     */
    HAL_Delay(1);
} 

double ADC_ReadVoltage(uint32_t ch) {
    ADC_Select_Channel(ch);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return ((double)HAL_ADC_GetValue(&hadc1))*3.3/4096.0;
}

// http://en.wikipedia.org/wiki/Jenkins_hash_function
uint32_t jenkinsHash(const uint8_t *data, const size_t len) {
    uint32_t hash, i;
    for(hash = i = 0; i < len; ++i) {
        hash += data[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

void loadKeyerMessage() {
    HAL_StatusTypeDef status;
    uint16_t savedChecksum;
    KeyerMessage_t savedKeyerMessage;

    for(;;) {
        status = HAL_I2C_IsDeviceReady(&hi2c1, eeprom_i2c_addr, 3, HAL_MAX_DELAY);
        if(status == HAL_OK)
            break;
    }

    HAL_I2C_Mem_Read(&hi2c1, eeprom_i2c_addr, KEYER_MESSAGE_EEPROM_ADDR, I2C_MEMADD_SIZE_16BIT,
            (uint8_t*)&savedKeyerMessage, sizeof(savedKeyerMessage), HAL_MAX_DELAY);

    savedChecksum = savedKeyerMessage.checksum;
    savedKeyerMessage.checksum = 0;

    if((jenkinsHash((const uint8_t*)&savedKeyerMessage, sizeof(savedKeyerMessage)) & 0xFFFF) == savedChecksum) {
        keyerMessage = savedKeyerMessage;
    } else {
        LCD_Goto(0, 0);
        LCD_SendString("MSG HASH");
        LCD_Goto(1, 0);
        LCD_SendString(" ERROR! ");
        HAL_Delay(2000);
        LCD_Clear();
    }
}

void saveKeyerMessage() {
    HAL_StatusTypeDef status;
    uint16_t checksum;
    size_t bytes_to_write = sizeof(keyerMessage);
    uint8_t* struct_offset = (uint8_t*)&keyerMessage;
    uint16_t eeprom_addr = KEYER_MESSAGE_EEPROM_ADDR;

    keyerMessage.checksum = 0;
    checksum = (jenkinsHash((const uint8_t*)&keyerMessage, sizeof(keyerMessage)) & 0xFFFF);
    keyerMessage.checksum = checksum;

    // sizeof(keyerMessage) exceeds EEPROM_PAGE_SIZE
    // so we have to write it in batches
    while(bytes_to_write > 0) {
        size_t batch_size = bytes_to_write;
        if(batch_size > EEPROM_PAGE_SIZE) {
            batch_size = EEPROM_PAGE_SIZE;
        }

        for(;;) {
            status = HAL_I2C_IsDeviceReady(&hi2c1, eeprom_i2c_addr, 3, HAL_MAX_DELAY);
            if(status == HAL_OK)
                break;
        }

        HAL_I2C_Mem_Write(&hi2c1, eeprom_i2c_addr, eeprom_addr, I2C_MEMADD_SIZE_16BIT,
            struct_offset, batch_size, HAL_MAX_DELAY);

        bytes_to_write -= batch_size;
        struct_offset += batch_size;
        eeprom_addr += batch_size;
    }
}

void loadKeyerConfig() {
    HAL_StatusTypeDef status;
    uint16_t savedChecksum;
    KeyerConfig_t savedKeyerConfig;

    for(;;) {
        status = HAL_I2C_IsDeviceReady(&hi2c1, eeprom_i2c_addr, 3, HAL_MAX_DELAY);
        if(status == HAL_OK)
            break;
    }

    HAL_I2C_Mem_Read(&hi2c1, eeprom_i2c_addr, KEYER_CONFIG_EEPROM_ADDR, I2C_MEMADD_SIZE_16BIT,
        (uint8_t*)&savedKeyerConfig, sizeof(savedKeyerConfig), HAL_MAX_DELAY);

    savedChecksum = savedKeyerConfig.checksum;
    savedKeyerConfig.checksum = 0;

    if((jenkinsHash((const uint8_t*)&savedKeyerConfig, sizeof(savedKeyerConfig)) & 0xFFFF) == savedChecksum) {
        keyerConfig = savedKeyerConfig;
    } else {
        LCD_Goto(0, 0);
        LCD_SendString("CFG HASH");
        LCD_Goto(1, 0);
        LCD_SendString(" ERROR! ");
        HAL_Delay(2000);
        LCD_Clear();
    }
}

void saveKeyerConfig() {
    HAL_StatusTypeDef status;
    uint16_t checksum;
    keyerConfig.checksum = 0;

    checksum = (jenkinsHash((const uint8_t*)&keyerConfig, sizeof(keyerConfig)) & 0xFFFF);
    keyerConfig.checksum = checksum;

    for(;;) {
        status = HAL_I2C_IsDeviceReady(&hi2c1, eeprom_i2c_addr, 3, HAL_MAX_DELAY);
        if(status == HAL_OK)
            break;
    }

    HAL_I2C_Mem_Write(&hi2c1, eeprom_i2c_addr, KEYER_CONFIG_EEPROM_ADDR, I2C_MEMADD_SIZE_16BIT,
        (uint8_t*)&keyerConfig, sizeof(keyerConfig), HAL_MAX_DELAY);
}

void changeKeyerSpeed(int32_t delta) {
    keyerConfig.speedWPM += delta;
    if(keyerConfig.speedWPM < 10) {
        keyerConfig.speedWPM = 10;
        keyerConfig.straightKey = true;
        keyerConfig.ditTimeMs = 60*1000/(50*(17+1)); // as for 17 WPM, for XMIT
    } else {
        keyerConfig.straightKey = false;
        if(keyerConfig.speedWPM > 30) {
            keyerConfig.speedWPM = 30;
        }

        /* +1 compensates the signal rise/fall time */
        keyerConfig.ditTimeMs = 60*1000/(50*(keyerConfig.speedWPM + 1));
    }
}

void SetupCLK(uint8_t output, int32_t Fclk, si5351DriveStrength_t driveStrength) {
    static bool pllSetupDone = false;
    si5351PLLConfig_t pll_conf;
    si5351OutputConfig_t out_conf;

    si5351_Calc(Fclk, &pll_conf, &out_conf);
    if(!pllSetupDone) {
        si5351_SetupPLL(SI5351_PLL_A, &pll_conf);
        pllSetupDone = true;
    }
    si5351_SetupOutput(output, SI5351_PLL_A, driveStrength, &out_conf);
}

void enableTx(bool enable) {
    // ENABLE_TX_12V
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void switchBFO(bool enable) {
    /*
     *  enable - use high frequency BFO, e.g. 9_000_633
     * !enable - use  low frequency BFO, e.g. 8_997_600
     */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool enabledSSBMode() {
    /*
     * B4 is low  - SSB mode
     * B4 is high -  CW mode
     */
    return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET);
}

void changeFrequency(int32_t delta, bool tx, bool force) {
    static int32_t prevFvfo = 0;

    if((!force) && lockMode) {
        return;
    }

    if(fastMode) {
        delta *= 10;
    }
    bands[currentBand].lastFreq += 100*delta;
    if(fastMode && (delta != 0)) {
        bands[currentBand].lastFreq -= bands[currentBand].lastFreq % 1000;
    }

    if(bands[currentBand].lastFreq < bands[currentBand].minFreq) {
        bands[currentBand].lastFreq = bands[currentBand].minFreq;
    } else if(bands[currentBand].lastFreq > bands[currentBand].maxFreq) {
        bands[currentBand].lastFreq = bands[currentBand].maxFreq;
    }

    if(enabledSSBMode()) {
        if((currentBand == 0) || (currentBand == 1)) {
            /* Low frequency BFO is used on 80 and 40 meters */
            Fvfo = bands[currentBand].lastFreq + SSBFilterLowFrequency;
        } else {
            Fvfo = bands[currentBand].lastFreq + SSBFilterHighFrequency;
        }
    } else {
        // this path is executed only when receiving CW
        Fvfo = bands[currentBand].lastFreq + CWFilterCenterFrequency;
    }

    if(tx) {
        // this path is executed only when transmitting SSB
        if(clarMode == CLAR_MODE_XIT) {
            Fvfo += clarOffset;
        }
    } else {
        if(clarMode == CLAR_MODE_RIT) {
            Fvfo += clarOffset;
        }
    }

    if(force || (Fvfo != prevFvfo)) {
        SetupCLK(CH_VFO, Fvfo, VFODriveStrength);
    }

    prevFvfo = Fvfo;
}

void checkIfRxModeHasChanged() {
    static bool firstCall = true;
    static bool prevBFOSwitch = false;
    static bool prevSSBMode = false;
    
    bool ssbMode = enabledSSBMode();

    /*
     * use high frequency BFO (i.e. spectrum inversion)
     * for SSB on bands other than 80 and 40 meters
     */
    bool bfoSwitch = ssbMode && (currentBand != 0) && (currentBand != 1);

    if((prevBFOSwitch != bfoSwitch) || firstCall) {
        switchBFO(bfoSwitch);
        prevBFOSwitch = bfoSwitch;
    }

    if((prevSSBMode != ssbMode) || firstCall) {
        changeFrequency(0, false, true);
        // ENABLE_SSB_12V
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, ssbMode ? GPIO_PIN_SET : GPIO_PIN_RESET);
        prevSSBMode = ssbMode;
    }

    firstCall = false;
}

void displayFrequency() {
    char buff[16];
    snprintf(buff, sizeof(buff), "%02ld.%03ld.%01ld",
        bands[currentBand].lastFreq / 1000000,
        (bands[currentBand].lastFreq % 1000000) / 1000,
        (bands[currentBand].lastFreq % 1000) / 100);

    LCD_Goto(0, 0);
    LCD_SendString(buff);
}

bool buttonDitPressed() {
    return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET);
}

bool buttonDahPressed() {
    return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET);
}

void displaySMeterOrMode(bool force) {
    static const char* prevSValue = NULL;
    static uint32_t lastSMeterUpdateTime = 0;
    // don't forget to add '-u _printf_float' to LDFLAGS
    char buff[16];

    if(force) {
        LCD_Goto(1, 0);
        if(fastMode) {
            LCD_SendString("= FAST =");
            return;
        } else if(lockMode) {
            LCD_SendString("= LOCK =");
            return;
        } else if(clarMode != CLAR_MODE_DISABLED) {
            snprintf(buff, sizeof(buff), "%s%s%04d",
                clarMode == CLAR_MODE_RIT ? "RIT" : "XIT",
                clarOffset < 0 ? "-" : "+",
                abs(clarOffset));
            LCD_SendString(buff);
            return;
        }
    }

    uint32_t tstamp = HAL_GetTick();
    if(force || (tstamp - lastSMeterUpdateTime > 500)) {
        const char* sValue = NULL;
        double voltage = ADC_ReadVoltage(ADC_CHANNEL_2);

        if(voltage <= 0.035) {
            sValue = "S0      ";
        } else if(voltage <= 0.038) {
            sValue = "S1      ";
        } else if(voltage <= 0.043) {
            sValue = "S2\x02     ";
        } else if(voltage <= 0.048) {
            sValue = "S3\x02     ";
        } else if(voltage <= 0.080) {
            sValue = "S4\x02\x03    ";
        } else if(voltage <= 0.240) {
            sValue = "S5\x02\x03    ";
        } else if(voltage <= 0.700) {
            sValue = "S6\x02\x03\x04   ";
        } else if(voltage <= 1.750) {
            sValue = "S7\x02\x03\x04\x05  ";
        } else if(voltage <= 2.400) {
            sValue = "S8\x02\x03\x04\x05\x06 ";
        } else if(voltage <= 2.500) {
            sValue = "S9\x02\x03\x04\x05\x06\x07";
        } else {
            sValue = "S+\x02\x03\x04\x05\x06\x07";
        }

        if(force || (prevSValue != sValue)) {
            // Don't re-render the S-meter if we are about to transmit
            if(!buttonDitPressed() && !buttonDahPressed()) {
                LCD_Goto(1, 0);
                LCD_SendString(sValue);
            }
        }

        prevSValue = sValue;
        lastSMeterUpdateTime = tstamp;
    }
}

void keyDown() {
    if(inTransmitMode) {
        // enable CH_CW
        si5351_EnableOutputs(1 << CH_CW);

        // ENABLE_KEYED_VCC
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
    }

    // CW tone ON
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

void keyUp() {
    if(inTransmitMode) {
        // disable CH_CW, but only during actual TX
        // e.g. not during playbackSavedMessage()
        si5351_EnableOutputs(0);
    }

    // make sure ENABLE_KEYED_VCC is low regardless of inTransmitMode value
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);

    // CW tone OFF
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}

void switchLPFs(UseLPF_t lpf) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, lpf == USE_LPF_10_12 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, lpf == USE_LPF_15_17 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, lpf == USE_LPF_20_30 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, lpf == USE_LPF_40 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, lpf == USE_LPF_80 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void switchBPFs(UseBPF_t bpf) {
    switch(bpf) {
    case USE_BPF_10:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
        break;
    case USE_BPF_12:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
        break;
    case USE_BPF_15:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
        break;
    case USE_BPF_17:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
        break;
    case USE_BPF_20:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
        break;
    case USE_BPF_30:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
        break;
    case USE_BPF_40:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
        break;
    default: // USE_BPF_80
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
        break;
    }
}

void changeBand(int32_t delta) {
    currentBand += delta;
    while(currentBand < 0) {
        currentBand += sizeof(bands)/sizeof(bands[0]);
    }
    currentBand %= sizeof(bands)/sizeof(bands[0]);

    switchBPFs(bands[currentBand].bpf);
    switchLPFs(bands[currentBand].lpf);

    changeFrequency(0, false, false);
    si5351_EnableOutputs(1 << CH_VFO);
    displayFrequency();
}

void ensureTransmitMode() {
    if(inTransmitMode) {
        return;
    }

    if(transmittingSSB) {
        // changes VFO frequency accordingly
        changeFrequency(0, true, true);

        // ENABLE_KEYED_VCC
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
    } else {
        int32_t targetFrequency = bands[currentBand].lastFreq;
        if(clarMode == CLAR_MODE_XIT) {
            targetFrequency += clarOffset;
        }

        SetupCLK(CH_CW, targetFrequency, bands[currentBand].txDriveStrength);
        keyUp(); // calls si5351_EnableOutputs()
    }

    enableTx(true);

    LCD_Goto(1, 0);
    LCD_SendString("SWR  ---");

    inTransmitMode = true;
}

void ensureReceiveMode() {
    if(!inTransmitMode) {
        return;
    }

    keyUp(); // ENABLE_KEYED_VCC = LOW, etc
    enableTx(false);

    // Restore the original VFO
    changeFrequency(0, false, true);
    si5351_EnableOutputs(1 << CH_VFO);
    displaySMeterOrMode(true);
    inTransmitMode = false;

    /*
     * As it turned out, the receiver is sensitive enough to receive
     * signals from the configured channels even when the channels are
     * disabled. For this reason here the channels are configured so that
     * the signals and their harmonics are above anything we may possible
     * receive. Doing it for both CH_CW and CH_CAL, just to play it safe.
     */
    SetupCLK(CH_CW,  80000000, SI5351_DRIVE_STRENGTH_2MA);
    SetupCLK(CH_CAL, 80000000, SI5351_DRIVE_STRENGTH_2MA);
}

ButtonStatus_t buttonPressed(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t* lastPressed) {
    if(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_RESET) {
        uint32_t now = HAL_GetTick();
        if(now - *lastPressed > BUTTON_DEBOUNCE_TIME_MS) {
            *lastPressed = now;
            return BUTTON_STATUS_PRESSED;
        } else {
            return BUTTON_STATUS_DEBOUNCE;
        }
    }
    return BUTTON_STATUS_RELEASED;
}

ButtonStatus_t buttonKeyerPressed() {
    static uint32_t lastPressed = 0;
    return buttonPressed(GPIOA, GPIO_PIN_10, &lastPressed);
}

ButtonStatus_t buttonXmitPressed() {
    static uint32_t lastPressed = 0;
    return buttonPressed(GPIOA, GPIO_PIN_11, &lastPressed);
}

ButtonStatus_t buttonNextPressed() {
    static uint32_t lastPressed = 0;
    return buttonPressed(GPIOA, GPIO_PIN_12, &lastPressed);
}

ButtonStatus_t buttonClarPressed() {
    static uint32_t lastPressed = 0;
    return buttonPressed(GPIOB, GPIO_PIN_0, &lastPressed);
}

ButtonStatus_t buttonPrevPressed() {
    static uint32_t lastPressed = 0;
    return buttonPressed(GPIOB, GPIO_PIN_1, &lastPressed);
}

ButtonStatus_t buttonFastPressed() {
    static uint32_t lastPressed = 0;
    return buttonPressed(GPIOB, GPIO_PIN_10, &lastPressed);
}

ButtonStatus_t buttonLockPressed() {
    static uint32_t lastPressed = 0;
    return buttonPressed(GPIOB, GPIO_PIN_11, &lastPressed);
}

// `straightKeyerKeyIsDown` is used to avoid calling keyUp()/keyDown() if we did it already.
// Calling HAL_GPIO_WritePin() too often generates a hearable noise.
bool straightKeyerKeyIsDown;

void initStraightKeyer() {
    straightKeyerKeyIsDown = false;
}

void processStraightKeyerLogic(bool pressed) {
    if(pressed) {
        if(!straightKeyerKeyIsDown) {
            keyDown();
            straightKeyerKeyIsDown = true;
        }
    } else if(straightKeyerKeyIsDown) {
        keyUp();
        straightKeyerKeyIsDown = false;
    }
}

void initIambicKeyer() {
    // iambic keyer has nothing to init
}

static size_t iambicKeyerLogOffset = 0;
static char iambicKeyerLog[MAX_KEYER_MESSAGE_LENGTH] = {0};

void iambicKeyerLogReset() {
    iambicKeyerLogOffset = 0;
}

size_t iambicKeyerLogSize() {
    return iambicKeyerLogOffset;
}

size_t iambicKeyerLogSpaceLeft() {
    return MAX_KEYER_MESSAGE_LENGTH - iambicKeyerLogOffset;
}

// Iambic keyer logs up to MAX_KEYER_MESSAGE_LENGTH symbols.
// '.' is a dit
// '-' is a dah
// ' ' is a pause between letters
// '/' is a pause between words
// Multiple pauses between letters are logged as one pause.
// Multiple pauses between words are also logged as one pause.
// Pauses in the beginning of the message are not logged.
void iambicKeyerLogEmit(char c) {
    if(iambicKeyerLogSpaceLeft() == 0) {
        // truncate the message if there is not space left in the buffer
        return;
    }

    if(((c == ' ') || (c == '/')) && (iambicKeyerLogOffset == 0)) {
        // don't log the pauses in the beginning of the message
        return;
    }

    if((c == ' ') && iambicKeyerLog[iambicKeyerLogOffset-1] == ' ') {
        // multiple pauses between letters are logged as one pause
        return;
    }

    if((c == '/') && iambicKeyerLog[iambicKeyerLogOffset-1] == '/') {
        // multiple pauses between words are logged as one pause.
        return;
    }

    if((c == '/') && iambicKeyerLog[iambicKeyerLogOffset-1] == ' ') {
        // previously logged pause between letters became a pause between words
        iambicKeyerLog[iambicKeyerLogOffset-1] = '/';
        return;
    }

    if((c == ' ') && iambicKeyerLog[iambicKeyerLogOffset-1] == '/') {
        // to my knowledge this should never happen...
        return;
    }

    // log the symbol
    iambicKeyerLog[iambicKeyerLogOffset] = c;
    iambicKeyerLogOffset++;
}

void processIambicKeyerLogic(bool ditPressed, bool dahPressed) {
    static int32_t sendFinish = 0;
    static int32_t keyReadTimeout = 0;
    static int32_t sendTimeout = 0;
    static bool isSending = false;

    int32_t now = HAL_GetTick();

    if(isSending && (now >= sendFinish)) {
        keyUp();
        isSending = false;
    } else if(!isSending && (now - sendFinish) > keyerConfig.ditTimeMs*5) {
        // multiple pauses between words will be logged as one pause
        // pauses in the beginning of the message will be ignored
        iambicKeyerLogEmit('/');
    } else if(!isSending && (now - sendFinish) > keyerConfig.ditTimeMs*2) {
        // multiple pauses between letters will be logged as one pause
        // pauses in the beginning of the message will be ignored
        iambicKeyerLogEmit(' ');
    }

    if(nextSend == CW_SEND_NONE) {
        if(ditPressed && (lastSent == CW_SEND_DAH)) {
            nextSend = CW_SEND_DIT;
        } else if(dahPressed && (lastSent == CW_SEND_DIT)) {
            nextSend = CW_SEND_DAH;
        } else if(now > keyReadTimeout) {
            if(ditPressed) {
                nextSend = CW_SEND_DIT;
            } else if (dahPressed) {
                nextSend = CW_SEND_DAH;
            }
        }
    }

    if((now > sendTimeout) && (nextSend != CW_SEND_NONE)) {
        if(nextSend == CW_SEND_DIT) {
            iambicKeyerLogEmit('.');
            sendFinish = now + keyerConfig.ditTimeMs;
            keyReadTimeout = now + keyerConfig.ditTimeMs*2;
        } else if(nextSend == CW_SEND_DAH) {
            iambicKeyerLogEmit('-');
            sendFinish = now + keyerConfig.ditTimeMs*3;
            keyReadTimeout = now + keyerConfig.ditTimeMs*3;
        }

        lastSent = nextSend;
        nextSend = CW_SEND_NONE;
        sendTimeout = sendFinish + keyerConfig.ditTimeMs;
        isSending = true;
        keyDown();
    }
}

int32_t getDelta(TIM_HandleTypeDef* htim, int32_t *prevCounter, int32_t mult, int32_t div) {
    int32_t currCounter = __HAL_TIM_GET_COUNTER(htim);
    currCounter = mult*(currCounter / div);
    currCounter = 32767 - ((currCounter-1) & 0xFFFF) / 2;
    if(currCounter > 32768/2) {
        // convert ... 32766, 32767, 0, 1, 2 ... into:
        //               ... -2, -1, 0, 1, 2 ...
        // this simplifies `delta` calculation
        currCounter = currCounter-32768;
    }

    if(currCounter != *prevCounter) {
        int32_t delta = *prevCounter-currCounter;
        *prevCounter = currCounter;

        // debounce or skip counter overflow
        if((delta > -10) && (delta < 10)) {
            return delta;
        }
    }

    return 0;
}

void displayKeyerSpeedSettings() {
    char buff[16];
    if(keyerConfig.straightKey) {
        LCD_Goto(0, 0);
        LCD_SendString("SPEED --");
        LCD_Goto(1, 0);
        LCD_SendString("STRAIGHT");
    } else {
        snprintf(buff, sizeof(buff), "SPEED %02ld", keyerConfig.speedWPM);
        LCD_Goto(0, 0);
        LCD_SendString(buff);
        LCD_Goto(1, 0);
        LCD_SendString("IAMBIC  ");
    }
}

void displayKeyerSaveSettings() {
    LCD_Goto(0, 0);
    LCD_SendString("SAVE MSG");
    LCD_Goto(1, 0);
    LCD_SendString("CFM=LOCK");
}

void displayKeyerPlaySettings() {
    LCD_Goto(0, 0);
    LCD_SendString("PLAY MSG");
    LCD_Goto(1, 0);
    LCD_SendString("CFM=LOCK");  
}

bool anyButtonPressed(bool ignore_mode) {
    return (buttonNextPressed() == BUTTON_STATUS_PRESSED) ||
        (buttonFastPressed() == BUTTON_STATUS_PRESSED) ||
        (buttonLockPressed() == BUTTON_STATUS_PRESSED) ||
        (buttonPrevPressed() == BUTTON_STATUS_PRESSED) ||
        (buttonXmitPressed() == BUTTON_STATUS_PRESSED) ||
        (buttonKeyerPressed() == BUTTON_STATUS_PRESSED) ||
        (buttonClarPressed() == BUTTON_STATUS_PRESSED) ||
        buttonDitPressed() || buttonDahPressed() ||
        ((!ignore_mode) && (transmittingSSB != enabledSSBMode()));
}

uint32_t lastSWRCheckTime = 0;
double lastSWRValue = 0.0;

void resetSWRMeter() {
    lastSWRCheckTime = 0;
    lastSWRValue = 0.0;
}

void updateSWRMeter() {
    double v_fwd, v_ref, ratio, swr;
    uint32_t tstamp = HAL_GetTick();
    if(tstamp - lastSWRCheckTime < 100) {
        return;
    }

    v_fwd = ADC_ReadVoltage(ADC_CHANNEL_0);
    if(v_fwd < 0.2) {
        /* not transmitting */
        return;
    }

    lastSWRCheckTime = tstamp;
    v_ref = ADC_ReadVoltage(ADC_CHANNEL_1);
    if(v_ref < 0.2) {
        v_ref = 0.0;
    }

    if(v_ref > v_fwd) {
        swr = 25.0;
    } else {
        ratio = v_ref / v_fwd;
        swr = (1+ratio)/(1-ratio);
    }

    if(fabs(lastSWRValue - swr) <= 0.2) {
        return;
    }

    LCD_Goto(1, 5);
    if(swr >= 10.0) {
        LCD_SendString("10+");
    } else {
        char buff[8];
        snprintf(buff, sizeof(buff), "%.1f", swr);
        LCD_SendString(buff);
    }

    lastSWRValue = swr;
}

void playbackSavedMessage(bool renderCounter, bool renderSWRMeter) {
    int32_t sendStart = 0;
    int32_t sendFinish = 0;
    bool isSending = false;
    int32_t now;
    size_t symbolsLeft = keyerMessage.length;
    size_t currentSymbolIdx = 0;
    char c;

    /* This ensures that anyButtonPressed() will work properly */
    transmittingSSB = false;

    if(renderCounter) {
        char buff[16];
        snprintf(buff, sizeof(buff), "%03d", symbolsLeft);
        LCD_Goto(0, 5);
        LCD_SendString(buff);
    }

    while(symbolsLeft > 0) {
        now = HAL_GetTick();

        if((!isSending) && (now >= sendStart)) {
            // time to send the next symbol
            c = keyerMessage.message[currentSymbolIdx];
            isSending = true;

            if(c == '/') {
                // Pause between words.
                // We already waited for ditTimeMs, thus -1.
                // Act as if we are sending another symbol,
                // although keyDown() is not called.
                sendFinish = now + keyerConfig.ditTimeMs*(7-1);
            } else if(c == ' ') {
                // Pause between letters.
                // We already waited for ditTimeMs, thus -1.
                // Act as if we are sending another symbol,
                // although keyDown() is not called.
                sendFinish = now + keyerConfig.ditTimeMs*(3-1);
            } else if(c == '-') {
                keyDown();
                sendFinish = now + keyerConfig.ditTimeMs*3;
            } else { // c == '.'
                keyDown();
                sendFinish = now + keyerConfig.ditTimeMs;
            }
        }

        if(isSending && (now >= sendFinish)) {
            // another symbol was sent
            keyUp();
            symbolsLeft--;
            currentSymbolIdx++;
            isSending = false;

            // `ditTimeMs` is a pause between dits and dahs
            sendStart = now + keyerConfig.ditTimeMs;

            if(renderCounter) {
                char buff[16];
                snprintf(buff, sizeof(buff), "%03d", symbolsLeft);
                LCD_Goto(0, 5);
                LCD_SendString(buff);
            }
        }

        if(anyButtonPressed(false)) {
            break;
        }

        if(renderSWRMeter) {
            updateSWRMeter();
        }

        HAL_Delay(LOOP_DELAY);
    }

    keyUp();

    if(renderCounter) {
        LCD_Goto(0, 5);
        LCD_SendString("MSG");
    }
}

void enterKeyerSaveMode() {
    bool keyerInitDone = false;
    uint32_t keyerInitTime = 0;
    char buff[8];
    size_t spaceLeft, prevSpaceLeft;

    iambicKeyerLogReset();
    spaceLeft = iambicKeyerLogSpaceLeft();
    prevSpaceLeft = spaceLeft;

    snprintf(buff, sizeof(buff), "%03d", spaceLeft);
    LCD_Goto(0, 5);
    LCD_SendString(buff);

    for(;;) {
        bool ditPressed = buttonDitPressed();
        bool dahPressed = buttonDahPressed();

        if((ditPressed || dahPressed)) {
            keyerInitTime = HAL_GetTick();
            if(!keyerInitDone) {
                keyerInitDone = true;
                // use iambic keyer even in straight key mode
                initIambicKeyer();
            }
        } else {
            uint32_t tstamp = HAL_GetTick();
            if(tstamp - keyerInitTime > keyerConfig.ditTimeMs*8) {
                keyerInitDone = false;
            }
        }

        if(keyerInitDone) {
            processIambicKeyerLogic(ditPressed, dahPressed);
        }

        if(buttonLockPressed() == BUTTON_STATUS_PRESSED) {
            // SAVE confirmed
            keyerMessage.length = iambicKeyerLogSize();
            memcpy(keyerMessage.message, iambicKeyerLog, keyerMessage.length);
            
            // truncate the pauses in the end of the message
            while(keyerMessage.length > 0) {
                if(keyerMessage.message[keyerMessage.length-1] == ' ') {
                    keyerMessage.length--;
                    continue;
                }

                if(keyerMessage.message[keyerMessage.length-1] == '/') {
                    keyerMessage.length--;
                    continue;
                }

                break;
            }

            saveKeyerMessage();
            break;
        } else if(buttonKeyerPressed() == BUTTON_STATUS_PRESSED) {
            // SAVE aborted
            break;
        }

        spaceLeft = iambicKeyerLogSpaceLeft();
        if(spaceLeft != prevSpaceLeft) {
            // re-render
            snprintf(buff, sizeof(buff), "%03d", spaceLeft);
            LCD_Goto(0, 5);
            LCD_SendString(buff);
            prevSpaceLeft = spaceLeft;
        }

        HAL_Delay(LOOP_DELAY);
    }

    LCD_Goto(0, 5);
    LCD_SendString("MSG");
}

void loopKeyer() {
    bool keyerInitDone = false;
    uint32_t keyerInitTime = 0;
    int32_t prevCounter = 0;

    // init the last state
    if(keyerConfig.settingsPage == KEYER_SETINGS_PAGE_SPEED) {
        // reset the counter
        (void)getDelta(&htim1, &prevCounter, MAIN_DELTA_MULT, MAIN_DELTA_DIV);
        displayKeyerSpeedSettings();
    } else if(keyerConfig.settingsPage == KEYER_SETINGS_PAGE_SAVE) {
        displayKeyerSaveSettings();
    } else { // keyerConfig.settingsPage == KEYER_SETINGS_PAGE_PLAY
        displayKeyerPlaySettings();
    }

    for(;;) {
        /* process CW/SSB switch while in KEYER menu */
        checkIfRxModeHasChanged();

        if(keyerConfig.settingsPage == KEYER_SETINGS_PAGE_SPEED) {
            bool ditPressed = buttonDitPressed();
            bool dahPressed = buttonDahPressed();

            if((ditPressed || dahPressed)) {
                keyerInitTime = HAL_GetTick();
                if(!keyerInitDone) {
                    keyerInitDone = true;
                    if(keyerConfig.straightKey) {
                        initStraightKeyer();
                    } else {
                        initIambicKeyer();
                    }
                }
            } else {
                uint32_t tstamp = HAL_GetTick();
                if(tstamp - keyerInitTime > keyerConfig.ditTimeMs*8) {
                    keyerInitDone = false;
                }
            }

            if(keyerInitDone) {
                if(keyerConfig.straightKey) {
                    processStraightKeyerLogic(ditPressed);
                } else {
                    processIambicKeyerLogic(ditPressed, dahPressed);
                }
            }

            int32_t delta = getDelta(&htim1, &prevCounter, MAIN_DELTA_MULT, MAIN_DELTA_DIV);
            if(delta != 0) {
                // don't forget to re-init the keyer next time
                keyerInitDone = false;

                keyUp();
                changeKeyerSpeed(delta);

                // re-render
                displayKeyerSpeedSettings();
            }

            if(buttonNextPressed() == BUTTON_STATUS_PRESSED) {
                displayKeyerSaveSettings();
                keyerConfig.settingsPage = KEYER_SETINGS_PAGE_SAVE;
            } else if(buttonPrevPressed() == BUTTON_STATUS_PRESSED) {
                displayKeyerPlaySettings();
                keyerConfig.settingsPage = KEYER_SETINGS_PAGE_PLAY;
            } else if(buttonKeyerPressed() == BUTTON_STATUS_PRESSED) {
                break;
            }
        } else if(keyerConfig.settingsPage == KEYER_SETINGS_PAGE_SAVE) {
            if(buttonNextPressed() == BUTTON_STATUS_PRESSED) {
                displayKeyerPlaySettings();
                keyerConfig.settingsPage = KEYER_SETINGS_PAGE_PLAY;
            } else if(buttonPrevPressed() == BUTTON_STATUS_PRESSED) {
                // reset the counter
                (void)getDelta(&htim1, &prevCounter, MAIN_DELTA_MULT, MAIN_DELTA_DIV);
                displayKeyerSpeedSettings();
                keyerConfig.settingsPage = KEYER_SETINGS_PAGE_SPEED;
            } else if(buttonLockPressed() == BUTTON_STATUS_PRESSED) {
                enterKeyerSaveMode();
            } else if(buttonKeyerPressed() == BUTTON_STATUS_PRESSED) {
                break;
            }
        } else { // keyerConfig.settingsPage == KEYER_SETINGS_PAGE_PLAY
            if(buttonNextPressed() == BUTTON_STATUS_PRESSED) {
                // reset the counter
                (void)getDelta(&htim1, &prevCounter, MAIN_DELTA_MULT, MAIN_DELTA_DIV);
                displayKeyerSpeedSettings();
                keyerConfig.settingsPage = KEYER_SETINGS_PAGE_SPEED;
            } else if(buttonPrevPressed() == BUTTON_STATUS_PRESSED) {
                displayKeyerSaveSettings();
                keyerConfig.settingsPage = KEYER_SETINGS_PAGE_SAVE;
            } else if(buttonLockPressed() == BUTTON_STATUS_PRESSED) {
                playbackSavedMessage(true, false);
            } else if(buttonKeyerPressed() == BUTTON_STATUS_PRESSED) {
                break;
            }
        }

        HAL_Delay(LOOP_DELAY);
    }

    saveKeyerConfig();
    displayFrequency();
    displaySMeterOrMode(true);
}

void loopMain() {
    static int32_t prevMainCounter = 0;
    static int32_t prevClarCounter = 0;
    static uint32_t transmitModeEnterTime = 0;
    bool ditPressed = buttonDitPressed();
    bool dahPressed = buttonDahPressed();

    if(ditPressed || dahPressed) {
        transmitModeEnterTime = HAL_GetTick();
        if(!inTransmitMode) {
            /* enter transmit mode */
            transmittingSSB = enabledSSBMode();
            ensureTransmitMode();
            resetSWRMeter();

            if(!transmittingSSB) {
                if(keyerConfig.straightKey) {
                    initStraightKeyer();
                } else {
                    initIambicKeyer();
                }
            }
        }
    } else if(inTransmitMode) {
        /* exit TX mode due to inactivity? */
        uint32_t tstamp = HAL_GetTick();
        uint32_t delay = keyerConfig.straightKey ? 1000 : keyerConfig.ditTimeMs*15;

        if(transmittingSSB || (tstamp - transmitModeEnterTime > delay)) {
            ensureReceiveMode();
            /* discard any changes in counters */
            (void)getDelta(&htim1, &prevMainCounter, MAIN_DELTA_MULT, MAIN_DELTA_DIV);
            (void)getDelta(&htim2, &prevClarCounter, CLAR_DELTA_MULT, CLAR_DELTA_DIV);
        }
    }

    if(inTransmitMode) {
        if(transmittingSSB != enabledSSBMode()) {
            /*
             * Mode has changed while transmitting,
             * entering RX mode.
             */
            ensureReceiveMode();
            /* discard any changes in counters */
            (void)getDelta(&htim1, &prevMainCounter, MAIN_DELTA_MULT, MAIN_DELTA_DIV);
            (void)getDelta(&htim2, &prevClarCounter, CLAR_DELTA_MULT, CLAR_DELTA_DIV);
        } else {
            if(!transmittingSSB) {
                if(keyerConfig.straightKey) {
                    processStraightKeyerLogic(ditPressed);
                } else {
                    processIambicKeyerLogic(ditPressed, dahPressed);
                }
            }

            updateSWRMeter();
        }
    } else {
        /* do it only in RX mode */
        checkIfRxModeHasChanged();

        int32_t delta = getDelta(&htim1, &prevMainCounter, MAIN_DELTA_MULT, MAIN_DELTA_DIV);
        if(delta != 0) {
            changeFrequency(delta, false, false); // will do nothing in LOCK mode
            displayFrequency();
        }

        if((clarMode != CLAR_MODE_DISABLED) && (!fastMode) && (!lockMode)) {
            delta = getDelta(&htim2, &prevClarCounter, CLAR_DELTA_MULT, CLAR_DELTA_DIV);
            if(delta != 0) {
                clarOffset = clarOffset + delta*50;
                if(clarOffset < -5000) {
                    clarOffset = -5000;
                } else if (clarOffset > 5000) {
                    clarOffset = 5000;
                }

                if(clarMode == CLAR_MODE_RIT) {
                    changeFrequency(0, false, false);
                }
                displaySMeterOrMode(true);
            }
        }

        if((buttonLockPressed() == BUTTON_STATUS_PRESSED) && (!fastMode)) {
            lockMode = !lockMode;
            if(!lockMode) {
                // discard any changes in counters
                (void)getDelta(&htim1, &prevMainCounter, MAIN_DELTA_MULT, MAIN_DELTA_DIV);
                (void)getDelta(&htim2, &prevClarCounter, CLAR_DELTA_MULT, CLAR_DELTA_DIV);
            }
            displaySMeterOrMode(true);
        } else if(!lockMode) {
            if(buttonNextPressed() == BUTTON_STATUS_PRESSED) {
                if(fastMode) {
                    int32_t i;
                    for(i = 0; i < FAST_FREQUENCIES_NUMBER; i++) {
                        if(bands[currentBand].fastFreqs[i] == -1)
                            continue;

                        if(bands[currentBand].fastFreqs[i] > bands[currentBand].lastFreq) {
                            bands[currentBand].lastFreq = bands[currentBand].fastFreqs[i];
                            changeFrequency(0, false, false);
                            displayFrequency();
                            break;
                        }
                    }
                } else {
                    changeBand(1);
                }
            } else if(buttonPrevPressed() == BUTTON_STATUS_PRESSED) {
                if(fastMode) {
                    int32_t i;
                    for(i = FAST_FREQUENCIES_NUMBER-1; i >= 0; i--) {
                        if(bands[currentBand].fastFreqs[i] == -1)
                            continue;

                        if(bands[currentBand].fastFreqs[i] < bands[currentBand].lastFreq) {
                            bands[currentBand].lastFreq = bands[currentBand].fastFreqs[i];
                            changeFrequency(0, false, false);
                            displayFrequency();
                            break;
                        }
                    }
                } else {
                    changeBand(-1);
                }
            } else if(buttonFastPressed() == BUTTON_STATUS_PRESSED) {
                fastMode = !fastMode;
                if(!fastMode) {
                    // discard any changes in counters
                    (void)getDelta(&htim1, &prevMainCounter, MAIN_DELTA_MULT, MAIN_DELTA_DIV);
                    (void)getDelta(&htim2, &prevClarCounter, CLAR_DELTA_MULT, CLAR_DELTA_DIV);
                }
                displaySMeterOrMode(true);
            } else if(!fastMode && (buttonClarPressed() == BUTTON_STATUS_PRESSED)) {
                if(clarMode == CLAR_MODE_DISABLED) {
                    clarMode = CLAR_MODE_RIT;
                    clarOffset = 0;
                    // discard any changes in counter
                    (void)getDelta(&htim2, &prevClarCounter, CLAR_DELTA_MULT, CLAR_DELTA_DIV);
                } else if(clarMode == CLAR_MODE_RIT) {
                    clarMode = CLAR_MODE_XIT;
                    clarOffset = 0;
                    // discard any changes in counter
                    (void)getDelta(&htim2, &prevClarCounter, CLAR_DELTA_MULT, CLAR_DELTA_DIV);
                } else {
                    clarMode = CLAR_MODE_DISABLED;
                }

                changeFrequency(0, false, true);
                displaySMeterOrMode(true);
            } else if(buttonKeyerPressed() == BUTTON_STATUS_PRESSED) {
                loopKeyer();
                // discard any changes in counters
                (void)getDelta(&htim1, &prevMainCounter, MAIN_DELTA_MULT, MAIN_DELTA_DIV);
                (void)getDelta(&htim2, &prevClarCounter, CLAR_DELTA_MULT, CLAR_DELTA_DIV);
            } else if(buttonXmitPressed() == BUTTON_STATUS_PRESSED) {
                if(enabledSSBMode()) {
                    /* In SSB mode XMIT works as TUNE */
                    uint32_t tstamp;

                    // ENABLE_SSB_12V; disable while transmitting the carrier
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

                    transmittingSSB = false;
                    ensureTransmitMode();
                    resetSWRMeter();

                    transmitModeEnterTime = HAL_GetTick();
                    keyDown();

                    do {
                        updateSWRMeter();
                        tstamp = HAL_GetTick();
                    } while((!anyButtonPressed(true)) && (tstamp - transmitModeEnterTime < 10000));

                    keyUp();
                    ensureReceiveMode();

                    // ENABLE_SSB_12V; restore the original pin state
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);

                    /* discard any changes in counters */
                    (void)getDelta(&htim1, &prevMainCounter, MAIN_DELTA_MULT, MAIN_DELTA_DIV);
                    (void)getDelta(&htim2, &prevClarCounter, CLAR_DELTA_MULT, CLAR_DELTA_DIV);
                } else {
                    /* In CW mode XMIT sends the saved message */
                    ensureTransmitMode();
                    resetSWRMeter();

                    if(keyerConfig.straightKey) {
                        initStraightKeyer();
                    } else {
                        initIambicKeyer();
                    }
                    playbackSavedMessage(false, true);

                    transmitModeEnterTime = HAL_GetTick();

                    /* loopMain() will be called again from the main loop */
                    return;
                }
            }
        }

        if((clarMode == CLAR_MODE_DISABLED) && (!lockMode) && (!fastMode)) {
            displaySMeterOrMode(false);
        }
    }

    HAL_Delay(LOOP_DELAY);
}

void init() {
/*
    // Code for determining the correction factor for Si5351
    char calmsg[16];

    LCD_Init();
    LCD_Goto(0, 0);
    LCD_SendString(" Si5351 ");
    LCD_Goto(1, 0);
    snprintf(calmsg, sizeof(calmsg), "Cal; CH%d", CH_CAL);
    LCD_SendString(calmsg);

    si5351_Init(0);
    si5351PLLConfig_t pll_conf;
    si5351OutputConfig_t out_conf;
    int32_t Fclk = 10000000;

    si5351_Calc(Fclk, &pll_conf, &out_conf);
    si5351_SetupPLL(SI5351_PLL_A, &pll_conf);
    si5351_SetupOutput(CH_CAL, SI5351_PLL_A, SI5351_DRIVE_STRENGTH_4MA, &out_conf);
    si5351_EnableOutputs(1<<CH_CAL);

    while(1) {
        HAL_Delay(100);
    }
*/
    enableTx(false);
    switchBFO(false);
    keyUp();

    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    HAL_ADC_Start(&hadc1);

    LCD_Init();
    LCD_Goto(0, 0);
    LCD_SendString("HBR MKII");
    LCD_Goto(1, 0);
    LCD_SendString("May 2024");
    HAL_Delay(1000);
    LCD_Clear();

    LCD_Goto(0, 0);
    LCD_SendString("I2C:  ");
    bool first_line = true;
    for(uint16_t i = 1; i < 128; i++) {
        HAL_StatusTypeDef res;
        res = HAL_I2C_IsDeviceReady(&hi2c1, i << 1, 1, 10);
        if(res == HAL_OK) {
            char msg[64];
            snprintf(msg, sizeof(msg), "%02X ", i);
            LCD_SendString(msg);
            if(first_line) {
                LCD_Goto(1, 0);
                first_line = false;
            }
        }
    }  
    HAL_Delay(1000);
    LCD_Clear();

    loadKeyerConfig();
    loadKeyerMessage();
    displaySMeterOrMode(true);

    si5351_Init(si5351_correction);
    changeBand(0); // calls changeFrequency()

    inTransmitMode = true;
    ensureReceiveMode();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    loopMain();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /**Common config 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA3 PA4 PA5 PA6 
                           PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB10 PB11 
                           PB12 PB13 PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB14 PB15 PB5 PB6 
                           PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA10 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
