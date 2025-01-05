# Trabalho Prático 1 - Software Básico
Implementação em C++ de um montador e ligador da disciplina "CIC0104 - Software Básico" para o sistema operacional Linux.

## Integrantes
- Arthur Diehl Barroso - 221029991
- Guilherme da Rocha Cunha - 221030007

## Dependências
- G++ 13.3.0
- Linux

## Compilação
Para compilar o montador, copie este comando no terminal:

```bash
g++ -Wall -o MONTADOR src/montador.cpp
```

Para compilar o ligador, copie este comando no terminal:

```bash
g++ -Wall -o LIGADOR src/ligador.cpp
```

## Execução
### Montador
Uma vez com os programas compilados, o Montador possui dois modos:
1. Para montar um arquivo `.pre` a partir de um arquivo `.asm`, copie este comando no terminal:

```bash
./MONTADOR myfile.asm
```

2. Para montar um arquivo `.obj` a partir de um arquivo `.pre`, copie este comando no terminal:

```bash
./MONTADOR myfile.pre
```

### Ligador
O Ligador gera um arquivo `.e` a partir de dois arquivos `.obj` com ligação ao rodar este comando no terminal:

```bash
./LIGADOR prog1.obj prog2.obj
```

Link para o repositório do Github: [link](https://github.com/GRochaC/Trabalho_1_SB)