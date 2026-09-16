# Desenvolver e testar

---

## Índice

- [Introdução](#introdução)
- [Plano de trabalho](#plano-de-trabalho)
- [Locais do código](#locais-do-código)
- [TODO](#todo)
- [Limitações e bugs](#limitações-e-bugs)

## Introdução

`source/` contém o snapshot corrigido usado por esta edição. As dependências
locais estão incluídas como arquivos comuns. `source/UPSTREAM.json` registra a
origem e as revisões; esta organização não mantém os antigos submódulos Git.

## Plano de trabalho

### Compilar para Switch

Use Linux, WSL ou o ambiente compatível do devkitPro, com Python 3.10+, Make,
devkitA64, libnx e as portlibs Switch. A lista de link está em
`source/platforms/switch/Makefile`: SDL2, png, zlib, EGL/Mesa, drm_nouveau, curl e mbedTLS.
Consulte a [documentação devkitPro](https://devkitpro.org/wiki/Getting_Started)
para instalar a toolchain e os pacotes correspondentes. Defina `DEVKITPRO`.

Na raiz deste repositório:

```sh
bash scripts/build_switch.sh
```

O script regenera a lista de fontes, compila a versão USA e gera o pacote público
com os assets externos de `release/switch/tmc/assets/`, sem ROM. Não é preciso colocar uma ROM em `source/` para compilar
essa versão do port: os cabeçalhos e tabelas de compilação necessários estão no snapshot.
A ROM do próprio usuário é necessária para jogar e executar os testes comparativos.

O build inicial mantém diagnósticos (`RELEASE=0`) para facilitar relatos.
Para alterar o modo de compilação, use uma pasta de objetos nova ou remova
apenas os objetos gerados antes de reconstruir: o Makefile não trata a troca
de flags como dependência de todos os objetos.

### Testes de regressão

Com compiladores nativos GCC/G++ e Python no PATH, forneça o caminho de seu dump:

```sh
bash scripts/test_regressions.sh /caminho/para/baserom.gba
```

Para verificar o empacotamento, incluindo a identidade do ícone e a versão embutida,
execute `python scripts/test_tools.py`.

Os testes usam ASan/UBSan para detectar acessos inválidos à memória. Não
distribua a ROM junto aos testes. A suíte cobre pisos, portas, sobreposições,
o Minish da biblioteca, limites de sprites de NPCs, contexto de Festari e caminhos
de assets. Os testes nativos não emulam uma partida completa no Switch. Para validar o pacote e a documentação:

```sh
python scripts/validate_repository.py
```

### Incorporar melhorias

Altere `source/`, registre passos de reprodução e preserve os testes existentes.
Atualize versão, changelog e estado da validação ao preparar uma Release.
O antigo repositório de trabalho não é sincronizado automaticamente com esta cópia.

## Locais do código

| Recurso | Local | Descrição |
| --- | --- | --- |
| Implementação do jogo | `source/src/`, `source/include/` | Código da base e correções |
| Camada do port | `source/port/` | Carregamento, renderização, áudio e integração |
| Switch | `source/platforms/switch/` | Build, libnx e adaptação SDL |
| Extração | `source/tools/src/assets_extractor/` | Gera assets da ROM do jogador |
| Metadados de extração | `source/assets/` | Descrições de offsets; não é um cache gráfico/sonoro |
| Distribuição | `scripts/package_release.py`, `scripts/release_assets.py`, `release/` | Empacotamento e validação de NRO + assets, sem ROM |
| Dados de versão | `version.json` | Nome, versão e região da edição |

## TODO

- [ ] Confirmar instalação nova no hardware com o NRO e os assets do pacote público.
- [ ] Documentar cenários adicionais conforme novos bugs forem corrigidos.

## Limitações e bugs

USA é a base validada. Os avisos existentes da compilação não foram todos
resolvidos. Documentos antigos dentro de `source/` registram o histórico upstream;
os guias da raiz deste repositório descrevem a distribuição atual.
