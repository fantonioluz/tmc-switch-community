# Minish Cap â Switch Community Edition

> **Candidato de teste 0.1.2-rc.3:** corrige o overflow do relÃ³gio que bloqueia
> a espera entre frames e restaura a escala/rotaÃ§Ã£o dos sprites ao fechar o menu.
> Testes locais aprovados; as duas correÃ§Ãµes ainda aguardam confirmaÃ§Ã£o no Switch.

> **Candidato de teste 0.1.2-rc.3:** corrige um acesso invÃ¡lido no ataque dos gatos,
> remove escrita contÃ­nua de diÃ¡logos e adiciona relatÃ³rios de crash/congelamento.
> O percurso do Dr. Left ainda precisa de confirmaÃ§Ã£o no Switch.


<p align="center"><img src="branding/icon.jpg" width="256" height="256" alt="Ãcone de Minish Cap com Ezlo e a espada"></p>

Um port nativo de **The Legend of Zelda: The Minish Cap** para Nintendo Switch,
mantido a partir de melhorias encontradas durante as partidas.

[English](README.en.md) Â· [InstalaÃ§Ã£o](docs/INSTALLATION.md) Â· [MudanÃ§as](CHANGELOG.md) Â· [CrÃ©ditos](CREDITS.md)

Esta ediÃ§Ã£o comeÃ§a com a correÃ§Ã£o das portas de Hyrule e dos pisos pretos que
faziam uma espada aparecer perto delas. O responsÃ¡vel pelo projeto confirmou
essas correÃ§Ãµes no Switch em 15 de setembro de 2026.

**VocÃª precisa ter uma cÃ³pia original do jogo e fazer o dump do seu prÃ³prio
cartucho USA.** O pacote inclui o **NRO e os assets fornecidos pelo mantenedor**.
A ROM nÃ£o estÃ¡ incluÃ­da: cada jogador coloca seu prÃ³prio dump na pasta do jogo.

[InstruÃ§Ãµes do teste e relatÃ³rios](docs/STABILITY.md) Â· [VersÃ£o estÃ¡vel anterior](https://github.com/fantonioluz/tmc-switch-community/releases/tag/v0.1.1)

## Como jogar

1. Baixe o [candidato v0.1.2-rc.3](https://github.com/fantonioluz/tmc-switch-community/releases/download/v0.1.2-rc.3/tmc-switch-community-0.1.2-rc.3-usa.zip), ou use a pasta
   [`release/switch/tmc/`](release/switch/tmc/) deste repositÃ³rio.
2. Copie a pasta `switch` do pacote para a raiz do cartÃ£o SD.
3. Coloque o dump USA do seu cartucho em `switch/tmc/`, com o nome `baserom.gba`.
4. Abra o Homebrew Menu em modo Aplicativo: mantenha **R** pressionado ao abrir
   um jogo instalado. Inicie **Minish Cap â Switch Community**.
5. Mantenha a pasta `assets/` incluÃ­da no pacote ao lado do NRO. O jogo reutiliza
   esses arquivos; se precisar regenerÃ¡-los, aguarde a extraÃ§Ã£o terminar.

```text
SD:/
âââ switch/
    âââ tmc/
        âââ tmc.nro          â fornecido pelo projeto
        âââ assets/          â 21 arquivos incluÃ­dos no pacote
        âââ baserom.gba      â dump feito por vocÃª, nÃ£o distribuÃ­do
```

Ã necessÃ¡rio um Switch jÃ¡ preparado para executar homebrew. Use modo Aplicativo;
abrir pelo Ãlbum oferece memÃ³ria limitada. Consulte o [guia completo](docs/INSTALLATION.md).

## O que esta ediÃ§Ã£o inclui

- Port nativo para Switch baseado nos projetos listados em [CrÃ©ditos](CREDITS.md).
- CorreÃ§Ã£o do carregamento das posiÃ§Ãµes das portas de Hyrule.
- CorreÃ§Ã£o da interaÃ§Ã£o que apagava pisos e criava espadas indevidamente.
- Conversa do Minish da biblioteca e acesso a Lake Hylia corrigidos, confirmados pelo mantenedor no Switch em 16/09/2026.
- Ãcone prÃ³prio de Ezlo embutido no NRO e disponÃ­vel para criaÃ§Ã£o de atalhos.
- Testes para preservar essas correÃ§Ãµes nas prÃ³ximas versÃµes.
- CÃ³digo-fonte da ediÃ§Ã£o em [`source/`](source/), com as dependÃªncias locais usadas pelo port.

Esta base Ã© diferente da ediÃ§Ã£o Alek's Ultimate NX. O repositÃ³rio dela foi uma
referÃªncia para a organizaÃ§Ã£o desta distribuiÃ§Ã£o e a investigaÃ§Ã£o de falhas de NPCs e extraÃ§Ã£o; seus recursos exclusivos nÃ£o
sÃ£o anunciados como recursos desta ediÃ§Ã£o.

## Compatibilidade e estado do projeto

| Item | SituaÃ§Ã£o |
| --- | --- |
| ROM de referÃªncia | USA, cabeÃ§alho `BZME` |
| SHA-1 do dump de referÃªncia | `b4bd50e4131b027c334547b4524e2dbbd4227130` |
| Portas e pisos em Hyrule | CorreÃ§Ãµes confirmadas pelo mantenedor no Switch |
| Biblioteca e Lake Hylia | CorreÃ§Ãµes confirmadas pelo mantenedor no Switch em 16/09/2026 |
| Pacote com NRO e assets | Integridade e estrutura verificadas; primeira instalaÃ§Ã£o deste conjunto no Switch ainda precisa de confirmaÃ§Ã£o |
| Outras regiÃµes e ROMs modificadas | NÃ£o validadas nesta ediÃ§Ã£o |
| Jogo completo | Em validaÃ§Ã£o conforme o mantenedor joga |

O idioma desta documentaÃ§Ã£o nÃ£o altera os diÃ¡logos do jogo. A base distribuÃ­da
Ã© USA. NÃ£o hÃ¡ promessa de compatibilidade com patches de traduÃ§Ã£o.

## AtualizaÃ§Ãµes e problemas

FaÃ§a backup de `switch/tmc/`, feche o jogo e copie o NRO e a pasta `assets/` do pacote. Preserve seu
`tmc.sav`, sua ROM e suas configuraÃ§Ãµes. Veja [AtualizaÃ§Ã£o e saves](docs/UPDATING.md).

Encontrou um problema? Abra uma **Issue** com a versÃ£o, o local e os passos para
reproduzir. [Como relatar](CONTRIBUTING.md) Â· [Problemas conhecidos](docs/KNOWN_ISSUES.md).

## Para contribuir

O cÃ³digo desta ediÃ§Ã£o estÃ¡ em `source/`. Consulte o
[guia de desenvolvimento](docs/DEVELOPMENT.md) e o [guia de publicaÃ§Ã£o](docs/RELEASING.md).
Os executÃ¡veis para jogadores ficam em `release/`; os pacotes ZIP gerados ficam
em `dist/` e sÃ£o destinados aos anexos de uma Release do GitHub.

Projeto de fÃ£s, sem vÃ­nculo com Nintendo ou Capcom. Os nomes do jogo e seus
personagens pertencem aos respectivos titulares. Veja [Licenciamento](LICENSE.md)
e [avisos de terceiros](THIRD_PARTY_NOTICES.md).

## Menu Community e atualizações

No Switch, pressione **Minus (-)** durante o jogo para abrir o menu Community. Ele inclui Imagem, Audio, Controles, Saves/Backups, Diagnostico, Conquistas, Atualizacoes e Ajuda. O backup manual cria arquivos 	mc.sav.bak-AAAAMMDD-HHMMSSmc.sav.bak-AAAAMMDD-HHMMSS.

Em **Atualizacoes**, o jogo consulta a Release mais recente, baixa o 	mc.sav.bak-AAAAMMDD-HHMMSSmc.nro por HTTPS e aplica a troca no proximo boot. O NRO precisa estar anexado à Release; saves, ROM e assets permanecem preservados.

