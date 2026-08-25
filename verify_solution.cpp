/*
 * verify_solution.cpp
 *
 * VERIFICADOR INDEPENDIENTE del makespan reportado.
 * Lee un fichero _Sols.csv (Run;Solution;Objective Value), donde Solution es el
 * ORDEN DE TAREAS (indices internos) del schedule final, lo re-decodifica con el
 * SGS indicado y recalcula el makespan componentwise (igual que
 * EvaluationIJSP_Makespan), comparandolo con el intervalo reportado.
 *
 * Uso: verify_solution <instancia> <_Sols.csv> [insertion|append]
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <unistd.h>

#include "ParameterDB.h"
#include "ProblemIJSP.h"
#include "SGS_IJSP_Insertion.h"
#include "SGS_IJSP_Append.h"
#include "ScheduleIJSP.h"
#include "SharedVars.h"
#include "Interval.h"

using namespace std;
using namespace FuzzyFW;
using namespace IJSP;

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "Uso: verify_solution <instancia> <_Sols.csv> [insertion|append]" << endl;
        return 1;
    }
    const char *instFile = argv[1];
    const char *solsFile = argv[2];
    string sgsType = (argc > 3) ? argv[3] : "insertion";

    ParameterDB params;
    params.setParameter("problem.hasbounds", "NO");
    ProblemIJSP problem(&params, instFile);
    unsigned int nJobs = problem.getNumberJobs();
    unsigned int nTasks = problem.getNumberTasks();

    SGS_IJSP *sgs;
    if (sgsType == "append") { SGS_IJSP_Append *s = new SGS_IJSP_Append(); s->intervalMaximum = Interval::M_COMPONENT; sgs = s; }
    else                     { SGS_IJSP_Insertion *s = new SGS_IJSP_Insertion(); sgs = s; }

    SharedVars svars;
    svars.problem = &problem;
    svars.parameters = NULL;
    svars.rng = NULL;

    ifstream in(solsFile);
    if (!in.is_open()) { cerr << "No se puede abrir " << solsFile << endl; return 2; }

    string line;
    getline(in, line);   // cabecera
    int n = 0, mismatches = 0;
    double maxDiff = 0.0;
    cout << "run  reportado           recomputado         dif" << endl;
    while (getline(in, line)) {
        if (line.empty()) continue;
        // Run;Solution;Objective Value    con Objective como "(a, b)"
        size_t p1 = line.find(';');
        size_t p2 = line.find(';', p1 + 1);
        if (p1 == string::npos || p2 == string::npos) continue;
        int run = atoi(line.substr(0, p1).c_str());
        string sol = line.substr(p1 + 1, p2 - p1 - 1);
        string obj = line.substr(p2 + 1);

        vector<int> order;
        { stringstream ss(sol); int v; while (ss >> v) order.push_back(v); }
        if (order.size() != nTasks) {
            cerr << "run " << run << ": " << order.size() << " tareas != " << nTasks << endl;
            continue;
        }
        double ra = 0, rb = 0;
        { for (char &c : obj) if (c=='('||c==')'||c==',') c = ' ';
          stringstream ss(obj); ss >> ra >> rb; }

        ScheduleIJSP *sch = sgs->buildSchedule(&svars, order);
        Interval mk(0, 0);
        for (unsigned int i = 0; i < nJobs; i++)
            mk = maximum(mk, sch->getCTJob(i), Interval::M_COMPONENT);

        double d = fabs((mk.a + mk.b) / 2.0 - (ra + rb) / 2.0);
        if (d > maxDiff) maxDiff = d;
        if (d > 1e-6) mismatches++;
        if (n < 5)
            printf("%3d  [%7.1f,%7.1f]   [%7.1f,%7.1f]  %+.2f\n", run, ra, rb, mk.a, mk.b,
                   (mk.a + mk.b) / 2.0 - (ra + rb) / 2.0);
        n++;
    }
    cout << "\nruns verificados: " << n << "  |  discrepancias: " << mismatches
         << "  |  maxDif E[Cmax]: " << maxDiff << endl;
    cout << (mismatches == 0 ? "OK: el makespan reportado se reproduce exactamente."
                             : "ATENCION: el makespan reportado NO coincide.") << endl;
    cout.flush();
    _exit(mismatches == 0 ? 0 : 3);
}
