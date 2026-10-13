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

## Instalar o `tar` para atualizações ZIP

O modo **Instalação sem clone (ZIP)** e o modo **Automático** quando a pasta não é um clone Git precisam do utilitário `tar` disponível no `PATH` para extrair os arquivos baixados do GitHub.

### Windows

O Windows 10 (versão 1803 ou mais recente) e o Windows 11 incluem `tar.exe` (`C:\Windows\System32\tar.exe`). Para verificar no PowerShell:

```powershell
tar --version
```

Se o comando não for encontrado, instale o 7-Zip pelo `winget`:

```powershell
winget install --id 7zip.7zip --exact
```

Feche e abra novamente o terminal e adicione a pasta de instalação do 7-Zip ao `PATH` do usuário, se o instalador não a tiver adicionado. O launcher procura o executável `tar`, não `7z`; portanto, o `tar.exe` do Windows deve estar disponível no `PATH`.

### Linux

Em Debian/Ubuntu:

```sh
sudo apt update
sudo apt install tar
```

Em Fedora:

```sh
sudo dnf install tar
```

Em Arch Linux:

```sh
sudo pacman -S tar
```

Confirme que o executável está disponível:

```sh
tar --version
```

## Editar a interface

Abra `src/ui/mainwindow.ui` no Qt Designer para editar visualmente a estrutura da janela. O comportamento da interface permanece implementado em C++.

No terminal UCRT64 do MSYS2, o formulário pode ser aberto com:

```sh
designer src/ui/mainwindow.ui
```

## Estado atual

A aplicação oferece configuração do executável Python, diretório de trabalho, URL HTTPS do repositório GitHub do SistemaBMCM e modo de atualização, além dos argumentos WSGI, URL do BMCM e endpoint de saúde opcional. Ao iniciar, verifica e aplica automaticamente atualizações da branch `main` do repositório configurado (por padrão, `Grupo-9-PII/sistemaBMCM-II`). O modo **Automático** usa Git quando o diretório é um clone e, quando não é, baixa o ZIP da branch; também é possível selecionar explicitamente **Clone Git** ou **Instalação sem clone (ZIP)** em **Configurar**. O modo Git exige um clone limpo e aplica somente avanço linear. O modo ZIP consulta o SHA no GitHub para evitar downloads sem atualização e sobrepõe arquivos distribuídos pelo repositório sem excluir arquivos locais; preserva `.env*`, `instance/`, `venv/`, `.venv/`, `static/uploads/` e arquivos de banco de dados, e não os copia caso estejam versionados no ZIP. Requer `tar` disponível no sistema para extrair o arquivo. Se a consulta/extração falhar, o launcher informa o problema. A atualização não instala dependências Python. O próprio backend pode executar migrações ao iniciar.

Quando o endpoint de saúde é configurado, o navegador só é aberto após uma resposta HTTP bem-sucedida. Sem endpoint, o status informa que o processo está ativo, mas que a prontidão HTTP não foi verificada. A configuração é salva em JSON no diretório de configuração do usuário, separado da pasta do backend. Ao abrir manualmente, o launcher não inicia o backend; quando iniciado pelo autostart do sistema, atualiza primeiro, depois inicia o backend e fica oculto na bandeja, se disponível.

A aplicação mantém uma única instância, envia novas ativações à janela existente e oferece controles de backend pela bandeja do sistema, quando disponível. Fechar a janela a oculta na bandeja; use “Sair” no menu da bandeja para encerrar o launcher e o backend. O autostart pode ser configurado para o usuário atual (Registro `HKCU` no Windows e arquivo `.desktop` no Linux).

No Windows, ao iniciar, o launcher consulta o último GitHub Release de `Grupo-9-PII/bmcmtray`. Se encontrar uma versão `vX.Y.Z` mais recente que a instalada, baixa o pacote portátil, encerra o launcher, aplica a atualização e inicia novamente o executável. A configuração fica em AppData, separada da pasta do programa. Para publicar uma versão, atualize `project(... VERSION ...)` em `CMakeLists.txt`, envie as alterações e crie/envie a tag correspondente, por exemplo:

```sh
git tag v1.1.14
git push origin v1.1.14
```

O workflow compila e testa o launcher, gera `BMCMTrayLauncherQt-windows-x64.zip` e anexa o pacote ao Release. A opção de instalador personalizado em **Configurar** é uma verificação separada e opcional.

A versão oficial é definida pelo campo `VERSION` de `project()` em `CMakeLists.txt`. O CMake gera as constantes `APP_VERSION_MAJOR`, `APP_VERSION_UPDATE`, `APP_VERSION_COUNT` e o texto de versão utilizado pela interface. Registre cada evolução em [CHANGELOG.md](CHANGELOG.md), seguindo a regra de versionamento de [Project_context.md](Project_context.md).
