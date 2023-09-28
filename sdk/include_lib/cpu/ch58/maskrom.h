


void chip_reset();

struct maskrom_argv {
    void (*pchar)(char);
    void (*exp_hook)(u32 *);
    void (*local_irq_enable)();
    void (*local_irq_disable)();
    int (*flt)(char **, char *, double, int, int, char, int);
};









void mask_init(struct maskrom_argv *argv);
