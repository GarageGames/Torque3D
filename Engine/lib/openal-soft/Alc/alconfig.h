#ifndef ALCONFIG_H
#define ALCONFIG_H

void ReadALConfig(void);
void FreeALConfig(void);

int ConfigValueExists(const char *devName, const char *blockName, const char *keyName);
const char *GetConfigValue(const char *devName, const char *blockName, const char *keyName, const char *def);
int GetConfigValueBool(const char *devName, const char *blockName, const char *keyName, int def);

int ConfigValueStr(const char *devName, const char *blockName, const char *keyName, const char **ret);
int ConfigValueInt(const char *devName, const char *blockName, const char *keyName, int *ret);
int ConfigValueUInt(const char *devName, const char *blockName, const char *keyName, unsigned int *ret);
int ConfigValueFloat(const char *devName, const char *blockName, const char *keyName, float *ret);
int ConfigValueBool(const char *devName, const char *blockName, const char *keyName, int *ret);

#endif /* ALCONFIG_H */
