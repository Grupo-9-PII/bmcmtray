# BMCM Tray Launcher Qt

Nova implementação nativa do launcher, isolada do projeto legado. Consulte [Project_context.md](Project_context.md) para as regras arquiteturais e o escopo.

## Requisitos

- C++17
- Qt 6.4 ou mais recente, com os módulos Widgets e Network
- CMake 3.21 ou mais recente
- Ninja (recomendado)

## Compilar

Com o compilador e o Qt disponíveis no ambiente:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

No MSYS2 Windows, execute os comandos em um terminal UCRT64 para que o CMake encontre o compilador GCC e o Qt instalados nesse ambiente. No Linux, instale os pacotes de desenvolvimento do Qt 6 Widgets e Network, CMake, Ninja e um compilador C++ antes de configurar o projeto.

No Windows, o passo de build executa `windeployqt` e, com MinGW/MSYS2, `ntldd` para copiar as DLLs transitivas do Qt e do runtime do compilador para junto do executável em `build/`. Instale também o pacote `mingw-w64-ucrt-x86_64-ntldd` no ambiente UCRT64 do MSYS2.

## Editar a interface

Abra `src/ui/mainwindow.ui` no Qt Designer para editar visualmente a estrutura da janela. O comportamento da interface permanece implementado em C++.

No terminal UCRT64 do MSYS2, o formulário pode ser aberto com:

```sh
designer src/ui/mainwindow.ui
```

## Estado atual

A aplicação oferece configuração do executável Python, diretório de trabalho, argumentos WSGI, URL do BMCM e endpoint de saúde opcional; permite iniciar, parar e reiniciar o backend; registra a saída do processo em uma janela de log e em arquivo; e pode programar reinícios após encerramento inesperado. Quando o endpoint de saúde é configurado, o navegador só é aberto após uma resposta HTTP bem-sucedida. Sem endpoint, o status informa que o processo está ativo, mas que a prontidão HTTP não foi verificada. A configuração é salva em JSON no diretório de configuração do usuário, separado da pasta do backend. Ao abrir manualmente, o launcher não inicia o backend; quando iniciado pelo autostart do sistema, inicia o backend e fica oculto na bandeja, se disponível.

A aplicação mantém uma única instância, envia novas ativações à janela existente e oferece controles de backend pela bandeja do sistema, quando disponível. Fechar a janela a oculta na bandeja; use “Sair” no menu da bandeja para encerrar o launcher e o backend. O autostart pode ser configurado para o usuário atual (Registro `HKCU` no Windows e arquivo `.desktop` no Linux). A checagem opcional consulta uma versão `x.y.z` via HTTPS e pede confirmação antes de baixar e novamente antes de abrir o instalador. As opções ficam disponíveis em **Configurar**.

A versão oficial é definida pelo campo `VERSION` de `project()` em `CMakeLists.txt`. O CMake gera as constantes `APP_VERSION_MAJOR`, `APP_VERSION_UPDATE`, `APP_VERSION_COUNT` e o texto de versão utilizado pela interface. Registre cada evolução em [CHANGELOG.md](CHANGELOG.md), seguindo a regra de versionamento de [Project_context.md](Project_context.md).
