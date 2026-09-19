# flags de compilacion
#
# No -flto here, and that is a measured decision rather than an oversight: it
# looked like a clear win (+9.4 %) in a block comparison and turned out to be
# nothing at all when the two builds were run side by side. See the journal
# entry for 2026-09-19.
CXXFLAGS = -O3 -march=native -mtune=native -ffast-math -std=c++14 -I.. -I. -I /usr/local/src

# flags de enlazado
LDFLAGS = -lm
# lista de archivos fuente y ejecutable
SOURCES = AlgorithmClassRegister.cpp SharedVarsEvolutionary.cpp ArtificialBeeColony.cpp ArtificialBeeColonyCell.cpp ArtificialBeeColonyPSO.cpp ArtificialBeeColonyThread.cpp RandomMT.cpp DebugFileWriter.cpp Creation.cpp Crossover.cpp DataPrinter.cpp Evaluation.cpp EvaluationClassRegister.cpp EvoLauncher.cpp EvolutiveAlgorithm.cpp Fitness.cpp FitnessMO.cpp GeneticAlgorithm.cpp GeneticClassRegister.cpp Individual.cpp LocalSearch.cpp LocalSearchAmico.cpp LS_TabuBackJump.cpp LocalSearchClassRegister.cpp main.cpp MemeticAlgorithm.cpp MemeticNeri.cpp Mutation.cpp Neighbour.cpp ParameterDB.cpp Population.cpp Random.cpp RandomPopulation.cpp Replacement.cpp Selection_Base.cpp Selection_Tournament.cpp Selection_Roulette.cpp Selection_SUS.cpp Selection_Shuffle.cpp Selection_Random.cpp Selection_Elite.cpp Selection_CellR5.cpp Selection_CellR13.cpp Selection_CellL5.cpp Selection_CellL9.cpp Selection_CellC9.cpp Selection_CellC13.cpp StatisticsClassRegister.cpp StatisticsDiversity.cpp TabuList.cpp TimeWindow.cpp TimeWindowClassRegister.cpp CrispTime.cpp TaskIJSP.cpp ProblemIJSP.cpp ScheduleIJSP.cpp SGS_IJSP.cpp SGS_IJSP_Append.cpp SGS_IJSP_Insertion.cpp IJSPClassRegister.cpp EncoderIJSP.cpp DecoderIJSP.cpp EvaluationIJSP.cpp CreationIJSP.cpp CreationIJSP_SRT.cpp CreationIJSP_LRTF.cpp CreationIJSP_LRTFInverse.cpp CreationIJSP_SNTF.cpp CreationIJSP_SCTF.cpp CreationIJSP_LCTF.cpp CreationIJSP_SPJF.cpp CreationIJSP_SPJFInverse.cpp CreationIJSP_Manager.cpp CrossoverIJSP_Base.cpp CrossoverIJSP_JOX.cpp CrossoverIJSP_GOX.cpp CrossoverIJSP_GPMX.cpp CrossoverIJSP_PPX.cpp StatisticsIJSP.cpp NeighbourIJSP.cpp NeighbourhoodIJSP_Base.cpp NeighbourhoodIJSP_N1.cpp NeighbourhoodIJSP_N2.cpp NeighbourhoodIJSP_N3.cpp NeighbourhoodIJSP_NH.cpp NeighbourhoodIJSP_N2Minus.cpp NeighbourhoodIJSP_N2Plus.cpp NeighbourhoodIJSP_N2Inter.cpp NeighbourhoodIJSP_Next.cpp NeighbourhoodIJSP_N8.cpp JSPCertificateAnalyzer.cpp PostExecutionClassRegister.cpp PostExecutionManager.cpp MonotonicAdaptativeCooling.cpp MonSimulatedCoolingClassRegister.cpp NonMonotonicAdaptativeCooling.cpp NonMonSimulatedCoolingClassRegister.cpp

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
