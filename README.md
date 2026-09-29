# Evolution Simulation — C++17 + Python/Matplotlib

Simulador d'un ecosistema evolutiu amb **herbívors, depredadors, aigua, menjar, reproducció i mutacions genètiques**.

Aquesta carpeta conté una reescriptura de la simulació original, que estava feta en Python, cap a **C++17**. La reestructuració i bona part de la implementació d'aquesta versió s'han fet **amb ajuda d'IA**, a partir del codi original proporcionat per l'autor. L'objectiu ha estat fer el projecte més modular, ràpid, reproduïble i fàcil d'ampliar, no simplement traduir línia per línia.

> **Important:** la IA és una eina utilitzada durant el desenvolupament; el comportament de la simulació continua depenent de les regles que defineix el projecte i dels paràmetres del model.

## Què simula?

Cada individu té, entre d'altres, els següents trets:

- **Atractiu:** afecta la probabilitat d'aparellament.
- **Visió:** determina fins on pot detectar recursos o perills/preses.
- **Velocitat:** determina la velocitat de moviment i influeix en la freqüència amb què un individu pot actuar.
- **Mida:** afecta principalment la capacitat energètica i el guany d'aliment.
- **Metabolisme:** augmenta el consum de recursos; és útil però té un cost.

La idea no és dir al simulador que "la velocitat ha de pujar" o que "la visió ha de millorar". Els traits es transmeten entre generacions i poden mutar, mentre que la selecció emergeix de les conseqüències que aquests traits tenen sobre la supervivència i la reproducció.

## Per què no és una còpia exacta del Python original?

La traducció directa hauria conservat alguns problemes del model original. Aquesta versió aprofita la migració a C++ per corregir-los.

### 1. Correcció de la velocitat

En el Python original, la velocitat rebuda pel constructor es multiplicava per `0.5` als herbívors i per `0.6` als depredadors. Després, els fills heretaven la velocitat ja escalada.

Això podia provocar una degradació artificial generació rere generació.

En aquesta versió, el trait `speed` és el valor genètic real i **no es torna a escalar en construir un individu**.

### 2. Herència + mutació

Els fills parteixen de la mitjana dels dos progenitors. A continuació, cada trait pot patir una mutació petita:

- probabilitat de mutació configurable;
- magnitud de mutació configurable;
- límits mínim i màxim per espècie.

Això fa que els canvis siguin graduals en lloc de saltar constantment `+1/-1`.

### 3. Selecció basada en el model

Els traits tenen costos i beneficis reals. Per exemple, un metabolisme alt pot donar lloc a individus amb determinades capacitats, però també augmenta el consum energètic.

Per tant, **no s'ha implementat una regla artificial que premiï sempre els valors alts**.

### 4. Poblacions més regulades

La reproducció depèn de la disponibilitat de recursos i d'un factor de densitat. Quan una població s'apropa a la seva capacitat suau, la probabilitat de naixement disminueix.

Això evita, en general, una explosió poblacional immediata.

### 5. Temps determinista

La versió original depenia dels segons reals del rellotge. Ara la simulació funciona en **ticks**.

Amb la mateixa configuració i la mateixa `seed`, la simulació és reproduïble.

### 6. Món toroidal

El món utilitza distància toroidal de manera coherent: sortir per una vora fa aparèixer l'individu per l'altra.

### 7. Optimització

S'han eliminat diverses operacions innecessàries del Python original. Entre d'altres:

- comparacions de distància amb distància al quadrat quan no cal calcular `sqrt`;
- eliminació eficient d'individus morts amb `remove_if`;
- vectors reservats prèviament per reduir reallocacions;
- separació del motor de simulació, món i estadístiques.

## Estructura del projecte

```text
 evolution_sim_cpp/
 ├── CMakeLists.txt
 ├── README.md
 ├── .gitignore
 ├── include/
 │   ├── config.hpp
 │   ├── organisms.hpp
 │   ├── simulation.hpp
 │   ├── statistics.hpp
 │   ├── types.hpp
 │   └── world.hpp
 ├── src/
 │   ├── main.cpp
 │   ├── simulation.cpp
 │   ├── statistics.cpp
 │   └── world.cpp
 ├── scripts/
 │   └── plot_results.py
 └── tests/
     └── smoke_test.cpp
```

### `include/`

Conté les declaracions i les estructures principals:

- `config.hpp`: paràmetres de la simulació.
- `types.hpp`: tipus comuns, vectors, sexes i traits.
- `organisms.hpp`: estructures d'herbívors i depredadors.
- `world.hpp`: món, aigua i menjar.
- `simulation.hpp`: interfície del motor de simulació.
- `statistics.hpp`: dades que s'exporten al CSV.

### `src/`

Conté la implementació real.

- `main.cpp`: entrada del programa i arguments de terminal.
- `simulation.cpp`: bucle principal, moviment, alimentació, caça i reproducció.
- `world.cpp`: recursos i geometria del món.
- `statistics.cpp`: estadístiques i exportació CSV.

### `scripts/plot_results.py`

Script auxiliar en Python que llegeix el CSV generat per C++ i crea les gràfiques amb **Matplotlib**.

He separat les gràfiques del motor perquè el C++ quedi centrat en la simulació i no necessiti una llibreria gràfica per executar-se.

## Requisits

Per compilar el simulador:

- compilador compatible amb **C++17**;
- **CMake 3.16** o superior;
- `make` o un altre generador suportat per CMake.

Per fer les gràfiques:

- **Python 3**;
- **Matplotlib**.

## Compilar

Des de la carpeta arrel del projecte:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

El binari principal quedarà a:

```text
build/evolution_sim
```

## Executar

### Opció 1: valors per defecte

```bash
./build/evolution_sim
```

### Opció 2: especificar tots els paràmetres

```bash
./build/evolution_sim 100 16 40 80 12 5000 42
```

L'ordre dels arguments és:

```text
mida_mon aigua menjar herbivors depredadors ticks seed
```

Per exemple:

```text
100     -> món de 100 x 100
16      -> 16 fonts d'aigua
40      -> 40 fonts de menjar
80      -> 80 herbívors inicials
12      -> 12 depredadors inicials
5000    -> 5000 ticks
42      -> seed aleatòria
```

En acabar, el programa crea un fitxer semblant a:

```text
evolution_stats_42.csv
```

## Fer les gràfiques

El motor de simulació és C++, però les gràfiques es fan amb Python + Matplotlib.

Primer instal·la Matplotlib:

```bash
python3 -m pip install matplotlib
```

Després, des de l'arrel del projecte:

```bash
python3 scripts/plot_results.py evolution_stats_42.csv
```

Per defecte es crea una carpeta:

```text
plots/
```

amb quatre gràfiques:

```text
plots/
├── evolution_stats_42_populations.png
├── evolution_stats_42_herbivore_traits.png
├── evolution_stats_42_predator_traits.png
└── evolution_stats_42_trait_comparison.png
```

També pots indicar una altra carpeta:

```bash
python3 scripts/plot_results.py evolution_stats_42.csv --output-dir graphs
```

## Quines gràfiques es generen?

### Poblacions

Mostra l'evolució del nombre d'herbívors i depredadors al llarg dels ticks.

### Traits dels herbívors

Mostra l'evolució de la mitjana de:

- atractiu;
- visió;
- velocitat;
- mida;
- metabolisme.

### Traits dels depredadors

La mateixa informació per als depredadors.

### Comparació

Mostra herbívors i depredadors junts per poder veure com evolucionen els traits de les dues poblacions.

## Tests

El projecte inclou un test de fum.

Executa:

```bash
ctest --test-dir build --output-on-failure
```

També pots fer una compilació + tests en una sola seqüència:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Com experimentar amb l'evolució

La majoria dels paràmetres es troben a:

```text
include/config.hpp
```

Alguns dels més importants són:

```cpp
double mutation_probability = 0.08;
double mutation_sigma = 0.22;

int herbivore_maturity = 12;
int predator_maturity = 10;

double herbivore_base_birth_chance = 0.16;
double predator_base_birth_chance = 0.16;

int food_patch_capacity = 12;
int food_respawn_ticks = 30;
```

### Si vols més mutació

Augmenta `mutation_probability` o `mutation_sigma`.

Això pot fer que l'evolució sigui més ràpida, però també més sorollosa.

### Si vols poblacions més grans

Pots augmentar els recursos o reduir les restriccions reproductives.

### Si vols més pressió de selecció dels depredadors

Pots modificar la detecció de preses, la velocitat, el cost energètic o la disponibilitat d'herbívors.

No hi ha un conjunt únic de paràmetres que sigui "el correcte": depèn de quin tipus d'ecosistema vulguis representar.

## Resultats i estabilitat

S'han fet proves amb diverses `seed` i configuracions de prova per comprovar que el projecte compila i que la dinàmica bàsica funciona.

En proves amb aproximadament:

```text
món       = 100 x 100
aigua     = 16
menjar    = 40
herbívors = 80
depredadors = 12
ticks     = 3000
```

les poblacions han entrat en règims oscil·latoris en lloc d'explotar immediatament o desaparèixer totes en els primers ticks.

Això **no és una demostració matemàtica d'estabilitat** per a qualsevol combinació de paràmetres. El comportament depèn fortament de les regles i de la `seed`.

## Reproduïbilitat

Per comparar dos experiments, utilitza exactament la mateixa configuració i la mateixa `seed`.

Per exemple:

```bash
./build/evolution_sim 100 16 40 80 12 5000 42
./build/evolution_sim 100 16 40 80 12 5000 43
```

Els dos experiments tenen els mateixos paràmetres però una inicialització aleatòria diferent.

Això és útil per estudiar si un resultat és robust o depèn d'una sola execució.

## GitHub

Aquest projecte està pensat per poder-se publicar com una repo pública.

Una estructura recomanada és:

```text
README.md
CMakeLists.txt
.gitignore
include/
src/
scripts/
tests/
```

No cal pujar la carpeta `build/` ni els CSV/gràfics generats durant experiments, tret que els vulguis conservar expressament.

El fitxer `.gitignore` del projecte està preparat per evitar els artefactes habituals de compilació.

## Possibles millores futures

Algunes extensions interessants serien:

- paral·lelitzar la simulació per a poblacions molt grans;
- implementar una graella espacial o spatial hashing per accelerar la detecció de veïns;
- afegir més espècies o nivells tròfics;
- afegir registre d'ADN/genotip individual en lloc de només traits fenotípics;
- guardar més estadístiques sobre morts, naixements, caça i consum de recursos;
- afegir un visualitzador en temps real;
- fer experiments automàtics sobre moltes `seed` i exportar resums estadístics.

## Llicència

No s'ha definit cap llicència encara. Abans de publicar la repo, afegeix una llicència explícita (per exemple, MIT, GPL o la que correspongui al projecte i a les dependències).
