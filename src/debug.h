#pragma once

namespace Debug
{
    // Deve rodar na thread do jogo: escreve posicao/orientacao do jogador
    // no log e mostra uma mensagem na HUD.
    void PlayerSnapshot();

    // Teste de detecção: verifica se há algo à frente do jogador
    void TestTraversalDetection();
}
