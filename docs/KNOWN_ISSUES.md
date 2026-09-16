# Estado da validação

## Corrigido e confirmado no Switch

- Portas de Hyrule fora de posição por leitura incompleta de uma tabela fragmentada.
- Pisos pretos e espadas surgindo perto de portas por leitura além do fim da tabela de interações.

Confirmação do mantenedor em 15/09/2026. Detalhes em [Correções de Hyrule](HYRULE_FIXES.md).

## Implementado, aguardando confirmação no console

- Biblioteca: corrigida a marca de script do primeiro Minish da estante, comparada com a ROM USA.
- Lake Hylia: adicionadas proteções para os acessos a sprites e contextos de NPCs descritos pela edição de referência. Sem um crash dump deste relato, não é possível afirmar que a causa observada pelo jogador era exatamente a mesma.
- Primeira instalação: corrigida a formação dos caminhos de assets com prefixo `sdmc:`.

Os testes de memória passaram. Eles não substituem entrar em Lake Hylia e conversar
com o Minish no Switch. Veja [Investigação dos NPCs](NPC_FIXES.md).

## Limitações atuais

- A campanha completa ainda não foi validada nesta edição.
- A base de referência é USA; outras regiões e ROMs modificadas não foram validadas.
- Uma instalação nova do NRO atualizado com os assets fornecidos ainda precisa de confirmação em hardware.
- Se o cache estiver ausente ou incompatível, o jogo pode extraí-lo novamente; aguarde a conclusão.
- A compilação preserva avisos existentes no port; não houve revisão integral de todos os sistemas.

## Se algo falhar

Informe versão, local, passos para reproduzir, região da ROM e se a instalação
é nova ou reaproveita assets. Uma foto ajuda em problemas visuais. O build inicial
mantém diagnóstico; se `tmc.log` existir em `switch/tmc/`, confira seu conteúdo
antes de compartilhar. Não publique ROMs, dados de conta,
configurações com credenciais ou saves pessoais em Issues públicas.
