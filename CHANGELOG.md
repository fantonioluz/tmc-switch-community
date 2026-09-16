# Histórico de mudanças

## 0.1.1 — 2026-09-16

- Ícone aprovado de Ezlo incorporado ao NRO como JPEG RGB de 256×256.
- JPEG incluído no pacote para ferramentas de criação de atalhos da tela inicial.
- Makefile e empacotador usam o ícone do projeto e a versão de `version.json`.
- Verificação do ícone e dos dados de nome, autor e versão realmente embutidos no NRO.
- Conversa do Minish da biblioteca e acesso a Lake Hylia confirmados no Switch pelo mantenedor em 16/09/2026.
- README, instalação, roadmap e notas de validação atualizados.

O código executável do jogo e os 21 assets são os mesmos da versão 0.1.0;
esta versão muda o ícone, os metadados e a documentação. Uma instalação nova
e a aparência do ícone no console continuam pendentes de confirmação.

## 0.1.0 — 2026-09-16

Primeira versão local da Switch Community Edition. A publicação no GitHub é feita pelo mantenedor.

### Corrigido

- Posições das portas de Hyrule: carregamento completo da tabela de propriedades fragmentada.
- Pisos pretos e espadas indevidas próximos às portas: término explícito das tabelas de interação e superfície.
- Limites de acesso às definições de interação e aos bits de ação.
- Ajustes anteriores da sobreposição de grama/água nos pés preservados.

- Minish da estante da biblioteca: restaurada a marca de entidade com script (`0x4f`), conforme a entrada da ROM USA.
- Lake Hylia: consultas de sprites de Townsperson/Kid verificam limites; Festari e Stockwell usam o contexto de script nativo; a exclusão de entidades limpa esse contexto.
- Extração no Switch: caminhos relativos dos assets calculados sem canonicalizar o prefixo `sdmc:`.

### Incluído

- Código-fonte correspondente ao estado local corrigido, com bibliotecas locais e registro de origem.
- NRO com assets externos: 21 arquivos fornecidos pelo mantenedor em `switch/tmc/assets/`; a ROM fica a cargo do jogador.
- Marcador de cache nomeado `.asset_build_state.json`, conforme o carregador, preservando o conteúdo enviado.
- Manifesto e SHA-256 para o NRO e cada asset; empacotamento por lista explícita de arquivos.
- Guias de instalação, atualização, desenvolvimento e publicação.
- Testes de portas, renderização e interação com pisos.

### Validação

- Portas e desaparecimento dos blocos/espadas confirmados pelo mantenedor no Switch.
- 65.536 consultas de piso e 224 interações com pisos comuns aprovadas nos testes com ASan/UBSan.
- 15 portas, 2 scripts e 32 casos de sobreposição aprovados.
- Teste da entidade da biblioteca contra a ROM e 512 consultas de tipos de NPC com ASan/UBSan.
- Teste de contexto ausente/desatualizado de Festari e de caminhos de assets no Switch.
- No lançamento de 0.1.0, biblioteca e Lake Hylia aguardavam teste no console. A confirmação posterior está registrada em 0.1.1.
- Estrutura dos 9 PAKs e leitura dos 12 JSONs verificadas; cópias conferidas por SHA-256.
- Primeira instalação do NRO atualizado com os assets fornecidos: ainda precisa de confirmação em hardware.

## Próximas versões

Registrar aqui cada mudança depois de implementada, com a região testada e o
resultado da validação. Os planos ficam em [ROADMAP.md](ROADMAP.md).
