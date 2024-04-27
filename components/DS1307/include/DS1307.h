#ifndef DS1307_H
#define DS1307_H

#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"

// I2C master clock
#define DS1307_I2C_FREQ_HZ 400000 

// Device address
#define DS1307_ADDR 0x68

// DS1307 internal registers addresses
#define DS1307_ADDR_SECONDS 0x00
#define DS1307_ADDR_MINUTES 0x01
#define DS1307_ADDR_HOURS 0x02
#define DS1307_ADDR_WEEK_DAY 0x03
#define DS1307_ADDR_DATE_DAY 0x04
#define DS1307_ADDR_MONTH 0x05
#define DS1307_ADDR_YEAR 0x06
#define DS1307_ADDR_CONTROL 0x07

#define DS1307_HOUR_MODE 0x00 // Set to 24 hour format

#define WRITE_BIT I2C_MASTER_WRITE // I2C master write
#define READ_BIT I2C_MASTER_READ   // I2C master read
#define ACK_CHECK_EN 0x1           // I2C master will check ack from slave
#define ACK_CHECK_DIS 0x0          // I2C master will not check ack from slave
#define ACK_VAL 0x0                // I2C ack value
#define NACK_VAL 0x1               // I2C nack value

esp_err_t ds1307Begin(bool install_driver);
esp_err_t ds1307SetDate(uint8_t day, uint8_t month, uint8_t year, uint8_t week_day);
esp_err_t ds1307SetTime(uint8_t hour, uint8_t minute, uint8_t second);

uint8_t ds1307ReadByte(uint8_t reg);
esp_err_t ds1307WriteByte(uint8_t reg, uint8_t value);

uint8_t ds1307GetSecond();
uint8_t ds1307GetMinute();
uint8_t ds1307GetHour();
uint8_t ds1307GetWeekDay();
uint8_t ds1307GetDateDay();
uint8_t ds1307GetMonth();
uint8_t ds1307GetYear();

void ds1307GetTime(char *time);
void ds1307GetDate(char *date);
void ds1307GetTimestamp(char *timestamp);

uint8_t dec2bcd(uint8_t num);
uint8_t bcd2dec(uint8_t num);

#endif
