#define SPI_DDR		DDRB
#define SPI_MOSI		DDB2
#define SPI_MISO		DDB3
#define SPI_SCK		DDB1

#define SLAVE_PORT		PORTA
#define SLAVE_PIN		3

#define MMC_PORT		PORTE
#define MMC_PIN		3

#define SPI_TX			1
#define SPI_RX			0

#define SPI_NONE		0
#define SPI_SLAVE		1
#define SPI_MMC		2

#define SPI_DISABLE	0
#define SPI_ENABLE		1

#define SPI_READ		0
#define SPI_WRITE		1

void spi_init(unsigned char, unsigned char);
void spi_enable(unsigned char);
unsigned char spi(unsigned char);

void write_SPI(char dataspi);
