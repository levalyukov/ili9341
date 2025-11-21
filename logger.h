#ifndef LOGGER_H
#define LOGGER_H

#define LOG_INFO(module)  printf("[*] %s\n", module);
#define LOG_OK(module)    printf("[+] %s\n", module);
#define LOG_ERROR(module) printf("[-] %s\n", module);

#endif
