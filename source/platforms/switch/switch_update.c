#include <stdio.h>
#include <unistd.h>

void Port_Switch_ApplyPendingUpdate(void) {
    if (access("tmc.nro.update", F_OK) != 0) return;
    remove("tmc.nro.previous");
    if (rename("tmc.nro", "tmc.nro.previous") != 0) return;
    if (rename("tmc.nro.update", "tmc.nro") != 0) {
        rename("tmc.nro.previous", "tmc.nro");
        return;
    }
    FILE* f = fopen("update.log", "ab");
    if (f) { fputs("[update] Atualizacao aplicada no boot\n", f); fclose(f); }
}
