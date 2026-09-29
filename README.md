# Evolution Simulation — C++17

Reescriptura de la simulació original en Python amb una estructura modular i un model evolutiu més estable.

## Estructura

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
└── tests/
    └── smoke_test.cpp
```

## Compilar

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Executar

Amb valors per defecte:

```bash
./build/evolution_sim
```

O passant-los explícitament:

```bash
./build/evolution_sim 100 16 40 80 12 5000 42
```

Ordre: `mida_mon aigua menjar herbivors depredadors ticks seed`.

## Canvis importants respecte Python

1. **Velocitat corregida**: el trait `speed` és el valor genètic real. No se li aplica `*0.5` o `*0.6` en el constructor. Això elimina la degradació generacional artificial que hi havia al Python original.

2. **Mutació contínua i petita**: els descendents parteixen de la mitjana dels progenitors i, amb baixa probabilitat, reben una mutació gaussiana petita. Això redueix salts de `-1/+1` massa grans.

3. **Selecció emergent**: la velocitat, visió, mida i metabolisme tenen conseqüències en el model. L'atractiu afecta la probabilitat d'aparellament. No es força que un trait pugi: puja quan dona avantatge en aquest entorn.

4. **Regulació de població**: la reproducció baixa amb la densitat i es calcula una capacitat suau a partir de fonts de menjar/aigua. Això evita explosions demogràfiques mantenint l'ecosistema dinàmic.

5. **Temps determinista**: s'utilitzen ticks, no segons de rellotge real. Dues execucions amb el mateix `seed` i configuració donen la mateixa simulació.

6. **Món toroidal coherent**: ara tant `x` com `y` utilitzen distància toroidal i els límits no introdueixen una asimetria artificial.

7. **Més eficient**: per a les comprovacions de distància s'usen distàncies al quadrat i s'eliminen comprovacions `object in list` repetitives.

8. **CSV ampliat**: es guarden també els traits dels depredadors.

## Nota sobre “millorar els traits”

Una simulació evolutiva no hauria de fer que els traits augmentin obligatòriament. Això seria optimització artificial, no selecció natural. El codi evita sobretot que els traits empitjorin per un error numèric o per una regla de reproducció. A partir d'aquí, l'evolució depèn del cost/benefici real de cada trait.

## Validació feta

He compilat amb CMake en `Release`, he passat el test de fum amb `ctest` i he executat 5 llavors durant 3000 ticks. Amb els valors de prova (`100 x 100`, 16 aigües, 40 menjars, 80 herbívors, 12 depredadors), les poblacions es mantenen en un règim oscil·latori aproximat d'uns 200–230 herbívors i 25–36 depredadors en aquests 5 assajos. Això no demostra estabilitat matemàtica per a qualsevol paràmetre, però sí que evita l'explosió o extinció immediata observada en les primeres proves.
