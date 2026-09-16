# Minish Cap — arquivos para o cartão SD

Este pacote contém o NRO e os 21 assets fornecidos pelo mantenedor. **A ROM não
está incluída.** Cada jogador deve ter uma cópia original USA e fazer seu próprio dump.

1. Copie a pasta `switch/` inteira para a raiz do SD.
2. Coloque seu dump em `SD:/switch/tmc/baserom.gba`, ao lado de `tmc.nro`.
3. Preserve `SD:/switch/tmc/assets/` com todos os arquivos, inclusive
   `.asset_build_state.json` (com o ponto inicial).
4. Abra o Homebrew Menu em modo Aplicativo, segurando R ao abrir um jogo instalado,
   e inicie Minish Cap — Switch Community.

Se o jogo regenerar os assets, aguarde a conclusão. Faça backup de saves e
configurações antes de atualizar. Os ajustes novos de biblioteca e Lake Hylia
ainda aguardam confirmação no console.

O ZIP inclui `docs/INSTALLATION.md` e `docs/ASSETS.md`. No repositório, os mesmos
guias ficam na pasta `docs/` da raiz. `manifest.json` e `SHA256SUMS.txt` identificam
o NRO e todos os assets; não copiam ou distribuem sua ROM.
