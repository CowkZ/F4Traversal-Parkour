# Dev Path: F4Traversal (Port de Mod de Skyrim para Fallout 4)

## Visão Geral
Objetivo: Portar as funcionalidades de movimentação avançada (parkour/traversal) do Skyrim para o Fallout 4 utilizando CommonLibF4 e F4SE.

---

## FASE 1: Engenharia Reversa da Movimentação (Análise)
- [ ] Identificar a classe de "Movement" ou "Character Controller" no FO4.
- [ ] Localizar a função de atualização de posição (`Update/Tick`) do jogador.
- [ ] Comparar a lógica de eixo Z (altura) entre Skyrim e Fallout 4.

## FASE 2: Implementação de Low-Level Hooks
- [ ] Implementar sistema de Hooks/Detours para interceptar funções de física.
- [ ] Validar hooks através de testes simples (ex: alterar gravidade ou congelar jogador).
- [ ] Criar a estrutura de estados (`IsClimbing`, `IsSliding`, etc).

## FASE 3: Portabilidade da Lógica de Gameplay
- [ ] Implementar detecção de superfícies via Raycasting (detectar paredes).
- [ ] Portar a matemática de vetores de impulso e escalada do mod original.
- [ ] Vincular as ações a inputs/hotkeys.

## FASE 4: Polimento e Estabilidade
- [ ] Ajustar o "Game Feel" (física específica do FO4).
- [ ] Corrigir bugs de colisão (evitar "clipping" em paredes).
- [ ] Otimização de performance e estabilidade de memória.
