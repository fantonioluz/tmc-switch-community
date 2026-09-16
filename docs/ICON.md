# Ícone do port e atalhos

---

## Índice

- [Introdução](#introdução)
- [Uso](#uso)
- [Locais do código](#locais-do-código)
- [TODO](#todo)
- [Limitações](#limitações)

## Introdução

A versão 0.1.1 incorpora a arte aprovada de Ezlo e da espada ao próprio NRO.
O arquivo usado é um JPEG RGB de 256×256, sem transparência nem metadados EXIF.
Esse é o [formato de ícone do NRO](https://switchbrew.org/wiki/NRO0).

## Uso

### Homebrew Menu

Substitua `SD:/switch/tmc/tmc.nro` pelo novo arquivo. O ícone faz parte do NRO;
não precisa colocar um JPEG solto na pasta do jogo para o Homebrew Menu lê-lo.

### Atalho na tela inicial

Ao criar um atalho, importe o NRO desta versão na ferramenta que você já utiliza.
Ferramentas como [NTON](https://github.com/rlaphoenix/nton) extraem a imagem e os
dados do NRO durante a criação do atalho. Se sua ferramenta pedir uma imagem
separada, use `branding/icon.jpg`, incluído no ZIP. Configure o destino como
`/switch/tmc/tmc.nro`.

Um atalho instalado tem sua própria imagem. Para trocar a arte na tela inicial,
recrie/atualize esse atalho com o NRO novo ou com o JPEG fornecido. Apenas substituir
o NRO no SD não reescreve a imagem de um atalho que já estava instalado.

O pacote não contém um NSP/atalho pronto. Preserve saves e a ROM do jogador ao atualizar.

## Locais do código

| Recurso | Local | Descrição |
| --- | --- | --- |
| Arte original | `branding/minish-cap-icon-v1.png` | Imagem gerada com image_gen e aprovada pelo mantenedor |
| Ícone do executável | `branding/icon.jpg` | Conversão para 256×256 usada pelo NRO |
| Prompt | `branding/minish-cap-icon-v1-prompt.txt` | Registro da geração original |
| Build Switch | `source/platforms/switch/Makefile` | Dependência explícita do ícone e da versão |
| Empacotamento | `scripts/package_release.py` | Passa o JPEG a elf2nro e inclui cópia no ZIP |
| Verificação | `scripts/nro_branding.py` | Compara a imagem e os dados NACP embutidos |

## TODO

- [ ] Confirmar a exibição no Homebrew Menu e em um atalho criado no console.

## Limitações

O arquivo e sua incorporação ao NRO foram verificados automaticamente. A arte
se refere ao jogo original e não representa vínculo oficial com Nintendo ou Capcom.
A atualização do pacote não instala nem altera atalhos no console automaticamente.
