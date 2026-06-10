# flags de compilacion
CXXFLAGS = -O3 -march=native -mtune=native -ffast-math -std=c++14 -I.. -I. -I /usr/local/src

# flags de enlazado
LDFLAGS = -lm
# lista de archivos fuente y ejecutable
SOURCES = AlgorithmClassRegister.cpp SharedVarsEvolutionary.cpp ArtificialBeeColony.cpp ArtificialBeeColonyCell.cpp ArtificialBeeColonyPSO.cpp ArtificialBeeColonyThread.cpp RandomMT.cpp FJSPRobustnessAnalyzer.cpp FJSPRobustnessAnalyzerMakespan.cpp FJSPScenarioManager.cpp DebugFileWriter.cpp Creation.cpp CreationFJSP.cpp Crossover.cpp CrossoverFJSP.cpp CustomerFVRP.cpp DataPrinter.cpp DecoderFJSP.cpp DecoderFVRP.cpp EncoderFJSP.cpp EncoderFVRP.cpp Evaluation.cpp EvaluationClassRegister.cpp EvaluationFJSP.cpp EvaluationFVRP.cpp EvoLauncher.cpp EvolutiveAlgorithm.cpp Fitness.cpp FitnessMO.cpp FJSPClassRegister.cpp FVRPClassRegister.cpp GeneticAlgorithm.cpp GeneticClassRegister.cpp Individual.cpp LocalSearch.cpp LocalSearchAmico.cpp LocalSearchAmicoFJSP.cpp LocalSearchClassRegister.cpp main.cpp MemeticAlgorithm.cpp MemeticNeri.cpp Mutation.cpp Neighbour.cpp NeighbourFJSP.cpp NeighbourFVRP.cpp NeighbourhoodFJSP_AI.cpp NeighbourhoodFJSP_Cmax.cpp NeighbourhoodFJSP_ESD.cpp NeighbourhoodFVRP.cpp NeighbourhoodFVRP_TW.cpp ParameterDB.cpp Population.cpp ProblemFJSP.cpp ProblemFVRP.cpp Random.cpp RandomPopulation.cpp Replacement.cpp RouteFVRP.cpp ScheduleFJSP.cpp Selection_Base.cpp Selection_Tournament.cpp Selection_Roulette.cpp Selection_SUS.cpp Selection_Shuffle.cpp Selection_Random.cpp Selection_Elite.cpp Selection_CellR5.cpp Selection_CellR13.cpp Selection_CellL5.cpp Selection_CellL9.cpp Selection_CellC9.cpp Selection_CellC13.cpp SGS_FJSP.cpp SGS_FJSP_Append.cpp SGS_FJSP_Insertion.cpp SGS_FVRP.cpp SGS_FVRP_Split.cpp SGS_FVRP_Split_TW.cpp StatisticsClassRegister.cpp StatisticsDiversity.cpp StatisticsFJSP.cpp StatisticsFVRP.cpp TabuList.cpp TaskFJSP.cpp TFN.cpp TimeWindow.cpp TimeWindowClassRegister.cpp Interval.cpp TaskIJSP.cpp ProblemIJSP.cpp ScheduleIJSP.cpp SGS_IJSP.cpp SGS_IJSP_Append.cpp SGS_IJSP_Insertion.cpp IJSPClassRegister.cpp EncoderIJSP.cpp DecoderIJSP.cpp EvaluationIJSP.cpp CreationIJSP.cpp CreationIJSP_SRT.cpp CreationIJSP_LRTF.cpp CreationIJSP_LRTFInverse.cpp CreationIJSP_SNTF.cpp CreationIJSP_SCTF.cpp CreationIJSP_LCTF.cpp CreationIJSP_SPJF.cpp CreationIJSP_SPJFInverse.cpp CreationIJSP_Manager.cpp CrossoverIJSP_Base.cpp CrossoverIJSP_JOX.cpp CrossoverIJSP_GOX.cpp CrossoverIJSP_GPMX.cpp CrossoverIJSP_PPX.cpp StatisticsIJSP.cpp NeighbourIJSP.cpp NeighbourhoodIJSP_Base.cpp NeighbourhoodIJSP_N1.cpp NeighbourhoodIJSP_N2.cpp NeighbourhoodIJSP_N3.cpp NeighbourhoodIJSP_NH.cpp NeighbourhoodIJSP_N2Minus.cpp NeighbourhoodIJSP_N2Plus.cpp NeighbourhoodIJSP_N2Inter.cpp NeighbourhoodIJSP_Next.cpp NeighbourhoodIJSP_N8.cpp RobustnessFileWriter.cpp IJSPRobustnessAnalyzer.cpp ScenarioManager.cpp IJSPRobustnessAnalyzerMakespan.cpp IJSPRobustnessAnalyzerTardiness.cpp MakespanMRAnalyzer.cpp MakespanMRFileWriter.cpp PostExecutionClassRegister.cpp PostExecutionManager.cpp MonotonicAdaptativeCooling.cpp MonSimulatedCoolingClassRegister.cpp NonMonotonicAdaptativeCooling.cpp NonMonSimulatedCoolingClassRegister.cpp PathRelinkIJSP.cpp ElitePool.cpp IPRTS.cpp

EXE = ../FuzzyFW

# lista de archivos .o y .d
objetos = $(SOURCES:.cpp=.o)
dependencias = $(SOURCES:.cpp=.d)

# regla del ejecutable
$(EXE): $(objetos) 
	g++ $^ -o $@ $(LDFLAGS)

# regla para limpiar el directorio y dejar solo los ficheros fuentes
clean:
	rm -f $(EXE) *~ *.o *.d ../*~ ../*.o ../*.d


# regla de archivos de dependencias
%.d: %.cpp
	$(SHELL) -ec '$(CXX) -MM -MT "$*.o $@" $(CXXFLAGS) $< > $@; \
	[ -s $@ ] || rm -f $@'


# inclusion de archivos de dependencias
include $(dependencias)
