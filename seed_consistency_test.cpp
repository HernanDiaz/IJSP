/*
 * seed_consistency_test.cpp
 *
 * Decodifica un pool con el SGS semiactivo del repo (SGS_IJSP_Append, maximum
 * COMPONENT) y el makespan de EvaluationIJSP_Makespan (maximum COMPONENT sobre
 * las terminaciones de cada trabajo).
 *
 * Modos:
 *   1) Comprobacion (por defecto):
 *        seed_consistency_test <instancia> <pool.csv> [maxLines]
 *      Compara el [lower,upper] del pool contra dos convenciones de agregacion
 *      del makespan: componentwise (repo) y lex-por-upper (doc del pool).
 *
 *   2) Reescritura + estadisticas:
 *        seed_consistency_test <instancia> <pool.csv> --rewrite <out.csv> [--lb LB]
 *      Escribe una copia del pool con el intervalo RECOMPUTADO componentwise
 *      (la permutacion queda intacta) y saca best/mediana E[Cmax] y RE vs LB
 *      bajo ambas convenciones.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <unistd.h>

#include "ParameterDB.h"
#include "ProblemIJSP.h"
#include "SGS_IJSP_Append.h"
#include "ScheduleIJSP.h"
#include "SharedVars.h"
#include "Interval.h"

using namespace std;
using namespace FuzzyFW;
using namespace IJSP;

static void stats(vector<double> &v, double lb, const char *label) {
    if (v.empty()) return;
    sort(v.begin(), v.end());
    double best = v.front();
    size_t n = v.size();
    double median = (n % 2) ? v[n / 2] : 0.5 * (v[n / 2 - 1] + v[n / 2]);
    cout << "  [" << label << "] best E[Cmax]=" << best
         << "  mediana E[Cmax]=" << median;
    if (lb > 0)
        cout << "  |  RE best=" << (100.0 * (best - lb) / lb) << "%"
             << "  RE mediana=" << (100.0 * (median - lb) / lb) << "%";
    cout << endl;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "Uso: seed_consistency_test <instancia> <pool.csv> [maxLines]" << endl;
        cerr << "     seed_consistency_test <instancia> <pool.csv> --rewrite <out.csv> [--lb LB]" << endl;
        return 1;
    }
    const char *instanceFile = argv[1];
    const char *poolFile = argv[2];

    long maxLines = -1;
    string outFile;
    double lb = -1.0;
    for (int i = 3; i < argc; i++) {
        string a = argv[i];
        if (a == "--rewrite" && i + 1 < argc) outFile = argv[++i];
        else if (a == "--lb" && i + 1 < argc) lb = atof(argv[++i]);
        else maxLines = atol(a.c_str());
    }

    ParameterDB params;
    params.setParameter("problem.hasbounds", "NO");

    ProblemIJSP problem(&params, instanceFile);
    unsigned int nJobs = problem.getNumberJobs();
    unsigned int nTasks = problem.getNumberTasks();

    SGS_IJSP_Append sgs;
    sgs.intervalMaximum = Interval::M_COMPONENT;

    SharedVars svars;
    svars.problem = &problem;
    svars.parameters = NULL;   // SharedVars::~SharedVars hace delete; no apuntar a pila
    svars.rng = NULL;

    ifstream pool(poolFile);
    if (!pool.is_open()) { cerr << "No se puede abrir el pool: " << poolFile << endl; return 2; }

    ofstream out;
    bool rewrite = !outFile.empty();
    if (rewrite) {
        out.open(outFile.c_str());
        if (!out.is_open()) { cerr << "No se puede escribir: " << outFile << endl; return 2; }
    }

    string line;
    long lineNo = 0, checked = 0, parseErrors = 0;
    long matchComp = 0, matchLex = 0, matchBoth = 0, mismatchesComp = 0;
    double maxErr = 0.0;
    vector<double> ecComp, ecLex;

    while (getline(pool, line)) {
        if (line.empty()) continue;
        lineNo++;
        if (maxLines > 0 && lineNo > maxLines) { lineNo--; break; }

        size_t sc = line.find(';');
        string permPart = (sc == string::npos) ? line : line.substr(0, sc);
        string intPart = (sc == string::npos) ? "" : line.substr(sc + 1);

        vector<int> jobs;
        { stringstream ss(permPart); int v; while (ss >> v) jobs.push_back(v); }
        if (jobs.size() != nTasks) {
            if (parseErrors < 10)
                cerr << "Linea " << lineNo << ": " << jobs.size()
                     << " tokens != nTasks " << nTasks << endl;
            parseErrors++;
            continue;
        }

        double ea = 0, eb = 0;
        { for (char &c : intPart) if (c == '[' || c == ']' || c == ',') c = ' ';
          stringstream ss(intPart); ss >> ea >> eb; }

        vector<int> occ(nJobs, 0);
        vector<int> order; order.reserve(nTasks);
        bool ok = true;
        for (int j : jobs) {
            int job0 = j - 1;
            if (job0 < 0 || job0 >= (int)nJobs) { ok = false; break; }
            int tid = problem.getTaskId(job0, occ[job0]);
            if (tid < 0) { ok = false; break; }
            occ[job0]++;
            order.push_back(tid);
        }
        if (!ok) {
            if (parseErrors < 10)
                cerr << "Linea " << lineNo << ": trabajo/ocurrencia invalida" << endl;
            parseErrors++;
            continue;
        }

        ScheduleIJSP *sch = sgs.buildSchedule(&svars, order);

        Interval mkComp(0, 0);
        for (unsigned int i = 0; i < nJobs; i++)
            mkComp = maximum(mkComp, sch->getCTJob(i), Interval::M_COMPONENT);

        Interval mkLex(0, 0);
        for (unsigned int i = 0; i < nJobs; i++) {
            Interval ct = sch->getCTJob(i);
            if (ct.b > mkLex.b + 1e-9 ||
                (fabs(ct.b - mkLex.b) < 1e-9 && ct.a > mkLex.a))
                mkLex = ct;
        }

        checked++;
        ecComp.push_back((mkComp.a + mkComp.b) / 2.0);
        ecLex.push_back((mkLex.a + mkLex.b) / 2.0);

        if (rewrite)
            out << permPart << ";[" << (long long)llround(mkComp.a)
                << ", " << (long long)llround(mkComp.b) << "]\n";

        bool okComp = (fabs(mkComp.a - ea) < 1e-6 && fabs(mkComp.b - eb) < 1e-6);
        bool okLex  = (fabs(mkLex.a  - ea) < 1e-6 && fabs(mkLex.b  - eb) < 1e-6);
        if (okComp) matchComp++;
        if (okLex)  matchLex++;
        if (okComp && okLex) matchBoth++;
        if (!okComp) {
            mismatchesComp++;
            double da = fabs(mkComp.a - ea), db = fabs(mkComp.b - eb);
            if (da > maxErr) maxErr = da;
            if (db > maxErr) maxErr = db;
        }
    }
    if (rewrite) out.close();

    cout << "Instancia: " << instanceFile << "  (trabajos=" << nJobs << " tareas=" << nTasks << ")" << endl;
    cout << "Pool: " << poolFile << "  lineas=" << checked
         << "  parseErrors=" << parseErrors << endl;
    cout << "  coincide componentwise (repo): " << matchComp << "/" << checked
         << "  |  lex-por-upper (doc): " << matchLex << "/" << checked
         << "  |  maxErrLower(comp): " << maxErr << endl;
    stats(ecComp, lb, "componentwise/repo");
    stats(ecLex, lb, "lex-por-upper/doc ");
    if (rewrite)
        cout << "  -> pool recomputado escrito en: " << outFile << endl;

    cout.flush();
    _exit(parseErrors == 0 ? 0 : 3);
}
