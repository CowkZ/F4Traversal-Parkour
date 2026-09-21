#!/usr/bin/env bash
# Checagem OFFLINE (Linux/gcc) com headers falsos. Nao substitui o build do CI nem o teste no jogo.
set -euo pipefail
cd "$(dirname "$0")/../.."
g++ -std=c++23 -Wall -Wextra -include tools/offline_test/stub.h -Isrc \
    tools/offline_test/test.cpp src/ledge_detector.cpp -o /tmp/f4traversal_offline_test
/tmp/f4traversal_offline_test | grep -E "FAIL|TUDO OK|COM FALHAS"
for f in src/main.cpp src/hotkey.cpp src/debug.cpp; do
    g++ -std=c++23 -Wall -Wextra -fsyntax-only -include tools/offline_test/stub2.h \
        -Itools/offline_test/win -Isrc "$f"
done
echo "sintaxe ok: main.cpp hotkey.cpp debug.cpp ledge_detector.cpp"
