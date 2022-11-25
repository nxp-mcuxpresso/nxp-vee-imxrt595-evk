#define PROBE_INIT(pin) GPIO_PortInit_noreset(GPIO, 3); \
                                 GPIO->DIR[3] |= 1UL << pin; GPIO->CLR[3] = (1UL << pin);
#define PROBE_SET(pin) GPIO->SET[3] = (1UL << pin)
#define PROBE_CLR(pin) GPIO->CLR[3] = (1UL << pin)
