# Contexto do Projeto - BMCM Tray Launcher Qt

## 1. Objetivo

Este diretório é o espaço de trabalho isolado para a evolução nativa e multiplataforma do BMCM Tray Launcher. O projeto migra a implementação legada em C#/.NET Windows Forms para **C++ + Qt + CMake**, tendo Windows e Linux como plataformas-alvo.

Este projeto não é uma reescrita do sistema de gestão BMCM. O launcher continua sendo um componente auxiliar de implantação e execução do sistema BMCM.

## 2. Relação com o projeto legado

- Implementação legada e referência de comportamento: `../BMCMTrayLauncher/`.
- Mantenha o projeto legado intacto enquanto a versão Qt é desenvolvida e validada.
- Não coloque novos arquivos-fonte C++/Qt, artefatos de compilação ou configurações específicas do Qt no diretório legado.
- Use o código-fonte legado e sua configuração atual como referências do comportamento existente do launcher, não como arquivos a modificar durante a migração nativa.
- Mantenha o backend BMCM e sua lógica de negócio fora deste projeto do launcher.

## 3. Fronteira arquitetural

**BMCM é o sistema. O Launcher é um componente auxiliar de execução.**

O launcher pode preparar e verificar o ambiente de execução, iniciar o servidor WSGI adequado à plataforma, monitorá-lo e oferecer controles operacionais. No Windows, a referência atual é o Waitress; no Linux, o servidor previsto é o Gunicorn. Caso o macOS seja incluído como plataforma futura, a opção prevista também é o Gunicorn. O launcher não deve implementar nem ser responsável pelas regras de negócio ou pelos dados do BMCM, incluindo alunos, cursos, frequência, gerenciamento de usuários ou relatórios.

Regra arquitetural:

> Se o launcher for removido, o BMCM continua sendo o BMCM. O launcher não faz parte da lógica funcional de negócio do sistema BMCM.

O backend BMCM continua sendo a autoridade sobre o comportamento de negócio e os dados. O launcher se comunica com ele por meio da execução de processos e, quando apropriado, de sua interface HTTP.

## 4. Comportamentos atuais do launcher a preservar

A implementação Qt representa uma evolução, não uma redução de escopo. Preserve os comportamentos úteis do launcher atual, incluindo:

- Execução de uma única instância e notificação ao usuário quando outra instância for aberta.
- Ícone na bandeja do sistema, status e menu operacional.
- Inicialização, encerramento e reinicialização do backend Python com o servidor WSGI configurado para a plataforma (Waitress no Windows e Gunicorn no Linux; também no macOS, caso venha a ser suportado).
- Monitoramento do processo do backend, registro das saídas e dos erros e reinicialização configurável após falhas.
- Interface de configuração do executável Python, diretório de trabalho, argumentos do servidor, URL da aplicação, política de reinicialização, comportamento do navegador, inicialização automática e atualizações.
- Teste da configuração e da inicialização do backend, quando viável.
- Opção de abrir o BMCM no navegador padrão.
- Opção de iniciar automaticamente com o sistema operacional.
- Verificação opcional de atualizações do launcher e download/execução do instalador.
- Logs persistentes e mensagens de erro operacionais claras para o usuário.

Os comportamentos existentes estão documentados em `../BMCMTrayLauncher/MainForm.cs`, `../BMCMTrayLauncher/ConfigForm.cs`, `../BMCMTrayLauncher/Program.cs` e `../BMCMTrayLauncher/appsettings.json`.

## 5. Evolução operacional planejada

O launcher pode adicionar recursos de manutenção do ambiente de execução do BMCM, pois é responsável por preparar e iniciar o servidor:

- Verificar se o executável Python e o ambiente virtual configurados existem e podem ser usados.
- Inspecionar os pacotes instalados no ambiente virtual configurado.
- Comparar as versões dos pacotes instalados com o `requirements.txt` do BMCM e informar pacotes ausentes, incompatíveis ou extras, conforme apropriado.
- Oferecer a instalação ou atualização de dependências como uma operação explícita e observável.
- Verificar se o servidor está pronto por meio de um endpoint HTTP de saúde acordado com o BMCM, em vez de considerar a mera existência do processo como prova de que a aplicação está pronta. Essa verificação já está implementada na versão nativa e pode ser configurada; sua integração depende de o BMCM fornecer um endpoint compatível.
- Iniciar ou reiniciar o servidor após uma alteração aprovada no ambiente e informar se ele ficou pronto.

Esses recursos não devem transformar o launcher em um gerenciador de pacotes com efeitos colaterais ocultos. Como padrão, não altere o ambiente virtual do BMCM sem avisar. Mostre as mudanças propostas, obtenha a confirmação do usuário antes de instalar ou atualizar, exiba a saída dos comandos e os erros, e evite alterar pacotes enquanto o servidor gerenciado estiver em execução, a menos que a operação coordene explicitamente um encerramento e uma reinicialização seguros.

O `requirements.txt` do BMCM é a fonte oficial das dependências Python. Execute o interpretador do ambiente configurado com `-m pip`; não dependa da ativação de um ambiente de shell nem de um `pip` global. Respeite os marcadores de dependência do Python e informe quando não for possível resolver os requisitos. Não invente nem duplique neste repositório dependências de negócio do BMCM.

## 6. Requisitos de tecnologia e portabilidade

- Linguagem: C++ moderno (C++17 ou mais recente, a menos que restrições da implementação justifiquem outro padrão).
- Interface gráfica e framework multiplataforma: Qt 6 Widgets.
- Sistema de compilação: CMake; ofereça suporte ao gerador Ninja quando disponível.
- Use apenas os módulos Qt necessários. Entre os módulos previstos estão Core, Widgets e Network.
- Use o suporte a JSON do Qt (`QJsonDocument` e classes relacionadas) para a configuração do launcher; não adicione outra biblioteca JSON sem uma necessidade concreta.
- Use as APIs de processos do Qt para iniciar e monitorar o processo Python e o servidor WSGI da plataforma (Waitress ou Gunicorn). Passe o executável e os argumentos separadamente; não monte comandos de shell a partir de configurações controladas pelo usuário.
- Mantenha o comportamento específico de cada plataforma atrás de adaptadores pequenos e bem definidos quando necessário (inicialização automática, empacotamento, caminhos e integração com o ambiente gráfico).
- Mantenha a configuração portável sempre que possível, mas permita caminhos de executáveis e comportamentos de inicialização específicos de cada plataforma.
- Não pressuponha sintaxe de caminhos do Windows, APIs exclusivas do Windows ou empacotamento exclusivo do Windows no código compartilhado.
- Os builds para Windows e Linux são os entregáveis inicialmente suportados. Um build apenas para Windows não estabelece suporte ao Linux. O macOS é uma possibilidade de expansão futura e só será considerado suportado após implementação e validação próprias.

### Interface gráfica e modelo de desenvolvimento da UI

O projeto utiliza **Qt 6 Widgets** para a interface gráfica. O Qt Designer pode ser usado para construir visualmente as interfaces por meio de arquivos `.ui`, mas a aplicação não deve ser tratada como um projeto Windows Forms tradicional.

- Os arquivos `.ui` representam a estrutura visual da interface e podem ser editados no Qt Designer.
- O código C++ implementa comportamento, eventos, lógica de interação e integração com os demais componentes.
- Não é necessário construir toda a interface manualmente por coordenadas em C++, salvo quando houver uma necessidade específica.
- Prefira layouts do Qt (`QVBoxLayout`, `QHBoxLayout`, `QGridLayout` etc.) em vez de posicionar controles com coordenadas absolutas.
- Não concentre toda a lógica do launcher em `MainWindow`; distribua as responsabilidades entre componentes apropriados.
- O desenvolvimento da UI pode combinar construção visual no Qt Designer e implementação comportamental em C++.
- Não presuma que o projeto segue o modelo de designer/código do Windows Forms nem que todos os controles precisam ser criados e posicionados diretamente em C++.
- Quando uma tela puder ser adequadamente representada no Qt Designer, prefira a combinação de `.ui` e C++ em vez de gerar manualmente toda a estrutura visual no código.

## 7. Comportamento por plataforma

### Windows

Ofereça suporte à bandeja do sistema, à inicialização automática para o usuário atual, ao gerenciamento do ciclo de vida dos processos, à abertura do navegador, à configuração e ao empacotamento sem exigir privilégios de administrador para o uso normal, sempre que viável. Inicie o backend BMCM com Waitress, conforme a configuração e as dependências do ambiente Windows.

### Linux

Ofereça suporte aos ambientes de área de trabalho e à disponibilidade de bandeja definidos como alvo da versão. Considere as diferenças nas convenções de inicialização automática, permissões de execução, encerramento de processos, abertura do navegador e empacotamento. A disponibilidade da bandeja pode depender do suporte do ambiente de área de trabalho; forneça uma alternativa compreensível quando ela não estiver disponível. Inicie o backend BMCM com Gunicorn, conforme a configuração e as dependências do ambiente Linux.

### macOS (possibilidade futura)

O macOS não faz parte dos alvos da primeira entrega, mas poderá ser avaliado como uma expansão futura do launcher. Caso seja incluído, será necessário validar a interface Qt, a integração com a bandeja e o início automático, o empacotamento e o ciclo de vida dos processos nesse sistema. O backend BMCM deverá ser iniciado com Gunicorn, conforme as dependências do ambiente macOS.

## 8. Configuração e segurança operacional

- Mantenha a configuração do launcher separada da configuração da aplicação BMCM e dos dados de negócio.
- O launcher não deve depender de componentes do BMCM para executar suas funções básicas de diagnóstico, configuração e manutenção. Se o backend não puder iniciar, o launcher deve permanecer operacional para apresentar os erros e permitir ações de recuperação compatíveis.
- Valide os caminhos, URLs e valores numéricos obrigatórios antes de salvar ou iniciar processos.
- Preserve as configurações existentes sempre que viável durante a migração; documente alterações intencionais no formato da configuração.
- Não registre segredos nem dados confidenciais do usuário nos logs.
- Torne visíveis as ações potencialmente disruptivas (atualização de dependências, execução do instalador e encerramento/reinicialização do backend) e informe seus resultados.
- Diferencie o estado do processo da prontidão HTTP nas mensagens de status.
- Evite excluir, substituir ou reescrever dados do BMCM durante a instalação, atualização ou manutenção do ambiente pelo launcher.

## 9. Regra pétrea de versionamento

O controle de versão da aplicação é uma regra permanente e obrigatória do projeto BMCMTrayLauncherQt. Nenhuma alteração relevante na aplicação pode ser considerada concluída sem a atualização correspondente da versão.

Toda correção, nova rotina, novo formulário, alteração estrutural, migração de banco de dados ou melhoria relevante deve atualizar a versão da aplicação antes de ser considerada concluída. A versão utiliza sempre três componentes no formato `principal.atualizacao.contagem`, por exemplo, `1.4.5`.

Os componentes têm os seguintes significados:

- `principal`: versão estrutural principal do sistema.
- `atualizacao`: etapa, ciclo ou evolução funcional.
- `contagem`: número incremental das alterações realizadas.

Regras obrigatórias:

1. Cada alteração relevante concluída incrementa o terceiro componente.
2. A contagem não é reiniciada por data; cada alteração parte da versão já registrada.
3. Quando o terceiro componente ultrapassar 10, ele volta a 0 e o segundo componente é incrementado.
4. Quando o segundo componente ultrapassar 10, ele volta a 0 e o primeiro componente é incrementado.
5. Uma nova etapa funcional pode incrementar diretamente o segundo componente; mudanças estruturais incompatíveis podem incrementar diretamente o primeiro.
6. A versão oficial deve ser registrada no código-fonte ou em um arquivo de configuração da aplicação, como `.env` ou `config.json`, usando as constantes `APP_VERSION_MAJOR`, `APP_VERSION_UPDATE` e `APP_VERSION_COUNT`.
7. A versão não pode ser alterada pelas configurações administrativas da aplicação.
8. A versão exibida nas telas deve corresponder à mesma versão oficial registrada no código-fonte ou arquivo de configuração adotado pelo projeto.
9. Toda publicação ou atualização deve registrar a nova versão e suas alterações no histórico do projeto.

Esta regra não deve ser removida, ignorada ou substituída por numeração independente em módulos. A fonte oficial da versão deve ser única para a aplicação, e todas as telas e registros de publicação devem usá-la.

## 10. Migração e critérios de aceite

A versão nativa só estará pronta para substituir o launcher legado quando:

1. O build para Windows usa CMake e Qt e não requer o runtime .NET.
2. Também existe um build para Linux, verificado em um ambiente Linux documentado.
3. Os recursos atuais do launcher listados na seção 4 são preservados ou qualquer alteração intencional é documentada.
4. A inicialização, o encerramento e a reinicialização de processos, o tratamento de falhas e o registro das saídas funcionam em cada plataforma suportada.
5. A configuração é mantida entre reinicializações da aplicação e configurações inválidas geram erros que orientam o usuário sobre como corrigi-las.
6. Quando uma verificação de saúde estiver configurada, a prontidão é informada com base na resposta do servidor BMCM.
7. As verificações do ambiente de execução e das dependências permanecem somente para leitura até que o usuário aprove uma instalação ou atualização.
8. Nenhuma lógica de negócio ou dado de domínio do BMCM foi transferido para o launcher.
9. As instruções de empacotamento e instalação estão documentadas separadamente para Windows e Linux.

Não remova nem reutilize o projeto legado para outra finalidade durante o desenvolvimento inicial. Mantenha-o disponível para comparação até que a implementação nativa tenha passado pelos critérios de aceite pertinentes.

## 11. Status atual da implementação

Estado registrado em 2026-10-12: a nova implementação está na versão **1.1.14**, definida em `CMakeLists.txt` e refletida nas constantes geradas `APP_VERSION_MAJOR`, `APP_VERSION_UPDATE` e `APP_VERSION_COUNT`. O código C++/Qt permanece isolado em `BMCMTrayLauncherQt/`; o projeto legado continua em `../BMCMTrayLauncher/`.

### Implementado

- Aplicação C++17 com Qt 6 Widgets, formulários Qt Designer (`.ui`) e build CMake.
- Carregamento e salvamento da configuração JSON no diretório de configuração do usuário, separado dos arquivos do backend.
- Configuração do executável Python, diretório de trabalho, URL do repositório GitHub do BMCM e modo de atualização (automático, clone Git ou ZIP), argumentos do servidor WSGI, URL do BMCM, endpoint de saúde opcional, política de reinício, abertura do navegador, autostart e atualização do launcher.
- Inicialização, encerramento e reinicialização do processo Python/WSGI; captura da saída e dos erros; log persistente e reinício configurável após encerramento inesperado.
- Ícone e menu de bandeja quando o ambiente oferece suporte; fechamento da janela oculta o launcher na bandeja, e a opção explícita “Sair” encerra o launcher e o backend.
- Instância única, com ativação da instância já aberta quando uma segunda execução é iniciada.
- Verificação HTTP opcional de prontidão. O status diferencia processo iniciado de BMCM pronto; quando configurada, a abertura automática do navegador aguarda uma resposta HTTP bem-sucedida.
- Autostart no escopo do usuário implementado para Windows (Registro `HKCU`) e Linux (arquivo `.desktop` no diretório de autostart do usuário); quando acionado pelo sistema, o backend é iniciado e o launcher fica oculto na bandeja, se disponível.
- Verificação de atualização opcional via HTTPS, comparação de versões no formato `principal.atualizacao.contagem` e confirmação do usuário antes de baixar e antes de abrir o instalador.
- Verificação automática do repositório GitHub do SistemaBMCM, configurável no arquivo de configurações e pela tela **Configurar**; o modo automático usa Git em clones e ZIP em instalações sem clone. O modo Git aplica apenas atualizações fast-forward em clone sem alterações locais. O modo ZIP consulta o SHA da branch pela API do GitHub, baixa e extrai o arquivo da versão remota e sobrepõe arquivos da aplicação sem excluir arquivos locais; preserva `.env*`, `instance/`, ambientes virtuais, uploads e arquivos de banco de dados. Esses caminhos são excluídos já durante a extração, incluindo o `.venv` versionado no repositório e seus links simbólicos incompatíveis com o `tar` do Windows. Nenhum modo instala dependências.
- Verificação automática de versões do launcher em GitHub Releases do repositório `Grupo-9-PII/bmcmtray`; no Windows, baixa o pacote portátil `BMCMTrayLauncherQt-windows-x64.zip`, aplica os arquivos depois que o launcher termina e inicia novamente o executável. A configuração da aplicação fica em AppData, fora da pasta do executável. O workflow de release compila e publica o pacote quando uma tag `vX.Y.Z` é criada.
- Build Windows com GCC/MinGW, Qt, CMake e Ninja; implantação de DLLs Qt e MinGW junto ao executável.

### Validação realizada

- Build e execução da versão Windows validados neste ambiente; o executável inicia sem depender do `PATH` do MSYS2.
- Testes automatizados cobrem serialização da configuração, comparação de versões, lock de instância única, resposta e timeout da verificação HTTP, ciclo básico de execução de processo e atualização fast-forward em um repositório Git local, com preservação do banco local e bloqueio de alterações locais e arquivos de banco versionados.
- A atualização do SistemaBMCM valida o clone, o repositório remoto e a branch, e preserva arquivos locais ignorados pelo Git (incluindo `instance/`); o backend pode executar suas próprias migrações ao iniciar.
- O build Linux, o autostart em uma sessão Linux real, a integração com um backend BMCM real e o fluxo real de download/instalação de atualização ainda não foram validados.

### Ainda planejado

- Inspecionar o Python/`venv` configurado e comparar as dependências instaladas com o `requirements.txt` do BMCM.
- Apresentar divergências de dependências e oferecer reparo/atualização somente após confirmação explícita, coordenando a parada e a reinicialização do servidor.
- Acrescentar um teste operacional dedicado que valide a configuração e a inicialização do servidor sem assumir que o processo está saudável apenas por permanecer ativo.
- Validar build, empacotamento, autostart, bandeja e ciclo de processos em Linux. macOS continua sendo uma possibilidade futura, fora dos alvos iniciais.

Ao abrir manualmente, o launcher não inicia automaticamente o backend; o usuário inicia o servidor pelos controles da janela ou da bandeja. Quando o autostart do sistema está habilitado, a entrada de inicialização usa `--autostart`, que inicia o backend e oculta a janela se houver bandeja disponível. Se não houver bandeja, a janela permanece visível para permitir controle e diagnóstico.

## 12. Nota de desenvolvimento

Este arquivo é a referência arquitetural básica do novo projeto. Atualize-o quando houver mudanças no escopo, no suporte às plataformas, no formato da configuração ou nas responsabilidades operacionais. Registre desvios importantes destas regras e suas justificativas para que o launcher continue sendo um componente bem delimitado e testável de implantação e execução do BMCM.
