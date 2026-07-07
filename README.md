# Entregable 3: Tablas Hash

El objetivo principal es resolver el problema de contar cuántos tweets publicó cada usuario en un dataset real de 180.000 registros sobre las elecciones australianas de 2019 (`auspol2019.csv`). Para evaluar la eficiencia y el impacto del tipo de dato, cada estructura se prueba utilizando dos claves distintas: el identificador numérico de usuario (`user_id` en `int64_t`) y el nombre de usuario de texto (`user_screen_name` en `std::string`).

## Estructuras de Datos Implementadas

Se evalúan cinco configuraciones agrupadas en tres categorías:

1. **Tabla Hash con Hashing Abierto (`ChainingHashTable`)**
   - Implementada en `chaining_table.hpp` utilizando encadenamiento separado con arreglos dinámicos (`std::vector`) y listas enlazadas (`std::list`).
   - Política de redimensionamiento con factor de carga máximo de 1.0 y duplicación de capacidad por potencias de 2.

2. **Tabla Hash con Hashing Cerrado (`OpenAddressingHashTable`)**
   - Implementada en `open_addressing.hpp` utilizando un arreglo contiguo de slots con tres estrategias de manejo de colisiones:
     - **Linear Probing**: Sondeo lineal secuencial.
     - **Quadratic Probing**: Sondeo cuadrático básico.
     - **Double Hashing**: Sondeo utilizando un segundo hash independiente.
   - Política de redimensionamiento con factor de carga máximo de 0.7, ajustando siempre la capacidad al número primo más cercano.

3. **Biblioteca Estándar de C++ (`std::unordered_map`)**
   - Implementación nativa de la STL como punto de comparación base, preasignando memoria inicial mediante `reserve(1024)`.

## Funciones Hash Utilizadas

Definidas en `hash_functions.hpp`:

- **Para `int64_t` (`user_id`)**:
  - Hash primaria (`UserIdHash1`): Hashing multiplicativo basado en el método de Knuth (constante `2654435761ULL`).
  - Hash secundaria (`UserIdHash2`): Operaciones de mezcla de bits con XOR y desplazamiento, acotada al rango `[1, 97]` para garantizar saltos válidos en Double Hashing.
- **Para `std::string` (`user_screen_name`)**:
  - Hash primaria (`ScreenNameHash1`): Algoritmo no criptográfico FNV-1a de 64 bits.
  - Hash secundaria (`ScreenNameHash2`): Variante del algoritmo `djb2`, acotada al rango `[1, 97]`.

## Requisitos del Sistema

- Compilador de C++ con soporte para el estándar C++20 (`g++`, `clang++` o MSVC).
- Herramienta `make` o `mingw32-make` (en entornos Windows con MinGW).
- Python 3.8+ con las bibliotecas `pandas` y `matplotlib` (para la generación del reporte gráfico y análisis estadístico).

## Configuración y Estructura del Proyecto

El dataset de tweets (`auspol2019.csv`) debe ubicarse dentro de un directorio llamado `data/` en la raíz del proyecto antes de ejecutar el programa:

```text
├── data/
│   └── auspol2019.csv
├── analyze_benchmark.py
├── benchmark.hpp
├── chaining_table.hpp
├── csv_reader.hpp
├── hash_functions.hpp
├── main.cpp
├── open_addressing.hpp
└── README.md
```

El lector personalizado (`csv_reader.hpp` y `main.cpp`) se encarga de parsear el CSV respetando comillas dobles, comas y saltos de línea internos dentro de campos de texto extensos.

## Compilación y Ejecución

Para compilar el proyecto manualmente con un compilador como `g++`, ejecuta en la terminal aplicando optimizaciones de nivel 2:

```bash
g++ -std=c++20 -O2 main.cpp -o benchmark_app
```

Si dispones de un archivo `Makefile` en el proyecto, simplemente ejecuta:

```bash
make
```

En entornos Windows que utilicen la cadena de herramientas MinGW, utiliza:

```bash
mingw32-make
```

Una vez compilado, ejecuta el binario generado para iniciar las mediciones:

```bash
./benchmark_app
```

En Windows (PowerShell o CMD):

```cmd
benchmark_app.exe
```

## Flujo del Experimento y Generación de Gráficos

1. **Ejecución de C++**: Al correr el ejecutable, se cargarán los registros en memoria y se ejecutarán 20 repeticiones completas para cada una de las 10 combinaciones de estructura y clave. Durante el proceso, se tomarán mediciones de tiempo y memoria en 18 checkpoints acumulativos (de 10.000 a 180.000 tweets), generando como salida el archivo `benchmark_results.csv`.
2. **Análisis en Python**: Para generar el reporte estadístico en texto y exportar las figuras comparativas finales en formato PNG, ejecuta el script de análisis:

```bash
python analyze_benchmark.py
```

Esto creará automáticamente la carpeta `output/` con los siguientes archivos generados:

* `benchmark_report.txt`: Resumen textual con promedios, desviaciones estándar y consumo de memoria.
* `benchmark_comparacion_memoria.png`: Gráfico de barras comparando el footprint de memoria en el checkpoint final.
* `benchmark_escalabilidad_creacion.png`: Curvas de tiempo de ejecución en función de la cantidad de tweets procesados.
