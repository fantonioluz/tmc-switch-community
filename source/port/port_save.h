#ifndef PORT_SAVE_H
#define PORT_SAVE_H
#ifdef __cplusplus
extern "C" {
#endif
int Port_Save_CreateBackup(void);
int Port_Save_RestoreLatestBackup(void);
#ifdef __cplusplus
}
#endif
#endif
