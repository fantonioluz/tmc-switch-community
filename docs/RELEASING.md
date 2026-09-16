# Preparar e publicar versões

## Repositório no GitHub

O repositório desta edição é [fantonioluz/tmc-switch-community](https://github.com/fantonioluz/tmc-switch-community).
Para começar a contribuir, clone-o:

```sh
git clone https://github.com/fantonioluz/tmc-switch-community.git
cd tmc-switch-community
```

A primeira publicação usa a tag `v0.1.0`, marcada como pré-lançamento enquanto
a instalação nova e os ajustes de biblioteca/Lake Hylia aguardam confirmação no console.
As próximas versões devem registrar os resultados desses testes.

Esta edição é um snapshot derivado dos projetos creditados, com layout próprio;
não usa a relação visual de Fork do GitHub. As origens estão em `source/UPSTREAM.json`.

## Gerar o pacote

1. Atualize `version.json`, `CHANGELOG.md` e as notas de validação.
2. Execute `bash scripts/build_switch.sh` no ambiente devkitPro.
3. Execute os testes de regressão com seu dump USA.
4. Execute `python scripts/validate_repository.py`.
5. Confirme no Switch a instalação com os assets incluídos e os locais afetados pelas mudanças.

Para reempacotar um ELF já compilado, sem recompilar o jogo:

```sh
python scripts/package_release.py --elf source/platforms/switch/tmc_switch_usa.elf
```

Para gerar o NRO de um ELF, o empacotador precisa de `elf2nro` e `nacptool` no PATH
e de `DEVKITPRO` para o ícone padrão. Para apenas refazer o ZIP com um NRO pronto,
use `python scripts/package_release.py --nro release/switch/tmc/tmc.nro`;
nesse modo, basta Python. Ele cria `release/switch/tmc/tmc.nro`, `release/manifest.json`,
`release/SHA256SUMS.txt` e o ZIP em `dist/`. Os assets ficam externos ao NRO,
em `switch/tmc/assets/`. O empacotador exige os 21 arquivos de `scripts/release_assets.py`,
verifica a estrutura dos PAKs e registra o SHA-256 de cada arquivo. O conjunto de
arquivos permitido no pacote não inclui uma ROM. Veja [Assets](ASSETS.md).

## Conteúdo da publicação

- Envie código, documentação, scripts, licenças e a pasta `release/` para o repositório.
- Anexe o ZIP de `dist/` à Release correspondente, com as notas do changelog.
- Preserve a árvore-fonte e os avisos de licença correspondentes ao binário.
- ROM, saves, configurações pessoais, logs e builds intermediários ficam fora
  da publicação. O `.gitignore` libera apenas os assets especificados da Release.
- Ao trocar os assets, atualize o conjunto completo e regenere manifesto, hashes e ZIP.

O ZIP leva `switch/tmc/tmc.nro`, `switch/tmc/assets/`, instruções e avisos de licença. Seu jogador
acrescenta o próprio `baserom.gba` no SD.

## Checklist desta primeira versão

- [x] Correções de Hyrule confirmadas pelo mantenedor em hardware.
- [x] Testes de regressão implementados.
- [x] Pacote com NRO e assets externos preparado, sem ROM.
- [ ] Confirmar uma primeira instalação desse pacote no Switch, usando os assets incluídos.
- [x] Preparar a publicação em `fantonioluz/tmc-switch-community`, com tag `v0.1.0`.
