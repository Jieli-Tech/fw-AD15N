#ifndef __POWER_MANAGE_H__
#define __POWER_MANAGE_H__

//******************************************************************************************
struct _phw_dev {
    void *hw;
    void *pdata;
    struct phw_dev_ops *ops;
};

struct phw_dev_ops {
    void *(*early_init)(u32 arg);
    u32(*init)(struct _phw_dev *dev, u32 arg);
    u32(*ioctl)(struct _phw_dev *dev, u32 cmd, u32 arg);

    u32(*sleep_already)(struct _phw_dev *dev, u32 arg);
    u32(*sleep_prepare)(struct _phw_dev *dev, u32 arg);
    u32(*sleep_enter)(struct _phw_dev *dev, u32 arg);
    u32(*sleep_exit)(struct _phw_dev *dev, u32 arg);
    u32(*sleep_post)(struct _phw_dev *dev, u32 arg);

    u32(*soff_prepare)(struct _phw_dev *dev, u32 arg);
    u32(*soff_enter)(struct _phw_dev *dev, u32 arg);
    u32(*soff_exit)(struct _phw_dev *dev, u32 arg);

    u32(*deepsleep_enter)(struct _phw_dev *dev, u32 arg);
    u32(*deepsleep_exit)(struct _phw_dev *dev, u32 arg);
};

#define REGISTER_PHW_DEV_PMU_OPS(ops) \
		const struct phw_dev_ops *phw_pmu_ops = &ops
extern const struct phw_dev_ops *phw_pmu_ops;


//******************************************************************************************

#endif
