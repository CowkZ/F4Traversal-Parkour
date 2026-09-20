# F4Traversal (base de debug)

Plugin F4SE minimo (CommonLibF4) para validar o ciclo inteiro antes de portar a
logica de parkour: compilar -> instalar -> carregar no jogo -> acao na thread do jogo.

## O que faz

- Carrega e escreve `F4Traversal.log` (pasta `Documents/My Games/Fallout4*/F4SE`,
  dentro do prefixo do Proton se voce joga no Linux).
- Loga cada mensagem do F4SE (PostLoad, GameDataReady, PostLoadGame, ...).
- **F8 dentro do jogo**: le posicao, yaw e celula do jogador, escreve no log e
  mostra na HUD. Se isso funciona, o pipeline inteiro esta ok.

## Compilar (sem Windows)

O GitHub Actions compila em um Windows deles. O workflow (`.github/workflows/build.yml`)
baixa o CommonLibF4 sozinho, fixado no commit que mira o jogo **1.11.221**
(campo `ref:`), e gera o artefato `F4Traversal` com `F4SE/Plugins/F4Traversal.dll`.

Para mirar 1.11.240 ou mais novo, troque o `ref:` no workflow por `main`.

## Instalar e testar

1. Copie `F4SE/Plugins/F4Traversal.dll` para `Data/F4SE/Plugins/`.
2. Precisa do F4SE e do Address Library for F4SE Plugins (versao 1.11.221) e o jogo
   deve ser iniciado pelo `f4se_loader.exe`.
3. Carregue um save, aperte F8 e confira a HUD e o log.

## Proximos passos

- Raycast (deteccao de borda): o CommonLibF4 expoe `RE::bhkPickData`, mas nao a
  funcao do mundo fisico que executa o pick. Precisa achar esse ID no Address Library.
- Mover o jogador (interpolacao de posicao) e disparar animacao existente.
- Ler configuracao de um INI em vez de constantes.

Licenca: GPL-3.0 (o CommonLibF4 e GPL-3.0 com excecao de modding).
