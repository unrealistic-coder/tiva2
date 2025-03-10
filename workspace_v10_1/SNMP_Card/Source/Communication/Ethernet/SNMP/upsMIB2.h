/*
Generated by LwipMibCompiler
*/

#ifndef UPS_MIB2_H
#define UPS_MIB2_H

#include "lwip/apps/snmp_opts.h"
#if LWIP_SNMP

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "lwip/apps/snmp.h"
#include "lwip/apps/snmp_core.h"
#include "lwip/apps/snmp_scalar.h"
#include "lwip/apps/snmp_table.h"


s16_t UPSMIBinput_treenode_get_value(struct snmp_node_instance *instance, void *value);
snmp_err_t UPSMIBinputtable_get_instance(const u32_t *column, const u32_t *row_oid, u8_t row_oid_len, struct snmp_node_instance *cell_instance);
snmp_err_t UPSMIBinputtable_get_next_instance(const u32_t *column, struct snmp_obj_id *row_oid, struct snmp_node_instance *cell_instance);
s16_t UPSMIBinputtable_get_value(struct snmp_node_instance *cell_instance, void *value);

s16_t UPSMIBoutput_treenode_get_value(struct snmp_node_instance *instance, void *value);
snmp_err_t UPSMIBoutputtable_get_instance(const u32_t *column, const u32_t *row_oid, u8_t row_oid_len, struct snmp_node_instance *cell_instance);
snmp_err_t UPSMIBoutputtable_get_next_instance(const u32_t *column, struct snmp_obj_id *row_oid, struct snmp_node_instance *cell_instance);
s16_t UPSMIBoutputtable_get_value(struct snmp_node_instance *cell_instance, void *value);

s16_t UPSMIBbypass_treenode_get_value(struct snmp_node_instance *instance, void *value);
snmp_err_t UPSMIBbypasstable_get_instance(const u32_t *column, const u32_t *row_oid, u8_t row_oid_len, struct snmp_node_instance *cell_instance);
snmp_err_t UPSMIBbypasstable_get_next_instance(const u32_t *column, struct snmp_obj_id *row_oid, struct snmp_node_instance *cell_instance);
s16_t UPSMIBbypasstable_get_value(struct snmp_node_instance *cell_instance, void *value);


s16_t UPSMIBAlarmsPresent_get_value(struct snmp_node_instance *instance, void *value);
snmp_err_t UPSMIBalarmtable_get_instance(const u32_t *column, const u32_t *row_oid, u8_t row_oid_len, struct snmp_node_instance *cell_instance);
snmp_err_t UPSMIBalarmtable_get_next_instance(const u32_t *column, struct snmp_obj_id *row_oid, struct snmp_node_instance *cell_instance);
s16_t UPSMIBalarmtable_get_value(struct snmp_node_instance *instance, void *value);

s16_t UPSMIBident_scalars_get_value(const struct snmp_scalar_array_node_def *node, void *value);
snmp_err_t UPSMIBident_scalars_set_test(const struct snmp_scalar_array_node_def *node, u16_t len, void *value);
snmp_err_t UPSMIBident_scalars_set_value(const struct snmp_scalar_array_node_def *node, u16_t len, void *value);

s16_t UPSMIBbattery_scalars_get_value(const struct snmp_scalar_array_node_def *node, void *value);
s16_t UPSMIBtest_scalars_get_value(const struct snmp_scalar_array_node_def *node, void *value);
snmp_err_t UPSMIBtest_scalars_set_test(const struct snmp_scalar_array_node_def *node, u16_t len, void *value);
snmp_err_t UPSMIBtest_scalars_set_value(const struct snmp_scalar_array_node_def *node, u16_t len, void *value);

s16_t UPSMIBcontrol_scalars_get_value(const struct snmp_scalar_array_node_def *node, void *value);
snmp_err_t UPSMIBcontrol_scalars_set_test(const struct snmp_scalar_array_node_def *node, u16_t len, void *value);
snmp_err_t UPSMIBcontrol_scalars_set_value(const struct snmp_scalar_array_node_def *node, u16_t len, void *value);

s16_t UPSMIBconfig_scalars_get_value(const struct snmp_scalar_array_node_def *node, void *value);
snmp_err_t UPSMIBconfig_scalars_set_test(const struct snmp_scalar_array_node_def *node, u16_t len, void *value);
snmp_err_t UPSMIBconfig_scalars_set_value(const struct snmp_scalar_array_node_def *node, u16_t len, void *value);


extern const struct snmp_mib upsmib;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWIP_SNMP */
#endif /* EXAMPLE2_H */
