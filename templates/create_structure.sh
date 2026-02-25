#!/bin/bash

# Descobre o diretório do script
SCRIPT_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"

#Estrutura do projeto
APP_DIR="app"
INCLUDES_DIR="$APP_DIR/includes"
SRC_DIR="$APP_DIR/src"
OBJS_DIR="$APP_DIR/obj"
TEST_DIR="$APP_DIR/tests"
BIN_DIR="$APP_DIR/bin"
DOC_DIR="doc"
ASSETS_DIR="$DOC_DIR/assets"

#detro da includes vamos colocar o include.h
#dentro do src vamos colocar a main.c
#a pasta objs será utilizada no makefile para armazenar os arquivos objeto
#a pasta bin será utilizada para armazenar o arquivo executável gerado no makefile
#O arquivo README.md será criado na raiz, assim como o .gitignore e o Makefile

#verificar se o nome do projeto foi fornecido como único argumento
if [ $# -ne 1 ]; then
    echo "Usage: $0 <project_name>"
    exit 1
fi

PROJECT_NAME=$1 # Verificar se o nome do projeto foi fornecido
if [ -z "$PROJECT_NAME" ]; then
    echo "Usage: $0 <project_name>" 
    exit 1
fi

#Project name em maiúsculo para o header guard
PROJECT_NAME_UPPER=$(echo "$PROJECT_NAME" | tr '[:lower:]' '[:upper:]')

#Nome das variáveis para o template do header
INCLUDE_NAME="$PROJECT_NAME_UPPER"_H

#inclunir uma validação para verificar se o pasta app/includes existe, caso contrário criar a pasta
if [ ! -d "$INCLUDES_DIR" ]; then
    mkdir -p "$INCLUDES_DIR"
fi

if [ ! -d "$SRC_DIR" ]; then
    mkdir -p "$SRC_DIR"
fi

if [ ! -d "$OBJS_DIR" ]; then
    mkdir -p "$OBJS_DIR"
fi

if [ ! -d "$TEST_DIR" ]; then
    mkdir -p "$TEST_DIR"
fi

if [ ! -d "$BIN_DIR" ]; then
    mkdir -p "$BIN_DIR"
fi

if [ ! -d "$DOC_DIR" ]; then
    mkdir -p "$DOC_DIR"
fi

if [ ! -d "$ASSETS_DIR" ]; then
    mkdir -p "$ASSETS_DIR"
fi

sed -e "s/{{INCLUDE_NAME}}/$INCLUDE_NAME/g" \
    -e "s/{{FUNC_NAME}}/$FUNC_NAME/g" "$SCRIPT_DIR/default/include.tpl" \
    > app/includes/$PROJECT_NAME.h

sed -e "s/{{PROJECT_NAME}}/$PROJECT_NAME/g" \
    "$SCRIPT_DIR/default/main.tpl" \
    > app/src/main.c

sed -e "s/{{PROJECT_NAME}}/$PROJECT_NAME/g" \
    "$SCRIPT_DIR/default/makefile.tpl" \
    > Makefile

sed -e "s/{{PROJECT_NAME}}/$PROJECT_NAME/g" \
    "$SCRIPT_DIR/default/readme.tpl" \
    > README.md