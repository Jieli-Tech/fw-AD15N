void chip_reset();
struct maskrom_argv {
    void (*exp_hook)(u32 *);
    void (*enter_critical_hook)(void);
    void (*exit_critical_hook)(void);
};

void mask_init(struct maskrom_argv *argv);
