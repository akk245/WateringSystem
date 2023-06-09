#ifndef SCREEN_HAL_H
#define SCREEN_HAL_H

#include <stdint.h>

// hardware specific I2C
#define I2C_MASTER_SCL_IO           21      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           19      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       50

#define DISPLAY_ADDR                 0x3C
#define CONTROL_BYTE_CMD             0x00
#define CONTROL_BYTE_DATA            0x40


// Display Commands
#define DISPLAY_WAKEUP               0xAF
#define DISPLAY_SLEEP                0xAE

#define DISPLAY_ALL_ON_CMD           0xA5
#define DISPLAY_ALL_OFF_CMD          0xA4

#define SET_COLUMN_ADDR              0x21
#define SET_PAGE_ADDR                0x22

void HAL_init(void);
void sendScreenFrame(uint8_t *frameBuffer);

#endif // SCREEN_HAL_H