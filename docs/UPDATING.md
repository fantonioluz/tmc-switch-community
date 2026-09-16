# Atualizações e saves

## Atualizar uma instalação

1. Feche o jogo completamente.
2. Faça uma cópia de segurança de `SD:/switch/tmc/` no computador.
3. Copie o novo `tmc.nro` e a pasta `assets/` do pacote, substituindo os arquivos correspondentes.
4. Preserve `baserom.gba`, `tmc.sav`, `config.json` e os demais arquivos pessoais.
5. Abra o jogo novamente e confira se o save aparece.

Não há atualizador próprio configurado para esta edição. As atualizações são
manuais, usando os arquivos publicados pelo mantenedor.

## Instalação anterior com outro nome de executável

Mantenha apenas um atalho do port no Homebrew Menu para não abrir a versão antiga
por engano. Um forwarder existente pode apontar para outro nome: ajuste-o para
`/switch/tmc/tmc.nro` conforme a ferramenta que você utiliza.

## Assets e configurações

Os assets do pacote são o cache fornecido pelo mantenedor. Use o conjunto da
mesma Release do NRO; o jogo também pode gerar um cache a partir da sua ROM.
Se uma versão exigir regeneração, isso deve aparecer nas notas da Release.
Para investigar cache danificado, com o jogo fechado e um backup feito, renomeie
`assets/` para `assets-backup/` e deixe a próxima abertura criar um novo cache.
Não apague a ROM nem `tmc.sav` para executar esse procedimento.

Instalações baseadas em outra ROM ou em patches não são equivalentes à base USA
validada. A compatibilidade de saves de outras versões deve ser testada numa
cópia, preservando sempre o original.
