#pragma once

namespace Debug
{
    // Deve rodar na thread do jogo: escreve posicao/orientacao do jogador
    // no log e mostra uma mensagem na HUD.
    void PlayerSnapshot();

    // Mostra a direcao da frente (pelo yaw) e o ultimo resultado do detector.
    void TestTraversalDetection();
}
