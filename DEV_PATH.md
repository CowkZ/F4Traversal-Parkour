# Dev Path: F4Traversal (Port de Mod de Skyrim para Fallout 4)

## Visão Geral
Objetivo: Portar as funcionalidades de movimentação avançada (parkour/traversal) do Skyrim para o Fallout 4 utilizando CommonLibF4 e F4SE.

---

## FASE 1: Engenharia Reversa da Movimentação (Análise)
- [x] Identificar a classe de "Movement" ou "Character Controller" no FO4.
- [x] Localizar a função de atualização de posição (`Update/Tick`) do jogador.
- [x] Comparar a lógica de eixo Z (altura) entre Skyrim e Fallout 4.

## FASE 2: Implementação de Low-Level Hooks
- [x] Implementar sistema de Hooks/Detours para interceptar funções de física.
- [x] Validar hooks através de testes simples (ex: alterar gravidade ou congelar jogador).
- [x] Criar a estrutura de estados (`IsClimbing`, `IsSliding`, etc).

## FASE 3: Portabilidade da Lógica de Gameplay
- [x] Implementar detecção de superfícies via Raycasting (detectar paredes). -> *Implementado via `TESObjectCELL::Pick` + `bhkPickData` (método de 3 raios). AINDA NÃO TESTADO NO JOGO: liga só com F7.*
- [~] Marcador de borda: texto na HUD ([MAO] ...) implementado no código (não testado no jogo); ícone 2D (mãozinha) pendente (projeção via `NiCamera::WorldPtToScreenPt3`, desenho via Scaleform).
- [ ] Implementar sistema de animações de escalada (Trigger/Playback).
- [ ] Portar a matemática de vetores de impulso e escalada do mod original.
- [x] Vincular as ações a inputs/hotkeys.

## FASE 4: Polimento e Estabilidade
- [ ] Ajustar o "Game Feel" (física específica do FO4).
- [ ] Corrigir bugs de colisão (evitar "clipping" em paredes).
- [ ] Otimização de performance e estabilidade de memória.
