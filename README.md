# 📋 Menu Interativo - Embarcatec

![C](https://img.shields.io/badge/C-11-blue)
![License](https://img.shields.io/github/license/jardsonalan/menu-embarcatec)

## 📖 Sobre o projeto

O **Menu Interativo** é uma atividade desenvolvida como parte da Tarefa 6.2 do curso Embarcatec. Este projeto tem como objetivo implementar um sistema de menu interativo em linguagem C para sistemas embarcados, facilitando a navegação e seleção de opções em dispositivos com recursos limitados.

## 🛠️ Tecnologias utilizadas

- **Linguagem C**: Utilizada para o desenvolvimento do sistema de menu, devido à sua eficiência e controle de baixo nível, essenciais em sistemas embarcados.
- **CMake**: Ferramenta de automação de compilação que gerencia o processo de build de forma multiplataforma.
- **Pico SDK**: Kit de desenvolvimento de software para a plataforma Raspberry Pi Pico, fornecendo bibliotecas e ferramentas necessárias para o desenvolvimento.

## 📂 Estrutura do projeto

O repositório está organizado da seguinte forma:

- `.vscode/`: Configurações específicas para o Visual Studio Code;
- `inc/`: Diretório contendo os arquivos de cabeçalho (`.h`);
- `.gitignore`: Arquivo especificando quais arquivos ou pastas devem ser ignorados pelo Git;
- `CMakeLists.txt`: Script de configuração do CMake para o projeto;
- `menu.c`: Implementação principal do sistema de menu;
- `pico_sdk_import.cmake`: Script para importação do Pico SDK.

## ⚙️ Componentes utilizados

Todos os componentes utilizados já vieram soldados na placa BitDogLab:

- LED RGB;
- Display OLED;
- Joystick;
- Buzzer;
- Raspberry Pi Pico W.

## 🚀 Como compilar e executar

Para compilar e executar o projeto, siga os passos abaixo:

1. **Clone o repositório:**

   ```bash
   git clone https://github.com/jardsonalan/menu-embarcatec.git
   ```

2. **Acesse o diretório do projeto:**

   ```bash
   cd menu-embarcatec
   ```

3. **Crie uma pasta para build e acesse-a:**

   ```bash
   mkdir build
   cd build
   ```

4. **Configure o projeto com o CMake:**

   ```bash
   cmake ..
   ```

5. **Compile o projeto:**

   ```bash
   make
   ```

6. **Carregue o binário gerado no seu dispositivo** conforme as instruções específicas do hardware utilizado.

## 🤝 Contribuições

Contribuições são bem-vindas! Se você deseja melhorar este projeto ou corrigir algum problema, por favor:

1. Faça um **fork** deste repositório;
2. Crie uma **branch** com a sua feature ou correção: `git checkout -b minha-feature`;
3. Faça o **commit** das suas alterações: `git commit -m 'Minha nova feature'`;
4. Envie para o **repositório remoto**: `git push origin minha-feature`;
5. Abra um **Pull Request** explicando as alterações realizadas.

## 📄 Licença

Este projeto está licenciado sob a licença MIT. Consulte o arquivo **LICENSE** para mais detalhes.
