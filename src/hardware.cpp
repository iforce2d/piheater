#include "hardware.h"
#include <stdio.h>
#include <math.h>

#include "settings.h"

int faultFlags = 0;
float ambientTemperature = 0;
float sensorTemperature = 0;
float pwmPercent = 0;
bool relayOn = false;
bool coolingOn = false;
bool buzzerOn = false;

#ifdef __arm__

#include "bcm2835.h"
#include "bcm2835.c"

int rt_bcm2835_init(void)
{
    int  memfd;
    int  ok;
    FILE *fp;

    if (debug)
    {
        bcm2835_peripherals = (uint32_t*)BCM2835_PERI_BASE;

        bcm2835_pads = bcm2835_peripherals + BCM2835_GPIO_PADS/4;
        bcm2835_clk  = bcm2835_peripherals + BCM2835_CLOCK_BASE/4;
        bcm2835_gpio = bcm2835_peripherals + BCM2835_GPIO_BASE/4;
        bcm2835_pwm  = bcm2835_peripherals + BCM2835_GPIO_PWM/4;
        bcm2835_spi0 = bcm2835_peripherals + BCM2835_SPI0_BASE/4;
        bcm2835_bsc0 = bcm2835_peripherals + BCM2835_BSC0_BASE/4;
        bcm2835_bsc1 = bcm2835_peripherals + BCM2835_BSC1_BASE/4;
        bcm2835_st   = bcm2835_peripherals + BCM2835_ST_BASE/4;
        bcm2835_aux  = bcm2835_peripherals + BCM2835_AUX_BASE/4;
        bcm2835_spi1 = bcm2835_peripherals + BCM2835_SPI1_BASE/4;

        return 1; /* Success */
    }

    /* Figure out the base and size of the peripheral address block
    // using the device-tree. Required for RPi2/3/4, optional for RPi 1
    */
    if ((fp = fopen(BMC2835_RPI2_DT_FILENAME , "rb")))
    {
        unsigned char buf[16];
        uint32_t base_address;
        uint32_t peri_size;
        if (fread(buf, 1, sizeof(buf), fp) >= 8)
        {
            base_address = (buf[4] << 24) |
                           (buf[5] << 16) |
                           (buf[6] << 8) |
                           (buf[7] << 0);

            peri_size = (buf[8] << 24) |
                        (buf[9] << 16) |
                        (buf[10] << 8) |
                        (buf[11] << 0);

            if (!base_address)
            {
                /* looks like RPI 4 */
                base_address = (buf[8] << 24) |
                               (buf[9] << 16) |
                               (buf[10] << 8) |
                               (buf[11] << 0);

                peri_size = (buf[12] << 24) |
                            (buf[13] << 16) |
                            (buf[14] << 8) |
                            (buf[15] << 0);
            }
            /* check for valid known range formats */
            if ((buf[0] == 0x7e) &&
                (buf[1] == 0x00) &&
                (buf[2] == 0x00) &&
                (buf[3] == 0x00) &&
                ((base_address == BCM2835_PERI_BASE) || (base_address == BCM2835_RPI2_PERI_BASE) || (base_address == BCM2835_RPI4_PERI_BASE)))
            {
                bcm2835_peripherals_base = (off_t)base_address;
                bcm2835_peripherals_size = (size_t)peri_size;
                if( base_address == BCM2835_RPI4_PERI_BASE )
                {
                    pud_type_rpi4 = 1;
                }
            }

        }

        fclose(fp);
    }
    /* else we are prob on RPi 1 with BCM2835, and use the hardwired defaults */

    /* Now get ready to map the peripherals block
     * If we are not root, try for the new /dev/gpiomem interface and accept
     * the fact that we can only access GPIO
     * else try for the /dev/mem interface and get access to everything
     */
    memfd = -1;
    ok = 0;
    if (geteuid() == 0)
    {
        /* Open the master /dev/mem device */
        if ((memfd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
        {
            fprintf(stderr, "bcm2835_init: Unable to open /dev/mem: %s\n",
                    strerror(errno)) ;
            goto exit;
        }

        /* Base of the peripherals block is mapped to VM */
        bcm2835_peripherals = (uint32_t*)mapmem("gpio", bcm2835_peripherals_size, memfd, bcm2835_peripherals_base);
        if (bcm2835_peripherals == MAP_FAILED) goto exit;

        /* Now compute the base addresses of various peripherals,
      // which are at fixed offsets within the mapped peripherals block
      // Caution: bcm2835_peripherals is uint32_t*, so divide offsets by 4
      */
        bcm2835_gpio = bcm2835_peripherals + BCM2835_GPIO_BASE/4;
        bcm2835_pwm  = bcm2835_peripherals + BCM2835_GPIO_PWM/4;
        bcm2835_clk  = bcm2835_peripherals + BCM2835_CLOCK_BASE/4;
        bcm2835_pads = bcm2835_peripherals + BCM2835_GPIO_PADS/4;
        bcm2835_spi0 = bcm2835_peripherals + BCM2835_SPI0_BASE/4;
        bcm2835_bsc0 = bcm2835_peripherals + BCM2835_BSC0_BASE/4; /* I2C */
        bcm2835_bsc1 = bcm2835_peripherals + BCM2835_BSC1_BASE/4; /* I2C */
        bcm2835_st   = bcm2835_peripherals + BCM2835_ST_BASE/4;
        bcm2835_aux  = bcm2835_peripherals + BCM2835_AUX_BASE/4;
        bcm2835_spi1 = bcm2835_peripherals + BCM2835_SPI1_BASE/4;

        ok = 1;
    }
    else
    {
        /* Not root, try /dev/gpiomem */
        /* Open the master /dev/mem device */
        if ((memfd = open("/dev/gpiomem", O_RDWR | O_SYNC) ) < 0)
        {
            fprintf(stderr, "bcm2835_init: Unable to open /dev/gpiomem: %s\n",
                    strerror(errno)) ;
            goto exit;
        }

        /* Base of the peripherals block is mapped to VM */
        bcm2835_peripherals_base = 0;
        bcm2835_peripherals = (uint32_t*)mapmem("gpio", bcm2835_peripherals_size, memfd, bcm2835_peripherals_base);
        if (bcm2835_peripherals == MAP_FAILED) goto exit;
        bcm2835_gpio = bcm2835_peripherals;
        ok = 1;
    }

exit:
    if (memfd >= 0)
        close(memfd);

    if (!ok)
        bcm2835_close();

    return ok;
}

#define FAULT_CODE							9999				// Fault code to be returned if no thermocouple is attached or there are short circuits
#define OOR_CODE							8888				// Out of range code to be returned if the thermocouple microvolts reading is outside our -200 to 350C LUT range
#define COLD_JUNC_SENSITIVITY_COEFF_T       41.276				// Coeff used by MAX31855 to calculate temperature for type K thermocouple (units in uV/C)

#define RELAY_PIN       RPI_GPIO_P1_11
#define COOLING_PIN     RPI_GPIO_P1_15
#define BUZZER_PIN      RPI_GPIO_P1_16

// PWM output on RPi Plug P1 pin 12 (which is GPIO pin 18) in alt fun 5.
// Note that this is the _only_ PWM pin available on the RPi IO headers
#define PWM_PIN RPI_GPIO_P1_12
// and it is controlled by PWM channel 0
#define PWM_CHANNEL 0
// This controls the max range of the PWM signal
#define RANGE 1024

uint32_t spiRead32(void) {
    uint32_t d = 0;
    char buf[4];
    char bytesOut[4];

    bcm2835_spi_transfernb( bytesOut, buf, 4 );

    d = buf[0];
    d <<= 8;
    d |= buf[1];
    d <<= 8;
    d |= buf[2];
    d <<= 8;
    d |= buf[3];

    return d;
}

#define MAX31855_FAULT_ALL  (0x07)
#define MAX31855_FAULT_OC   (0x01)
#define MAX31855_FAULT_GND  (0x02)
#define MAX31855_FAULT_VCC  (0x04)

void readMAX31855() {

    int32_t v = spiRead32();

    //printf("v = %08X\n", v); fflush(stdout);

    faultFlags = 0;

    if (v & MAX31855_FAULT_OC) {
        printf("fault - open connection\n");
        fflush(stdout);
        faultFlags |= MAX31855_FAULT_OC;
        //return;
    }

    if (v & MAX31855_FAULT_GND) {
        printf("fault - ground\n");
        fflush(stdout);
        faultFlags |= MAX31855_FAULT_GND;
        //return;
    }

    if (v & MAX31855_FAULT_VCC) {
        printf("fault - vcc\n");
        fflush(stdout);
        faultFlags |= MAX31855_FAULT_VCC;
        //return;
    }

    int32_t vbak = v;

    if ( ! faultFlags ) {
        if (v & 0x80000000) {
            // Negative value, drop the lower 18 bits and explicitly extend sign bits.
            v = 0xFFFFC000 | ((v >> 18) & 0x00003FFF);
        } else {
            // Positive value, just drop the lower 18 bits.
            v >>= 18;
        }

        float centigrade = v;

        // LSB = 0.25 degrees C
        centigrade *= 0.25;
        sensorTemperature = centigrade;
    }
    else {
        sensorTemperature = 999;
    }

    v = vbak;
    v >>= 4;

    // pull the bottom 11 bits off
    float internal = v & 0x7FF;
    // check sign bit!
    if (v & 0x800) {
        // Convert to negative value by extending sign and casting to signed type.
        int16_t tmp = 0xF800 | (v & 0x7FF);
        internal = tmp;
    }
    internal *= 0.0625; // LSB = 0.0625 degrees

    ambientTemperature = internal;

}

#endif // __arm__

bool initHardware()
{
#ifndef __arm__
    // not running on RaspberryPi
    printf("Skipping initHardware on non-RPi\n");
    return true;
#else
    if ( ! rt_bcm2835_init() ) {
        printf("rt_bcm2835_init failed. Are you running with root privileges?\n");
        return false;
    }
    printf("rt_bcm2835_init succeeded.\n");

    // Set the pin to be an output
    bcm2835_gpio_fsel(RELAY_PIN,   BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(COOLING_PIN, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(BUZZER_PIN,  BCM2835_GPIO_FSEL_OUTP);

    if (!bcm2835_spi_begin()) {
        printf("bcm2835_spi_begin failed. Are you running as root??\n");
        return false;
    }
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

    // Set the output pin to Alt Fun 5, to allow PWM channel 0 to be output there
    bcm2835_gpio_fsel(PWM_PIN, BCM2835_GPIO_FSEL_ALT5);

    // With a divider of 16 and a RANGE of 1024, in MARKSPACE mode, the pulse repetition
    // frequency will be 1.2MHz/1024 = 1171.875Hz
    bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_16);
    bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 1);
    bcm2835_pwm_set_range(PWM_CHANNEL, RANGE);

    setHardwareSafeValues();

    return true;
#endif
}

void setHardwareSafeValues()
{
#ifndef __arm__
    // not running on RaspberryPi
    printf("Skipping setHardwareSafeValues on non-RPi\n");
    return;
#else
    // set all outputs to safe values
    bcm2835_pwm_set_data(PWM_CHANNEL, 0);
    bcm2835_gpio_write(RELAY_PIN,   LOW);
    bcm2835_gpio_write(COOLING_PIN, LOW);
    bcm2835_gpio_write(BUZZER_PIN,  LOW);
#endif
}

void denitHardware()
{
#ifndef __arm__
    // not running on RaspberryPi
    printf("Skipping denitHardware on non-RPi\n");
    return;
#else
    setHardwareSafeValues();
    bcm2835_delay(100);
    bcm2835_close();
#endif
}

void updateHardwareInputs()
{
#ifndef __arm__
    ambientTemperature = 25;
    sensorTemperature = 30;
    return;
#else
    readMAX31855();
#endif
}

void updateHardwareOutputs() {
#ifdef __arm__
    int t = pwmPercent * (0.01f * RANGE) * (0.01f * Settings::getInstance().getOutputPWMScale());
    bcm2835_pwm_set_data(PWM_CHANNEL, t);
    bcm2835_gpio_write(RELAY_PIN, relayOn);
    bcm2835_gpio_write(COOLING_PIN, coolingOn);
    bcm2835_gpio_write(BUZZER_PIN, buzzerOn);
#endif
}

float getTemperatureAmbient() { return ambientTemperature; }
float getTemperatureSensor() { return sensorTemperature; }

void setPWMPercent(float v) { pwmPercent = v; updateHardwareOutputs(); }
float getPWMPercent() { return pwmPercent; }

void setCoolingOn(bool tf) { coolingOn = tf; updateHardwareOutputs(); }
bool getCoolingOn() { return coolingOn; }

void setRelayOn(bool tf) { relayOn = tf; updateHardwareOutputs(); }
bool getRelayOn() { return relayOn; }

void setBuzzerOn(bool tf) { buzzerOn = tf; updateHardwareOutputs(); }
bool getBuzzerOn() { return buzzerOn; }

int getFaultFlags() { return faultFlags; }


