# Histórico de alterações

## 1.1.14 - 2026-10-12

- Adicionada atualização automática do launcher por GitHub Releases, com aplicação do pacote portátil após o encerramento e reinício do processo.
- Adicionado workflow para compilar, testar e publicar o pacote Windows ao criar uma tag de versão `vX.Y.Z`.

## 1.1.13 - 2026-10-12

- A opção **Sair** da bandeja agora encerra explicitamente o event loop do launcher após solicitar a parada do backend.

## 1.1.12 - 2026-10-12

- Corrigida a validação de caminhos da atualização ZIP no Windows, comparando caminhos canônicos por relação relativa à instalação.

## 1.1.11 - 2026-10-12

- Excluídos ambientes virtuais e dados locais durante a extração do ZIP, evitando erros do `tar` do Windows com links simbólicos do `.venv` versionado no repositório.

## 1.1.10 - 2026-10-12

- Corrigida a extração de atualizações ZIP para executar o `tar` do sistema, em vez de passar seus argumentos ao Git.

## 1.1.9 - 2026-10-12

- Adicionado modo de atualização por ZIP para instalações do SistemaBMCM que não são clones Git, com seleção explícita e detecção automática.

## 1.1.8 - 2026-10-12

- Adicionada verificação e atualização automática do SistemaBMCM pela branch `main` do GitHub ao iniciar, preservando `instance/` e arquivos de banco de dados e sem instalar dependências automaticamente.

## 1.1.7 - 2026-10-06

- O autostart agora inicia o backend automaticamente e mantém o launcher oculto na bandeja; sem bandeja, a janela permanece visível.

## 1.1.6 - 2026-10-06

- Adicionados autostart por usuário no Windows/Linux, verificação de atualização via HTTPS e fluxo de download/abertura com confirmação explícita.

## 1.1.5 - 2026-10-06

- Adicionados ícone e menu da bandeja, ativação da instância existente e verificação HTTP opcional de prontidão do BMCM.

## 1.1.4 - 2026-10-06

- Adicionados configuração JSON, janela de configurações e controles de iniciar, parar e reiniciar o backend com registro de saída.

## 1.1.3 - 2026-10-06

- Corrigida a distribuição das dependências transitivas Qt/MinGW para permitir iniciar o aplicativo sem o MSYS2 no PATH.

## 1.1.2 - 2026-10-06

- Corrigida a execução do aplicativo Windows fora do MSYS2 implantando automaticamente as DLLs do Qt e do runtime do compilador junto ao executável.

## 1.1.1 - 2026-10-05

- Iniciada a base nativa do BMCM Tray Launcher com C++17, Qt 6 Widgets e CMake.
- Criado o primeiro formulário Qt Designer e geração da versão exibida a partir da versão oficial do projeto.
- Mantido o backend BMCM fora desta etapa inicial; integração e recursos operacionais serão implementados em etapas seguintes.
