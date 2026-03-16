# Progetto API – Navigazione su Griglia Esagonale

Progetto finale del corso **Algoritmi e Principi dell'Informatica** (Politecnico di Milano).

Il programma gestisce una mappa basata su una griglia di esagoni, supportando la creazione di rotte aeree dirette, la modifica dinamica dei costi e il calcolo del percorso di costo minimo tramite l'algoritmo di **Dijkstra**.

La specifica completa del progetto è disponibile nel file [`specifica-progetto.pdf`](specifica-progetto.pdf).

---

## Indice

- [Descrizione](#descrizione)
- [Comandi](#comandi)
- [Strutture Dati e Algoritmi](#strutture-dati-e-algoritmi)
- [Compilazione ed Esecuzione](#compilazione-ed-esecuzione)
- [Test](#test)
- [Struttura del Repository](#struttura-del-repository)

---

## Descrizione

Il programma opera su una mappa esagonale dove:

- Ogni esagono ha un **costo di transito** compreso tra 0 e 100 (inizializzato a 1).
- È possibile creare **rotte aeree** dirette tra esagoni (massimo 5 per esagono sorgente).
- I costi possono essere **modificati dinamicamente** su un'area circolare con attenuazione proporzionale alla distanza.
- Il **percorso di costo minimo** tra due punti viene calcolato con l'algoritmo di Dijkstra, considerando sia le adiacenze sulla griglia che le rotte aeree.

La navigazione sulla griglia esagonale utilizza il sistema di coordinate **odd-r offset**, convertito internamente in **coordinate assiali** per gestire le 6 direzioni di adiacenza.

---

## Comandi

Il programma legge comandi da `stdin`, uno per riga. I comandi disponibili sono:

### `init <colonne> <righe>`

Inizializza (o reinizializza) la mappa esagonale. Tutti gli esagoni vengono creati con costo 1 e senza rotte aeree. La cache dei percorsi viene svuotata.

- **Output:** `OK` se i parametri sono validi (entrambi > 0), `KO` altrimenti.

### `change_cost <x> <y> <v> <raggio>`

Modifica il costo degli esagoni in un'area circolare centrata su `(x, y)`. Il delta applicato è decrescente dal centro verso il bordo:

```
delta = floor(v × (raggio - distanza) / raggio)
```

- `v` ∈ [-10, +10], `raggio` > 0
- I costi restano vincolati nell'intervallo [0, 100]
- Vengono aggiornati anche i costi delle rotte aeree degli esagoni coinvolti
- **Output:** `OK` o `KO`

### `toggle_air_route <xs> <ys> <xd> <yd>`

Attiva o disattiva una rotta aerea diretta dall'esagono `(xs, ys)` verso `(xd, yd)`.

- Se la rotta esiste già, viene rimossa.
- Se non esiste, viene aggiunta con costo calcolato come media: `(somma costi rotte esistenti + costo esagono sorgente) / (num. rotte + 1)`.
- Massimo 5 rotte per esagono sorgente.
- **Output:** `OK` o `KO`

### `travel_cost <xs> <ys> <xd> <yd>`

Calcola il costo del percorso minimo da `(xs, ys)` a `(xd, yd)`, considerando sia le adiacenze esagonali che le rotte aeree. Esagoni con costo 0 non sono attraversabili.

- **Output:** il costo minimo, oppure `-1` se non esiste un percorso.

---

## Strutture Dati e Algoritmi

| Struttura / Algoritmo | Utilizzo |
|---|---|
| **Min-heap binaria** | Coda a priorità per l'algoritmo di Dijkstra |
| **Coda circolare (BFS)** | Visita per livelli in `change_cost` |
| **Cache FIFO** | Memorizzazione degli ultimi 10.000 percorsi calcolati; invalidata ad ogni modifica della mappa |
| **Dijkstra** | Calcolo del cammino minimo in `travel_cost` — O((V+E) log V) |
| **BFS** | Propagazione dei costi in `change_cost` — O(r²) |
| **Coordinate assiali** | Navigazione efficiente sulle 6 direzioni della griglia esagonale |

---

## Compilazione ed Esecuzione

### Requisiti

- Compilatore C con supporto allo standard **C11** (es. GCC)
- **CMake** ≥ 3.31

### Build

```bash
mkdir build && cd build
cmake ..
make
```

### Esecuzione

```bash
# Da file di input
./progettoAPI < input.txt

# Interattivo (terminare con Ctrl+D)
./progettoAPI
```

### Esempio

```
$ ./progettoAPI
init 100 100
OK
change_cost 10 20 -10 5
OK
travel_cost 0 0 20 0
20
travel_cost 30 95 30 97
12
```

---

## Test

La cartella `test_pubblici/` contiene test case con i rispettivi output attesi nella sotto-cartella `Results/`.

```bash
# Esegui un test e confronta l'output atteso
./progettoAPI < test_pubblici/example.txt | diff - test_pubblici/Results/example.txt.result
```

| Test | Descrizione |
|---|---|
| `example.txt` | Esempio base |
| `edge_cases.txt` | Casi limite |
| `large.txt` | Mappa di grandi dimensioni |
| `long.txt` | Sequenza lunga di operazioni |
| `empty.txt` | Mappa senza costi |
| `stress_cache.txt` | Stress test della cache |
| `stress_cache2.txt` | Stress test della cache (variante) |

---

## Struttura del Repository

```
progetto-API/
├── main.c                    # Codice sorgente
├── CMakeLists.txt            # Configurazione di build
├── specifica-progetto.pdf    # Specifica del progetto
├── test_pubblici/            # Test case pubblici
│   ├── example.txt
│   ├── edge_cases.txt
│   ├── large.txt
│   ├── long.txt
│   ├── empty.txt
│   ├── stress_cache.txt
│   ├── stress_cache2.txt
│   └── Results/              # Output attesi
│       ├── example.txt.result
│       ├── edge_cases.txt.result
│       └── ...
└── README.md
```
