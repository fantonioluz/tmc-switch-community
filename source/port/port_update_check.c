#ifdef __SWITCH__
#include "port_update_check.h"
#include "port_version.h"
#include "../platforms/switch/switch_net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TMC_REPO_API "https://api.github.com/repos/fantonioluz/tmc-switch-community/releases/latest"
static char sLatest[32];
static char sStatus[128] = "Pressione A para verificar";
static int sAvailable;
static void UpdateLog(const char* message) { FILE* file=fopen("update.log","ab"); if(file){fprintf(file,"[update] %s\n",message);fclose(file);} }
static int JsonString(const char* json,const char* key,char* out,size_t cap) { char needle[64]; snprintf(needle,sizeof(needle),"\"%s\"",key); const char* p=strstr(json,needle); if(!p)return 0; p=strchr(p+strlen(needle),':'); if(!p)return 0; p=strchr(p,'\"'); if(!p)return 0; ++p; size_t n=0; while(p[n]&&p[n]!='\"'&&n+1<cap){out[n]=p[n];++n;} out[n]='\0'; return n!=0; }
static int IsNewer(const char* tag) { int a[3]={0,0,0},b[3]={0,0,0}; if(sscanf(tag,"v%d.%d.%d",&a[0],&a[1],&a[2])!=3||sscanf(TMC_COMMUNITY_VERSION,"%d.%d.%d",&b[0],&b[1],&b[2])!=3)return 0; for(int i=0;i<3;++i){if(a[i]!=b[i])return a[i]>b[i];} return 0; }
static int DownloadNro(const char* url) { char* body=NULL; size_t len=0; long status=Port_Net_HttpRequest(url,NULL,NULL,&body,&len); if(status!=200||!body||len<1024*1024||len<0x14||memcmp(body+0x10,"NRO0",4)!=0){char m[128];snprintf(m,sizeof(m),"Download falhou HTTP %ld bytes %lu",status,(unsigned long)len);UpdateLog(m);free(body);return 0;} FILE* file=fopen("tmc.nro.update","wb"); int ok=file&&fwrite(body,1,len,file)==len; if(file)fclose(file); free(body); if(!ok)remove("tmc.nro.update"); return ok; }
int Port_Update_Check(void) { sAvailable=0;sLatest[0]='\0';strcpy(sStatus,"Verificando atualizacoes..."); char* json=NULL;size_t len=0;long status=Port_Net_HttpRequest(TMC_REPO_API,NULL,NULL,&json,&len); if(status!=200||!json){snprintf(sStatus,sizeof(sStatus),"Falha na rede (HTTP %ld)",status);UpdateLog(sStatus);free(json);return -1;} if(!JsonString(json,"tag_name",sLatest,sizeof(sLatest))){strcpy(sStatus,"Resposta do GitHub invalida");UpdateLog(sStatus);free(json);return -1;} char m[128];snprintf(m,sizeof(m),"Remota %s; atual %s",sLatest,TMC_COMMUNITY_VERSION);UpdateLog(m);sAvailable=IsNewer(sLatest);snprintf(sStatus,sizeof(sStatus),"%s",sAvailable?"Atualizacao disponivel":"Voce ja esta atualizado");free(json);return sAvailable?1:0; }
int Port_Update_Download(void) { if(!sAvailable||!sLatest[0])return 0; char url[512];snprintf(url,sizeof(url),"https://raw.githubusercontent.com/fantonioluz/tmc-switch-community/%s/release/switch/tmc/tmc.nro",sLatest);if(!DownloadNro(url)){strcpy(sStatus,"Download falhou; veja update.log");return 0;}strcpy(sStatus,"Baixado; reinicie o jogo para aplicar");UpdateLog("NRO baixado; sera aplicado no proximo inicio");return 1; }
const char* Port_Update_CurrentVersion(void){return TMC_COMMUNITY_VERSION;} const char* Port_Update_LatestVersion(void){return sLatest[0]?sLatest:"desconhecida";} const char* Port_Update_Status(void){return sStatus;} int Port_Update_IsAvailable(void){return sAvailable;}
void Port_CheckForUpdates(SDL_Window* window){(void)window;Port_Update_Check();}
#else
#include "port_update_check.h"
void Port_CheckForUpdates(SDL_Window* window){(void)window;} int Port_Update_Check(void){return 0;} int Port_Update_Download(void){return 0;} const char* Port_Update_CurrentVersion(void){return "0.0.0";} const char* Port_Update_LatestVersion(void){return "desconhecida";} const char* Port_Update_Status(void){return "Indisponivel nesta plataforma";} int Port_Update_IsAvailable(void){return 0;}
#endif
