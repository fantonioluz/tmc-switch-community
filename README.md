# Minish Cap — Switch Community Edition

> **Candidato de teste 0.1.2-rc.2:** corrige um acesso inválido no ataque dos gatos,
> remove escrita contínua de diálogos e adiciona relatórios de crash/congelamento.
> O percurso do Dr. Left ainda precisa de confirmação no Switch.


<p align="center"><img src="branding/icon.jpg" width="256" height="256" alt="Ícone de Minish Cap com Ezlo e a espada"></p>

Um port nativo de **The Legend of Zelda: The Minish Cap** para Nintendo Switch,
mantido a partir de melhorias encontradas durante as partidas.

[English](README.en.md) · [Instalação](docs/INSTALLATION.md) · [Mudanças](CHANGELOG.md) · [Créditos](CREDITS.md)

Esta edição começa com a correção das portas de Hyrule e dos pisos pretos que
faziam uma espada aparecer perto delas. O responsável pelo projeto confirmou
essas correções no Switch em 15 de setembro de 2026.

**Você precisa ter uma cópia original do jogo e fazer o dump do seu próprio
cartucho USA.** O pacote inclui o **NRO e os assets fornecidos pelo mantenedor**.
A ROM não está incluída: cada jogador coloca seu próprio dump na pasta do jogo.

[Instruções do teste e relatórios](docs/STABILITY.md) · [Versão estável anterior](https://github.com/fantonioluz/tmc-switch-community/releases/tag/v0.1.1)

## Como jogar

1. Baixe o [candidato v0.1.2-rc.2](https://github.com/fantonioluz/tmc-switch-community/releases/download/v0.1.2-rc.2/tmc-switch-community-0.1.2-rc.2-usa.zip), ou use a pasta
   [`release/switch/tmc/`](release/switch/tmc/) deste repositório.
2. Copie a pasta `switch` do pacote para a raiz do cartão SD.
3. Coloque o dump USA do seu cartucho em `switch/tmc/`, com o nome `baserom.gba`.
4. Abra o Homebrew Menu em modo Aplicativo: mantenha **R** pressionado ao abrir
   um jogo instalado. Inicie **Minish Cap — Switch Community**.
5. Mantenha a pasta `assets/` incluída no pacote ao lado do NRO. O jogo reutiliza
   esses arquivos; se precisar regenerá-los, aguarde a extração terminar.

```text
SD:/
└── switch/
    └── tmc/
        ├── tmc.nro          ← fornecido pelo projeto
        ├── assets/          ← 21 arquivos incluídos no pacote
        └── baserom.gba      ← dump feito por você, não distribuído
```

É necessário um Switch já preparado para executar homebrew. Use modo Aplicativo;
abrir pelo Álbum oferece memória limitada. Consulte o [guia completo](docs/INSTALLATION.md).

## O que esta edição inclui

- Port nativo para Switch baseado nos projetos listados em [Créditos](CREDITS.md).
- Correção do carregamento das posições das portas de Hyrule.
- Correção da interação que apagava pisos e criava espadas indevidamente.
- Conversa do Minish da biblioteca e acesso a Lake Hylia corrigidos, confirmados pelo mantenedor no Switch em 16/09/2026.
- Ícone próprio de Ezlo embutido no NRO e disponível para criação de atalhos.
- Testes para preservar essas correções nas próximas versões.
- Código-fonte da edição em [`source/`](source/), com as dependências locais usadas pelo port.

Esta base é diferente da edição Alek's Ultimate NX. O repositório dela foi uma
referência para a organização desta distribuição e a investigação de falhas de NPCs e extração; seus recursos exclusivos não
são anunciados como recursos desta edição.

## Compatibilidade e estado do projeto

| Item | Situação |
| --- | --- |
| ROM de referência | USA, cabeçalho `BZME` |
| SHA-1 do dump de referência | `b4bd50e4131b027c334547b4524e2dbbd4227130` |
| Portas e pisos em Hyrule | Correções confirmadas pelo mantenedor no Switch |
| Biblioteca e Lake Hylia | Correções confirmadas pelo mantenedor no Switch em 16/09/2026 |
| Pacote com NRO e assets | Integridade e estrutura verificadas; primeira instalação deste conjunto no Switch ainda precisa de confirmação |
| Outras regiões e ROMs modificadas | Não validadas nesta edição |
| Jogo completo | Em validação conforme o mantenedor joga |

O idioma desta documentação não altera os diálogos do jogo. A base distribuída
é USA. Não há promessa de compatibilidade com patches de tradução.

## Atualizações e problemas

Faça backup de `switch/tmc/`, feche o jogo e copie o NRO e a pasta `assets/` do pacote. Preserve seu
`tmc.sav`, sua ROM e suas configurações. Veja [Atualização e saves](docs/UPDATING.md).

Encontrou um problema? Abra uma **Issue** com a versão, o local e os passos para
reproduzir. [Como relatar](CONTRIBUTING.md) · [Problemas conhecidos](docs/KNOWN_ISSUES.md).

## Para contribuir

O código desta edição está em `source/`. Consulte o
[guia de desenvolvimento](docs/DEVELOPMENT.md) e o [guia de publicação](docs/RELEASING.md).
Os executáveis para jogadores ficam em `release/`; os pacotes ZIP gerados ficam
em `dist/` e são destinados aos anexos de uma Release do GitHub.

Projeto de fãs, sem vínculo com Nintendo ou Capcom. Os nomes do jogo e seus
personagens pertencem aos respectivos titulares. Veja [Licenciamento](LICENSE.md)
e [avisos de terceiros](THIRD_PARTY_NOTICES.md).
