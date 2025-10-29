Servidor de navegador e cliente HTTP em C.

Este projeto é um simples servidor HTTP em C onde se pode baixar o arquivo que estão nele no cliente, no navegador é possivel baixar a imagem.jpg que se encontra na pasta.

use o makefile para compilar os programas, vai fornecer o meu_servidor.exe e o meu_navegador.exe. Usando a porta 8080 no LocalHost.

Por motivos de este prograter ter sido feito no Winsows ele foi compilado pelo MinGW
meu_servidor:  gcc -o meu_servidor.exe meu_servidor.c -lws2_32
               ./meu_servidor.exe
meu_navegador: gcc -o meu_navegador.exe meu_navegador.c -lws2_32
               ./meu_navegador.exe
