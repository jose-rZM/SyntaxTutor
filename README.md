# SyntaxTutor

SyntaxTutor es una aplicación de escritorio escrita en C++20 y Qt 6. Su objetivo es ayudar a comprender el análisis sintáctico mediante un tutor interactivo para los algoritmos LL(1) y SLR(1).

## Funcionalidades principales

- Generación de gramáticas de distintos niveles de dificultad.
- Tutor interactivo para practicar la construcción de tablas LL(1) y SLR(1).
- Puntuación y progreso por niveles.
- Exportación de las conversaciones a PDF.
- Interfaz disponible en español e inglés.

## Compilación

Para compilar la aplicación se necesita Qt 6 (qmake6) y un compilador C++20 compatible.

### Linux

```bash
qmake6 SyntaxTutor.pro -config release
make
```

### macOS

```bash
qmake6 SyntaxTutor.pro -config release
make
```

### Windows (MSVC)

```cmd
qmake6 SyntaxTutor.pro -config release
nmake
```

El binario resultante se llamará `SyntaxTutor` (o `SyntaxTutor.exe` en Windows).

## Ejecución de pruebas

El archivo `backend/tests.cpp` contiene un conjunto de pruebas basado en Google Test. Para compilarlas se deben tener instaladas las bibliotecas de gtest. Un ejemplo de compilación es:

```bash
g++ -std=c++20 backend/grammar.cpp backend/grammar_factory.cpp \
    backend/ll1_parser.cpp backend/lr0_item.cpp backend/slr1_parser.cpp \
    backend/symbol_table.cpp backend/tests.cpp -Ibackend \
    -lgtest -lgtest_main -o test_runner
./test_runner
```

## Dependencias

- **Qt 6** – Framework principal de la interfaz.
- **Google Test** – Utilizado únicamente para `backend/tests.cpp`.
- Archivos de traducción: `translations/st_es.qm` y `translations/st_en.qm`.

