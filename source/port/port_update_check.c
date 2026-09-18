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
#else

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "port_update_check.h"
#include "port_version.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TMC_RELEASES_URL "https://github.com/999sian/tmc/releases"
#define TMC_WIDEN2(value) L##value
#define TMC_WIDEN(value) TMC_WIDEN2(value)

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#endif

static void FreeString(char* value) {
    free(value);
}

static bool AppendBytes(char** buffer, size_t* size, size_t* capacity, const char* bytes, size_t byteCount) {
    if (*size + byteCount + 1 > *capacity) {
        size_t newCapacity = *capacity ? *capacity * 2 : 8192;
        while (*size + byteCount + 1 > newCapacity) {
            newCapacity *= 2;
        }

        char* newBuffer = (char*)realloc(*buffer, newCapacity);
        if (!newBuffer) {
            return false;
        }

        *buffer = newBuffer;
        *capacity = newCapacity;
    }

    memcpy(*buffer + *size, bytes, byteCount);
    *size += byteCount;
    (*buffer)[*size] = '\0';
    return true;
}

static char* DuplicateRange(const char* begin, const char* end) {
    size_t length = (size_t)(end - begin);
    char* result = (char*)malloc(length + 1);
    if (!result) {
        return NULL;
    }

    memcpy(result, begin, length);
    result[length] = '\0';
    return result;
}

static char* ExtractJsonString(const char* json, const char* key) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    const char* pos = strstr(json, pattern);
    if (!pos) {
        return NULL;
    }

    pos += strlen(pattern);
    while (*pos && isspace((unsigned char)*pos)) {
        pos++;
    }
    if (*pos != ':') {
        return NULL;
    }
    pos++;
    while (*pos && isspace((unsigned char)*pos)) {
        pos++;
    }
    if (*pos != '"') {
        return NULL;
    }
    pos++;

    const char* valueStart = pos;
    while (*pos) {
        if (*pos == '\\' && pos[1]) {
            pos += 2;
            continue;
        }
        if (*pos == '"') {
            return DuplicateRange(valueStart, pos);
        }
        pos++;
    }

    return NULL;
}

static bool ParseVersionNumbers(const char* version, int parts[3]) {
    const char* p = version;
    if (*p == 'v' || *p == 'V') {
        p++;
    }

    for (int i = 0; i < 3; i++) {
        if (!isdigit((unsigned char)*p)) {
            return false;
        }

        int value = 0;
        while (isdigit((unsigned char)*p)) {
            value = value * 10 + (*p - '0');
            p++;
        }
        parts[i] = value;

        if (i < 2) {
            if (*p != '.') {
                return false;
            }
            p++;
        }
    }

    return true;
}

static bool IsNewerVersion(const char* latestTag, const char* currentVersion) {
    int latest[3] = { 0, 0, 0 };
    int current[3] = { 0, 0, 0 };

    if (!ParseVersionNumbers(latestTag, latest) || !ParseVersionNumbers(currentVersion, current)) {
        return false;
    }

    for (int i = 0; i < 3; i++) {
        if (latest[i] > current[i]) {
            return true;
        }
        if (latest[i] < current[i]) {
            return false;
        }
    }

    return false;
}

#ifdef _WIN32
static char* FetchLatestReleaseJson(void) {
    char* response = NULL;
    size_t responseSize = 0;
    size_t responseCapacity = 0;

    HINTERNET session = WinHttpOpen(L"tmc_pc/" TMC_WIDEN(TMC_PC_VERSION),
                                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME,
                                    WINHTTP_NO_PROXY_BYPASS,
                                    0);
    if (!session) {
        return NULL;
    }

    WinHttpSetTimeouts(session, 3000, 3000, 3000, 3000);

    HINTERNET connect = WinHttpConnect(session, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connect) {
        WinHttpCloseHandle(session);
        return NULL;
    }

    HINTERNET request = WinHttpOpenRequest(connect,
                                           L"GET",
                                           L"/repos/999sian/tmc/releases?per_page=1",
                                           NULL,
                                           WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           WINHTTP_FLAG_SECURE);
    if (!request) {
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);
        return NULL;
    }

    static const wchar_t headers[] =
        L"Accept: application/vnd.github+json\r\n"
        L"User-Agent: tmc_pc\r\n";

    BOOL ok = WinHttpSendRequest(request,
                                 headers,
                                 (DWORD)-1L,
                                 WINHTTP_NO_REQUEST_DATA,
                                 0,
                                 0,
                                 0) &&
              WinHttpReceiveResponse(request, NULL);

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    if (ok) {
        ok = WinHttpQueryHeaders(request,
                                 WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                 WINHTTP_HEADER_NAME_BY_INDEX,
                                 &statusCode,
                                 &statusSize,
                                 WINHTTP_NO_HEADER_INDEX) &&
             statusCode == 200;
    }

    while (ok) {
        DWORD available = 0;
        if (!WinHttpQueryDataAvailable(request, &available) || available == 0) {
            break;
        }

        char* chunk = (char*)malloc(available);
        if (!chunk) {
            ok = FALSE;
            break;
        }

        DWORD bytesRead = 0;
        if (!WinHttpReadData(request, chunk, available, &bytesRead)) {
            free(chunk);
            ok = FALSE;
            break;
        }

        ok = AppendBytes(&response, &responseSize, &responseCapacity, chunk, bytesRead);
        free(chunk);
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);

    if (!ok || responseSize == 0) {
        FreeString(response);
        return NULL;
    }

    return response;
}
#else
static char* ReadCommandOutput(const char* command) {
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        return NULL;
    }

    char* response = NULL;
    size_t responseSize = 0;
    size_t responseCapacity = 0;
    char chunk[4096];

    while (fgets(chunk, sizeof(chunk), pipe)) {
        if (!AppendBytes(&response, &responseSize, &responseCapacity, chunk, strlen(chunk))) {
            FreeString(response);
            pclose(pipe);
            return NULL;
        }
    }

    int status = pclose(pipe);
    if (status != 0 || responseSize == 0) {
        FreeString(response);
        return NULL;
    }

    return response;
}

static char* FetchLatestReleaseJson(void) {
    char* response = ReadCommandOutput(
        "curl -fsSL --max-time 3 -H 'Accept: application/vnd.github+json' "
        "-H 'User-Agent: tmc_pc' 'https://api.github.com/repos/999sian/tmc/releases?per_page=1' 2>/dev/null");
    if (response) {
        return response;
    }

    return ReadCommandOutput(
        "wget -q -T 3 --header='Accept: application/vnd.github+json' "
        "--header='User-Agent: tmc_pc' -O - 'https://api.github.com/repos/999sian/tmc/releases?per_page=1' 2>/dev/null");
}
#endif

static void ShowUpdateDialog(SDL_Window* window, const char* latestTag, const char* releaseUrl) {
    char message[512];
    snprintf(message,
             sizeof(message),
             "A new version of TMC PC is available.\n\n"
             "Installed version: %s\n"
             "Latest version: %s\n\n"
             "Open the GitHub release page?",
             TMC_PC_VERSION,
             latestTag);

    const SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Later" },
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "View release" },
    };
    const SDL_MessageBoxData data = {
        SDL_MESSAGEBOX_INFORMATION,
        window,
        "Update available",
        message,
        (int)(sizeof(buttons) / sizeof(buttons[0])),
        buttons,
        NULL,
    };

    int buttonId = 0;
    if (SDL_ShowMessageBox(&data, &buttonId) && buttonId == 1) {
        SDL_OpenURL(releaseUrl && releaseUrl[0] ? releaseUrl : TMC_RELEASES_URL);
    }
}

void Port_CheckForUpdates(SDL_Window* window) {
    char* json = FetchLatestReleaseJson();
    if (!json) {
        return;
    }

    char* latestTag = ExtractJsonString(json, "tag_name");
    char* releaseUrl = ExtractJsonString(json, "html_url");

    if (latestTag && IsNewerVersion(latestTag, TMC_PC_VERSION)) {
        ShowUpdateDialog(window, latestTag, releaseUrl ? releaseUrl : TMC_RELEASES_URL);
    }

    FreeString(releaseUrl);
    FreeString(latestTag);
    FreeString(json);
}

#endif /* __SWITCH__ */
