# Créditos e origem

Esta edição acrescenta correções e organização de distribuição a um trabalho
coletivo. A decompilação e a base do port foram feitas pelos projetos abaixo.

| Projeto | Papel |
| --- | --- |
| [zeldaret/tmc](https://github.com/zeldaret/tmc) | Decompilação de The Minish Cap |
| [999sian/tmc — Project Picori](https://github.com/999sian/tmc) | Base do port nativo para PC |
| [HayatoG/tmc](https://github.com/HayatoG/tmc) | Base upstream direta da árvore de desenvolvimento |
| [fantonioluz/tmc](https://github.com/fantonioluz/tmc) | Repositório de trabalho do qual esta edição foi preparada |
| [VirtuaPPU](https://github.com/MatheoVignaud/VirtuaPPU) e [VirtuaAPU](https://github.com/MatheoVignaud/VirtuaAPU) | Componentes de vídeo e áudio |
| [agbplay](https://github.com/ipatix/agbplay) | Motor de áudio usado pelo port |
| [RetroAchievements/rcheevos](https://github.com/RetroAchievements/rcheevos) | Biblioteca de integração com RetroAchievements |
| [devkitPro](https://devkitpro.org/) | Ferramentas e bibliotecas de desenvolvimento para Switch |
| [EstebanPdN/zelda-tmc-3ds](https://github.com/EstebanPdN/zelda-tmc-3ds) | Referência consultada na investigação das interações com pisos |
| [Alek's Ultimate NX Edition](https://github.com/Alexgg1014/The-Legend-of-Zelda-The-Minish-Cap-Alek-s-Ultimate-NX-Edition) | Referência de organização e investigação de crashes de NPCs e da extração inicial |

O snapshot parte do commit `70e61b45c2203a220ead3c995a61b3119bf8b9c5`, acrescido
das alterações locais presentes durante a preparação. As revisões das bibliotecas
estão em [`source/UPSTREAM.json`](source/UPSTREAM.json).

As bibliotecas em `source/libs/` foram copiadas como arquivos comuns, preservando
as alterações locais utilizadas na compilação. Não é necessário inicializar
submódulos para usar este snapshot.

O mantenedor decide as melhorias e realiza a validação no console. Houve auxílio
do Codex na investigação, implementação das correções, testes e preparação deste
repositório. Isso não atribui ao projeto a autoria do trabalho upstream.

Nintendo e Capcom são responsáveis pelo jogo original. Esta edição é um projeto
independente de fãs, sem afiliação ou endosso dessas empresas.
