# 📡 SNMP协议实现模块

> **📌 简介**：本模块实现了SNMP V1、V2C和V3协议栈，采用模块化设计便于维护和扩展。支持MD5/SHA-1认证和DES/AES加密功能。

---

## 📚 文档目录

- [模块简介](#-模块简介)
- [模块结构](#-模块结构)
- [文件组织](#-文件组织)
- [模块功能说明](#-模块功能说明)
- [接口使用指南](#-接口使用指南)
- [注意事项](#-注意事项)
- [参考文档](#-参考文档)

---

## 📌 模块简介

本模块是SNMP协议的完整实现，具有以下特点：

| 特性 | 说明 |
|:-----|:-----|
| **协议支持** | 完整支持SNMP V1、V2C和V3协议 |
| **认证机制** | 支持MD5、SHA-1认证算法 |
| **加密功能** | 支持DES、AES加密算法 |
| **模块化设计** | 采用清晰的模块划分，便于维护和扩展 |
| **ASN.1编解码** | 完整实现ASN.1基本编码规则 |
| **MIB管理** | 提供标准MIB和自定义MIB支持 |

> **💡 提示**：本实现专为嵌入式系统设计，特别适用于FreeRTOS环境下的网络监控应用。

---

## 📂 模块结构

SNMP协议实现采用以下模块化设计：

```
SNMP协议实现
├── asn1模块    → ASN.1编解码功能
├── core模块    → SNMP核心协议处理
├── crypto模块  → 安全相关功能
├── mib模块     → MIB节点管理
└── network模块 → 网络接口抽象
```

---

## 🗂️ 文件组织

```
Drivers/PROTOCOL/SNMP/
├── inc/                # 头文件目录
│   ├── asn1/           # ASN.1编解码头文件
│   ├── core/           # SNMP核心协议头文件
│   ├── crypto/         # 加密算法头文件
│   ├── mib/            # MIB管理头文件
│   └── network/        # 网络接口头文件
└── src/                # 源文件目录
    ├── asn1/           # ASN.1编解码实现
    ├── core/           # SNMP核心协议实现
    ├── crypto/         # 加密算法实现
    ├── mib/            # MIB管理实现
    └── network/        # 网络接口实现
```

### 核心文件列表

| 模块 | 头文件 | 源文件 | 说明 |
|:----|:------|:------|:-----|
| **asn1** | `asn.1.h` | `asn.1.c` | ASN.1编解码实现 |
| **core** | `snmp.h`, `snmp_decode.h`, `snmp_encode.h` | `snmp.c`, `snmp_decode.c`, `snmp_encode.c` | SNMP核心协议处理 |
| **crypto** | `snmp_crypto.h`, `aes/snmp_aes.h`, `des/snmp_des.h` | `snmp_crypto.c`, `aes/snmp_aes.c`, `des/snmp_des.c` | 安全认证与加密 |
| **mib** | `mib.h`, `rfc_1066.h` | `mib.c`, `rfc_1066.c` | MIB节点管理 |
| **network** | `network_interface.h` | `w5500_network_interface.c` | 网络接口抽象 |

---

## 🧩 模块功能说明

### 📡 asn1模块

> **ASN.1编解码功能**
> 
> - 实现ASN.1基本编码规则(BER)
> - 提供OID(对象标识符)的编码和解码
> - 支持TLV(Tag-Length-Value)结构的解析

### 🌐 core模块

> **SNMP核心协议处理**
> 
> - SNMP消息的解析和构建
> - PDU(协议数据单元)的处理
> - 变量绑定的处理
> - SNMPv3消息处理

### 🔒 crypto模块

> **安全认证与加密**
> 
> - MD5和SHA-1哈希算法实现
> - HMAC-MD5和HMAC-SHA1认证算法
> - DES和AES加密算法实现
> - 密码转密钥和密钥本地化功能

### 📚 mib模块

> **MIB节点管理**
> 
> - MIB节点注册和管理机制
> - RFC1066标准MIB实现
> - 自定义MIB节点处理
> - OID匹配和查找算法

### 🌐 network模块

> **网络接口抽象**
> 
> - 网络接口抽象层实现
> - UDP数据包的发送和接收
> - 网络状态检查和管理
> - 支持W5500等网络芯片

---

## 🛠️ 接口使用指南

### 1️⃣ SNMP代理初始化

```c
// 初始化SNMP代理参数
_snmp_params snmp_params = {
    .enabled = 1,
    .use_v3 = 1,
    .read_community = "public",
    .write_community = "private",
    .engine_id = {0x80, 0x00, 0x1F, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    .engine_id_len = 12,
    .traps_enabled = 1,
    .users = {
        {"admin", "adminpass", "adminpriv", AUTH_SHA, PRIV_AES, {0}, {0}, USER_LEVEL_READ_WRITE},
        {"user", "userpass", "userpriv", AUTH_MD5, PRIV_DES, {0}, {0}, USER_LEVEL_READ}
    }
};

// 初始化SNMP代理
snmp_init(&snmp_params);

// 初始化Trap功能
trap_init();
```

### 2️⃣ MIB节点注册

```c
// 注册自定义MIB节点
_mib_node your_mib_node = {
    .oid = {1, 3, 6, 1, 4, 1, 12345, 1},
    .oid_len = 7,
    .access = READ_WRITE,
    .type = ASN_INTEGER,
    .get_value = your_get_value_func,
    .set_value = your_set_value_func,
    .value = &your_value
};

mib_register(&your_mib_node);
```

### 3️⃣ 任务中处理SNMP请求

```c
// 创建SNMP消息队列
QueueHandle_t snmp_msg_queue = xQueueCreate(10, sizeof(snmp_msg_t));

// 创建SNMP接收任务
xTaskCreate(SNMP_Recv_Task, "SNMP_Recv", 512, NULL, 3, NULL);

// 创建SNMP处理任务
xTaskCreate(SNMP_Handle_Task, "SNMP_Handle", 512, NULL, 3, NULL);

// SNMP接收任务
void SNMP_Recv_Task(void *pvParameters)
{
    const network_interface_t* netif = &w5500_network_interface;
    while (1) {
        snmp_msg_t msg;
        if (netif->check_network_status(SOCK_SNMP_V3, SNMP_PORT) != 0) {
            vTaskDelay(100 / portTICK_RATE_MS);
            continue;
        }
        msg.len = netif->receive(SOCK_SNMP_V3, msg.data, SNMP_MSG_MAX_LEN, msg.remote_ip, &msg.remote_port);
        if (msg.len > 0) {
            xQueueSend(snmp_msg_queue, &msg, portMAX_DELAY);
        } else {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }
}

// SNMP处理任务
void SNMP_Handle_Task(void *pvParameters)
{
    const network_interface_t* netif = &w5500_network_interface;
    snmp_msg_t msg;
    while (1) {
        if (xQueueReceive(snmp_msg_queue, &msg, portMAX_DELAY) == pdTRUE) {
            uint16_t out_len = snmp_process(msg.data, msg.len, msg.data);
            if (out_len > 0) {
                netif->send(SOCK_SNMP_V3, msg.data, out_len, msg.remote_ip, msg.remote_port);
            }
        }
    }
}
```

### 4️⃣ 发送Trap

```c
// 发送warmStart Trap
trap_send_warmStart(&snmp_params);

// 发送自定义Trap
_trap_varbinding var_bindings[] = {
    {{1, 3, 6, 1, 4, 1, 12345, 1, 1}, 4, "Test", ASN_OCTET_STRING}
};
snmp_send_trap(3, target_ip, 162, &snmp_params.users[0], var_bindings, 1);
```

---

## ⚠️ 注意事项

> **📌 重要提示**
> 
> 1. **加密支持**：需要外部MD5、SHA1、AES、DES加密库支持，可使用本项目提供的实现
> 2. **内存管理**：在嵌入式系统中注意SNMP任务的栈空间分配，建议最小512字节
> 3. **安全配置**：SNMPv3安全配置需参考RFC文档要求
> 4. **网络接口**：当前实现基于网络接口抽象层，如需移植到其他平台需实现对应的网络接口
> 5. **任务优先级**：在FreeRTOS环境中，SNMP任务优先级建议设置为3
> 6. **协议兼容性**：确保与目标设备的SNMP版本兼容

---


## 📚 参考文档

- RFC 3410 - Introduction and Applicability Statements for Internet-Standard Management Framework
- RFC 3411 - An Architecture for Describing Simple Network Management Protocol (SNMP) Management Frameworks
- RFC 3412 - Message Processing and Dispatching for the Simple Network Management Protocol (SNMP)
- RFC 3413 - Simple Network Management Protocol (SNMP) Applications
- RFC 3414 - User-based Security Model (USM) for version 3 of the Simple Network Management Protocol (SNMP)
- RFC 3415 - View-based Access Control Model (VACM) for the Simple Network Management Protocol (SNMP)