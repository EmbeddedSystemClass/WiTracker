#ifndef I2C_H
#define I2C_H

#define I2C_MASTER_SCL_IO 22                /*!< gpio number for I2C master clock */
#define I2C__MASTER_SDA_IO 21               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0            /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */

#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read */

#define ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0       /*!< I2C ack value */
#define NACK_VAL 0x1      /*!< I2C nack value */

// Initialises the i2c config settings
extern void mc_i2c_init();

#endif /* I2C_H */
