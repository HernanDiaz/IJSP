# flags de compilacion
CXXFLAGS = -O3 -std=c++0x -I.. -I. -I /usr/local/src

# flags de enlazado
LDFLAGS = -lm
# lista de archivos fuente y ejecutable
SOURCES = Creation.cpp Crossover.cpp Decoder.cpp DueDate.cpp DueDateClassRegister.cpp Encoder.cpp Evaluation.cpp EvaluationClassRegister.cpp EvoLauncher.cpp EvolutiveAlgorithm.cpp Fitness.cpp FuzzyProblem.cpp FuzzySchedule.cpp FuzzySGS.cpp FuzzySGS_Insertion.cpp FuzzyTask.cpp GeneticAlgorithm.cpp GeneticClassRegister.cpp Individual.cpp main.cpp Mutation.cpp ParameterDB.cpp Population.cpp Random.cpp RandomMT.cpp Replacement.cpp SchedulingClassRegister.cpp Selection.cpp Statistics.cpp TFN.cpp

EXE = ../FJSP_Frame

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
