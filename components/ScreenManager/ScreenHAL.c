#include "ScreenHAL.h"
#include "driver/i2c.h"
#include <string.h>

void HAL_init(void);
void oled_init(void);
void sendScreenFrame(uint8_t *frameBuffer);
esp_err_t send_cmd_byte(uint8_t byte);
esp_err_t i2c_master_init(void);
static esp_err_t set_column_range(uint8_t start, uint8_t end);
static esp_err_t set_page_range(uint8_t start, uint8_t end);

void HAL_init(void)
{
    i2c_master_init();
    oled_init();
    set_column_range(0,127);
    set_page_range(0,7);
}

void sendScreenFrame(uint8_t *frameBuffer)
{
    uint32_t num_RAM_addresses = 128 * 8;
    uint8_t write_buf[num_RAM_addresses + 1];
    
    write_buf[0] = CONTROL_BYTE_DATA;
    
    memcpy(write_buf + 1, frameBuffer, num_RAM_addresses);

    i2c_master_write_to_device(I2C_MASTER_NUM, DISPLAY_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void oled_init(void)
{
    send_cmd_byte(0xA8);
    send_cmd_byte(0x3F);
    send_cmd_byte(0xD3);
    send_cmd_byte(0x00);
    send_cmd_byte(0x40);
    send_cmd_byte(0xA0);
    send_cmd_byte(0xC0);
    send_cmd_byte(0xDA);
    send_cmd_byte(0x12);
    send_cmd_byte(0x81);
    send_cmd_byte(0x7F);
    send_cmd_byte(0xA4);
    send_cmd_byte(0xA6);
    send_cmd_byte(0xD5);
    send_cmd_byte(0x80);
    send_cmd_byte(0x8D);
    send_cmd_byte(0x14);
    send_cmd_byte(0xAF);
    send_cmd_byte(0x20);
    send_cmd_byte(0x00);
}

esp_err_t send_cmd_byte(uint8_t byte)
{
    uint8_t write_buf[2] = {CONTROL_BYTE_CMD, byte};
    return i2c_master_write_to_device(I2C_MASTER_NUM, DISPLAY_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

static esp_err_t set_column_range(uint8_t start, uint8_t end)
{
    if (start > 127 || end > 127){
        return -1;
    }

    uint8_t write_buf[4];
    write_buf[0] = CONTROL_BYTE_CMD;
    write_buf[1] = SET_COLUMN_ADDR;
    write_buf[2] = start;
    write_buf[3] = end;
    return i2c_master_write_to_device(I2C_MASTER_NUM, DISPLAY_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

static esp_err_t set_page_range(uint8_t start, uint8_t end)
{
    if (start > 7 || end > 7){
        return -1;
    }

    uint8_t write_buf[4];
    write_buf[0] = CONTROL_BYTE_CMD;
    write_buf[1] = SET_PAGE_ADDR;
    write_buf[2] = start;
    write_buf[3] = end;
    return i2c_master_write_to_device(I2C_MASTER_NUM, DISPLAY_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}