#include "asm/power_interface.h"
#include "vm_api.h"
/* #include "syscfg_id.h" */


typedef struct {
    u8 except_cnt;
} pmu_voltage_type;

extern void update_vdd_table(u8 val);

void volatage_trim_init()
{

    pmu_voltage_type pmu_voltage;
    memset((u8 *)&pmu_voltage, 0xff, sizeof(pmu_voltage));

    int vm_len = vm_read(LIB_VM_PMU_VOLTAGE, (u8 *)&pmu_voltage, sizeof(pmu_voltage_type));

    /*上电第一次trim*/
    if (vm_len != sizeof(pmu_voltage_type)) {
        pmu_voltage.except_cnt = 0;
    }

    if (is_reset_source(P33_WDT_RST) || is_reset_source(P33_EXCEPTION_SOFT_RST)) {
        if ((pmu_voltage.except_cnt + 1) < 0xff) {
            pmu_voltage.except_cnt++;
            vm_write(LIB_VM_PMU_VOLTAGE, (u8 *)&pmu_voltage, sizeof(pmu_voltage_type));
        }
    }

    update_vdd_table(((pmu_voltage.except_cnt <= 2) ? pmu_voltage.except_cnt : 2));
}

