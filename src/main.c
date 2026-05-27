#include "game.h"

int main(void) {
    // Configura a janela para ser redimensionável e ativa anti-aliasing 4x (suaviza bordas)
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    // Cria a janela do jogo com largura, altura e título definidos em game.h
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ilha dos Paradoxos");

    InitAudioDevice();  // Inicializa o sistema de áudio da Raylib
    SetTargetFPS(FPS);  // Trava o jogo em 60 quadros por segundo (definido em game.h)
    SetExitKey(0);      // Desativa o ESC como saída automática; o jogo controla isso

    // Cria a struct principal que guarda TODO o estado do jogo, começando zerada
    GameState g = {0};
    GameInit(&g); // Inicializa mapa, NPCs, diálogos, ambiente, câmera...

    // LOOP PRINCIPAL: repete enquanto o jogador não fechar a janela
    while (!WindowShouldClose()) {
        g.delta = GetFrameTime(); // Tempo do último frame em segundos (ex: 0.016 para 60fps)
        g.frame++;                // Incrementa o contador global de frames

        GameUpdate(&g); // Atualiza toda a lógica: input, movimento, diálogos, deduções...

        BeginDrawing();           // Inicia o buffer de renderização
        ClearBackground(COL_BG); // Limpa a tela com a cor de fundo escura do jogo
        GameDraw(&g);            // Desenha tudo: mapa, NPCs, UI, efeitos...
        EndDrawing();            // Envia o buffer para a tela (swap de buffers)
    }

    // Ao sair do loop: libera todos os recursos alocados
    GameCleanup(&g);    // Descarrega texturas, fontes, etc.
    CloseAudioDevice(); // Encerra o áudio
    CloseWindow();      // Fecha a janela
    return 0;
}
