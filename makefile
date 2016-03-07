COMP	= mpicc
OPT     = -O3 -w

contar_paralelo: contar_paralelo.c
	$(COMP)  $(OPT)  -o $@ $@.c 

clean: 
	rm contar_paralelo
