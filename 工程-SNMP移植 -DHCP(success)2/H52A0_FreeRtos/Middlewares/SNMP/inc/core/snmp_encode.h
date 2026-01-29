#ifndef SNMP_ENCODE_H
#define SNMP_ENCODE_H

// #include "snmp_types.h"
#include "./core/snmp.h"

int build_snmp_v1(unsigned char *out_buf, _snmp_data *data);
int build_snmp_v2c(unsigned char *out_buf, _snmp_data *data);
int build_snmp_v3(unsigned char *out_buf, _snmp_data *data);
int snmp_build_trap_v1(unsigned char *buffer, void *auth, _trap_varbinding *var_bindings, unsigned int num_bindings);
int snmp_build_trap_v2c(unsigned char *buffer, void *auth, _trap_varbinding *var_bindings, unsigned int num_bindings);
int snmp_build_trap_v3(unsigned char *buffer, void *auth, _trap_varbinding *var_bindings, unsigned int num_bindings);

#endif // SNMP_ENCODE_H
