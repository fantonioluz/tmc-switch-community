# Minish Cap — arquivos para o cartão SD

> **Candidato de teste 0.1.2-rc.2:** corrige um acesso inválido no ataque dos gatos,
> remove escrita contínua de diálogos e adiciona relatórios de crash/congelamento.
> O percurso do Dr. Left ainda precisa de confirmação no Switch.


Este pacote contém o NRO e os 21 assets fornecidos pelo mantenedor. **A ROM não
está incluída.** Cada jogador deve ter uma cópia original USA e fazer seu próprio dump.

1. Copie a pasta `switch/` inteira para a raiz do SD.
2. Coloque seu dump em `SD:/switch/tmc/baserom.gba`, ao lado de `tmc.nro`.
3. Preserve `SD:/switch/tmc/assets/` com todos os arquivos, inclusive
   `.asset_build_state.json` (com o ponto inicial).
4. Abra o Homebrew Menu em modo Aplicativo, segurando R ao abrir um jogo instalado,
   e inicie Minish Cap — Switch Community.

Se o jogo regenerar os assets, aguarde a conclusão. Faça backup de saves e
configurações antes de atualizar. O mantenedor confirmou as correções da biblioteca
e de Lake Hylia no Switch em 16/09/2026. O NRO da versão 0.1.1 já inclui o ícone novo;
para atalhos da tela inicial, consulte `docs/ICON.md` e use `branding/icon.jpg`.

O ZIP inclui `docs/INSTALLATION.md` e `docs/ASSETS.md`. No repositório, os mesmos
guias ficam na pasta `docs/` da raiz. `manifest.json` e `SHA256SUMS.txt` identificam
o NRO e todos os assets; não copiam ou distribuem sua ROM.
