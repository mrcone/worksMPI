# Algoritmo de Cannon MPI

Implementación paralela, sobre MPI, del producto matriz por matriz, utilizando el algoritmo desarrollado para una arquitectura de malla ([algoritmo de Cannon](https://en.wikipedia.org/wiki/Cannon%27s_algorithm))

### Compilación

Simplemente ejecutar en el directorio donde se encuentra mmalla.c:

```
make
```

### Uso

Ejecutar el siguiente código con mpirun, y número de procesos que ejecutarán el programa:

```
Uso: mpirun -np <numero cuadrado> mmalla
```

**Sólo** el número de procesos que ejecuten el programa debe ser una número cuadrado, para que la descomposición de Cannon sea efectiva

### Explicación teórica
Las matrices `A` y `B` están particionadas en `r` × `r` bloques. Los bloques de matrices que son necesarios definir: `A`, `B`, `C`, `ATMP` se considerarán almacenados en un vector.
Cuando los cálculos están realizados se comprueba que `A=C` para verificar que la multiplicación se ha efectuado correctamente.
Cada proceso cuenta el úumero de errores que ha cometido (si todo funciona corretamente serán cero errores).
