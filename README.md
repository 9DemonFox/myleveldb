# 编译
## 通过Cmake控制一些宏的开关
port/port_config.h.in 

```
set(LEVELDB_PUBLIC_INCLUDE_DIR "include/leveldb")
set(LEVELDB_PORT_CONFIG_DIR "include/port")

configure_file(
  "port/port_config.h.in"
  "${PROJECT_BINARY_DIR}/${LEVELDB_PORT_CONFIG_DIR}/port_config.h"
)

```

# Placement new
https://blog.csdn.net/andrewgithub/article/details/138178765
 
# 参考
- leveldb 解读
https://zhuanlan.zhihu.com/p/608790872
- 日志格式
https://zhuanlan.zhihu.com/p/35134533