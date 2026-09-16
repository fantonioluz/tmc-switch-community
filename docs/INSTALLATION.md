# Instalar e jogar

## Requisitos

- Nintendo Switch preparado para executar homebrew, com Homebrew Menu e modo Aplicativo.
- Cartão SD com espaço livre para o programa, sua ROM e os assets gerados.
- Uma cópia original USA de The Legend of Zelda: The Minish Cap e um dump feito por você.
- O arquivo `tmc.nro` e a pasta `assets/` desta edição, incluídos no pacote.

Este guia começa com o ambiente de homebrew já configurado. A documentação do
[Nintendo Homebrew](https://github.com/nh-server/switch-guide) explica esse ambiente.

## 1. Fazer o dump do cartucho

Dump é a cópia dos dados do seu cartucho para um arquivo. Use um leitor compatível
com cartuchos de Game Boy Advance, ou outro método de leitura que você já utilize,
seguindo a documentação do equipamento. Selecione a leitura da **ROM** e salve
o resultado em formato `.gba`. O arquivo de save do cartucho é outro arquivo.

A edição inicial utiliza o cartucho **USA**. Renomear uma ROM de outra região
não a transforma na versão USA. Não aplique patches antes de validar o dump.
O projeto não fornece downloads de ROMs.

Verificação opcional com Python 3.10 ou mais recente, na pasta do repositório:

```sh
python scripts/verify_rom.py "caminho/para/seu/dump.gba"
```

O verificador apenas lê o arquivo e compara tamanho, cabeçalho e SHA-1 com o dump
USA de referência; ele não copia, modifica ou envia a ROM. Um hash diferente
significa que o arquivo não corresponde à base validada desta edição.

## 2. Copiar para o cartão SD

Extraia o ZIP de uma Release e copie sua pasta `switch` para a raiz do SD.
Se estiver usando o repositório, copie `release/switch` para o SD.

Depois coloque seu dump em `SD:/switch/tmc/baserom.gba`:

```text
SD:/switch/tmc/
├── tmc.nro
├── assets/
│   ├── .asset_build_state.json
│   ├── animations.pak
│   ├── ... outros arquivos .pak e .json do pacote
│   └── tilemaps.pak
└── baserom.gba    ← seu dump, não incluído
```

Confira se o Windows não criou `baserom.gba.gba` ao ocultar extensões. Para evitar
que outra ROM seja escolhida pelo carregador, mantenha apenas o dump desejado
nessa pasta. Os assets já estão incluídos no ZIP. Copie todos os 21 arquivos, inclusive
`.asset_build_state.json` (o nome começa com um ponto). Veja [Assets](ASSETS.md).

## 3. Abrir o jogo

1. Abra o Homebrew Menu em modo Aplicativo, mantendo **R** pressionado enquanto
   abre um jogo instalado.
2. Selecione **Minish Cap — Switch Community**.
3. O jogo deve reconhecer os assets incluídos. Se iniciar uma extração por
   ausência ou incompatibilidade do cache, mantenha-o aberto até terminar.

Evite iniciar pelo Álbum: o modo applet oferece memória limitada.
A pasta `assets/` fornecida vai no SD junto do executável. Outros dados de execução,
como `rom_data/`, saves e configurações, são locais e não fazem parte do pacote.

## Ícone e atalho na tela inicial

O NRO inclui o ícone próprio do projeto. Para criar ou atualizar um atalho,
veja [Ícone e atalhos](ICON.md).

## Controles básicos

| Switch | Ação |
| --- | --- |
| Direcional / analógico esquerdo | Mover Link |
| A / B | Usar as ações e os itens atribuídos a A / B |
| L / R | Ações correspondentes do GBA |
| + | Start / menu do jogo |

Os controles e recursos adicionais seguem a configuração do port. Configurações
antigas podem alterar o comportamento; preserve um backup antes de mudá-las.

## Saves e primeira instalação

O save principal é `SD:/switch/tmc/tmc.sav`. Faça backup da pasta inteira antes
de atualizar. Não é necessário apagar seu save para instalar esta edição.

As correções anteriores de portas e pisos foram confirmadas em hardware.
A conversa na biblioteca e o acesso a Lake Hylia foram confirmados pelo mantenedor no Switch em 16/09/2026. O pacote público inclui
os assets enviados pelo mantenedor. Uma instalação nova com esse conjunto e
o NRO atualizado ainda precisa de confirmação no Switch.

Se houver falha, consulte [Problemas conhecidos](KNOWN_ISSUES.md) e
[Como relatar um problema](../CONTRIBUTING.md).
