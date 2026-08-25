// =========================================================================
// VERIFICADOR INDEPENDIENTE del makespan intervalar.
//
// A diferencia de verify_solution.cpp, este programa NO enlaza ninguna clase
// del solver: no usa SGS_IJSP_*, ScheduleIJSP, Interval ni ProblemIJSP.
// Reimplementa desde cero el parseo de instancias, la decodificacion y la
// aritmetica de intervalos, de modo que un defecto compartido en el framework
// no puede pasar inadvertido. Solo depende de la biblioteca estandar.
//
// Uso:  verify_independent <instancia.txt> <fichero_Sols.csv> [append|insertion]
//
// Formato de solucion:  Run;Solution;Objective Value
//   Solution      = permutacion de IDs de tarea (0-based, id = trabajo*m + op)
//   Objective Value = "(inf, sup)"
//
// Semantica de decodificacion:
//   append    : cada tarea empieza en max(fin del predecesor de trabajo,
//               fin del predecesor de maquina). Planificacion semiactiva.
//   insertion : ademas se intenta insertar la tarea en el primer hueco de su
//               maquina donde quepa sin violar la precedencia de trabajo.
//               Planificacion activa; es la que usa el marco.
//
// Aritmetica: suma componente a componente y maximo componente a componente,
// que es la convencion declarada en el articulo.
// =========================================================================
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

struct Iv {                       // intervalo cerrado [lo, hi]
    double lo, hi;
    Iv(double a = 0, double b = 0) : lo(a), hi(b) {}
    double mid() const { return (lo + hi) / 2.0; }
};
static Iv operator+(const Iv &a, const Iv &b) { return Iv(a.lo + b.lo, a.hi + b.hi); }
static Iv cmax(const Iv &a, const Iv &b) {                    // maximo componentwise
    return Iv(std::max(a.lo, b.lo), std::max(a.hi, b.hi));
}
// a domina a b si ambos extremos de a son <= los de b
static bool leq(const Iv &a, const Iv &b) { return a.lo <= b.lo && a.hi <= b.hi; }

struct Instancia {
    int n = 0, m = 0;                       // trabajos, maquinas
    std::vector<std::vector<int> > maq;     // maq[j][k] = maquina de la op k del trabajo j
    std::vector<std::vector<Iv> > dur;      // dur[j][k]
};

static std::string limpia(std::string s) {
    while (!s.empty() && (s[s.size()-1] == '\r' || s[s.size()-1] == '\n')) s.erase(s.size()-1);
    return s;
}

static bool leeInstancia(const char *ruta, Instancia &I) {
    std::ifstream f(ruta);
    if (!f) { fprintf(stderr, "no se puede abrir %s\n", ruta); return false; }
    std::string l; int seccion = 0;         // 1=n 2=m 3=secuencia 4=duraciones
    std::vector<std::vector<int> > seq;
    std::vector<std::vector<Iv> > dus;
    while (std::getline(f, l)) {
        l = limpia(l);
        if (l.find("NUMERO DE TRABAJOS") != std::string::npos) { seccion = 1; continue; }
        if (l.find("NUMERO DE RECURSOS") != std::string::npos) { seccion = 2; continue; }
        if (l.find("SECUENCIA DE MAQUINAS") != std::string::npos) { seccion = 3; continue; }
        if (l.find("DURACIONES") != std::string::npos) { seccion = 4; continue; }
        if (l.find("TIEMPOS MAXIMOS") != std::string::npos) { seccion = 0; continue; }
        if (l.empty() || l[0] == '/') continue;
        if (seccion == 1) { I.n = atoi(l.c_str()); seccion = 0; }
        else if (seccion == 2) { I.m = atoi(l.c_str()); seccion = 0; }
        else if (seccion == 3) {
            std::istringstream is(l); int v; std::vector<int> fila;
            while (is >> v) fila.push_back(v);
            if (!fila.empty()) seq.push_back(fila);
        }
        else if (seccion == 4) {
            // formato: (a, b) (a, b) ...
            std::string t = l;
            for (size_t i = 0; i < t.size(); i++) if (t[i]=='('||t[i]==')'||t[i]==',') t[i] = ' ';
            std::istringstream is(t); double a, b; std::vector<Iv> fila;
            while (is >> a >> b) fila.push_back(Iv(a, b));
            if (!fila.empty()) dus.push_back(fila);
        }
    }
    if (I.n <= 0 || I.m <= 0 || (int)seq.size() < I.n || (int)dus.size() < I.n) {
        fprintf(stderr, "instancia mal formada: n=%d m=%d seq=%zu dur=%zu\n",
                I.n, I.m, seq.size(), dus.size());
        return false;
    }
    seq.resize(I.n); dus.resize(I.n);
    I.maq = seq; I.dur = dus;
    for (int j = 0; j < I.n; j++)
        if ((int)I.maq[j].size() != I.m || (int)I.dur[j].size() != I.m) {
            fprintf(stderr, "trabajo %d: %zu maquinas, %zu duraciones (esperado %d)\n",
                    j, I.maq[j].size(), I.dur[j].size(), I.m);
            return false;
        }
    return true;
}

// Decodifica una secuencia de IDs de tarea y devuelve el makespan intervalar.
// modoInsercion = false -> semiactivo (append); true -> activo (insercion en huecos)
static long g_inserciones = 0;   // cuantas veces el SGS activo llega a rellenar un hueco
static long g_desordenes = 0, g_tareas = 0;   // diagnostico de recodificacion lamarckiana
static double g_ultimoInicio = -1e18;
static bool decodifica(const Instancia &I, const std::vector<int> &sec,
                       bool modoInsercion, Iv &makespan, std::string &err) {
    const int N = I.n * I.m;
    if ((int)sec.size() != N) { err = "longitud de la secuencia incorrecta"; return false; }

    g_ultimoInicio = -1e18;
    std::vector<int> hechas(I.n, 0);                  // ops ya colocadas de cada trabajo
    std::vector<Iv> finTrabajo(I.n, Iv(0,0));         // fin de la ultima op colocada
    // ocupacion por maquina: lista ordenada de (inicio, fin) de las tareas colocadas
    std::vector<std::vector<std::pair<Iv,Iv> > > ocup(I.m);

    for (size_t p = 0; p < sec.size(); p++) {
        int id = sec[p];
        if (id < 0 || id >= N) { err = "id de tarea fuera de rango"; return false; }
        int j = id / I.m, k = id % I.m;
        if (k != hechas[j]) { err = "orden de operaciones incoherente dentro de un trabajo"; return false; }
        int mq = I.maq[j][k];
        if (mq < 0 || mq >= I.m) { err = "maquina fuera de rango"; return false; }
        Iv d = I.dur[j][k];
        Iv listo = finTrabajo[j];                     // no puede empezar antes
        Iv ini, fin;
        bool colocada = false;

        if (modoInsercion) {
            // primer hueco de la maquina donde quepa respetando la precedencia
            std::vector<std::pair<Iv,Iv> > &oc = ocup[mq];
            const size_t nOc = oc.size();             // tamano FIJO: oc cambia al insertar
            Iv libre(0,0);
            for (size_t q = 0; q < nOc && !colocada; q++) {
                Iv arranque = cmax(libre, listo);
                Iv candFin = arranque + d;
                if (leq(candFin, oc[q].first)) {      // cabe antes de la tarea q
                    ini = arranque; fin = candFin; colocada = true; g_inserciones++;
                    oc.insert(oc.begin() + q, std::make_pair(ini, fin));
                }
                libre = oc[q + (colocada ? 1 : 0)].second;
            }
            if (!colocada) {                          // al final de la maquina
                Iv arranque = cmax(libre, listo);
                ini = arranque; fin = arranque + d; colocada = true;
                oc.push_back(std::make_pair(ini, fin));
            }
        }
        if (!colocada) {                               // append: detras de todo
            std::vector<std::pair<Iv,Iv> > &oc = ocup[mq];
            Iv ultima = oc.empty() ? Iv(0,0) : oc.back().second;
            ini = cmax(ultima, listo);
            fin = ini + d;
            oc.push_back(std::make_pair(ini, fin));
        }
        // DIAGNOSTICO: ¿viene el cromosoma recodificado desde un schedule real?
        // Si el algoritmo es lamarckiano, la secuencia almacenada es el orden en
        // que las tareas se ejecutaron, luego los inicios deben salir no
        // decrecientes al decodificar. Un solo descenso indica que la secuencia
        // NO es una recodificacion del schedule.
        if (ini.lo + 1e-9 < g_ultimoInicio) g_desordenes++;
        g_ultimoInicio = ini.lo;
        g_tareas++;

        finTrabajo[j] = fin;
        hechas[j]++;
    }
    for (int j = 0; j < I.n; j++)
        if (hechas[j] != I.m) { err = "algun trabajo no completo"; return false; }

    makespan = Iv(0,0);
    for (int j = 0; j < I.n; j++) makespan = cmax(makespan, finTrabajo[j]);
    return true;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "uso: %s <instancia.txt> <Sols.csv> [append|insertion]\n", argv[0]);
        return 2;
    }
    bool insercion = (argc < 4) || (strcmp(argv[3], "append") != 0);

    Instancia I;
    if (!leeInstancia(argv[1], I)) return 2;

    std::ifstream f(argv[2]);
    if (!f) { fprintf(stderr, "no se puede abrir %s\n", argv[2]); return 2; }
    std::string l;
    std::getline(f, l);                                // cabecera
    int nruns = 0, disc = 0; double maxdif = 0;
    printf("%-5s %20s %20s %10s\n", "run", "reportado E[Cmax]", "recomputado", "dif");
    while (std::getline(f, l)) {
        l = limpia(l);
        if (l.empty()) continue;
        size_t p1 = l.find(';'); if (p1 == std::string::npos) continue;
        size_t p2 = l.find(';', p1 + 1); if (p2 == std::string::npos) continue;
        int run = atoi(l.substr(0, p1).c_str());
        std::string sol = l.substr(p1 + 1, p2 - p1 - 1);
        std::string obj = l.substr(p2 + 1);
        // objetivo "(a, b)"
        for (size_t i = 0; i < obj.size(); i++) if (obj[i]=='('||obj[i]==')'||obj[i]==',') obj[i]=' ';
        double oa = 0, ob = 0; { std::istringstream is(obj); is >> oa >> ob; }
        std::vector<int> sec; { std::istringstream is(sol); int v; while (is >> v) sec.push_back(v); }

        Iv mk; std::string err;
        if (!decodifica(I, sec, insercion, mk, err)) {
            printf("%-5d  ERROR: %s\n", run, err.c_str());
            disc++; nruns++;
            continue;
        }
        double rep = (oa + ob) / 2.0, rec = mk.mid(), dif = rec - rep;
        if (dif < 0) dif = -dif;
        if (dif > 1e-6) { disc++; if (dif > maxdif) maxdif = dif; }
        printf("%-5d %20.1f %20.1f %+10.2f\n", run, rep, rec, rec - rep);
        nruns++;
    }
    printf("\nmodo: %s | runs verificados: %d | discrepancias: %d | maxDif E[Cmax]: %g\n",
           insercion ? "insertion" : "append", nruns, disc, maxdif);
    if (insercion)
        printf("huecos rellenados por el SGS activo: %ld en %ld tareas (%.4f%%)\n",
               g_inserciones, g_tareas,
               g_tareas ? 100.0 * g_inserciones / (double)g_tareas : 0.0);
    printf("orden de la secuencia vs inicios decodificados: %ld descensos en %ld tareas (%.4f%%)\n",
           g_desordenes, g_tareas, g_tareas ? 100.0 * g_desordenes / (double)g_tareas : 0.0);
    printf("  (0 descensos = la secuencia es una recodificacion del schedule ejecutado)\n");
    printf("%s\n", disc == 0 ? "OK: el makespan reportado se reproduce exactamente."
                             : "ATENCION: el makespan reportado NO coincide.");
    return disc == 0 ? 0 : 1;
}
